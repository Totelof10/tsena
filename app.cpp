#include "app.h"
#include "ui_app.h"
#include "custommessagebox.h"
#include "databasemanager.h"
#include "ajoutfournisseur.h"
#include "ajoutnouveauproduits.h"
#include "etatstock.h"
#include "mouvementdestock.h"
#include "ajoutclient.h"
#include "ajoutvente.h"

App::App(int userId, const QString& userStatus, MainWindow* mainWindow, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
    , m_currentUserId(userId)
    , m_currentUserStatus(userStatus)
    , m_mainWindow(mainWindow)
{
    ui->setupUi(this);
    attributionAcces();
    afficherProduit();
    //connect(ui->btnPageStock, &QPushButton::clicked, this, &App::afficherProduit);
    ui->tableStock->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stackedWidget->setCurrentWidget(ui->page_vente);
    connect(ui->btnPageVente, &QPushButton::clicked,this, &App::handleVenteFacturation);
    connect(ui->btnPageStock, &QPushButton::clicked,this, &App::handleStockReaprovisionnement);
    connect(ui->btnDeconnexion, &QPushButton::clicked, this, &App::handleDeconnexion);
    connect(ui->btnAjoutStock, &QPushButton::clicked, this, &App::ancienNouveau);
    connect(ui->btnAjoutVente, &QPushButton::clicked, this, &App::ancienNouveauClient);
    connect(ui->btnEtatStock, &QPushButton::clicked, this, &App::etatStock);
    connect(ui->btnMouvement, &QPushButton::clicked, this, &App::mouvementStock);
    ui->tableStock->setEditTriggers(QAbstractItemView::DoubleClicked);
    connect(ui->tableStock, &QTableWidget::itemChanged, this, &App::gererModificationCellule);
    connect(ui->btnSupprimerStock, &QPushButton::clicked, this, &App::supprimerLigne);
    connect(ui->tableStock, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        if (item) {
            ancienneValeur = item->text(); // Sauvegarder l'ancienne valeur
        }
    });

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
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Fermer la session");
    msgBox.setText("Souhaitez vous fermer la session?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        emit deconnexion();
        this->close();
    } else {

    }
}

//Bouton d'Ajout de nouveau fournisseur et nouveau produit
void App::ancienNouveau()
{
    CustomMessageBox msgBox;
    msgBox.setText("Est ce un nouveau fournisseur?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        AjoutFournisseur *ajout = new AjoutFournisseur(nullptr);
        connect(ajout, &AjoutFournisseur::fournisseurAjout, this, [this]() {
            AjoutNouveauProduits *ajoutProduit = new AjoutNouveauProduits(nullptr);
            connect(ajoutProduit, &AjoutNouveauProduits::ajoutProduit, this, &App::afficherProduit);
            //ajoutProduit->setWindowModality(Qt::ApplicationModal);
            ajoutProduit->show();
        });
        ajout->show();
    } else {
        AjoutNouveauProduits *ajout = new AjoutNouveauProduits(nullptr);
        connect(ajout, &AjoutNouveauProduits::ajoutProduit, this, &App::afficherProduit);
        ajout->show();

    }
}

void App::ancienNouveauClient(){
    CustomMessageBox msgBox;
    msgBox.setText("Est-ce un nouveau client?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        AjoutClient *client = new AjoutClient(nullptr);
        connect(client, &AjoutClient::ajouteClient, this, [this](){
            AjoutVente *vente = new AjoutVente(nullptr);
            connect(vente, &AjoutVente::ajouterVente, this, &App::afficherVente);
            vente->show();
        });
        client->show();
    }else{
        AjoutVente *vente = new AjoutVente(nullptr);
        connect(vente, &AjoutVente::ajouterVente, this, &App::afficherVente);
        vente->show();
    }
}

void App::afficherVente(){

}

