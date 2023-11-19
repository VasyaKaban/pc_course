#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include <QWidget>
#include <QAbstractSocket>

namespace Ui {
	class AdminPanel;
}

class AdminPanel : public QWidget
{
	Q_OBJECT

public:
	explicit AdminPanel(QWidget *parent = nullptr);
	~AdminPanel();
	virtual void closeEvent(QCloseEvent *event) override;

private slots:
	void CreateUser();
	void DeleteUser();

private:
	void OnModelError(std::variant<QAbstractSocket::SocketError, QString> error);
	Ui::AdminPanel *ui;
};

#endif // ADMINPANEL_H
