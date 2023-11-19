#pragma once

#include <QString>
#include <cstdint>
#include "User.hpp"

namespace Requests
{
	struct UserLogin
	{
		QString login;
		QString password;
		QString rank;

		UserLogin(const QString &_login, const QString &_password, const QString &_rank)
			: login(_login), password(_password), rank(_rank) {}

		static UserLogin FromUser()
		{
			User &usr = User::Create();
			return UserLogin(usr.GetLogin(), usr.GetSha256Password(), usr.GetRank());
		}
	};

	inline QByteArray Auth(const UserLogin &usr)
	{
		return QString::fromUtf8("request = 'auth' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank).toUtf8();
	}

	inline QByteArray GetUsers(const UserLogin &usr)
	{
		return QString::fromUtf8("request = 'get_users' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank).toUtf8();
	}

	inline QByteArray CreateUser(const UserLogin &usr,
						   const QString &login,
						   const QString &password,
						   const QString &sname,
						   const QString &name,
						   const QString &pname,
						   int rank_id)
	{
		return QString::fromUtf8("request = 'create_user' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}"
								 "new_user = "
								 "{"
								 "	login = '%4',"
								 "	password = '%5',"
								 "	sname = '%6',"
								 "	name = '%7',"
								 "	pname = '%8',"
								 "	rank = %9"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank,
				 login, password, sname, name, pname,
				 QString::number(rank_id)).toUtf8();
	}

	inline QByteArray DeleteUser(const UserLogin &usr, const QString &login)
	{
		return QString::fromUtf8("request = 'delete_user' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}"
								 "delete_user_login = '%4'\r\n")
			.arg(usr.login, usr.password, usr.rank, login).toUtf8();
	}

	inline QByteArray GetCustomers(const UserLogin &usr)
	{
		return QString::fromUtf8("request = 'get_customers' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank).toUtf8();
	}

	inline QByteArray CreateCustomer(const UserLogin &usr,
									 std::uint64_t tel_number,
									 const QString &sname,
									 const QString &name,
									 const QString &pname)
	{
		return QString::fromUtf8("request = 'create_customer' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}"
								 "new_customer = "
								 "{"
								 "	telephone = %4,"
								 "	sname = '%5',"
								 "	name = '%6',"
								 "	pname = '%7'"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank,
				 QString::number(tel_number), sname, name, pname).toUtf8();
	}

	inline QByteArray DeleteCustomer(const UserLogin &usr, std::uint64_t tel_number)
	{
		return QString::fromUtf8("request = 'delete_user' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}"
								 "delete_customer_telephone = %4\r\n")
			.arg(usr.login, usr.password, usr.rank, QString::number(tel_number)).toUtf8();
	}

	inline QByteArray GetItems(const UserLogin &usr)
	{
		return QString::fromUtf8("request = 'get_items' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank).toUtf8();
	}

	inline QByteArray CreateItem(const UserLogin &usr,
								 const QString &name,
								 double cost,
								 std::uint32_t count,
								 int distributor_id,
								 int type_id,
								 const QString &description)
	{
		return QString::fromUtf8("request = 'get_items' "
								 "user = "
								 "{"
								 "	login = '%1',"
								 "	password = '%2',"
								 "	rank = '%3'"
								 "}"
								 "new_item = "
								 "{"
								 "	name = '%4',"
								 "	cost = %5,"
								 "	count = %6,"
								 "	distributor_id = %7,"
								 "	type_id = %8,"
								 "	description = %9"
								 "}\r\n")
			.arg(usr.login, usr.password, usr.rank,
				 name, QString::number(cost), QString::number(count),
				 QString::number(distributor_id),
				 QString::number(type_id),
				 description).toUtf8();
	}
};
