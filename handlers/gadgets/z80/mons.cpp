#include "mons.h"
#include "ui_mons.h"

Mons::Mons(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mons)
{
    ui->setupUi(this);
}

Mons::~Mons()
{
    delete ui;
}
