#pragma once

#include "AsioDefs.hpp"
#include "LuaConfSerialize.hpp"
#include <ranges>
#include <charconv>

namespace UserRequest
{
	struct FieldAndType
	{
		std::string field_name;
		VMType type;
		bool deny_null;

		constexpr FieldAndType(std::string_view _field_name, VMType _type, bool _deny_null = true)
			: field_name(_field_name), type(_type), deny_null(_deny_null){}
	};

	inline auto serialize_one_query_result(VM &vm, mysqlpp::StoreQueryResult &res, const FieldAndType &field) -> std::string
	{
		int index = res.field_num(field.field_name);
		assert(index >= 0);

		assert(res.num_rows() >= 1);
		auto row_data = res.at(0).at(index);
		assert((!row_data.is_null() && field.deny_null) || !field.deny_null);
		if(row_data.is_null() && field.deny_null)
			return "";

		Ref ref;
		switch(field.type)
		{
			case VMType::Bool:
				{
					DataType::Int num = 0;
					std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
					assert(res.ec == std::errc());
					assert(res.ptr == row_data.data() + row_data.size());
					ref = vm.CreateRef(static_cast<DataType::Bool>(num));
				}
				break;
			case VMType::Int:
				{
					DataType::Int num = 0;
					std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
					assert(res.ec == std::errc());
					assert(res.ptr == row_data.data() + row_data.size());
					ref = vm.CreateRef(num);
				}
				break;
			case VMType::Number:
				{
					DataType::Number num = 0;
					std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
					assert(res.ec == std::errc());
					assert(res.ptr == row_data.data() + row_data.size());
					ref = vm.CreateRef(num);
				}
				break;
			case VMType::String:
				ref = vm.CreateRef(row_data);
				break;
			default:
				assert((false) && "Non supported type!");
				break;
		}

		return LuaConfSerialize(ref, field.field_name, 0);
	}

	template<std::size_t N>
	auto serialize_query_result(VM &vm, mysqlpp::StoreQueryResult &res,
		const std::string_view &table_name, const std::array<FieldAndType, N> &fields) -> std::string
	{
		std::array<int, N> indices;
		for(std::size_t i = 0; i < fields.size(); i++)
		{
			indices[i] = res.field_num(fields[i].field_name);
			assert(indices[i] >= 0);
		}

		Ref tbl = vm.CreateTable(res.num_rows(), 0, "");
		for(DataType::Int i = 1; auto &row : res)
		{
			Ref item = vm.CreateTable(0, fields.size());
			for(std::size_t j = 0; j < fields.size(); j++)
			{
				auto row_data = row.at(indices[j]);
				assert((!row_data.is_null() && fields[j].deny_null) || !fields[j].deny_null);
				if(row_data.is_null() && !fields[j].deny_null)
					continue;

				switch(fields[j].type)
				{
					case VMType::Bool:
						{
							DataType::Int num = 0;
							std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
							assert(res.ec == std::errc());
							assert(res.ptr == row_data.data() + row_data.size());
							item.SetRaw(fields[j].field_name, static_cast<DataType::Bool>(num));
						}
						break;
					case VMType::Int:
						{
							DataType::Int num = 0;
							std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
							assert(res.ec == std::errc());
							assert(res.ptr == row_data.data() + row_data.size());
							item.SetRaw(fields[j].field_name, num);
						}
						break;
					case VMType::Number:
						{
							DataType::Number num = 0;
							std::from_chars_result res = std::from_chars(row_data.data(), row_data.data() + row_data.size(), num);
							assert(res.ec == std::errc());
							assert(res.ptr == row_data.data() + row_data.size());
							item.SetRaw(fields[j].field_name, num);
						}
						break;
					case VMType::String:
						item.SetRaw(fields[j].field_name, row.at(indices[j]));
						break;
					default:
						assert((false) && "Non supported type!");
						break;
				}
			}

			tbl.SetRaw(i, item);
			i++;
		}

		return LuaConfSerialize(tbl, std::string(table_name), 0);
	}

