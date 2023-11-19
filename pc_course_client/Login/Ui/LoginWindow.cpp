#include "LoginWindow.h"
#include "./ui_LoginWindow.h"
#include "LuaConf.hpp"
#include <QCryptographicHash>
#include <QHostAddress>
#include "MessageBox.hpp"
#include <QMetaEnum>
#include "Settings.hpp"
#include "User.hpp"
#include "../Admin/Ui/AdminPanel.h"
#include "../Employee/Ui/EmployeePanel.h"
#include "../Accountant/Ui/AccountantPanel.h"
#include "Requests.hpp"

LoginWindow::LoginWindow(QWidget *parent)
	: QMainWindow(parent)
	  , ui(new Ui::LoginWindow)
{
	ui->setupUi(this);
	QObject::connect(ui->enter_btn, &QPushButton::clicked, this, &LoginWindow::CheckUserExists);
#warning SINGLE DATA!!!
	ui->rank_cbox->addItems({"Admin", "Employee", "Accountant"});
	ui->password_le->setEchoMode(QLineEdit::EchoMode::Password);
}

LoginWindow::~LoginWindow()
{
	delete ui;
}

void LoginWindow::CheckUserExists()
{
	if(ui->login_le->text().isEmpty() || ui->password_le->text().isEmpty())
		return;
	auto request = Requests::Auth(Requests::UserLogin(ui->login_le->text(),
													  User::HashPasswordAsString(ui->password_le->text()),
													  ui->rank_cbox->currentText()));

	auto error_f = [this](QAbstractSocket::SocketError error)
	{
		SocketErrorMessageBox(error);
		sender()->deleteLater();
	};
	auto receive_user_existance = [this](std::string message)
	{
		auto [fenv, response] = ParseAndGetResponse(message);
		sender()->deleteLater();
		if(!fenv)
		{
			LuaConfigErrorMessageBox(QString::fromUtf8(response.c_str()));
			return;
		}

		User &usr = User::Create();
		usr.SetLogin(ui->login_le->text());
		usr.SetSha256Password(User::HashPasswordAsString(ui->password_le->text()));
		usr.SetRank(ui->rank_cbox->currentText());
		if(response == "success")
		{
			QWidget *panel = nullptr;
			if(usr.GetRank().toUtf8() == "Admin")
				panel = new AdminPanel;
			else if(usr.GetRank().toUtf8() == "Employee")
				panel = new EmployeePanel;
			else if(usr.GetRank().toUtf8() == "Accountant")
				panel = new AccountantPanel;
			else
				assert(false);

			panel->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
			panel->show();
			close();
		}
		else
			MessageBox("Bad info", "Введены неверные данные");
	};

	ServerConnectionComplex(this, error_f, receive_user_existance, request);
	//block input???
}

