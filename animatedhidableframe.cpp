#include "animatedhidableframe.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMap>
#include <QDebug>

AnimatedHidableFrame::AnimatedHidableFrame(QWidget *parent) : QFrame(parent) {
    // Création du bouton toggle
    toggleButton = new QPushButton(this);
    toggleButton->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/liste-a-puces (1).png")); // Icône du bouton
    toggleButton->setFixedSize(24, 24);
    //centrer le toggle
    toggleButton->setStyleSheet("text-align: center;");
    toggleButton->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background: rgba(0, 0, 0, 50); }"
        );
    toggleButton->move(width() - toggleButton->width() - 10, 10);

    // Animation de largeur
    widthAnimation = new QPropertyAnimation(this, "maximumWidth");
    widthAnimation->setDuration(300);
    setMaximumWidth(200); // Définir une largeur initiale

    // Connexion du bouton
    connect(toggleButton, &QPushButton::clicked, this, &AnimatedHidableFrame::toggleFrame);
}

void AnimatedHidableFrame::setupButtons(QPushButton* venteButton,
                                         QPushButton* stockButton,
                                         QPushButton* bonButton,
                                         QPushButton* tiersButton)
{
    btnPageVente = venteButton;
    btnPageStock = stockButton;
    btnPageBon = bonButton;
    btnTiers = tiersButton;

    // Stocker le texte original et l'icône des boutons de navigation
    if (btnPageVente) {
        originalButtonTexts[btnPageVente] = btnPageVente->text();
        originalButtonIcons[btnPageVente] = btnPageVente->icon();
    }
    if (btnPageStock) {
        originalButtonTexts[btnPageStock] = btnPageStock->text();
        originalButtonIcons[btnPageStock] = btnPageStock->icon();
    }
    if (btnPageBon) {
        originalButtonTexts[btnPageBon] = btnPageBon->text();
        originalButtonIcons[btnPageBon] = btnPageBon->icon();
    }
    if (btnTiers) {
        originalButtonTexts[btnTiers] = btnTiers->text();
        originalButtonIcons[btnTiers] = btnTiers->icon();
    }
    qDebug() << "Initial originalButtonTexts (setupButtons):" << originalButtonTexts;
}

void AnimatedHidableFrame::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    toggleButton->move(width() - toggleButton->width() - 5, 5); // Ajuster la position lors du redimensionnement
}

void AnimatedHidableFrame::toggleFrame() {
    qDebug() << "toggleFrame called";
    qDebug() << "btnPageVente pointer:" << btnPageVente;
    qDebug() << "btnPageStock pointer:" << btnPageStock;
    qDebug() << "btnPageBon pointer:" << btnPageBon;
    qDebug() << "btnTiers pointer:" << btnTiers;

    bool isClosing = maximumWidth() > 50;
    qDebug() << "isClosing:" << isClosing << "maximumWidth:" << maximumWidth();

    if (isClosing) { // Navbar visible -> réduire et afficher les icônes
        toggleButton->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/liste-a-puces.png")); // Icône pour navbar fermé
        widthAnimation->setStartValue(width());
        widthAnimation->setEndValue(50);
        qDebug() << "Closing navbar";

        // Changer le texte des boutons de navigation en icônes
        if (btnPageVente) {
            qDebug() << "Changing icon for btnPageVente";
            btnPageVente->setText("");
            btnPageVente->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/bouton-daccueil (1).png"));
        }
        if (btnPageStock) {
            qDebug() << "Changing icon for btnPageStock";
            btnPageStock->setText("");
            btnPageStock->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/traits.png"));
        }
        if (btnPageBon) {
            qDebug() << "Changing icon for btnPageBon";
            btnPageBon->setText("");
            btnPageBon->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/livraison-rapide.png"));
        }
        if (btnTiers) {
            qDebug() << "Changing icon for btnTiers";
            btnTiers->setText("");
            btnTiers->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/manager-avec-tie.png"));
        }
    } else { // Navbar réduit -> agrandir et afficher le texte avec icône
        toggleButton->setIcon(QIcon("C:/Users/rtoha/Downloads/CartoBMW/img/liste-a-puces (1).png")); // Icône pour navbar ouvert
        widthAnimation->setStartValue(50);
        widthAnimation->setEndValue(200);
        show();
        qDebug() << "Opening navbar";

        // Restaurer le texte original des boutons de navigation avec l'icône
        if (btnPageVente && originalButtonTexts.contains(btnPageVente)) {
            qDebug() << "Restoring text for btnPageVente:" << originalButtonTexts[btnPageVente];
            btnPageVente->setText("  " + originalButtonTexts[btnPageVente]);
            btnPageVente->setIcon(originalButtonIcons.value(btnPageVente));
        }
        if (btnPageStock && originalButtonTexts.contains(btnPageStock)) {
            qDebug() << "Restoring text for btnPageStock:" << originalButtonTexts[btnPageStock];
            btnPageStock->setText("  " + originalButtonTexts[btnPageStock]);
            btnPageStock->setIcon(originalButtonIcons.value(btnPageStock));
        }
        if (btnPageBon && originalButtonTexts.contains(btnPageBon)) {
            qDebug() << "Restoring text for btnPageBon:" << originalButtonTexts[btnPageBon];
            btnPageBon->setText("  " + originalButtonTexts[btnPageBon]);
            btnPageBon->setIcon(originalButtonIcons.value(btnPageBon));
        }
        if (btnTiers && originalButtonTexts.contains(btnTiers)) {
            qDebug() << "Restoring text for btnTiers:" << originalButtonTexts[btnTiers];
            btnTiers->setText("  " + originalButtonTexts[btnTiers]);
            btnTiers->setIcon(originalButtonIcons.value(btnTiers));
        }
    }
    widthAnimation->start();
}
