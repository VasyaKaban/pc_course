#include "CustomersModel.h"
#include "LuaConf.hpp"
#include "Requests.hpp"
#include "ServerConnection.h"

CustomersModel::CustomersModel(QObject *parent)
	: QAbstractTableModel(parent) {}

int CustomersModel::rowCount(const QModelIndex &parent) const
{
	return customers.size();
}

int CustomersModel::columnCount(const QModelIndex &parent) const
{
	return headers.size();
}

QVariant CustomersModel::data(const QModelIndex &index, int role) const
{
	if(role != Qt::DisplayRole || !index.isValid())
		return {};

	QVariant value;
	switch(index.column())
	{
		case Column::TelNumber:
			value = QVariant::fromValue(customers[index.row()].tel_number);
			break;
		case Column::Name:
			value = customers[index.row()].name;
			break;
		case Column::Sname:
			value = customers[index.row()].sname;
			break;
		case Column::Pname:
			value = customers[index.row()].pname;
			break;
	}

	return value;
}

QVariant CustomersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		QVariant value;
		switch(section)
		{
			case Column::TelNumber:
				value = headers[Column::TelNumber];
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
		}

		return value;
	}

	return {};
}

bool CustomersModel::setHeaderData(int section,
								   Qt::Orientation orientation,
								   const QVariant &value,
								   int role)
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case Column::TelNumber:
				headers[Column::TelNumber] = value.toString();
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
		}
	}

	return false;
}

void CustomersModel::ReceiveRemoteData()
{
	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		/*beginRemoveRows({}, 0, customers.size() - 1);
		customers.clear();
		endRemoveRows();*/
		emit Error(error);
		sender()->deleteLater();
	};

	auto customers_received = [this](std::string message)
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
			QList<CustomerItem> new_customers;
			LuaWay::Ref customers_ref = fenv.GetRef("customers"s);
			bool is_error = false;
			try
			{
				for(int i = 1; i <= customers_ref.GetLength(); i++)
				{
					LuaWay::Ref usr = customers_ref.GetRef<LuaWay::DataType::Int>(i);
					CustomerItem customer_item;
					customer_item.tel_number = usr.GetRaw<LuaWay::DataType::Int>("telephone"s).value();
					customer_item.sname = usr.GetRaw<QString>("sname"s).value();
					customer_item.name = usr.GetRaw<QString>("name"s).value();
					customer_item.pname = usr.GetRaw<QString>("pname"s).value();
					new_customers.push_back(std::move(customer_item));
				}
			}
			catch(...)
			{
				is_error = true;
				/*beginRemoveRows({}, 0, customers.size());
				customers.clear();
				endRemoveRows();*/
				emit Error("Ошибка чтения данных!");
			}
			if(!is_error)
			{
				if(customers.size() == new_customers.size())
				{
					customers = std::move(new_customers);
				}
				else if(customers.size() < new_customers.size())
				{
					//only insert
					beginInsertRows({}, customers.size(),
									customers.size() + new_customers.size() - customers.size() - 1);
					customers = std::move(new_customers);
					endInsertRows();
				}
				else
				{
					//users.size() > new_users.size()
					beginRemoveRows({}, new_customers.size(), customers.size() - 1);
					customers.erase(customers.begin() + new_customers.size(), customers.end());
					endRemoveRows();
					for(std::size_t i = 0; i < new_customers.size(); i++)
						customers[i] = std::move(new_customers[i]);
				}
			}
		}
		else
		{
			/*beginRemoveRows({}, 0, customers.size());
			customers.clear();
			endRemoveRows();*/
			emit Error("Введены неверные данные");
		}
	};
	auto request = Requests::GetCustomers(Requests::UserLogin::FromUser());
	ServerConnectionComplex(this, error_f, customers_received, request);
	//block???
}

void CustomersModel::Insert(const CustomerItem &cust_item)
{
	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		emit Error(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error));
		sender()->deleteLater();
	};
	auto customer_added = [this, cust_item](std::string message)
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
			beginInsertRows({}, customers.size(), customers.size());
			customers.push_back(cust_item);
			endInsertRows();
		}
	};
	auto request = Requests::CreateCustomer(Requests::UserLogin::FromUser(),
											cust_item.tel_number,
											cust_item.sname,
											cust_item.name,
											cust_item.pname);
	ServerConnectionComplex(this, error_f, customer_added, request);
	//block???
}

void CustomersModel::Remove(const QModelIndex &index)
{
	if(!index.isValid())
		return;

	if(index.row() >= customers.size() || index.row() < 0)
		return;

	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		emit Error(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error));
		sender()->deleteLater();
	};
	auto customer_removed = [this, ind = index.row()](std::string message)
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
			customers.removeAt(ind);
			endRemoveRows();
		}
	};
	auto request = Requests::DeleteCustomer(Requests::UserLogin::FromUser(), customers[index.row()].tel_number);
	ServerConnectionComplex(this, error_f, customer_removed, request);
	//block???
}
