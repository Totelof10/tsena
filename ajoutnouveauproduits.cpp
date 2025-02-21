#include "ajoutnouveauproduits.h"
#include "ui_ajoutnouveauproduits.h"
#include "databasemanager.h"
#include "custommessagebox.h"

AjoutNouveauProduits::AjoutNouveauProduits(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AjoutNouveauProduits)
{
    ui->setupUi(this);
    afficherFournisseur();
    connect(ui->btnAjouter, &QPushButton::clicked, this, &AjoutNouveauProduits::ajouterLeProduit);
    connect(ui->btnAnnuler, &QPushButton::clicked, this, &AjoutNouveauProduits::fermerFenetre);
}

void AjoutNouveauProduits::afficherFournisseur(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Base de données fermées";
    }

    QSqlQuery queryFournisseur(sqlitedb);
    if(!queryFournisseur.prepare("SELECT id_fournisseur, nom FROM fournisseur")){
        qDebug() << queryFournisseur.lastError();
        return;
    };

    if (!queryFournisseur.exec()) {
        qDebug() << "Erreur SQL : " << queryFournisseur.lastError().text();
        return;
    }
    ui->comboFournisseur->clear();
    ui->comboFournisseur->addItem("");

    while(queryFournisseur.next()){
        int id = queryFournisseur.value(0).toInt();
        QString nom = queryFournisseur.value(1).toString();
        qDebug() << nom;  // Afficher chaque armoire pour déboguer
        ui->comboFournisseur->addItem(nom, id);
    }

}

void AjoutNouveauProduits::ajouterLeProduit(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug() <<"Erreur lors de l'ouverture de la base de données";
    }
    QString abreviation = ui->lineEditAbreviation->text().trimmed();
    QString nom = ui->lineEditNom->text().trimmed();
    QString prix_base = ui->lineEditPrixBase->text().trimmed();
    QString prix_unitaire = ui->lineEditPrixUnitaire->text().trimmed();
    QString prix_detail = ui->lineEditPrixDetail->text().trimmed();
    QString prix_remise = ui->lineEditEditPrixRemise->text().trimmed();
    int fournisseur_id = ui->comboFournisseur->currentData().toInt();
    QString unite = ui->lineEditUnite->text().trimmed();
    QString categorie = ui->lineEditCategorie->text().trimmed();
    //int quantite = 0;
    QSqlQuery query(sqlitedb);
    query.prepare("INSERT INTO produits (abreviation, nom, prix_unitaire, fournisseur_id, unite, categorie, prix_base, prix_detail, prix_remise) VALUES (:abreviation, :nom, :prix_unitaire, :fournisseur_id, :unite, :categorie, :prix_base, :prix_detail, :prix_remise)");
    query.bindValue(":abreviation", abreviation);
    query.bindValue(":nom", nom);
    bool ok;
    double prix = prix_unitaire.toDouble(&ok);
    if(!ok){
        qDebug () << "erreur du format du prix";
    }
    query.bindValue(":prix_unitaire", prix);
    query.bindValue(":fournisseur_id", fournisseur_id);
    query.bindValue(":categorie", categorie);
    query.bindValue(":unite", unite);
    query.bindValue(":prix_base", prix_base);
    query.bindValue(":prix_detail", prix_detail);
    query.bindValue(":prix_remise", prix_remise);
    if(!query.exec()){
        msgBox.showError("","Erreur lors de l'ajout du produit");
        qDebug()<<"Erreur lors de l'ajout du produit"<<query.lastError();
        sqlitedb.rollback();
        this->close();
    }

    int idNouveauProduit = query.lastInsertId().toInt();
    int quantite = 0;
    //int seuil_alert = 480;
    if(!query.prepare("INSERT INTO stock (produit_id, quantite, date_du_dernier_entree) VALUES (:produit_id, :quantite, :date_du_dernier_entree)")){
        qDebug()<< "Erreur lors de la récupération des données" <<query.lastError();
        sqlitedb.rollback();
        return;
    }
    query.bindValue(":produit_id", idNouveauProduit);
    query.bindValue(":quantite", quantite);
    //query.bindValue(":sueil_alert", seuil_alert);
    query.bindValue(":date_du_dernier_entree", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
    if(!query.exec()){
        qDebug()<< "Erreur lors de l'insertion des données"<<query.lastError();
        sqlitedb.rollback();
        return;
    }

    //query.bindValue(":quantite", quantite);
    msgBox.showInformation("", "Ajout réussi!");
    clearForm();
    emit ajoutProduit();

}
void AjoutNouveauProduits::clearForm(){
    ui->lineEditAbreviation->clear();
    ui->lineEditCategorie->clear();
    ui->lineEditNom->clear();
    ui->lineEditPrixUnitaire->clear();
    ui->lineEditUnite->clear();
    ui->comboFournisseur->setCurrentIndex(0);
    ui->lineEditPrixBase->clear();
}

void AjoutNouveauProduits::fermerFenetre(){
    this->close();
}

AjoutNouveauProduits::~AjoutNouveauProduits()
{
    delete ui;
}
