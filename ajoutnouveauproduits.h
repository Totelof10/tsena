#ifndef AJOUTNOUVEAUPRODUITS_H
#define AJOUTNOUVEAUPRODUITS_H

#include <QWidget>
#include <QDateTime>

namespace Ui {
class AjoutNouveauProduits;
}

class AjoutNouveauProduits : public QWidget
{
    Q_OBJECT

public:
    explicit AjoutNouveauProduits(QWidget *parent = nullptr);
    ~AjoutNouveauProduits();

private slots:
    void afficherFournisseur();
    void ajouterLeProduit();
    void fermerFenetre();
    void clearForm();

signals:
    void ajoutProduit();

private:
    Ui::AjoutNouveauProduits *ui;
};

#endif // AJOUTNOUVEAUPRODUITS_H
