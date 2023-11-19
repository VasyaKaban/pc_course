#pragma once

#include "boost/asio.hpp"
#include "VM.hpp"
#include <iostream>
#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include <unordered_map>
#include <unordered_set>

using namespace boost::asio;
using boost::system::error_code;
using std::cout, std::endl;
using namespace LuaWay;

struct ThreadProperty
{
	mysqlpp::Connection connection;
	LuaWay::VM vm;
};

struct Request
{
	std::function<auto (Ref/*conf*/, VM &/*vm*/, mysqlpp::Connection &/*sql_connection*/) -> std::string> func;
	std::string_view request;

	constexpr auto operator==(const Request &req) const noexcept -> bool
	{
		return request == req.request;
	}
};

template<>
struct std::hash<Request>
{
	auto operator()(const Request &req) const noexcept -> std::size_t
	{
		return std::hash<std::string_view>{}(req.request);
	}
};

inline auto make_response_str(const std::string_view &response_msg, const std::string_view &end = "\n") -> std::string
{
	return std::format("response = '{}'{}", response_msg, end);
}

namespace LuaWay
{
	template<std::size_t N>
	struct Stack<char [N]>
	{
		using Type = char [N];
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushstring(state, value);
		}

		static auto Receive(lua_State *state, int pos) = delete;

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};

	template<>
	struct Stack<const char *>
	{
		using Type = const char *;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushstring(state, value);
		}

		static auto Receive(lua_State *state, int pos) = delete;

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};

	template<>
	struct Stack<std::string_view>
	{
		using Type = std::string_view;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlstring(state, value.data(), value.size());
		}

		static auto Receive(lua_State *state, int pos) = delete;

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};

	template<>
	struct Stack<mysqlpp::String>
	{
		using Type = mysqlpp::String;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlstring(state, value.c_str(), value.size());
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			std::size_t len = 0;
			const char *ptr = lua_tolstring(state, pos, &len);
			return mysqlpp::String(ptr, len);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};
};
