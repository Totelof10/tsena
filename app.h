#ifndef APP_H
#define APP_H

#include <QWidget>
#include <QDebug>
#include "mainwindow.h" // Inclure MainWindow.h
#include <QTableWidgetItem>
#include <QPainter>
#include <QShortcut>
#include <QFileDialog>

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
    QShortcut *saveShortcut;
    QShortcut *loadShortcut;
    QString databasePath;

public:
    App(int userId, const QString& userStatus, MainWindow* mainWindow, QWidget *parent = nullptr); // Constructeur modifié
    ~App();

private slots:
    void handleVenteFacturation();
    void handleStockReaprovisionnement();
    void handleBonDeLivraison();
    void attributionAcces();
    void handleDeconnexion(); // Slot pour le bouton de déconnexion
    void ancienNouveau();
    void ancienNouveauClient();
    void ancienNouveauClientBonDeLivraison();
    void afficherProduit();
    void afficherVente();
    void etatStock();
    void mouvementStock();
    void gererModificationCellule(QTableWidgetItem *item);
    void supprimerLigne();
    void supprimerVente();
    void chiffreDaffaire();
    void affichageFinance();
    void imprimerVente();
    void recherche();
    void filtrageDate();
    void reinitialiserAffichage();
    void saveDatabase();
    void loadDatabase();
    void afficherBonDeLivraison();
    void supprimerBonDeLivraison();
    void filtrageDateBonDeLivraison();
    void rechercheBl();
    void reinitialiserBl();
    void livrePaye();
signals:
    void deconnexion();
private:
    QString ancienneValeur;


};

#endif // APP_H
