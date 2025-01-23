#ifndef FINANCECOMPTA_H
#define FINANCECOMPTA_H

#include <QWidget>

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
    void recherche();
    void filtrerParDate();
    void reinitialiser();

private:
    Ui::FinanceCompta *ui;
};

#endif // FINANCECOMPTA_H
