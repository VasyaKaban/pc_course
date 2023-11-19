#include "Login/Ui/LoginWindow.h"

#include <QApplication>
#include <QtNetwork/QHostAddress>
#include "Settings.hpp"
#include "LuaConf.hpp"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Settings &settings = Settings::Create();
	LuaWay::VM &vm = LuaConf::Create();
	bool is_second_try_for_connect = false;
	settings.SetHostAddress("127.0.0.1");
	settings.SetPort(2028);

	/*QObject::connect(&connection, &ServerConnection::Connected, [&is_second_try_for_connect]()
	{
		is_second_try_for_connect = false;
		qDebug()<<"Connected!";
	});
	QObject::connect(&connection, &ServerConnection::Disconnected, []()
	{
		qDebug()<<"Disonnected!";
	});


	QObject::connect(&connection, &ServerConnection::ErrorOccured,
					 [&connection, &settings, &is_second_try_for_connect](QAbstractSocket::SocketError err)
	{
		switch(err)
		{
			case QAbstractSocket::SocketError::ConnectionRefusedError:
				if(!is_second_try_for_connect)
				{
					is_second_try_for_connect = true;
					connection.Connect(QHostAddress(settings.GetHostAddress().c_str()), settings.GetPort());
				}
				else
				{
					qDebug()<<err;
					exit(err);
				}
				break;
			default:
				qDebug()<<err;
				exit(err);
				break;
		}
	});*/
	/*QObject::connect(&connection, &ServerConnection::MessageCarried, []()
					{
						qDebug()<<"Carried!";
					});
	QObject::connect(&connection, &ServerConnection::MessageArrived, [](std::string message)
					{
						qDebug()<<message.c_str();
					});*/


	/*if(!connection.IsConnected())
		exit(-1);

	connection.Write("request = 'get_ranks'\r\n");
	connection.Read("\r\n");
	*/

	LoginWindow w;
	w.show();

	return a.exec();
}
