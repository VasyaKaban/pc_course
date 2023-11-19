#pragma once

#include <QAbstractItemModel>

void UpdateModel(QAbstractItemModel &model)
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
