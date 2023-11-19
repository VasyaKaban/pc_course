#include "NewItemDialog.h"
#include "ui_NewItemDialog.h"

NewItemDialog::NewItemDialog(QWidget *parent) :
												QWidget(parent),
												ui(new Ui::NewItemDialog)
{
	ui->setupUi(this);
}

NewItemDialog::~NewItemDialog()
{
	delete ui;
}
