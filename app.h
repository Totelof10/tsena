#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QDebug>
#include "mainwindow.h" // Inclure MainWindow.h
#include <QTableWidgetItem>

namespace Ui {
class App;
}

class App : public QWidget
{
    Q_OBJECT

private:
    Ui::App *ui;
    int m_currentUserId;
    QString m_currentUserStatus;
    MainWindow* m_mainWindow; // Pointeur vers MainWindow

public:
    App(int userId, const QString& userStatus, MainWindow* mainWindow, QWidget *parent = nullptr); // Constructeur modifié
    ~App();

private slots:
    void handleVenteFacturation();
    void handleStockReaprovisionnement();
    void attributionAcces();
    void handleDeconnexion(); // Slot pour le bouton de déconnexion
    void ancienNouveau();
    void ancienNouveauClient();
    void afficherProduit();
    void afficherVente();
    void etatStock();
    void mouvementStock();
    void gererModificationCellule(QTableWidgetItem *item);
    void supprimerLigne();
signals:
    void deconnexion();
private:
    QString ancienneValeur;


};

#endif // APP_H
