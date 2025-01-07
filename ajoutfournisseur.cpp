#include "ajoutfournisseur.h"
#include "ui_ajoutfournisseur.h"
#include "databasemanager.h"
#include "custommessagebox.h"
#include "ajoutnouveauproduits.h"

AjoutFournisseur::AjoutFournisseur(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AjoutFournisseur)
{
    ui->setupUi(this);
    connect(ui->btnAjouter, &QPushButton::clicked, this, &AjoutFournisseur::ajouterNouveauFournisseur);
    connect(ui->btnAnnuler, &QPushButton::clicked, this, &AjoutFournisseur::fermerNouveauFournisseur);

}

void AjoutFournisseur::ajouterNouveauFournisseur(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();

    if (!sqlitedb.open()) {
        QMessageBox::critical(this, "Erreur", "Impossible d'accéder à la base de données");
        return;
    }

    QString fournisseur = ui->lineEditFournisseur->text().trimmed(); // Retirer les espaces inutiles
    QString coordonnees = ui->lineEditCoordonnees->text().trimmed();

    if (fournisseur.isEmpty()) { // Validation : le nom du fournisseur est obligatoire
        msgBox.showError("", "Le nom du fournisseur est obligatoire.");
        return;
    }

    QSqlQuery queryFournisseur(sqlitedb);
    queryFournisseur.prepare("INSERT INTO fournisseur (nom, coordonnees) VALUES (:nom, :coordonnees)");
    queryFournisseur.bindValue(":nom", fournisseur); // Utiliser :nom
    queryFournisseur.bindValue(":coordonnees", coordonnees);

    if(!queryFournisseur.exec()){
        msgBox.showError("", "Erreur lors de l'ajout du fournisseur : " + queryFournisseur.lastError().text()); // Message d'erreur plus clair
        sqlitedb.close(); // Fermer la base de données en cas d'erreur
        return;
    }

    msgBox.showInformation("", "Ajout réussi !");
    emit fournisseurAjout();

    // Important : Fermer explicitement la base de données après l'opération
    sqlitedb.close();

    // Créer et afficher la fenêtre AjoutNouveauProduits *APRÈS* l'ajout réussi
    //AjoutNouveauProduits *ajout = new AjoutNouveauProduits(nullptr); // Parent this pour la gestion de la mémoire
    //connect(ajout, &AjoutNouveauProduits::ajoutProduit,this, &AjoutFournisseur::ajouterNouveauFournisseur);
    //ajout->show();

    this->close(); // Fermer la fenêtre d'ajout de fournisseur
}

void AjoutFournisseur::fermerNouveauFournisseur(){
    this->close();
}
AjoutFournisseur::~AjoutFournisseur()
{
    delete ui;
}
