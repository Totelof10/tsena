#include "app.h"
#include "ui_app.h"
#include "custommessagebox.h"
#include "databasemanager.h"

App::App(int userId, const QString& userStatus, MainWindow* mainWindow, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
    , m_currentUserId(userId)
    , m_currentUserStatus(userStatus)
    , m_mainWindow(mainWindow)
{
    ui->setupUi(this);
    attributionAcces();
    ui->stackedWidget->setCurrentWidget(ui->page_vente);
    connect(ui->btnPageVente, &QPushButton::clicked,this, &App::handleVenteFacturation);
    connect(ui->btnPageStock, &QPushButton::clicked,this, &App::handleStockReaprovisionnement);
    connect(ui->btnDeconnexion, &QPushButton::clicked, this, &App::handleDeconnexion);
}

void App::handleVenteFacturation(){
    ui->stackedWidget->setCurrentWidget(ui->page_vente);
}
void App::handleStockReaprovisionnement(){
    ui->stackedWidget->setCurrentWidget(ui->page_stock);
}
void App::attributionAcces() {
    qDebug() << "ID de l'utilisateur connecté:" << m_currentUserId;
    qDebug() << "Statut de l'utilisateur connecté:" << m_currentUserStatus;

    if (m_currentUserStatus == "Vendeur") {
        ui->btnPageStock->setDisabled(true);
        ui->btnPageFinance->setDisabled(true);
        ui->btnModifierVente->setDisabled(true);
        ui->btnSupprimerVente->setDisabled(true);
    } else {
        ui->btnPageStock->setDisabled(false);
        ui->btnPageFinance->setDisabled(false);
        ui->btnModifierVente->setDisabled(false);
        ui->btnSupprimerVente->setDisabled(false);
    }
}
void App::handleDeconnexion()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Fermer la session");
    msgBox.setText("Souhaitez vous fermer la session?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        emit deconnexion();
        this->close();
    } else {
        // Abbrechen: Nichts tun
    }     // Fermer la fenêtre App
}

App::~App()
{
    delete ui;
}
