#include "AccountantPanel.h"
#include "ui_AccountantPanel.h"

AccountantPanel::AccountantPanel(QWidget *parent) :
													QWidget(parent),
													ui(new Ui::AccountantPanel)
{
	ui->setupUi(this);
}

AccountantPanel::~AccountantPanel()
{
	delete ui;
}
