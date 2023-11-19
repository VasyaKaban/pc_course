#pragma once

#include <sstream>
#include "Ref.hpp"
#include <ranges>
#include <iomanip>

inline auto LuaConfSerialize(const LuaWay::Ref &ref, const std::string &name, std::size_t tabs) -> std::string
{
	if(!ref)
		return "";

	std::ostringstream ostr;
	auto write_tabs = [&](std::size_t tabs_cnt)
	{
		for(std::size_t i = 0; i < tabs_cnt; i++)
			ostr<<'\t';
	};

	using namespace LuaWay;
	VMType vm_type = ref.Type();
	switch(vm_type)
	{
		case VMType::Function:
		case VMType::None:
			return "";
			break;
		case VMType::Table:
			{
				if(!name.empty())
				{
					write_tabs(tabs);
					ostr<<name<<" =\n";
				}

				write_tabs(tabs);
				ostr<<"{\n";

				bool is_first = true;
				for(auto [key, value] : ref)
				{
					if(key.Holds(VMType::String) && (!value.Holds(VMType::Function) && !value.Holds(VMType::None)))
					{
						if(!is_first)
							ostr<<",\n";
						else
							is_first = false;
						ostr<<LuaConfSerialize(value, key.As<VMType::String>().value(), tabs + 1);
					}
				}

				for(std::size_t i = 1; i <= ref.GetLength(); i++)
				{
					auto value_ref = ref.GetRawRef(static_cast<DataType::Int>(i));
					if(!value_ref.Holds(VMType::Function) && !value_ref.Holds(VMType::None))
					{
						if(!is_first)
							ostr<<",\n";
						else
							is_first = false;
						ostr<<LuaConfSerialize(value_ref, "", tabs + 1);
					}
				}

				ostr<<"\n";
				write_tabs(tabs);
				ostr<<"}";
			}
			break;
		default:
			write_tabs(tabs);
			if(!name.empty())
				ostr<<name<<" = ";

			switch(vm_type)
			{
				case VMType::Bool:
					ostr<<std::boolalpha<<ref.As<VMType::Bool>().value();
					break;
				case VMType::CFunction:
					ostr<<ref.As<VMType::CFunction>().value();
					break;
				case VMType::Number:
					ostr<<ref.As<VMType::Number>().value();
					break;
				case VMType::Int:
					ostr<<ref.As<VMType::Int>().value();
					break;
				case VMType::LightUserdata:
					ostr<<ref.As<VMType::LightUserdata>().value().data;
					break;
				case VMType::Nil:
					ostr<<"nil";
					break;
				case VMType::String:
					ostr<<std::quoted(ref.As<VMType::String>().value());
					break;
				case VMType::Userdata:
					ostr<<ref.As<VMType::Userdata>().value().data;
					break;
				case VMType::Thread:
					ostr<<ref.As<VMType::Thread>().value();
					break;
				default: break;
			}
			break;

	}

	return ostr.str();
}
