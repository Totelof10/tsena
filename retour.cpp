#include "retour.h"
#include "ui_retour.h"

Retour::Retour(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Retour)
{
    ui->setupUi(this);
}

Retour::~Retour()
{
    delete ui;
}
