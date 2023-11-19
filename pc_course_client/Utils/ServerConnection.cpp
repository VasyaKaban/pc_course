#include "ServerConnection.h"
#include <Qt>

ServerConnection::ServerConnection(QObject *parent)
	: QObject{parent}
{
	connect(&socket, &QAbstractSocket::connected, this, &ServerConnection::Connected);
	connect(&socket, &QAbstractSocket::disconnected, this, &ServerConnection::Disconnected);
	connect(&socket, &QAbstractSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error)
	{
		input_message.clear();
		emit ErrorOccured(error);
	});
}

ServerConnection::ServerConnection(const QHostAddress &addr, quint16 host, QObject *parent)
	: ServerConnection(parent)
{
	socket.connectToHost(addr, host);
}

void ServerConnection::Connect(const QHostAddress &addr, quint16 host)
{
	socket.connectToHost(addr, host);
}

bool ServerConnection::IsConnected()
{
	return socket.isOpen();
}

void ServerConnection::Disconnect()
{
	socket.disconnectFromHost();
}

bool ServerConnection::WaitForConnected(int timeout_msec)
{
	return socket.waitForConnected(timeout_msec);
}

void ServerConnection::Write(const QByteArray &message, const QHostAddress &addr, quint16 host, int timeout_msec)
{
	this->Connect(addr, host);
	auto res = this->WaitForConnected(timeout_msec);
	if(!res)
		return;

	Write(message);
}

void ServerConnection::Write(const QByteArray &message)
{
	std::size_t start_ind = 0;
	std::size_t target_size = message.size();
	while(target_size != 0)
	{
		auto res = socket.write(message.data() + start_ind, target_size);
		if(res == -1)
			break;
		else
		{
			start_ind += res;
			target_size -= res;
		}
	}

	if(target_size == 0)
		emit MessageCarried();
}

void ServerConnection::Read(const std::string &delim, const QHostAddress &addr, quint16 host, int timeout_msec)
{
	this->Connect(addr, host);
	auto res = this->WaitForConnected(timeout_msec);
	if(!res)
		return;

	Read(delim);
}

void ServerConnection::Read(const std::string &delim)
{
	std::unique_ptr<QObject> context(new QObject);
	QObject *ptr = context.get();
	connect(&socket, &QAbstractSocket::readyRead, ptr,
			[this, delim, context = std::move(context)]() mutable
	{
		auto block = socket.read(socket.bytesAvailable());
		auto pre_size = input_message.size();
		auto search_start_pos = (pre_size >= (delim.size() - 1) ? (pre_size  - delim.size() - 1) : 0);
		input_message.append(block);
		if(auto ind = input_message.indexOf(delim.c_str(), search_start_pos); ind != -1)
		{
			std::string message(input_message.begin(), input_message.begin() + ind);
			input_message.clear();
			context.reset();
			emit MessageArrived(message);
		}
	});
}
