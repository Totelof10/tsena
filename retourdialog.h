#ifndef RETOURDIALOG_H
#define RETOURDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QList>
#include <QPair>

class RetourDialog : public QDialog {
    Q_OBJECT

public:
    // Le paramètre "produits" contient la liste des produits avec leurs quantités livrées
    RetourDialog(const QList<QPair<QString, int>>& produits, QWidget* parent = nullptr);

    // Cette fonction retourne les quantités saisies par l'utilisateur
    QList<int> getQuantitesRetournees() const;

private:
    QList<QSpinBox*> spinBoxes;
};

#endif // RETOURDIALOG_H
