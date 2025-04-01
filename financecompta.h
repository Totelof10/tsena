#ifndef FINANCECOMPTA_H
#define FINANCECOMPTA_H

#include <QWidget>
#include <QInputDialog>

namespace Ui {
class FinanceCompta;
}

class FinanceCompta : public QWidget
{
    Q_OBJECT

public:
    explicit FinanceCompta(QWidget *parent = nullptr);
    ~FinanceCompta();

private slots:
    void affichageDesVentes();
    void affichageDesCharges();
    void recherche();
    void filtrerParDate();
    void reinitialiser();
    void ajouterUneCharge();
    void calcul();
private:
    Ui::FinanceCompta *ui;
};

#endif // FINANCECOMPTA_H
