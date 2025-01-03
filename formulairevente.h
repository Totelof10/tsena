#ifndef FORMULAIREVENTE_H
#define FORMULAIREVENTE_H

#include <QWidget>

namespace Ui {
class FormulaireVente;
}

class FormulaireVente : public QWidget
{
    Q_OBJECT

public:
    explicit FormulaireVente(QWidget *parent = nullptr);
    ~FormulaireVente();

private:
    Ui::FormulaireVente *ui;
};

#endif // FORMULAIREVENTE_H
