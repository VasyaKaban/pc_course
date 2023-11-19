#pragma once

#include <QMessageBox>
#include <QtNetwork/QAbstractSocket>
#include <QMetaEnum>

inline int MessageBox(const QString &title = "",
			   const QString &text = "",
			   QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Ok,
			   QMessageBox::Icon icon = QMessageBox::Icon::Warning)
{
	QMessageBox msg_box;
	msg_box.setText(text);
	msg_box.setWindowTitle(title);
	msg_box.setStandardButtons(buttons);
	msg_box.setIcon(icon);
	return msg_box.exec();
}

inline int SocketErrorMessageBox(QAbstractSocket::SocketError error,
						  QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Ok)
{
	return MessageBox("Connection error",
					  QString("Error: ") + QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error),
					  buttons,
					  QMessageBox::Icon::Critical);
}

inline int LuaConfigErrorMessageBox(const QString &message,
							 QMessageBox::StandardButtons buttons = QMessageBox::StandardButton::Ok)
{
	return MessageBox("Bad config",
					  QString("Error: ") + message,
					  buttons,
					  QMessageBox::Icon::Critical);
}