void App::afficherProduit(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        msgBox.showError("","Erreur lors de l'ouverture de la base de données");
        return;
    }
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT p.id_produit, p.abreviation, p.nom, p.prix_unitaire, f.nom, p.unite, p.categorie FROM produits p "
                  "INNER JOIN fournisseur f ON p.fournisseur_id = f.id_fournisseur");
    if(!query.exec()){
        msgBox.showError("","Erreur lors de la récupération des données");
        qDebug()<<query.lastError();
        return;
    }
    ui->tableStock->setRowCount(0);

    int row = 0;
    while(query.next()){
        ui->tableStock->insertRow(row);
        QString id_produits = query.value(0).toString();
        QString abreviation = query.value(1).toString();
        QString nom = query.value(2).toString();
        QString fournisseur = query.value(3).toString();
        QString prix_unitaire = query.value(4).toString();
        QString unite = query.value(5).toString();
        QString categorie = query.value(6).toString();
        //int quantite = query.value(7).toInt();

        ui->tableStock->setItem(row, 0, new QTableWidgetItem(id_produits));
        ui->tableStock->setItem(row, 1, new QTableWidgetItem(abreviation));
        ui->tableStock->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableStock->setItem(row, 3, new QTableWidgetItem(fournisseur));
        ui->tableStock->setItem(row, 4, new QTableWidgetItem(prix_unitaire));
        ui->tableStock->setItem(row, 5, new QTableWidgetItem(unite));
        ui->tableStock->setItem(row, 6, new QTableWidgetItem(categorie));
        //ui->tableStock->setItem(row, 7, new QTableWidgetItem(QString::number(quantite)));

        row++;
    }

    qDebug ()<<"Affiche du tableau";
}

void App::gererModificationCellule(QTableWidgetItem *item) {
    int row = item->row();
    int col = item->column();

    // Vérifier si la colonne est modifiable
    if (col < 1 || col > 6) return; // Colonnes modifiables : de 1 (abreviation) à 6 (categorie)

    QString nouvelleValeur = item->text();
    if (nouvelleValeur.isEmpty()) {
        qDebug() << "La valeur entrée est vide.";
        return;
    }

    // Message de confirmation
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Souhaitez-vous modifier cette valeur ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        // Mise à jour de la base de données
        QString idProduit = ui->tableStock->item(row, 0)->text(); // ID du produit
        QString colonne;

        switch (col) {
        case 1: colonne = "abreviation"; break;
        case 2: colonne = "nom"; break;
        case 3: colonne = "prix_unitaire"; break;
        case 4: colonne = "fournisseur_id"; break;
        case 5: colonne = "unite"; break;
        case 6: colonne = "categorie"; break;
        }

        QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
        QSqlQuery query(sqlitedb);
        query.prepare(QString("UPDATE produits SET %1 = :valeur WHERE id_produit = :id").arg(colonne));
        query.bindValue(":valeur", nouvelleValeur);
        query.bindValue(":id", idProduit);

        if (!query.exec()) {
            qDebug() << "Erreur lors de la mise à jour de la base de données :" << query.lastError();
            CustomMessageBox().showError("Erreur", "Échec de la mise à jour de la base de données.");
            item->setText(ancienneValeur); // Restaurer l'ancienne valeur
        }
    } else {
        // Restaurer l'ancienne valeur si l'utilisateur annule
        item->setText(ancienneValeur);
    }
}

void App::supprimerLigne() {
    // Récupérer la ligne sélectionnée
    int row = ui->tableStock->currentRow();
    if (row < 0) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une ligne à supprimer.");
        return;
    }

    // Récupérer l'ID du produit (colonne 0)
    QTableWidgetItem *celluleId = ui->tableStock->item(row, 0);
    if (!celluleId) {
        CustomMessageBox().showError("Erreur", "Impossible de trouver l'ID du produit.");
        return;
    }

    QString idProduit = celluleId->text();

    // Message de confirmation
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Êtes-vous sûr de vouloir supprimer cet élément ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        // Supprimer de la base de données
        QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
        QSqlQuery query(sqlitedb);
        query.prepare("DELETE FROM produits WHERE id_produit = :id");
        query.bindValue(":id", idProduit);

        if (!query.exec()) {
            qDebug() << "Erreur lors de la suppression de la base de données :" << query.lastError();
            CustomMessageBox().showError("Erreur", "Échec de la suppression de la base de données.");
            return;
        }

        // Supprimer la ligne du tableau
        ui->tableStock->removeRow(row);

        // Afficher un message de succès
        CustomMessageBox().showInformation("Succès", "L'élément a été supprimé avec succès.");
    }
}




void App::etatStock(){
    EtatStock *stock = new EtatStock(nullptr);
    stock->show();
}

void App::mouvementStock(){
    MouvementDeStock *stock = new MouvementDeStock(nullptr);
    stock->show();
}
App::~App()
{
    delete ui;
}
