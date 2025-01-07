#ifndef AJOUTFOURNISSEUR_H
#define AJOUTFOURNISSEUR_H

#include <QWidget>

namespace Ui {
class AjoutFournisseur;
}

class AjoutFournisseur : public QWidget
{
    Q_OBJECT

public:
    explicit AjoutFournisseur(QWidget *parent = nullptr);
    ~AjoutFournisseur();

private slots:
    void ajouterNouveauFournisseur();
    void fermerNouveauFournisseur();

signals:
    void fournisseurAjout();

private:
    Ui::AjoutFournisseur *ui;
};

#endif // AJOUTFOURNISSEUR_H
