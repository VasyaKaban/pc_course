#include <iostream>
#include <fstream>
#include "Server.h"
#include "Request.hpp"

using namespace std;

auto client_func()
{
	struct RequestData
	{
		std::string request_body;
		std::string_view request_name;
	};

	const std::vector<RequestData> requests
	{
		/*RequestData
		{
			"request = 'get_users'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_users"
		},
		RequestData
		{
			"request = 'get_ranks'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_ranks"
		},
		RequestData
		{
			"request = 'get_customers'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_customers"
		},
		RequestData
		{
			"request = 'get_orders'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_orders"
		},
		RequestData
		{
			"request = 'get_order_items'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"order_id = 1"
			"\r\n",
			"get_order_items"
		},
		RequestData
		{
			"request = 'get_items'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_items"
		},
		RequestData
		{
			"request = 'get_distributors'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_distributors"
		},
		RequestData
		{
			"request = 'get_item_types'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_item_types"
		},*/
		/*RequestData
		{
			"request = 'create_user'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"new_user ="
			"{"
				"login = 'aboba',"
				"password = 'a6c79a27049109e472b246b5dfbe08aedff1e9e2259597e54032dbad4958d4ad',"
				"sname = 'aboba_sname',"
				"name = 'aboba_name',"
				"pname = 'aboba_pname',"
				"rank = 3"
			"}"
			"\r\n",
			"create_user"
		},
		RequestData
		{
			"request = 'get_users'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_users"
		},
		RequestData
		{
			"request = 'update_user'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"update_user ="
			"{"
				"login = 'aboba',"
				"sname = 'aboba_sname',"
				"name = 'aboba_name',"
				"pname = 'aboba_pname',"
				"rank = 2"
			"}"
			"\r\n",
			"update_user"
		},
		RequestData
		{
			"request = 'get_users'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_users"
		},
		RequestData
		{
			"request = 'delete_user'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"delete_user_login = 'aboba'"
			"\r\n",
			"delete_user"
		},
		RequestData
		{
			"request = 'get_users'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_users"
		},*/

		RequestData
		{
			"request = 'complete_order'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"order_id = 1"
			"\r\n",
			"complete_order"
		},
		RequestData
		{
			"request = 'get_expired_orders'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"\r\n",
			"get_expired_orders"
		},
		RequestData
		{
			"request = 'get_completed_orders'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"dates = "
			"{"
				"start_date = 0,"
				"end_date = 1800000000"
			"}"
			"\r\n",
			"get_completed_orders"
		},
		RequestData
		{
			"request = 'get_income'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"dates = "
			"{"
				"start_date = 0,"
				"end_date = 1800000000"
			"}"
			"\r\n",
			"get_income"
		},
		RequestData
		{
			"request = 'get_statistics'"
			"user ="
			"{"
				"login = 'amogus',"
				"password = '80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9',"
				"rank = 'Admin'"
			"}"
			"dates = "
			"{"
				"start_date = 0,"
				"end_date = 1800000000"
			"}"
			"\r\n",
			"get_statistics"
		}
	};

	io_context ctx;
	ip::tcp::socket sock(ctx);
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 2028));
	boost::asio::write(sock, buffer("hello!"));
	boost::asio::streambuf input_buf;
	boost::system::error_code code;
	read_until(sock, input_buf, "\r\n", code);
	if(code.failed())
	{
		cout<<code.message()<<endl;
	}
	istream istr(&input_buf);
	/*ip::tcp::socket sock(ctx);
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 2028));
	boost::system::error_code code;
	for(const auto &req : requests)
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		boost::asio::write(sock, buffer(req.request_body), code);
		if(code.failed())
		{
			cout<<code.message()<<endl;
			break;
		}
		boost::asio::streambuf input_buf;
		read_until(sock, input_buf, "\r\n", code);
		if(code.failed())
		{
			cout<<code.message()<<endl;
			break;
		}
		istream istr(&input_buf);
		string response;
		getline(istr, response, '\r');
		cout<<req.request_name<<" -> :\n"<<response<<endl;
	}

	sock.shutdown(ip::tcp::socket::shutdown_both, code);*/
}

auto main(int argc, char **argv) -> int
{
	std::initializer_list<Request> requests
	{
		{UserRequest::auth, "auth"},
		{UserRequest::get_users, "get_users"},
		{UserRequest::get_ranks, "get_ranks"},
		{UserRequest::get_customers, "get_customers"},
		{UserRequest::get_orders, "get_orders"},
		{UserRequest::get_order_items, "get_order_items"},
		{UserRequest::get_items, "get_items"},
		{UserRequest::get_distributors, "get_distributors"},
		{UserRequest::get_item_types, "get_item_types"},
		{UserRequest::delete_user, "delete_user"},
		{UserRequest::update_user, "update_user"},
		{UserRequest::create_user, "create_user"},
		{UserRequest::create_customer, "create_customer"},
		{UserRequest::delete_customer, "delete_customer"},
		{UserRequest::add_item, "add_item"},
		{UserRequest::add_item_count, "add_item_count"},
		{UserRequest::complete_order, "complete_order"},
		{UserRequest::add_order, "add_order"},
		{UserRequest::get_expired_orders, "get_expired_orders"},
		{UserRequest::get_completed_orders, "get_completed_orders"},
		{UserRequest::get_income, "get_income"},
		{UserRequest::get_statistics, "get_statistics"},
	};

	auto default_settings = ServerSettings(4,
										   2028,
										   "localhost",
										   "vasyakaban",
										   "384557dd",
										   "pc_db",
										   "lua\r",
										   std::chrono::seconds(1337));
	if(auto settings = ServerSettings::ReadSettingsFile("settings.lua"); !settings.has_value())
		cout<<"Unexpected error when reading server settings file! Default settings were used!"<<endl;
	else
		default_settings = settings.value();

	cout<<default_settings<<endl;

	std::thread client;
	try
	{
		io_context context;
		Server server(context,
					  default_settings,
					  requests);

		server.Start();
		//client = std::thread(client_func);
		std::string inp;
		while(true)
		{
			std::getline(std::cin, inp);
			if(inp == "stop")
			{
				//server.Stop();
				break;
			}
			else
				cout<<"Bad!"<<endl;
		}
	}
	catch(const std::exception &std_ex)
	{
		cout<<std_ex.what()<<endl;
	}
	catch(...)
	{
		cout<<"Unexpected exception type!"<<endl;
	}

	client.join();

	return EXIT_SUCCESS;
}
