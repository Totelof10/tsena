#ifndef AJOUTVENTE_H
#define AJOUTVENTE_H

#include <QWidget>
#include <QListWidget>
#include <QDate>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>


namespace Ui {
class AjoutVente;
}

class AjoutVente : public QWidget
{
    Q_OBJECT

public:
    explicit AjoutVente(QWidget *parent = nullptr);
    ~AjoutVente();

private slots:
    void ajouterNouvelleVente();
    void afficherInformation();
    void ajouterPanier();
    void enleverPanier();
    void viderPanier();
    void mettreAJourPrix(int index);
    void mettreAJourTotal();
    void clearForm();
    void remettreAZero(int index);



signals:
    void ajouterVente();

private:
    Ui::AjoutVente *ui;
    QMap<int, double> mapPrixProduits;
    QMap<int, double> mapTotauxProduits; // Stocke les totaux par produit

    double totalCumulatif = 0.0; // Stocke le total cumulé


};

#endif // AJOUTVENTE_H
