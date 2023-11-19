#include "AdminPanel.h"
#include "ui_AdminPanel.h"
#include "../Models/UsersModel.h"
#include "User.hpp"
#include <QDebug>
#include "MessageBox.hpp"
#include "../Login/Ui/LoginWindow.h"

AdminPanel::AdminPanel(QWidget *parent) :
										  QWidget(parent),
										  ui(new Ui::AdminPanel)
{
	ui->setupUi(this);
	UsersModel *model = new UsersModel(this);
	ui->users_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->users_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->users_view->setModel(model);
	ui->users_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QObject::connect(model, &UsersModel::Error, this, &AdminPanel::OnModelError);
	model->SetRanks({{"Admin", 1}, {"Employee", 2}, {"Accountant", 3}});
	model->ReceiveRemoteData();
	QObject::connect(ui->update_btn, &QPushButton::clicked, this, [model]()
	{
		model->ReceiveRemoteData();
	});
	#warning MAYBE MAKE REQUEST FOR THIS DATA?!
	ui->rank_cbox->addItem("Admin", 1);
	ui->rank_cbox->addItem("Employee", 2);
	ui->rank_cbox->addItem("Accountant", 3);
	QObject::connect(ui->create_btn, &QPushButton::clicked, this, &AdminPanel::CreateUser);
	QObject::connect(ui->delete_btn, &QPushButton::clicked, this, &AdminPanel::DeleteUser);
}

AdminPanel::~AdminPanel()
{
	delete ui;
}

void AdminPanel::closeEvent(QCloseEvent *event)
{
	LoginWindow *login_window = new LoginWindow;
	login_window->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	login_window->show();
	close();
}

void AdminPanel::CreateUser()
{
	if(ui->login_le->text().isEmpty() || ui->password_le->text().isEmpty() ||
		ui->sname_le->text().isEmpty() || ui->name_le->text().isEmpty() ||
		ui->pname_le->text().isEmpty() || ui->rank_cbox->currentText().isEmpty())
		return;

	UsersModel::UserItem usr;
	usr.login = ui->login_le->text();
	usr.sname = ui->sname_le->text();
	usr.name = ui->name_le->text();
	usr.pname = ui->pname_le->text();
	usr.rank = ui->rank_cbox->currentText();
	UsersModel *model = qobject_cast<UsersModel *>(ui->users_view->model());
	model->Insert(usr, ui->password_le->text());
}

void AdminPanel::DeleteUser()
{
	if(!ui->users_view->selectionModel()->hasSelection())
		return;
	UsersModel *model = qobject_cast<UsersModel *>(ui->users_view->model());
	auto row = ui->users_view->selectionModel()->selectedRows()[0].row();
	QModelIndex index = model->index(row, UsersModel::Column::Login);
	if(User::Create().GetLogin() == model->data(index, Qt::DisplayRole).toString())
	{
		MessageBox("Неверное действие", "Вы не можете удалить свой аккаунт!");
		return;
	}

	int choose = MessageBox("Подтверждение",
							"Вы действительно хотите удалить данный акккаунт?",
							QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel,
							QMessageBox::Icon::Question);
	switch(choose)
	{
		case QMessageBox::StandardButton::Ok:
			model->Remove(index);
			break;
	}
}

void AdminPanel::OnModelError(std::variant<QAbstractSocket::SocketError, QString> error)
{
	if(std::holds_alternative<QAbstractSocket::SocketError>(error))
		SocketErrorMessageBox(std::get<QAbstractSocket::SocketError>(error));
	else
		MessageBox("Data error", std::get<QString>(error));
}
