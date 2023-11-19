#pragma once

#include "NoReplace.hpp"
#include "VM.hpp"
#include <QString>

class LuaConf : NoCopy, NoMove
{
public:
	static LuaWay::VM & Create()
	{
		#warning NO_THREAD_SAFE!
		static bool is_created = false;
		static LuaWay::VM vm;
		if(!is_created)
		{
			auto res = vm.Open(false);
			assert(res);
			is_created = true;
		}

		return vm;
	}
};

struct LuaParseResult
{
	LuaWay::Ref fenv;
	LuaWay::DataType::String response;

	LuaParseResult(LuaWay::Ref _fenv = {}, const LuaWay::DataType::String & _response = {})
		: fenv(_fenv), response(_response) {}

	bool IsValid() const noexcept
	{
		return static_cast<bool>(fenv);
	}
};

inline LuaParseResult ParseAndGetResponse(const std::string &script)
{
	LuaWay::VM &vm = LuaConf::Create();
	LuaWay::Ref fenv = vm.CreateTable(0, 0, "");
	auto res = vm.ExecuteString(script.c_str(), fenv);
	using namespace std::string_literals;
	try
	{
		LuaWay::DataType::String response = fenv.Get<LuaWay::DataType::String>("response"s).value();
		return LuaParseResult(fenv, response);
	}
	catch(...)
	{
		return LuaParseResult();
	}
}

namespace LuaWay
{
	template<>
	struct Stack<std::string_view>
	{
		using Type = std::string_view;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlstring(state, value.data(), value.size());
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = false;
	};
}

namespace LuaWay
{
	template<>
	struct Stack<QByteArray>
	{
		using Type = QByteArray;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlstring(state, value.data(), value.size());
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			std::size_t len = 0;
			const char *str = lua_tolstring(state, pos, &len);
			return Type(str, len);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};
}

namespace LuaWay
{
	template<>
	struct Stack<QString>
	{
		using Type = QString;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			Stack::Push(state, value.toUtf8());
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return QString::fromUtf8(Stack<QByteArray>::Receive(state, pos));
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};
}
