#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <string>
#include <string_view>
#include "Settings.hpp"
#include <concepts>

class ServerConnection : public QObject
{
	Q_OBJECT
	constexpr static quint64 MAX_BLOCK_SIZE = 1024;
public:
	explicit ServerConnection(QObject *parent = nullptr);
	explicit ServerConnection(const QHostAddress &addr, quint16 host, QObject *parent = nullptr);
	void Connect(const QHostAddress &addr, quint16 host);
	void Write(const QByteArray &message, const QHostAddress &addr, quint16 host, int timeout_msec = 3000);
	void Write(const QByteArray &message);
	void Read(const std::string &delim, const QHostAddress &addr, quint16 host, int timeout_msec = 3000);
	void Read(const std::string &delim);
	bool IsConnected();
	void Disconnect();
	bool WaitForConnected(int timeout_msec);
signals:
	void Connected();
	void Disconnected();
	void MessageArrived(std::string message);
	void MessageCarried();
	void ErrorOccured(QAbstractSocket::SocketError error);
private:
	QTcpSocket socket;
	QByteArray input_message;
};

template<std::invocable<QAbstractSocket::SocketError> ErrorF,
		  std::invocable<std::string> ReafF,
		  std::derived_from<QObject> O>
bool ServerConnectionComplex(O *parent, ErrorF &&error_f, ReafF &&read_f,
							 const QByteArray &request, const std::string &delim = "\r\n", int timeout_msec = 1000)
{
	Settings &settings = Settings::Create();
	ServerConnection *connection = new ServerConnection(QHostAddress(settings.GetHostAddress().c_str()),
														settings.GetPort(),
														parent);
	QObject::connect(connection, &ServerConnection::ErrorOccured, parent, std::forward<ErrorF>(error_f));
	bool wait_res = connection->WaitForConnected(timeout_msec);
	connection->Write(request);
	QObject::connect(connection, &ServerConnection::MessageArrived, parent, std::forward<ReafF>(read_f));
	connection->Read(delim);
	//block input???
	return wait_res;
}

#endif // SERVERCONNECTION_H
