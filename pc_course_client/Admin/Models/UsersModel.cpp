#include "UsersModel.h"
#include "ServerConnection.h"
#include "Settings.hpp"
#include "User.hpp"
#include "LuaConf.hpp"
#include "Requests.hpp"
#include <QHostAddress>
#include <QMessageBox>
#include <QCryptographicHash>

void UsersModel::SetRanks(const QMap<QString, int> &_ranks)
{
	ranks = _ranks;
}

QMap<QString, int> UsersModel::GetRanks() const
{
	return ranks;
}

UsersModel::UsersModel(QObject *parent)
	: QAbstractTableModel(parent) {}

int UsersModel::rowCount(const QModelIndex &parent) const
{
	return users.size();
}

int UsersModel::columnCount(const QModelIndex &parent) const
{
	return headers.size();
}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
	if(role != Qt::DisplayRole || !index.isValid())
		return {};

	QVariant value;
	switch(index.column())
	{
		case Column::Login:
			value = users[index.row()].login;
			break;
		case Column::Name:
			value = users[index.row()].name;
			break;
		case Column::Sname:
			value = users[index.row()].sname;
			break;
		case Column::Pname:
			value = users[index.row()].pname;
			break;
		case Column::Rank:
			value =  users[index.row()].rank;
			break;
	}

	return value;
}

QVariant UsersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		QVariant value;
		switch(section)
		{
			case Column::Login:
				value = headers[Column::Login];
				break;
			case Column::Name:
				value = headers[Column::Name];
				break;
			case Column::Sname:
				value = headers[Column::Sname];
				break;
			case Column::Pname:
				value = headers[Column::Pname];
				break;
			case Column::Rank:
				value = headers[Column::Rank];
				break;
		}

		return value;
	}

	return {};
}

bool UsersModel::setHeaderData(int section,
							   Qt::Orientation orientation,
							   const QVariant &value,
							   int role)
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case Column::Login:
				headers[Column::Login] = value.toString();
				return true;
			case Column::Sname:
				headers[Column::Sname] = value.toString();
				return true;
			case Column::Name:
				headers[Column::Name] = value.toString();
				return true;
			case Column::Pname:
				headers[Column::Pname] = value.toString();
				return true;
			case Column::Rank:
				headers[Column::Rank] = value.toString();
				return true;
		}
	}

	return false;
}

void UsersModel::ReceiveRemoteData()
{
	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		/*beginRemoveRows({}, 0, users.size() - 1);
		users.clear();
		endRemoveRows();*/
		emit Error(error);
		sender()->deleteLater();
	};

	auto users_received = [this](std::string message)
	{
		auto [fenv, response] = ParseAndGetResponse(message);
		sender()->deleteLater();
		if(!fenv)
		{
			emit Error("Bad response!");
			return;
		}

		using namespace std::string_literals;
		if(response == "success")
		{
			QList<UserItem> new_users;
			LuaWay::Ref users_ref = fenv.GetRef("users"s);
			bool is_error = false;
			try
			{
				for(int i = 1; i <= users_ref.GetLength(); i++)
				{
					LuaWay::Ref usr = users_ref.GetRef<LuaWay::DataType::Int>(i);
					UserItem user_item;
					user_item.login = usr.GetRaw<QString>("login"s).value();
					user_item.sname = usr.GetRaw<QString>("sname"s).value();
					user_item.name = usr.GetRaw<QString>("name"s).value();
					user_item.pname = usr.GetRaw<QString>("pname"s).value();
					user_item.rank = usr.GetRaw<QString>("rank"s).value();
					new_users.push_back(std::move(user_item));
				}
			}
			catch(...)
			{
				is_error = true;
				/*beginRemoveRows({}, 0, users.size());
				users.clear();
				endRemoveRows();*/
				emit Error("Ошибка чтения данных!");
			}
			if(!is_error)
			{
				if(users.size() == new_users.size())
				{
					users = std::move(new_users);
				}
				else if(users.size() < new_users.size())
				{
					//only insert
					beginInsertRows({}, users.size(), users.size() + new_users.size()  - users.size() - 1);
					users = std::move(new_users);
					endInsertRows();
				}
				else
				{
					//users.size() > new_users.size()
					beginRemoveRows({}, new_users.size(), users.size() - 1);
					users.erase(users.begin() + new_users.size(), users.end());
					endRemoveRows();
					for(std::size_t i = 0; i < new_users.size(); i++)
						users[i] = std::move(new_users[i]);
				}
			}
		}
		else
		{
			/*beginRemoveRows({}, 0, users.size());
			users.clear();
			endRemoveRows();*/
			emit Error("Введены неверные данные");
		}
	};
	auto request = Requests::GetUsers(Requests::UserLogin::FromUser());
	ServerConnectionComplex(this, error_f, users_received, request);
	//block???
}

void UsersModel::Insert(const UserItem &usr_item, const QString &password)
{
	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		emit Error(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error));
		sender()->deleteLater();
	};
	auto user_added = [this, usr_item](std::string message)
	{
		auto [fenv, response] = ParseAndGetResponse(message);
		sender()->deleteLater();
		if(!fenv)
		{
			emit Error("Bad response!");
			return;
		}
		if(response != "success")
			emit Error("Введены неверные данные");
		else
		{
			beginInsertRows({}, users.size(), users.size());
			users.push_back(usr_item);
			endInsertRows();
		}
	};
	auto request = Requests::CreateUser(Requests::UserLogin::FromUser(),
										usr_item.login, User::HashPasswordAsString(password),
										usr_item.sname, usr_item.name, usr_item.pname, ranks[usr_item.rank]);
	ServerConnectionComplex(this, error_f, user_added, request);
	//block???
}

void UsersModel::Remove(const QModelIndex &index)
{
	if(!index.isValid())
		return;

	if(index.row() >= users.size() || index.row() < 0)
		return;

	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		emit Error(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error));
		sender()->deleteLater();
	};
	auto user_removed = [this, ind = index.row()](std::string message)
	{
		auto [fenv, response] = ParseAndGetResponse(message);
		sender()->deleteLater();
		if(!fenv)
		{
			emit Error("Bad response!");
			return;
		}
		if(response != "success")
			emit Error("Введены неверные данные");
		else
		{
			beginRemoveRows({}, ind, ind);
			users.removeAt(ind);
			endRemoveRows();
		}
	};
	auto request = Requests::DeleteUser(Requests::UserLogin::FromUser(), users[index.row()].login);
	ServerConnectionComplex(this, error_f, user_removed, request);
	//block???
}
