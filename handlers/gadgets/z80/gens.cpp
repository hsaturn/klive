#include "gens.h"
#include "ui_gens.h"

Gens::Gens(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Gens)
{
    ui->setupUi(this);
}

Gens::~Gens()
{
    delete ui;
}
