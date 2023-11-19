#pragma once

#include <string>
#include <format>
#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include "VM.hpp"
#include "LuaConfSerialize.hpp"

namespace LuaWay
{
	template<>
	inline auto Stack::Push(lua_State *state, const mysqlpp::String &value) -> void
	{
		lua_pushlstring(state, value.c_str(), value.size());
	}

	template<>
	inline auto Stack::Push(lua_State *state, const char * const &value) -> void
	{
		lua_pushstring(state, value);
	}
}

template<std::size_t N>
auto CreateConfArray(LuaWay::VM &vm, mysqlpp::Connection &connection, const char *query, const std::array<const char *, N> &fields) -> LuaWay::Ref
{
	mysqlpp::Query prepared_query = connection.query(query);
	mysqlpp::StoreQueryResult result = prepared_query.store();

	std::array<int, N> indices;
	for(std::size_t i = 0; i < indices.size(); i++)
		indices[i] = result.field_num(fields[i]);

	LuaWay::Ref conf = vm.CreateTable(result.num_rows(), 0, "");
	for(std::size_t i = 0; auto row : result)
	{
		LuaWay::Ref item = vm.CreateTable(0, fields.size(), "");
		for(std::size_t ind = 0; ind < fields.size(); ind++)
			item.SetRaw(fields[ind], row.at(indices[ind]));

		conf.SetRaw(static_cast<LuaWay::DataType::Int>(i + 1), item);
		i++;
	}

	return conf;
}

inline auto GetUserRanks(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = "SELECT * FROM UserRank;";
	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "name"
		}), "user_ranks", 0);
}

inline auto GetUsers(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(SELECT
		User.login AS login,
		User.sname AS sname,
		User.name AS name,
		User.pname AS pname,
		UserRank.name AS rank
		FROM User
		INNER JOIN UserRank ON User.rank = UserRank.id;)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"login", "sname", "name", "pname", "rank"
		}), "users", 0);
}

inline auto GetCustomers(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(SELECT * FROM Customer)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "sname", "name", "pname"
		}), "customers", 0);
}

inline auto GetOrderStatuses(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(SELECT * FROM OrderStatus)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "name"
		}), "order_statuses", 0);
}

inline auto GetOrders(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(
		SELECT
		NewOrder.id AS id,
		NewOrder.customer_id AS customer_id,
		NewOrder.date_of_start AS date_of_start,
		NewOrder.date_of_expire AS date_of_expire,
		OrderStatus.name AS order_status,
		Customer.sname AS customer_sname,
		Customer.name AS customer_name,
		Customer.pname AS customer_pname
		FROM NewOrder
		INNER JOIN OrderStatus ON NewOrder.status = OrderStatus.id
		INNER JOIN Customer ON NewOrder.customer_id = Customer.id;)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "customer_id", "date_of_start", "date_of_expire",
			"order_status", "customer_sname", "customer_name", "customer_pname"
		}), "orders", 0);
}

inline auto GetItems(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(SELECT * FROM Item)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "name", "cost", "count_on_warehouse",
			"distributor", "description"
		}), "items", 0);
}

inline auto GetOrderedItems(LuaWay::VM &vm, mysqlpp::Connection &connection) -> std::string
{
	constexpr auto query = R"(SELECT * FROM OrderedItem)";

	return LuaConfSerialize(CreateConfArray(vm, connection, query,
		std::array
		{
			"id", "order_id", "item_id", "count_in_order"
		}), "ordered_items", 0);
}

inline auto GetOrderedItemsByOrderId(LuaWay::VM &vm, mysqlpp::Connection &connection, int id) -> std::string
{
	auto query = std::format("SELECT * FROM OrderedItem WHERE order_id = {};", id);

	return LuaConfSerialize(CreateConfArray(vm, connection, query.c_str(),
		std::array
		{
			"id", "item_id", "count_in_order"
		}), "ordered_items", 0);
}

