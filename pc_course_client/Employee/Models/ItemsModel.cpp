#include "ItemsModel.h"
#include "LuaConf.hpp"
#include "Requests.hpp"
#include "ServerConnection.h"

void ItemsModel::SetItemTypes(const QMap<QString, int> &_item_types)
{
	item_types = _item_types;
}

void ItemsModel::SetDistributors(const QMap<QString, int> &_distributors)
{
	distributors = _distributors;
}

QMap<QString, int> ItemsModel::GetItemTypes() const
{
	return item_types;
}

QMap<QString, int> ItemsModel::GetDistributors() const
{
	return distributors;
}

ItemsModel::ItemsModel(QObject *parent)
	: QAbstractListModel(parent) {}

int ItemsModel::rowCount(const QModelIndex &parent) const
{
	return items.size();
}

int ItemsModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return {};

	QVariant value;
	switch(role)
	{
		case Qt::DisplayRole:
			break;
		case Role::Id:
			value = QVariant::fromValue(items[index.row()].id);
			break;
		case Role::Name:
			value = items[index.row()].name;
			break;
		case Role::Cost:
			value = items[index.row()].cost;
			break;
		case Role::Count:
			value = QVariant::fromValue(items[index.row()].count_on_warehouse);
			break;
		case Role::Distributor:
			value = items[index.row()].distributor;
			break;
		case Role::Type:
			value = items[index.row()].type;
			break;
		case Role::Description:
			value = items[index.row()].description;
			break;
		case Role::All:
			value = QVariant::fromValue(items[index.row()]);
			break;
	}

	return value;
}

QVariant ItemsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
		return header;

	return {};
}

bool ItemsModel::setHeaderData(int section,
							   Qt::Orientation orientation,
							   const QVariant &value,
							   int role)
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		header = value.toString();
		return true;
	}

	return false;
}

void ItemsModel::ReceiveRemoteData()
{
	auto error_f = [this](QAbstractSocket::SocketError error)
	{

		emit Error(error);
		sender()->deleteLater();
	};

	auto items_received = [this](std::string message)
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
			QList<ItemItem> new_items;
			LuaWay::Ref items_ref = fenv.GetRef("items"s);
			bool is_error = false;
			try
			{
				for(int i = 1; i <= items_ref.GetLength(); i++)
				{
					LuaWay::Ref item = items_ref.GetRef<LuaWay::DataType::Int>(i);
					ItemItem model_item;

					//customer_item.tel_number = usr.GetRaw<LuaWay::DataType::Int>("telephone"s).value();
					new_items.push_back(std::move(model_item));
				}
			}
			catch(...)
			{
				is_error = true;
				emit Error("Ошибка чтения данных!");
			}
			if(!is_error)
			{
				if(items.size() == new_items.size())
				{
					items = std::move(new_items);
				}
				else if(items.size() < new_items.size())
				{
					//only insert
					beginInsertRows({}, items.size(),
									items.size() + new_items.size() - items.size() - 1);
					items = std::move(new_items);
					endInsertRows();
				}
				else
				{
					//users.size() > new_users.size()
					beginRemoveRows({}, new_items.size(), items.size() - 1);
					items.erase(items.begin() + new_items.size(), items.end());
					endRemoveRows();
					for(std::size_t i = 0; i < new_items.size(); i++)
						items[i] = std::move(new_items[i]);
				}
			}
		}
		else
			emit Error("Введены неверные данные");
	};
	auto request = Requests::GetItems(Requests::UserLogin::FromUser());
	ServerConnectionComplex(this, error_f, items_received, request);
	//block???
}

void ItemsModel::Insert(const ItemItem &item)
{

}
//void ItemsModel::Remove(const QModelIndex &index);
