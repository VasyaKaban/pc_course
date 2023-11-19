#include "Connection.h"
//#include "LuaConfSerialize.hpp"
#include <format>

/* proto:
request = '%name'
data =
{
	...
}
 */

Connection::Connection(ip::tcp::socket &&sock,
					   steady_timer &&_timer,
					   const std::unordered_set<Request> &_requests,
					   std::chrono::seconds _connection_lifetime,
					   std::string_view _end_of_message,
					   std::unordered_map<std::thread::id, ThreadProperty> &_props)
	: socket(std::move(sock)), timer(std::move(_timer)), requests(_requests), props(_props),
	  connection_lifetime(_connection_lifetime), end_of_message(_end_of_message)
{
	is_timeout.clear(std::memory_order_relaxed);
}

auto Connection::parse_config(const std::string &config_str) -> Ref
{
	auto &prop = props[std::this_thread::get_id()];
	Ref config = prop.vm.CreateTable(0, 2, "");
	auto res = prop.vm.ExecuteString(config_str.c_str(), config);
	if(!res.has_value())
		return {};

	return config;
}

auto Connection::get_config_request(Ref &config) -> std::string
{
	auto request = config.GetRaw<std::string>("request");
	if(!request)
		return "";

	return request.value();
}

Connection::~Connection()
{
	//socket.cancel();
	//socket.shutdown(ip::tcp::socket::shutdown_both);
}

auto Connection::Create(ip::tcp::socket &&sock,
						steady_timer &&_timer,
						const std::unordered_set<Request> &_requests,
						std::chrono::seconds _connection_lifetime,
						std::string_view _end_of_message,
						std::unordered_map<std::thread::id, ThreadProperty> &_props)
						-> std::shared_ptr<Connection>
{
	return std::shared_ptr<Connection>(new Connection(std::move(sock),
													  std::move(_timer),
													  _requests,
													  _connection_lifetime,
													  _end_of_message,
													  _props));
}

auto Connection::Start() -> void
{
	ReadRequest();
}

auto Connection::GetPlainSocket() -> ip::tcp::socket &
{
	return socket;
}

auto Connection::ReadRequest() -> void
{
	input_buf.consume(input_buf.size());
	is_timeout.clear(std::memory_order_relaxed);
	async_read_until(socket,
					 input_buf,
					 end_of_message,
					 [self = shared_from_this()](const error_code &code, std::size_t bytes)
					 {
						auto prev_flag = self->is_timeout.test_and_set();
						if(prev_flag)
						{
							cout<<self->socket.remote_endpoint()<<
								" -> has been disconnected due to timeout!"<<endl;
						}
						else if(code.failed())
						{
							if(code == boost::asio::error::eof)
							{
								cout<<self->socket.remote_endpoint()<<
									" -> has been disconnected!"<<endl;
								self->timer.cancel();
							}
							else if(code == boost::asio::error::operation_aborted)
								cout<<self->socket.remote_endpoint()<<
									" -> has been disconnected due to timeout!"<<endl;
							else
							{
								self->timer.cancel();
								cout<<code.message()<<endl;
							}
						}
						else
						{
							self->timer.cancel();
							std::istream istr(&self->input_buf);
							std::string request_str;
							request_str.reserve(1024);
							std::getline(istr, request_str, '\r');
							self->ProcessRequest(request_str);
						}
					 });

	timer.expires_after(connection_lifetime);
	timer.async_wait([self = shared_from_this()](const error_code &code)
					 {
						if(!code.failed() && !self->is_timeout.test())
						{
							//connection still reading
							self->socket.cancel();
							self->is_timeout.test_and_set();
						}
					 });
}

auto Connection::WriteRespone() -> void
{
	output_str += end_of_message;
	async_write(socket, buffer(output_str), [self = shared_from_this()](const error_code &code, std::size_t bytes)
	{
		if(code.failed())
		{
			if(code == boost::asio::error::eof)
				cout<<"Disconnected -> "<<self->socket.remote_endpoint()<<endl;
			else
				cout<<code.message()<<endl;
		}
		else
			self->ReadRequest();
	});
}

auto Connection::ProcessRequest(std::string config_str) -> void
{
	Ref config = parse_config(config_str);
	if(!config)
		output_str = make_response_str("bad_config");
	else
	{
		auto request = get_config_request(config);
		if(request.empty())
			output_str = make_response_str("bad_config");
		else
		{
			bool found = false;
			for(auto &req : requests)
			{
				if(req.request == request)
				{
					found = true;
					output_str = req.func(config, props[std::this_thread::get_id()].vm, props[std::this_thread::get_id()].connection);
					break;
				}
			}
			if(!found)
				output_str = make_response_str("bad_config");
		}
	}

	WriteRespone();
}
