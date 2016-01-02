#include "MassRangeDialog.h"
#include "ui_MassRangeDialog.h"

MassRangeDialog::MassRangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MassRangeDialog)
{
    ui->setupUi(this);
}

MassRangeDialog::~MassRangeDialog()
{
    delete ui;
}

void MassRangeDialog::setMax(int v)
{
    ui->maxMass.setValue(v);
}

void MassRangeDialog::setMin(int v)
{
    ui->minMass.setValue(v);
}

int MassRangeDialog::getMax(void)
{
    return ui->maxMass.value();
}

int MassRangeDialog::getMin(void)
{
    return ui->minMass.value();
}

