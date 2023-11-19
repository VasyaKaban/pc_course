#ifndef CUSTOMERSMODEL_H
#define CUSTOMERSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QAbstractSocket>
#include <cstdint>

class CustomersModel : public QAbstractTableModel
{
	Q_OBJECT
public:

	enum Column
	{
		TelNumber = 0,
		Sname = 1,
		Name = 2,
		Pname = 3
	};

	struct CustomerItem
	{
		std::uint64_t tel_number;
		QString sname;
		QString name;
		QString pname;
	};

	explicit CustomersModel(QObject *parent = nullptr);
	virtual int rowCount(const QModelIndex &parent) const override;
	virtual int columnCount(const QModelIndex &parent) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual bool setHeaderData(int section,
							   Qt::Orientation orientation,
							   const QVariant &value,
							   int role = Qt::EditRole) override;

	void ReceiveRemoteData();
	void Insert(const CustomerItem &cust_item);
	void Remove(const QModelIndex &index);

signals:
	void Error(std::variant<QAbstractSocket::SocketError, QString> error);

private:
	QList<CustomerItem> customers;
	std::array<QString, 4> headers;
};

#endif // CUSTOMERSMODEL_H
