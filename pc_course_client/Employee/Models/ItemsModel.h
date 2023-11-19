#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QAbstractSocket>

class ItemsModel : public QAbstractListModel
{
	Q_OBJECT
public:
	struct ItemItem
	{
		int id;
		QString name;
		double cost;
		std::uint32_t count_on_warehouse;
		QString distributor;
		QString type;
		QString description;
	};

	enum Role
	{
		Id = Qt::UserRole + 1,
		Name,
		Cost,
		Count,
		Distributor,
		Type,
		Description,
		All
	};

	void SetItemTypes(const QMap<QString, int> &_item_types);
	void SetDistributors(const QMap<QString, int> &_distributors);
	QMap<QString, int> GetItemTypes() const;
	QMap<QString, int> GetDistributors() const;

	explicit ItemsModel(QObject *parent = nullptr);
	virtual int rowCount(const QModelIndex &parent) const override;
	virtual int columnCount(const QModelIndex &parent) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual bool setHeaderData(int section,
							   Qt::Orientation orientation,
							   const QVariant &value,
							   int role = Qt::EditRole) override;

	void ReceiveRemoteData();
	void Insert(const ItemItem &item);
	//void Remove(const QModelIndex &index);

signals:
	void Error(std::variant<QAbstractSocket::SocketError, QString> error);

private:
	QList<ItemItem> items;
	QString header;
	QMap<QString, int> item_types;
	QMap<QString, int> distributors;
};

Q_DECLARE_METATYPE(ItemsModel::ItemItem);