	template<bool is_store, typename F>
		requires std::invocable<F, std::conditional_t<is_store, mysqlpp::StoreQueryResult &, bool>>
	auto invoke_stored_procedure(F &&f, mysqlpp::Query &query)
	{
		using ReturnType = std::invoke_result_t<std::remove_cvref_t<F>, mysqlpp::StoreQueryResult &>;
		std::conditional_t<is_store, mysqlpp::StoreQueryResult, bool> res;
		if constexpr(is_store)
			res = query.store();
		else
			res = query.exec();

		if constexpr(std::same_as<ReturnType, void>)
		{
			std::forward<F>(f)(res);
			while(query.store_next());
		}
		else
		{
			ReturnType value = std::forward<F>(f)(res);
			while(query.store_next());
			return value;
		}
	}

	template<std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_value_t<R>, std::string_view>
	inline auto check_user_existance(Ref config, mysqlpp::Connection &sql_connection, const R &ranks) -> std::string
	{
		try
		{
			Ref user = config.GetRawRef("user");
			if(!user.Holds(VMType::Table))
				return make_response_str("bad_config");

			std::string login = user.GetRaw<std::string>("login").value();
			std::string password = user.GetRaw<std::string>("password").value();
			std::string rank = user.GetRaw<std::string>("rank").value();
			bool rank_found = false;
			for(const auto &desired : ranks)
			{
				if(desired == rank)
				{
					rank_found = true;
					break;
				}
			}

			if(!rank_found)
				return make_response_str("access_denied");

			auto query = sql_connection.query(std::format("CALL CheckUserExists('{}', '{}', '{}');",
														  login, password, rank));

			return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
			{
				if(!res)
					return make_response_str(query.error());

				if(res.at(0).at(0) == "0")
					return make_response_str("bad_config");
				else
					return "";
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto auth(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin", "Employee", "Accountant"}); !str.empty())
			return str;

		return make_response_str("success");
	}

	inline auto get_users(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin"}); !str.empty())
			return str;

		auto query = sql_connection.query("CALL GetUsers();");
		return invoke_stored_procedure<true>([&vm](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "users",
			std::array
			{
				FieldAndType{"login", VMType::String},
				FieldAndType{"sname", VMType::String},
				FieldAndType{"name", VMType::String},
				FieldAndType{"pname", VMType::String},
				FieldAndType{"rank", VMType::String}
			});
		}, query);
	}

	inline auto get_ranks(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetRanks();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "ranks", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"name", VMType::String}
			});
		}, query);
	}

	inline auto get_customers(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetCustomers();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "customers", std::array
			{
				FieldAndType{"telephone", VMType::Int},
				FieldAndType{"sname", VMType::String},
				FieldAndType{"name", VMType::String},
				FieldAndType{"pname", VMType::String}
			});
		}, query);
	}

	inline auto get_orders(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetOrders();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "orders", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"date_of_start", VMType::Int},
				FieldAndType{"date_of_expire", VMType::Int},
				FieldAndType{"date_of_completion", VMType::Int, false},
				FieldAndType{"c_telephone", VMType::Int},
				FieldAndType{"c_sname", VMType::String},
				FieldAndType{"c_name", VMType::String},
				FieldAndType{"c_pname", VMType::String}
			});
		}, query);
	}

	inline auto get_order_items(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		DataType::Int order_id;
		try
		{
			order_id = config.GetRaw<DataType::Int>("order_id").value();
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}

		auto query = sql_connection.query(std::format("CALL GetOrderItems({});", order_id));
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "order_items", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"count_in_order", VMType::Int},
				FieldAndType{"name", VMType::String},
				FieldAndType{"cost", VMType::Number},
				FieldAndType{"distributor", VMType::String},
				FieldAndType{"description", VMType::String},
				FieldAndType{"common_cost", VMType::Number}
			});
		}, query);
	}

	inline auto get_items(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetItems();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "items", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"name", VMType::String},
				FieldAndType{"cost", VMType::Number},
				FieldAndType{"count_on_warehouse", VMType::Int},
				FieldAndType{"distributor", VMType::String},
				FieldAndType{"type", VMType::String},
				FieldAndType{"description", VMType::String}
			});
		}, query);
	}

	inline auto get_distributors(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetDistributors();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "distributors", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"name", VMType::String}
			});
		}, query);
	}

	inline auto get_item_types(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		auto query = sql_connection.query("CALL GetItemTypes();");
		return invoke_stored_procedure<true>([&](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "item_types", std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"name", VMType::String}
			});
		}, query);
	}

	inline auto delete_user(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin"}); !str.empty())
			return str;

		try
		{
			std::string user_login = config.GetRaw<std::string>("delete_user_login").value();
			auto query = sql_connection.query(std::format("CALL DeleteUser('{}');", user_login));
			//query.exec();
			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				return make_response_str("success");
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto update_user(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin"}); !str.empty())
			return str;

		try
		{
			Ref update_user = config.GetRawRef("update_user");
			std::string login = update_user.GetRaw<std::string>("login").value();
			std::string sname = update_user.GetRaw<std::string>("sname").value();
			std::string name = update_user.GetRaw<std::string>("name").value();
			std::string pname = update_user.GetRaw<std::string>("pname").value();
			DataType::Int rank = update_user.GetRaw<DataType::Int>("rank").value();
			auto query = sql_connection.query(std::format("CALL UpdateUser('{}', '{}', '{}', '{}', '{}');",
				login, sname, name, pname, rank));

			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				if(!res)
					return make_response_str(query.error());

				return make_response_str("success");
			}, query);
			/*if(query.exec())
				return make_response_str("success");
			else
				return make_response_str("bad_config");*/
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto create_user(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin"}); !str.empty())
			return str;

		try
		{
			Ref new_user = config.GetRawRef("new_user");
			std::string login = new_user.GetRaw<std::string>("login").value();
			std::string pwd = new_user.GetRaw<std::string>("password").value();
			std::string sname = new_user.GetRaw<std::string>("sname").value();
			std::string name = new_user.GetRaw<std::string>("name").value();
			std::string pname = new_user.GetRaw<std::string>("pname").value();
			DataType::Int rank = new_user.GetRaw<DataType::Int>("rank").value();
			auto query = sql_connection.query(std::format("CALL CreateUser('{}', '{}', '{}', '{}', '{}', '{}');",
														  login, pwd, sname, name, pname, rank));

			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				if(!res)
					return make_response_str(query.error());

				return make_response_str("success");
			}, query);
			/*if(!query.exec())
			{
				if(query.errnum() == 45000)
					return make_response_str(query.error());
				else
					return make_response_str("bad_config");
			}*/
		}
		catch(const mysqlpp::Exception &ex)
		{
			std::cout<<ex.what()<<std::endl;
			return make_response_str("bad_config");
		}
		catch(const std::exception &ex)
		{
			std::cout<<ex.what()<<std::endl;
			return make_response_str("bad_config");
		}

		/*catch(...)
		{
			return make_response_str("bad_config");
		}*/
	}

	inline auto create_customer(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			Ref new_customer = config.GetRawRef("new_customer");
			DataType::Int tel = new_customer.GetRaw<DataType::Int>("telephone").value();
			std::string sname = new_customer.GetRaw<std::string>("sname").value();
			std::string name = new_customer.GetRaw<std::string>("name").value();
			std::string pname = new_customer.GetRaw<std::string>("pname").value();
			auto query = sql_connection.query(std::format("CALL CreateCustomer('{}', '{}', '{}', '{}');",
														  tel, sname, name, pname));

			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				if(!res)
					return make_response_str(query.error());

				return make_response_str("success");
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto delete_customer(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			DataType::Int delete_customer_tel = config.GetRaw<DataType::Int>("delete_customer_telephone").value();
			auto query = sql_connection.query(std::format("CALL DeleteCustomer('{}');", delete_customer_tel));
			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				return make_response_str("success");
			}, query);
			//query.exec();
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto add_item(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			Ref new_item = config.GetRawRef("new_item");
			std::string name = new_item.GetRaw<std::string>("name").value();
			DataType::Number cost = new_item.GetRaw<DataType::Number>("cost").value();
			DataType::Int count = new_item.GetRaw<DataType::Int>("count").value();
			DataType::Int distributor_id = new_item.GetRaw<DataType::Int>("distributor_id").value();
			DataType::Int type_id = new_item.GetRaw<DataType::Int>("type_id").value();
			std::string description = new_item.GetRaw<std::string>("description").value();
			auto query = sql_connection.query(std::format("CALL AddItem('{}', {}, {}, {}, {}, '{}');",
														  name, cost, count, distributor_id, type_id, description));

			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				if(!res)
					return make_response_str(query.error());

				return make_response_str("success");
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto add_item_count(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			Ref item = config.GetRawRef("item");
			DataType::Int id = item.GetRaw<DataType::Int>("id").value();
			DataType::Int appended_count = item.GetRaw<DataType::Int>("appended_count").value();
			auto query = sql_connection.query(std::format("CALL AddItemCount('{}', '{}');",
														  id, appended_count));

			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				return make_response_str("success");
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto complete_order(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			DataType::Int id = config.GetRaw<DataType::Int>("order_id").value();
			auto query = sql_connection.query(std::format("CALL CompleteOrder('{}');",
														  id));
			mysqlpp::Transaction tr(sql_connection);
			return invoke_stored_procedure<false>([&](bool res) -> std::string
			{
				if(!res)
				{
					tr.rollback();
					return make_response_str(query.error());
				}
				else
				{
					tr.commit();
					return make_response_str("success");
				}
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto add_order(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Employee", "Admin"}); !str.empty())
			return str;

		try
		{
			Ref new_order = config.GetRawRef("new_order");
			DataType::Int customer_tel = new_order.GetRaw<DataType::Int>("customer_telephone").value();
			DataType::Int date_of_expire = new_order.GetRaw<DataType::Int>("date_of_expire").value();
			if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() > date_of_expire)
				return make_response_str("bad_config");

			struct OrderedItem
			{
				int item_id;
				uint32_t count_in_order;

				constexpr OrderedItem(int _item_id, uint32_t _count_in_order)
					: item_id(_item_id), count_in_order(_count_in_order) {}
			};

			std::vector<OrderedItem> items;
			items.reserve(new_order.GetLength());
			if(items.capacity() == 0)
				return make_response_str("bad_config");

			for(DataType::Int i = 1; i <= new_order.GetLength(); i++)
			{
				Ref ord_item = new_order.GetRawRef(i);
				items.push_back(OrderedItem(
					ord_item.GetRaw<DataType::Int>("item_id").value(),
					ord_item.GetRaw<DataType::Int>("count_in_order").value()));
			}

			auto query = sql_connection.query(std::format("CALL AddOrder('{}', 'FROM_UNIXTIME{}');",
														  customer_tel, date_of_expire));
			mysqlpp::Transaction tr(sql_connection);
			auto new_order_id = invoke_stored_procedure<false>([&](bool res) -> hrs::expected<int, std::string>
			 {
				if(!res)
				{
					tr.rollback();
					return make_response_str(query.error());
				}
				else
					return query.insert_id();
			}, query);

			if(!new_order_id.has_value())
				return new_order_id.error();

			/*if(!query.exec())
			{
				tr.rollback();
				return make_response_str(query.error());
			}*/

			//auto new_order_id = query.insert_id();
			for(const auto &item : items)
			{
				query = sql_connection.query(std::format("CALL AddOrderedItem('{}', '{}', '{}');",
													   new_order_id.value(), item.item_id, item.count_in_order));

				auto item_add_result = invoke_stored_procedure<false>([&](bool res) -> std::string
				{
					if(!res)
					{
						tr.rollback();
						return make_response_str(query.error());
					}
					else
						return "";
				}, query);

				if(!item_add_result.empty())
					return item_add_result;
				/*if(!query.exec())
				{
					tr.rollback();
					return make_response_str(query.error());
				}*/
			}

			tr.commit();
			return make_response_str("success");
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto get_expired_orders(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		if(auto str = check_user_existance(config, sql_connection, std::array{"Admin", "Employee", "Accountant"}); !str.empty())
			return str;

		auto query = sql_connection.query("CALL GetExpiredOrders();");
		return invoke_stored_procedure<true>([&vm](mysqlpp::StoreQueryResult &res) -> std::string
		{
			return make_response_str("success") + serialize_query_result(vm, res, "expired_orders",
			std::array
			{
				FieldAndType{"id", VMType::Int},
				FieldAndType{"date_of_start", VMType::Int},
				FieldAndType{"date_of_expire", VMType::Int},
				FieldAndType{"date_of_completion", VMType::Int, false},
				FieldAndType{"c_telephone", VMType::Int},
				FieldAndType{"c_sname", VMType::String},
				FieldAndType{"c_name", VMType::String},
				FieldAndType{"c_pname", VMType::String}
			});
		}, query);
	}

	inline auto get_completed_orders(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		try
		{
			Ref dates = config.GetRawRef("dates");
			DataType::Int start = dates.GetRaw<DataType::Int>("start_date").value();
			DataType::Int end = dates.GetRaw<DataType::Int>("end_date").value();
			auto query = sql_connection.query(std::format("CALL GetCompletedOrders(FROM_UNIXTIME({}), FROM_UNIXTIME({}));",
														  start, end));
			return invoke_stored_procedure<true>([&vm](mysqlpp::StoreQueryResult &res) -> std::string
			{
				return make_response_str("success") + serialize_query_result(vm, res, "completed_orders",
				std::array
				{
					FieldAndType{"id", VMType::Int},
					FieldAndType{"date_of_start", VMType::Int},
					FieldAndType{"date_of_expire", VMType::Int},
					FieldAndType{"date_of_completion", VMType::Int, false},
					FieldAndType{"c_telephone", VMType::Int},
					FieldAndType{"c_sname", VMType::String},
					FieldAndType{"c_name", VMType::String},
					FieldAndType{"c_pname", VMType::String}
				});
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto get_income(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		try
		{
			Ref dates = config.GetRawRef("dates");
			DataType::Int start = dates.GetRaw<DataType::Int>("start_date").value();
			DataType::Int end = dates.GetRaw<DataType::Int>("end_date").value();
			auto query = sql_connection.query(std::format("CALL GetIncome(FROM_UNIXTIME({}), FROM_UNIXTIME({}));",
														  start, end));
			return invoke_stored_procedure<true>([&vm](mysqlpp::StoreQueryResult &res) -> std::string
			{
				return make_response_str("success") + serialize_one_query_result(vm, res,
						FieldAndType{"income", VMType::Number});
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

	inline auto get_statistics(Ref config, VM &vm, mysqlpp::Connection &sql_connection) -> std::string
	{
		try
		{
			Ref dates = config.GetRawRef("dates");
			DataType::Int start = dates.GetRaw<DataType::Int>("start_date").value();
			DataType::Int end = dates.GetRaw<DataType::Int>("end_date").value();
			auto query = sql_connection.query(std::format("CALL GetSellsStatiscticsByDistributors(FROM_UNIXTIME({}), FROM_UNIXTIME({}));",
														  start, end));
			return invoke_stored_procedure<true>([&vm](mysqlpp::StoreQueryResult &res) -> std::string
			{
				return make_response_str("success") + serialize_query_result(vm, res, "statistics",
				std::array
				{
					FieldAndType{"common_cost", VMType::Number},
					FieldAndType{"common_count", VMType::Int},
					FieldAndType{"distributor", VMType::String}
				});
			}, query);
		}
		catch(...)
		{
			return make_response_str("bad_config");
		}
	}

};
