#ifndef MOUVEMENTDESTOCK_H
#define MOUVEMENTDESTOCK_H

#include <QWidget>
#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QPropertyAnimation>

namespace Ui {
class MouvementDeStock;
}

class MouvementDeStock : public QWidget
{
    Q_OBJECT

public:
    explicit MouvementDeStock(QWidget *parent = nullptr);
    ~MouvementDeStock();

private slots:
    void afficherMouvement();
    void recherche();
    void filtrageMouvement();
    void imprimerMouvement();

private:
    Ui::MouvementDeStock *ui;
};

#endif // MOUVEMENTDESTOCK_H
