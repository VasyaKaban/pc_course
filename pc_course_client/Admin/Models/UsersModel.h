#ifndef USERSMODEL_H
#define USERSMODEL_H

#include <QAbstractTableModel>
#include <QAbstractSocket>
#include <QList>
#include <QMap>

class UsersModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	enum Column
	{
		Login = 0,
		Sname = 1,
		Name = 2,
		Pname = 3,
		Rank = 4
	};

	struct UserItem
	{
		QString login;
		QString sname;
		QString name;
		QString pname;
		QString rank;
	};

	void SetRanks(const QMap<QString, int> &_ranks);
	QMap<QString, int> GetRanks() const;
	explicit UsersModel(QObject *parent = nullptr);
	virtual int rowCount(const QModelIndex &parent) const override;
	virtual int columnCount(const QModelIndex &parent) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	virtual bool setHeaderData(int section,
							   Qt::Orientation orientation,
							   const QVariant &value,
							   int role = Qt::EditRole) override;

	void ReceiveRemoteData();
	void Insert(const UserItem &usr_item, const QString &password);
	void Remove(const QModelIndex &index);

signals:
	void Error(std::variant<QAbstractSocket::SocketError, QString> error);

private:
	QList<UserItem> users;
	QMap<QString, int> ranks;
	std::array<QString, 5> headers;
};

#endif // USERSMODEL_H
