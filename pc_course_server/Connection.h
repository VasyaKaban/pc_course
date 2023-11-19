#pragma once

#include <memory>
#include "AsioDefs.hpp"
//#include "DatabaseQuery.hpp"
#include <chrono>

class Connection : public std::enable_shared_from_this<Connection>
{
private:

	Connection(ip::tcp::socket &&sock,
			   steady_timer &&_timer,
			   const std::unordered_set<Request> &_requests,
			   std::chrono::seconds _connection_lifetime,
			   std::string_view _end_of_message,
			   std::unordered_map<std::thread::id, ThreadProperty> &_props);

	auto parse_config(const std::string &config_str) -> Ref;
	auto get_config_request(Ref &config) -> std::string;

public:
	~Connection();
	static auto Create(ip::tcp::socket &&sock,
					   steady_timer &&_timer,
					   const std::unordered_set<Request> &_requests,
					   std::chrono::seconds _connection_lifetime,
					   std::string_view _end_of_message,
					   std::unordered_map<std::thread::id, ThreadProperty> &_props) -> std::shared_ptr<Connection>;
	auto Start() -> void;
	auto GetPlainSocket() -> ip::tcp::socket &;

	auto ReadRequest() -> void;
	auto WriteRespone() -> void;
	auto ProcessRequest(std::string config_str) -> void;

private:
	ip::tcp::socket socket;
	steady_timer timer;
	std::atomic_flag is_timeout;
	std::chrono::seconds connection_lifetime;
	std::string_view end_of_message;
	streambuf input_buf;
	std::string output_str;
	std::unordered_map<std::thread::id, ThreadProperty> &props;
	const std::unordered_set<Request> &requests;
};
