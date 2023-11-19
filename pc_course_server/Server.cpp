#include "Server.h"
#include "Connection.h"
#include <iostream>
#include <algorithm>

Server::Server(io_context &ctx,
			   const ServerSettings &_settings,
			   std::initializer_list<Request> _requests)
	: acceptor(ctx, ip::tcp::endpoint(ip::tcp::v4(), _settings.server_port)),
	  context(ctx), requests(_requests), settings(_settings)
{
	acceptor.set_option(socket_base::reuse_address(true));
	threads.reserve(std::max<std::size_t>(1, _settings.threads_count));
}

Server::~Server()
{
	Stop();
}

auto Server::GetEndpoint() -> ip::tcp::endpoint
{
	return acceptor.local_endpoint();
}

auto Server::Start() -> void
{
	assert(acceptor.is_open());
	accept();

	std::atomic_bool start;
	std::atomic_size_t count_execs;
	start.store(false);
	count_execs.store(0);
	threads_props.reserve(threads.capacity());
	for(std::size_t i = 0; i < threads.capacity(); i++)
	{
		std::thread thr = std::thread([&]()
		{
			 while(!start.load());
			count_execs.fetch_add(1);
			context.run();
		});
		mysqlpp::Connection sql_conn(false);
		sql_conn.connect(settings.db_database.c_str(),
						 settings.db_host.c_str(),
						 settings.db_user.c_str(),
						 settings.db_password.c_str());
		assert(sql_conn.connected());
		//mysqlpp::Connection sql_conn(db_name.c_str(), db_host.c_str(), db_user.c_str(), db_pwd.c_str());
		VM vm;
		vm.Open(true);
		threads_props.insert(std::pair{thr.get_id(), ThreadProperty{std::move(sql_conn), std::move(vm)}});
		threads.push_back(std::move(thr));
	}

	start.store(true);
	while(count_execs.load() != threads.size());
}

auto Server::Stop() -> void
{
	//acceptor.cancel();
	context.stop();

	for(auto &thr : threads)
		thr.join();

	/*for(auto &prop : threads_props)
	{
		//prop.second.connection.thread_end();
		prop.second.connection.shutdown();
		prop.second.vm.Close();
	}*/
}

auto Server::accept() -> void
{
	using namespace std::placeholders;
	ip::tcp::socket sock(context);
	steady_timer timer(context);
	auto connection = Connection::Create(std::move(sock),
										 std::move(timer),
										 requests,
										 settings.connection_lifetime,
										 settings.end_of_message,
										 threads_props);
	acceptor.async_accept(connection->GetPlainSocket(),
						[this, new_connection = connection](const error_code &error)
						{
							if(!error.failed())
							{
								cout<<new_connection->GetPlainSocket().remote_endpoint()<<
									" -> "<<"has been connected!"<<endl;
								new_connection->Start();
								this->accept();
							}
							else
								cout<<error.message();
						});
}
