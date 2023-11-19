#include "EmployeePanel.h"
#include "ui_EmployeePanel.h"
#include <QRegExpValidator>
#include "../Models/CustomersModel.h"
#include "MessageBox.hpp"
#include "../../Login/Ui/LoginWindow.h"

EmployeePanel::EmployeePanel(QWidget *parent) :
												QWidget(parent),
												ui(new Ui::EmployeePanel)
{
	ui->setupUi(this);
	ui->customer_telephone_le->setValidator(new QRegExpValidator(QRegExp("[1-9]\\d{,12}"), this));
	CustomersModel *model = new CustomersModel(this);
	ui->customers_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->customers_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->customers_view->setModel(model);
	ui->customers_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QObject::connect(model, &CustomersModel::Error, this, &EmployeePanel::OnModelError);
	model->ReceiveRemoteData();
	QObject::connect(ui->update_customers_btn, &QPushButton::clicked, this, [model]()
	{
		model->ReceiveRemoteData();
	});
	QObject::connect(ui->create_customer_btn, &QPushButton::clicked, this, &EmployeePanel::CreateCustomer);
	QObject::connect(ui->delete_customer_btn, &QPushButton::clicked, this, &EmployeePanel::DeleteCustomer);
}

EmployeePanel::~EmployeePanel()
{
	delete ui;
}

void EmployeePanel::closeEvent(QCloseEvent *event)
{
	LoginWindow *login_window = new LoginWindow;
	login_window->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	login_window->show();
	close();
}

void EmployeePanel::CreateCustomer()
{
	if(ui->customer_telephone_le->text().isEmpty() || ui->customer_sname_le->text().isEmpty() ||
		ui->customer_name_le->text().isEmpty() || ui->customer_pname_le->text().isEmpty())
		return;

	CustomersModel::CustomerItem cust;
	cust.tel_number = ui->customer_telephone_le->text().toULongLong();
	cust.sname = ui->customer_sname_le->text();
	cust.name = ui->customer_name_le->text();
	cust.pname = ui->customer_pname_le->text();
	CustomersModel *model = qobject_cast<CustomersModel *>(ui->customers_view->model());
	model->Insert(cust);
}

void EmployeePanel::DeleteCustomer()
{
	if(!ui->customers_view->selectionModel()->hasSelection())
		return;
	CustomersModel *model = qobject_cast<CustomersModel *>(ui->customers_view->model());
	auto row = ui->customers_view->selectionModel()->selectedRows()[0].row();
	QModelIndex index = model->index(row, CustomersModel::Column::TelNumber);
	int choose = MessageBox("Подтверждение",
							"Вы действительно хотите удалить данного улиента?",
							QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel,
							QMessageBox::Icon::Question);
	switch(choose)
	{
		case QMessageBox::StandardButton::Ok:
			model->Remove(index);
			break;
	}
}

void EmployeePanel::OnModelError(std::variant<QAbstractSocket::SocketError, QString> error)
{
	if(std::holds_alternative<QAbstractSocket::SocketError>(error))
		SocketErrorMessageBox(std::get<QAbstractSocket::SocketError>(error));
	else
		MessageBox("Data error", std::get<QString>(error));
}
