#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include "ServerConnection.h"

QT_BEGIN_NAMESPACE
	namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
	Q_OBJECT

public:
	LoginWindow(QWidget *parent = nullptr);
	~LoginWindow();

private:
	Ui::LoginWindow *ui;

	void CheckUserExists();
};
#endif // LOGINWINDOW_H