//usings
inline auto CheckCreateUser(LuaWay::Ref user, mysqlpp::Connection &connection) -> bool
{
	using namespace std::string_literals;
	bool is_exist = CheckConfExistance(user, std::array
		{
			std::pair{"login"s, LuaWay::VMType::String},
			std::pair{"password"s, LuaWay::VMType::String},
			std::pair{"sname"s, LuaWay::VMType::String},
			std::pair{"name"s, LuaWay::VMType::String},
			std::pair{"pname"s, LuaWay::VMType::String},
			std::pair{"rank"s, LuaWay::VMType::Number}
		});
	if(!is_exist)
		return false;


	int rank_id = ReceiveFromObject<LuaWay::DataType::Number>(user.GetRaw("rank", {}));
	mysqlpp::Query prepared_query = connection.query("SELECT id FROM UserRank;");
	auto result = prepared_query.store();
	bool rank_found = false;
	for(auto &row : result)
	{
		if(row.at(0) == rank_id)
		{
			rank_found = true;
			break;
		}
	}

	return rank_found;
}

inline auto CreateUser(LuaWay::Ref user, mysqlpp::Connection &connection) -> std::string
{
	if(!CheckCreateUser(user, connection))
		return "bad_config";

	auto query = std::format("INSERT INTO User VALUES ({}, {}, {}, {}, {}, {});",
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("login", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("password", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("sname", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("name", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("pname", {})),
							 ReceiveFromObject<LuaWay::DataType::Int>(user.GetRaw("rank", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	prepared_query.execute();
	return "";
}

inline auto CheckDeleteUser(LuaWay::Ref user) -> bool
{
	using namespace std::string_literals;
	return CheckConfExistance(user, std::array{std::pair{"login"s, LuaWay::VMType::String}});
}

inline auto DeleteUser(LuaWay::Ref user, mysqlpp::Connection &connection) -> void
{
	assert(CheckDeleteUser(user));
	auto query = std::format("DELETE FROM User WHERE login = {};",
							 ReceiveFromObject<LuaWay::DataType::String>(user.GetRaw("login", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	prepared_query.execute();
}

inline auto CheckCreateItem(LuaWay::Ref item) -> bool
{
	using namespace std::string_literals;
	bool is_exist = CheckConfExistance(item, std::array
		{
			std::pair{"name"s, LuaWay::VMType::String},
			std::pair{"cost"s, LuaWay::VMType::Number},
			std::pair{"count"s, LuaWay::VMType::Number},
			std::pair{"distributor"s, LuaWay::VMType::String},
			std::pair{"description"s, LuaWay::VMType::String}
		});
	if(!is_exist)
		return false;


	double cost = ReceiveFromObject<LuaWay::DataType::Number>(item.GetRaw("cost", {}));
	if(cost < 0.0)
		return false;

	return true;
}

inline auto CreateItem(LuaWay::Ref item, mysqlpp::Connection &connection) -> std::string
{
	if(!CheckCreateItem(item))
		return "bad_config";

	auto query = std::format("INSERT INTO Item VALUES (0, {}, {}, {}, {}, {});",
							 ReceiveFromObject<LuaWay::DataType::String>(item.GetRaw("name", {})),
							 ReceiveFromObject<LuaWay::DataType::Number>(item.GetRaw("cost", {})),
							 ReceiveFromObject<LuaWay::DataType::Int>(item.GetRaw("count", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(item.GetRaw("distributor", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(item.GetRaw("description", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	prepared_query.execute();
	return "";
}

inline auto CheckUpdateItem(LuaWay::Ref item) -> bool
{
	using namespace std::string_literals;
	return CheckConfExistance(item, std::array
		{
			std::pair{"id"s, LuaWay::VMType::Number},
			std::pair{"count"s, LuaWay::VMType::Number},
			std::pair{"cost"s, LuaWay::VMType::Number}
		});
}

inline auto UpdateItem(LuaWay::Ref item, mysqlpp::Connection &connection) -> std::string
{
	if(!CheckCreateItem(item))
		return "bad_config";

	auto query = std::format("UPDATE Item SET cost = {}, count_on_warehouse = {} WHERE id = {};",
							 ReceiveFromObject<LuaWay::DataType::Number>(item.GetRaw("cost", {})),
							 ReceiveFromObject<LuaWay::DataType::Int>(item.GetRaw("count", {})),
							 ReceiveFromObject<LuaWay::DataType::Int>(item.GetRaw("id", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	prepared_query.execute();
	return "";
}

inline auto CheckNewCustomer(LuaWay::Ref customer) -> bool
{
	using namespace std::string_literals;
	return CheckConfExistance(customer, std::array
		{
			std::pair{"name"s, LuaWay::VMType::Number},
			std::pair{"sname"s, LuaWay::VMType::Number},
			std::pair{"pname"s, LuaWay::VMType::Number}
		});
}

inline auto CreateCustomer(LuaWay::Ref customer, mysqlpp::Connection &connection) -> std::string
{
	if(!CheckNewCustomer(customer))
		return "bad_config";

	auto query = std::format("INSERT INTO Customer VALUES (0, {}, {}, {});",
							 ReceiveFromObject<LuaWay::DataType::String>(customer.GetRaw("sname", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(customer.GetRaw("name", {})),
							 ReceiveFromObject<LuaWay::DataType::String>(customer.GetRaw("pname", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	prepared_query.execute();
	return "";
}

inline auto CheckNewOrder(LuaWay::Ref order, mysqlpp::Connection &connection) -> bool
{
	using namespace std::string_literals;
	bool is_exist = CheckConfExistance(order, std::array
		{
			std::pair{"customer_id"s, LuaWay::VMType::Number},
			std::pair{"date_of_start"s, LuaWay::VMType::Number},
			std::pair{"date_of_expire"s, LuaWay::VMType::Number},
			std::pair{"status"s, LuaWay::VMType::Number},
		 });

	if(!is_exist)
		return false;

	std::time_t date_of_start = ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("date_of_start", {}));
	std::time_t date_of_expire = ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("date_of_expire", {}));
	if(date_of_start > date_of_expire)
		return false;

	int status_id = ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("status", {}));
	mysqlpp::Query prepared_query = connection.query("SELECT id FROM OrderStatus;");
	auto result = prepared_query.store();
	bool status_found = false;
	for(auto &row : result)
	{
		if(row.at(0) == status_id)
		{
			status_found = true;
			break;
		}
	}

	if(!status_found)
		return false;

	std::size_t count_of_ordered_items = order.GetLength();
	if(count_of_ordered_items == 0)
		return false;

	prepared_query = connection.query("SELECT id FROM Item;");
	result = prepared_query.store();

	for(std::size_t i = 1; i <= count_of_ordered_items; i++)
	{
		LuaWay::Ref ordered_item = ReceiveFromObject<LuaWay::Ref>(order.GetRaw(static_cast<LuaWay::DataType::Int>(i), hrs::Flags<LuaWay::VMType>::full_mask));
		if(!ordered_item.Holds(LuaWay::VMType::Table))
			return false;

		if(!CheckConfExistance(ordered_item, std::array
			{
				std::pair{"item_id"s, LuaWay::VMType::Number},
				std::pair{"count"s, LuaWay::VMType::Number}
			}))
			return false;

		bool item_id_found = false;
		for(auto &row : result)
		{
			if(row.at(0) == ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("item_id", {})))
			{
				item_id_found = true;
				break;
			}
		}

		if(!item_id_found)
			return false;
	}

	return true;
}

inline auto CreateNewOrder(LuaWay::Ref order, mysqlpp::Connection &connection) -> std::string
{
	if(!CheckNewOrder(order, connection))
		return "bad_config";

	auto query = std::format("INSERT INTO NewOrder VALUES (0, {}, FROM_UNIXTIME{}, FROM_INIXTIME{}, {});",
							 ReceiveFromObject<LuaWay::DataType::Int>(order.GetRaw("customer_id", {})),
							 ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("date_of_start", {})),
							 ReceiveFromObject<LuaWay::DataType::Number>(order.GetRaw("date_of_expire", {})),
							 ReceiveFromObject<LuaWay::DataType::Int>(order.GetRaw("status", {})));

	mysqlpp::Query prepared_query = connection.query(query);
	auto res = prepared_query.execute();

	for(std::size_t i = 1; i <= order.GetLength(); i++)
	{
		LuaWay::Ref ordered_item = ReceiveFromObject<LuaWay::Ref>(order.GetRaw(static_cast<LuaWay::DataType::Int>(i), {}));
		query = std::format("INSERT INTO OrderedItem VALUES (0, {}, {}, {});",
							ReceiveFromObject<LuaWay::DataType::Int>(ordered_item.GetRaw("item_id", {})),
							res.insert_id(),
							ReceiveFromObject<LuaWay::DataType::Int>(ordered_item.GetRaw("count", {})));

		prepared_query = connection.query(query);
		prepared_query.execute();
	}


	return "";
}


