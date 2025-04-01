#ifndef HISTORIQUE_H
#define HISTORIQUE_H

#include <QWidget>

namespace Ui {
class Historique;
}

class Historique : public QWidget
{
    Q_OBJECT

public:
    explicit Historique(QWidget *parent = nullptr);
    ~Historique();

private slots:
    void afficherHistoriqueVente();
    void afficherHistoriqueCharge();
    void calculBeneficeNet();
    void produitDansComboBox();
    void filtrageIntelligent();

private:
    Ui::Historique *ui;
};

#endif // HISTORIQUE_H
