#ifndef ACCOUNTANTPANEL_H
#define ACCOUNTANTPANEL_H

#include <QWidget>

namespace Ui {
	class AccountantPanel;
}

class AccountantPanel : public QWidget
{
	Q_OBJECT

		public:
				 explicit AccountantPanel(QWidget *parent = nullptr);
	~AccountantPanel();

private:
	Ui::AccountantPanel *ui;
};

#endif // ACCOUNTANTPANEL_H
