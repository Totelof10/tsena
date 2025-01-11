#include "ajoutclient.h"
#include "ui_ajoutclient.h"
#include "databasemanager.h"
#include "custommessagebox.h"

AjoutClient::AjoutClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AjoutClient)
{
    ui->setupUi(this);
    connect(ui->btnAjouter, &QPushButton::clicked, this, &AjoutClient::ajouterNouveauClient);
    connect(ui->btnAnnuler, &QPushButton::clicked, this, &AjoutClient::annuler);
}

void AjoutClient::ajouterNouveauClient(){
    CustomMessageBox msgBox;
    QString client = ui->lineEditClient->text().trimmed();
    QString coordonnees = ui->lineEditCoordonnees->text().trimmed();
    if (client.isEmpty()) { // Validation : le nom du client est obligatoire
        msgBox.showError("", "Le nom du client est obligatoire.");
        return;
    }
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
    }
    QSqlQuery query(sqlitedb);
    query.prepare("INSERT INTO clients (nom, coordonnees) VALUES (:nom, :coordonnees)");
    query.bindValue(":nom", client);
    query.bindValue(":coordonnees", coordonnees);
    if(!query.exec()){
        qDebug()<<"Erreur lors de l'insertion des données"<<query.lastError();
    }
    msgBox.showInformation("", "Client ajouté avec succés");
    emit ajouteClient();
    this->close();
}

void AjoutClient::annuler(){
    this->close();
}

AjoutClient::~AjoutClient()
{
    delete ui;
}
