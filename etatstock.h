#ifndef ETATSTOCK_H
#define ETATSTOCK_H

#include <QItemSelectionModel>
#include <QWidget>
#include <QDateTime>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

namespace Ui {
class EtatStock;
}

class EtatStock : public QWidget
{
    Q_OBJECT

public:
    explicit EtatStock(QWidget *parent = nullptr);
    ~EtatStock();

private slots:
    void affichageEtatStock();
    void ajouterQuantite();
    void recherche();
    void imprimerStock();
    void rafraichir();
    void retirerQuantite();

private:
    Ui::EtatStock *ui;
};

#endif // ETATSTOCK_H
