#ifndef TIERS_H
#define TIERS_H

#include <QWidget>
#include <QInputDialog>

namespace Ui {
class Tiers;
}

class Tiers : public QWidget
{
    Q_OBJECT

public:
    explicit Tiers(QWidget *parent = nullptr);
    ~Tiers();

private slots:
    void affichageFournisseur();
    void affichageClient();
    void comboAffichage();
    void filtrageFournisseur();
    void filtrageClient();
    void supprimerClient();
    void modifierClient();
    void supprimerFournisseur();
    void modifierFournisseur();
    void ajouterFournisseur();
    void ajouterClient();

private:
    Ui::Tiers *ui;
};

#endif // TIERS_H
