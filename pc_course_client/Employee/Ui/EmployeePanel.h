#ifndef EMPLOYEEPANEL_H
#define EMPLOYEEPANEL_H

#include <QWidget>
#include <QAbstractSocket>

namespace Ui {
	class EmployeePanel;
}

class EmployeePanel : public QWidget
{
	Q_OBJECT

public:
	explicit EmployeePanel(QWidget *parent = nullptr);
	~EmployeePanel();
	virtual void closeEvent(QCloseEvent *event) override;

private slots:
	void CreateCustomer();
	void DeleteCustomer();

private:
	void OnModelError(std::variant<QAbstractSocket::SocketError, QString> error);
	Ui::EmployeePanel *ui;
};

#endif // EMPLOYEEPANEL_H
