#include "formulairevente.h"
#include "ui_formulairevente.h"

FormulaireVente::FormulaireVente(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormulaireVente)
{
    ui->setupUi(this);
}

FormulaireVente::~FormulaireVente()
{
    delete ui;
}
