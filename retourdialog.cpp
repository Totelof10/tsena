#include "retourdialog.h"

RetourDialog::RetourDialog(const QList<QPair<QString, int>>& produits, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Spécifier les quantités à retourner");
    QVBoxLayout* layout = new QVBoxLayout(this);

    for (const auto& produit : produits) {
        QString nomProduit = produit.first;
        int quantiteLivree = produit.second;

        QLabel* label = new QLabel(nomProduit + " (max " + QString::number(quantiteLivree) + ") :", this);
        QSpinBox* spinBox = new QSpinBox(this);
        spinBox->setMaximum(quantiteLivree);
        spinBox->setValue(0);

        layout->addWidget(label);
        layout->addWidget(spinBox);
        spinBoxes.append(spinBox);
    }

    QPushButton* okButton = new QPushButton("Valider", this);
    connect(okButton, &QPushButton::clicked, this, &RetourDialog::accept);
    layout->addWidget(okButton);
}

QList<int> RetourDialog::getQuantitesRetournees() const {
    QList<int> retours;
    for (QSpinBox* sb : spinBoxes) {
        retours << sb->value();
    }
    return retours;
}
