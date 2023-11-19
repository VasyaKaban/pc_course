#pragma once

#include "AsioDefs.hpp"

struct ServerSettings
{
	size_t threads_count;
	size_t server_port;
	std::string db_host;
	std::string db_user;
	std::string db_password;
	std::string db_database;
	std::string end_of_message;
	std::chrono::seconds connection_lifetime;

	constexpr ServerSettings(size_t _threads_count = 0,
							 size_t _server_port = 0,
							 const std::string &_db_host = "",
							 const std::string &_db_user = "",
							 const std::string &_db_password = "",
							 const std::string &_db_database = "",
							 const std::string &_end_of_message = "",
							 std::chrono::seconds _connection_lifetime = std::chrono::seconds(0)) noexcept
		: threads_count(_threads_count), server_port(_server_port), db_host(_db_host),
		  db_user(_db_user), db_password(_db_password), db_database(_db_database),
		  end_of_message(_end_of_message), connection_lifetime(_connection_lifetime) {}

	constexpr ServerSettings(const ServerSettings &settings) noexcept
		: threads_count(settings.threads_count), server_port(settings.server_port), db_host(settings.db_host),
		  db_user(settings.db_user), db_password(settings.db_password), db_database(settings.db_database),
		  end_of_message(settings.end_of_message), connection_lifetime(settings.connection_lifetime){}

	constexpr auto operator=(const ServerSettings &settings) noexcept -> ServerSettings &
	{
		threads_count = settings.threads_count;
		server_port = settings.server_port;
		db_host = settings.db_host;
		db_user = settings.db_user;
		db_password = settings.db_password;
		db_database = settings.db_database;
		end_of_message = settings.end_of_message;
		connection_lifetime = settings.connection_lifetime;
		return *this;
	}

	static auto ReadSettingsFile(const std::filesystem::path &path) -> std::optional<ServerSettings>
	{
		VM vm;
		if(!vm.Open(true))
			return {};

		auto result = vm.ExecuteFile(path);
		if(!result.has_value())
			return {};

		Ref server = vm.Get({"server"});
		if(!server.Holds(VMType::Table))
			return {};

		auto _threads_count = server.GetRaw<DataType::Int>("threads_count");
		auto _server_port = server.GetRaw<DataType::Int>("server_port");
		auto _db_host = server.GetRaw<DataType::String>("db_host");
		auto _db_user = server.GetRaw<DataType::String>("db_user");
		auto _db_password = server.GetRaw<DataType::String>("db_password");
		auto _db_database = server.GetRaw<DataType::String>("db_database");
		auto _end_of_message = server.GetRaw<DataType::String>("end_of_message");
		auto _connection_lifetime = server.GetRaw<DataType::Int>("connection_lifetime");

		ServerSettings settings;
		if(_threads_count.has_value())
			settings.threads_count = _threads_count.value();

		if(_server_port.has_value())
			settings.server_port = _server_port.value();

		if(_db_host.has_value())
			settings.db_host = _db_host.value();

		if(_db_user.has_value())
			settings.db_user = _db_user.value();

		if(_db_password.has_value())
			settings.db_password = _db_password.value();

		if(_db_database.has_value())
			settings.db_database = _db_database.value();

		if(_end_of_message.has_value())
			settings.end_of_message = _end_of_message.value();

		if(_connection_lifetime.has_value())
			settings.connection_lifetime = std::chrono::seconds(_connection_lifetime.value());

		return settings;
	}

	struct SpecialString
	{
		std::string_view str;
		SpecialString(const std::string &_str) : str(_str){}
		friend auto operator<<(std::ostream &ostr, const SpecialString &spec) -> std::ostream &
		{
			for(const auto c : spec.str)
			{
				switch (c)
				{
					case '\'':
						ostr << "\\'";
						break;
					case '\"':
						ostr << "\\\"";
						break;
					case '\?':
						ostr << "\\?";
						break;
					case '\\':
						ostr << "\\\\";
						break;
					case '\a':
						ostr << "\\a";
						break;
					case '\b':
						ostr << "\\b";
						break;
					case '\f':
						ostr << "\\f";
						break;
					case '\n':
						ostr << "\\n";
						break;
					case '\r':
						ostr << "\\r";
						break;
					case '\t':
						ostr << "\\t";
						break;
					case '\v':
						ostr << "\\v";
						break;
					default:
						ostr << c;
				}
			}
			return ostr;
		}
	};

	friend auto operator<<(std::ostream &ostr, const ServerSettings &settings) -> std::ostream &
	{
		ostr<<
			"Settings = \n{\n"<<
			"\tthreads_count = "<<settings.threads_count<<"\n"<<
			"\tserver_port = "<<settings.server_port<<"\n"<<
			"\tdb_host = "<<settings.db_host<<"\n"<<
			"\tdb_user = "<<settings.db_user<<"\n"<<
			"\tdb_password = "<<settings.db_password<<"\n"<<
			"\tdb_database = "<<settings.db_database<<"\n"<<
			"\tend_of_message = "<<SpecialString(settings.end_of_message)<<"\n"<<
			"\tconnection_lifetime = "<<settings.connection_lifetime<<"\n"<<
			"}";

		return ostr;
	}
};
