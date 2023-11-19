#ifndef NEWITEMDIALOG_H
#define NEWITEMDIALOG_H

#include <QWidget>

namespace Ui {
	class NewItemDialog;
}

class NewItemDialog : public QWidget
{
	Q_OBJECT

		public:
				 explicit NewItemDialog(QWidget *parent = nullptr);
	~NewItemDialog();

private:
	Ui::NewItemDialog *ui;
};

#endif // NEWITEMDIALOG_H
