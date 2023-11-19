#pragma once

#include <string>
#include <cstdint>
#include "NoReplace.hpp"

class Settings : NoCopy, NoMove
{
private:
	Settings() = default;

public:
	static Settings & Create()
	{
		static Settings settings;
		return settings;
	}
	void SetHostAddress(const std::string_view &_host_addr) {host_addr = _host_addr;}
	void SetPort(uint16_t _port) {port = _port;}
	std::string GetHostAddress() {return host_addr;}
	uint16_t GetPort() {return port;}
private:
	std::string host_addr;
	uint16_t port;
};
