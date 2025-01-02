#include "CustomMessageBox.h"

CustomMessageBox::CustomMessageBox(QWidget *parent)
    : QMessageBox(parent)
{
    // Appliquer les styles généraux ici
    this->setStyleSheet("QMessageBox {"
                        "   background-color: white;"
                        "   border: 1px outset grey;"
                        " border-radius: 10px;"
                        "   padding: 0px;"               // Retirer le débordement
                        "   margin: 0px;"                // Ajuster les marges pour éviter tout débordement
                        "}"
                        "QLabel {"
                        "   color: black;"
                        "   font-size: 14px;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: black;"
                        "   padding: 5px 10px;"
                        "   border-radius: 4px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #81DAE3;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #6CBEC7;"
                        "}");

    // Retirer les boutons de fermeture native
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
}

void CustomMessageBox::showInformation(const QString &title, const QString &message)
{
    this->setIcon(QMessageBox::Information);
    this->setText(message);
    this->exec();
}

void CustomMessageBox::showWarning(const QString &title, const QString &message)
{
    this->setIcon(QMessageBox::Warning);
    this->setText(message);

    // Changer la couleur de la bordure pour un avertissement
    this->setStyleSheet("QMessageBox {"
                        "   background-color: white;"
                        "   border: 1px outset grey;"
                        "border-radius: 10px;"
                        "   padding: 0px;"               // Retirer le débordement
                        "   margin: 0px;"                // Ajuster les marges pour éviter tout débordement
                        "   overflow: hidden;"
                        "   clip-path: inset(0);"
                        "}"
                        "QLabel {"
                        "   color: black;"
                        "   font-size: 14px;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: black;"
                        "   padding: 5px 10px;"
                        "   border-radius: 4px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #FFDC7F;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #FABC3F;"
                        "}");
    this->exec();
}

void CustomMessageBox::showError(const QString &title, const QString &message)
{
    this->setIcon(QMessageBox::Critical);
    this->setText(message);

    // Changer la couleur de la bordure pour une erreur
    this->setStyleSheet("QMessageBox {"
                        "   background-color: white;"
                        "   border: 1px outset grey;"
                        " border-radius: 10px;"
                        "   padding: 0px;"               // Retirer le débordement
                        "   margin: 0px;"                // Ajuster les marges pour éviter tout débordement
                        "   overflow: hidden;"
                        "   clip-path: inset(0);"
                        "}"
                        "QLabel {"
                        "   color: black;"
                        "   font-size: 14px;"
                        "}"
                        "QPushButton {"
                        "   background-color: white;"
                        "   color: black;"
                        "   padding: 5px 10px;"
                        "   border-radius: 4px;"
                        "}"
                        "QPushButton:hover {"
                        "   background-color: #FF6969;"
                        "}"
                        "QPushButton:pressed {"
                        "   background-color: #FF0000;"
                        "}");
    this->exec();
}
