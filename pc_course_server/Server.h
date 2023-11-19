#pragma once

#include "AsioDefs.hpp"
#include "Connection.h"
#include <initializer_list>
#include "ServerSettings.hpp"

class Server
{
public:
	Server(io_context &ctx,
		   const ServerSettings &_settings,
		   std::initializer_list<Request> requests);
	~Server();
	Server(const Server &sv) = delete;
	Server(Server &&sv);
	auto GetEndpoint() -> ip::tcp::endpoint;
	auto Start() -> void;
	auto Stop() -> void;
private:

	auto accept() -> void;

private:
	io_context &context;
	ip::tcp::acceptor acceptor;
	ServerSettings settings;
	std::vector<std::thread> threads;
	std::unordered_map<std::thread::id, ThreadProperty> threads_props;
	std::unordered_set<Request> requests;
};
