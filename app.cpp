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
#include "financecompta.h"
#include "bondelivraison.h"
#include "operation.h"
#include "tiers.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

App::App(int userId, const QString& userStatus, MainWindow* mainWindow, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::App)
    , m_currentUserId(userId)
    , m_currentUserStatus(userStatus)
    , m_mainWindow(mainWindow)
{
    ui->setupUi(this);
    databasePath = "C:/db_test/toavina_yaourt.db";
    QFileInfo fileInfo(databasePath);
    ui->labelWorkspace->setText(fileInfo.fileName());
    attributionAcces();
    afficherProduit();
    afficherVente();
    chiffreDaffaire();
    afficherBonDeLivraison();
    ui->tableStock->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableVente->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableBonDeLivraison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stackedWidget->setCurrentWidget(ui->page_vente);
    connect(ui->btnPageVente, &QPushButton::clicked,this, &App::handleVenteFacturation);
    connect(ui->btnPageStock, &QPushButton::clicked,this, &App::handleStockReaprovisionnement);
    connect(ui->btnPageBon, &QPushButton::clicked,this, &App::handleBonDeLivraison);
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
    connect(ui->btnSupprimerVente, &QPushButton::clicked, this, &App::supprimerVente);
    connect(ui->btnFinance, &QPushButton::clicked, this, &App::affichageFinance);
    connect(ui->btnImprimerVente, &QPushButton::clicked, this, &App::imprimerVente);
    connect(ui->lineEditRecherche, &QLineEdit::textChanged, this, &App::recherche);
    connect(ui->btnFiltrerDate, &QPushButton::clicked, this, &App::filtrageDate);
    connect(ui->btnReinitialiser, &QPushButton::clicked, this, &App::reinitialiserAffichage);
    saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(saveShortcut, &QShortcut::activated, this, &App::saveDatabase);
    loadShortcut = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(loadShortcut, &QShortcut::activated, this, &App::loadDatabase);
    connect(ui->btnNouveauBonDeLivraison, &QPushButton::clicked, this, &App::ancienNouveauClientBonDeLivraison);
    connect(ui->btnFiltrageDateBl, &QPushButton::clicked, this, &App::filtrageDateBonDeLivraison);
    connect(ui->lineEditRechercheBl, &QLineEdit::textChanged, this, &App::rechercheBl);
    connect(ui->btnReinitialiserBl, &QPushButton::clicked, this, &App::reinitialiserBl);
    //connect(ui->btnSupprimerBonDeLivraison, &QPushButton::clicked, this, &App::supprimerBonDeLivraison);
    connect(ui->btnModifierBl, &QPushButton::clicked, this, &App::livrePaye);
    connect(ui->btnOperation, &QPushButton::clicked, this, &App::tableDesOperations);
    connect(ui->btnTiers, &QPushButton::clicked, this, &App::affichageTiers);
    connect(ui->btnReporterDateBl, &QPushButton::clicked, this, &App::reporterDateBl);
}

void App::tableDesOperations(){
    Operation *operation = new Operation();
    operation->show();
}

void App::handleBonDeLivraison(){
    ui->stackedWidget->setCurrentWidget(ui->pageBonDeLivraison);
}
void App::loadDatabase() {
    //Créer un fonction permettant de charger une autre base de données
    QString loadPath = QFileDialog::getOpenFileName(this, "Charger une base de données", "", "Fichier SQLite (*.db)");
    if(loadPath.isEmpty()){
        return;
    }

    // Fermer la base de données actuelle
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(sqlitedb.isOpen()){
        sqlitedb.close();
    }

    // Ouvrir la nouvelle base de données
    sqlitedb.setDatabaseName(loadPath);
    if(!sqlitedb.open()){
        CustomMessageBox msgBox;
        msgBox.showError("Erreur", "Impossible d'ouvrir la nouvelle base de données");
        qDebug() << sqlitedb.lastError().text();
        return;
    }

    CustomMessageBox msgBox;
    msgBox.showInformation("Succès", "Base de données chargée avec succès");
    QFileInfo fileInfo(loadPath);
    ui->labelWorkspace->setText(fileInfo.fileName());
    //Recharger les données
    afficherProduit();
    afficherVente();
    chiffreDaffaire();
}


void App::saveDatabase(){
    //Vérifier si la base de données est ouverte avant de la copier et la fermer
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        CustomMessageBox msgBox;
        msgBox.showError("Erreur", "Impossible d'ouvrir la base de données");
        return;
    }
    sqlitedb.close();

    QString savePath = QFileDialog::getSaveFileName(this, "Enregistrer la base de données", "", "Fichier SQLite (*.db)");
    if(savePath.isEmpty()){
        return;
    }
    QFile file(databasePath);
    if(!file.copy(savePath)){
        CustomMessageBox msgBox;
        msgBox.showError("Erreur", "Impossible de copier la base de données");
        return;
    }
    CustomMessageBox msgBox;
    msgBox.showInformation("Succès", "Base de données enregistrée avec succès");
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
        //ui->btnPageFinance->setDisabled(true);
        ui->btnModifierVente->setDisabled(true);
        ui->btnSupprimerVente->setDisabled(true);
    } else {
        ui->btnPageStock->setDisabled(false);
        //ui->btnPageFinance->setDisabled(false);
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
            connect(vente, &AjoutVente::CA, this, &App::chiffreDaffaire);
            vente->show();
        });
        client->show();
    }else{
        AjoutVente *vente = new AjoutVente(nullptr);
        connect(vente, &AjoutVente::ajouterVente, this, &App::afficherVente);
        connect(vente, &AjoutVente::CA, this, &App::chiffreDaffaire);
        vente->show();
    }
}

void App::ancienNouveauClientBonDeLivraison(){
    CustomMessageBox msgBox;
    msgBox.setText("Est-ce un nouveau client?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        AjoutClient *client = new AjoutClient(nullptr);
        connect(client, &AjoutClient::ajouteClient, this, [this](){
            BonDeLivraison *bl = new BonDeLivraison(nullptr);
            connect(bl, &BonDeLivraison::ajouterBonDeLivraison, this, &App::afficherBonDeLivraison);
            bl->show();
        });
        client->show();
    }else{
        BonDeLivraison *bl = new BonDeLivraison(nullptr);
        connect(bl, &BonDeLivraison::ajouterBonDeLivraison, this, &App::afficherBonDeLivraison);
        bl->show();
    }
}

void App::afficherBonDeLivraison(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_livraison, c.nom, p.nom, quantite, date, statut, prix_total_a_payer, prix_vente FROM bon_de_livraison b "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id");
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableBonDeLivraison->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableBonDeLivraison->insertRow(row);
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString nom_produit = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        QString statut = queryAffichage.value(5).toString();
        double prix_total = queryAffichage.value(6).toDouble();
        double prix_vente = queryAffichage.value(7).toDouble();

        ui->tableBonDeLivraison->setItem(row, 0, new QTableWidgetItem(id_livraison));
        ui->tableBonDeLivraison->setItem(row, 1, new QTableWidgetItem(nom_client));
        ui->tableBonDeLivraison->setItem(row, 2, new QTableWidgetItem(nom_produit));
        ui->tableBonDeLivraison->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableBonDeLivraison->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableBonDeLivraison->setItem(row, 5, new QTableWidgetItem(statut));
        ui->tableBonDeLivraison->setItem(row, 6, new QTableWidgetItem(QString::number(prix_total)));
        ui->tableBonDeLivraison->setItem(row, 7, new QTableWidgetItem(QString::number(prix_vente)));


        row++;
    }
}

void App::supprimerBonDeLivraison(){
    // Récupérer la ligne sélectionnée
    int row = ui->tableBonDeLivraison->currentRow();
    if (row < 0) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une ligne à supprimer.");
        return;
    }

    // Récupérer l'ID du produit (colonne 0)
    QTableWidgetItem *celluleId = ui->tableBonDeLivraison->item(row, 0);
    if (!celluleId) {
        CustomMessageBox().showError("Erreur", "Impossible de trouver l'ID du produit.");
        return;
    }

    QString idLivraison = celluleId->text();

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
        query.prepare("DELETE FROM bon_de_livraison WHERE id_livraison = :id");
        query.bindValue(":id", idLivraison);

        if (!query.exec()) {
            qDebug() << "Erreur lors de la suppression de la base de données :" << query.lastError();
            CustomMessageBox().showError("Erreur", "Échec de la suppression de la base de données.");
            return;
        }

        // Supprimer la ligne du tableau
        ui->tableBonDeLivraison->removeRow(row);

        // Afficher un message de succès
        CustomMessageBox().showInformation("Succès", "L'élément a été supprimé avec succès.");
    }
}

void App::livrePaye(){
    int row = ui->tableBonDeLivraison->currentRow();
    if(row < 0){
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une ligne à modifier.");
        return;
    }

    QTableWidgetItem *celluleStatut = ui->tableBonDeLivraison->item(row, 5);
    if(!celluleStatut){
        CustomMessageBox().showError("Erreur", "Impossible de trouver le statut de la livraison.");
        return;
    }

    QString statut = celluleStatut->text();
    if(statut == "Payé"){
        CustomMessageBox().showError("Erreur", "La livraison est déjà payée.");
        return;
    }

    QTableWidgetItem *itemIdLivraison = ui->tableBonDeLivraison->item(row, 0);
    QTableWidgetItem *itemQuantite = ui->tableBonDeLivraison->item(row, 3);
    QTableWidgetItem *itemNom = ui->tableBonDeLivraison->item(row, 2);

    if(!itemIdLivraison || !itemQuantite || !itemNom){
        CustomMessageBox().showError("Erreur", "Une cellule du tableau est invalide.");
        return;
    }
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Souhaitez-vous valider cette transaction comme 'Payée' ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {

    QString idLivraison = itemIdLivraison->text();
    int quantite = itemQuantite->text().toInt();
    QString nom = itemNom->text();

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();

    // Démarrer une transaction
    if (!sqlitedb.transaction()) {
        qDebug() << "Erreur lors du début de la transaction :" << sqlitedb.lastError();
        CustomMessageBox().showError("Erreur", "Impossible de démarrer la transaction.");
        return;
    }

    // 1. Mise à jour du statut dans `bon_de_livraison`
    QSqlQuery query(sqlitedb);
    query.prepare("UPDATE bon_de_livraison SET statut = 'Payé' WHERE id_livraison = :id");
    query.bindValue(":id", idLivraison);
    if(!query.exec()){
        qDebug() << "Erreur mise à jour bon_de_livraison :" << query.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Échec de la mise à jour de la base de données.");
        return;
    }

    // 2. Récupérer l'ID du produit
    QSqlQuery queryStock(sqlitedb);
    queryStock.prepare("SELECT produit_id FROM bon_de_livraison WHERE id_livraison = :id");
    queryStock.bindValue(":id", idLivraison);
    if(!queryStock.exec() || !queryStock.next()){
        qDebug() << "Erreur récupération produit_id :" << queryStock.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Impossible de récupérer l'ID du produit.");
        return;
    }
    int idProduit = queryStock.value(0).toInt();

    // 3. Mettre à jour le stock
    QSqlQuery queryUpdate(sqlitedb);
    queryUpdate.prepare("UPDATE stock SET quantite = quantite - :quantite WHERE produit_id = :id");
    queryUpdate.bindValue(":quantite", quantite);
    queryUpdate.bindValue(":id", idProduit);
    if(!queryUpdate.exec()){
        qDebug() << "Erreur mise à jour stock :" << queryUpdate.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Échec de la mise à jour du stock.");
        return;
    }

    // 4. Récupérer l'ID du stock
    QSqlQuery queryIdStock(sqlitedb);
    queryIdStock.prepare("SELECT id_stock, quantite FROM stock WHERE produit_id = :id");
    queryIdStock.bindValue(":id", idProduit);
    if(!queryIdStock.exec() || !queryIdStock.next()){
        qDebug() << "Erreur récupération id_stock :" << queryIdStock.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Impossible de récupérer l'ID du stock.");
        return;
    }
    int idStock = queryIdStock.value(0).toInt();
    int quantiteStock = queryIdStock.value(1).toInt();

    // 5. Insérer dans `mouvements_de_stock`
    QSqlQuery queryMouvement(sqlitedb);
    queryMouvement.prepare("INSERT INTO mouvements_de_stock (stock_id, nom, quantite, type_mouvement, date_mouvement, vente) "
                           "VALUES (:stock_id, :nom, :quantite, :type_mouvement, :date_mouvement, :vente)");
    queryMouvement.bindValue(":stock_id", idStock);
    queryMouvement.bindValue(":nom", nom);
    queryMouvement.bindValue(":quantite", quantite);
    queryMouvement.bindValue(":type_mouvement", "Sortie Livraison");
    queryMouvement.bindValue(":date_mouvement", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
    queryMouvement.bindValue(":vente", QString("Livraison du %1 pour %2").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy")).arg(ui->tableBonDeLivraison->item(row, 1)->text()));
    if(!queryMouvement.exec()){
        qDebug() << "Erreur insertion mouvements_de_stock :" << queryMouvement.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Échec de l'enregistrement des mouvements de stock.");
        return;
    }

    // 6. Insérer dans `ligne_vente`
    QSqlQuery queryInsertion(sqlitedb);
    queryInsertion.prepare("INSERT INTO ligne_vente (client_id, produit_id, quantite, date_vente, prix_total, num_bon_livraison, prix_vente) "
                           "SELECT client_id, produit_id, quantite, date, prix_total_a_payer, id_livraison, prix_vente FROM bon_de_livraison WHERE id_livraison = :id");
    queryInsertion.bindValue(":id", idLivraison);
    if(!queryInsertion.exec()){
        qDebug() << "Erreur insertion ligne_vente :" << queryInsertion.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Échec de l'insertion des données dans la vente.");
        return;
    }
    QSqlQuery queryOperation(sqlitedb);
    queryOperation.prepare("INSERT INTO operation (mouvement_id, stock_id, nom_produit, stock_depart, quantite_entree, quantite_sortie, stock_actuel, date_operation) "
                           "VALUES (:mouvement_id, :stock_id, :nom_produit, :stock_depart, :quantite_entree, :quantite_sortie, :stock_actuel, :date_operation)");
    queryOperation.bindValue(":mouvement_id", queryMouvement.lastInsertId());
    queryOperation.bindValue(":stock_id", idStock);
    queryOperation.bindValue(":nom_produit", ui->tableBonDeLivraison->item(row, 2)->text());
    queryOperation.bindValue(":stock_depart", quantiteStock);
    queryOperation.bindValue(":quantite_sortie", quantite);
    queryOperation.bindValue(":quantite_entree", 0);
    queryOperation.bindValue(":stock_actuel", quantiteStock - quantite);
    queryOperation.bindValue(":date_operation", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
    if (!queryOperation.exec()) {
        qDebug() << "Erreur lors de l'insertion des données" << queryOperation.lastError();
    }

    // Validation de la transaction
    if (!sqlitedb.commit()) {
        qDebug() << "Erreur commit transaction :" << sqlitedb.lastError();
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Impossible de valider la transaction.");
        return;
    }

    // Mise à jour de l'interface utilisateur
    celluleStatut->setText("Payé");
    CustomMessageBox().showInformation("Succès", "La livraison a été marquée comme payée.");

    afficherVente();
    chiffreDaffaire();
    }else{
        return;
    }
}



void App::reporterDateBl() {
    int row = ui->tableBonDeLivraison->currentRow();
    if (row < 0) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un BL à modifier.");
        return;
    }

    QString id_bl = ui->tableBonDeLivraison->item(row, 0)->text();
    QString date_bl = ui->tableBonDeLivraison->item(row, 4)->text();
    QDate dateActuelle = QDate::fromString(date_bl, "dd/MM/yyyy");

    // 📌 Création de la boîte de dialogue avec un QDateEdit
    QDialog dialog(this);
    dialog.setWindowTitle("Reporter la date");
    QVBoxLayout layout(&dialog);

    QDateEdit dateEdit;
    dateEdit.setCalendarPopup(true);
    dateEdit.setDate(dateActuelle.isValid() ? dateActuelle : QDate::currentDate());
    layout.addWidget(&dateEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Rejected) return;

    QDate nouvelle_date = dateEdit.date();

    // 📌 Mettre à jour la table avec la nouvelle date
    ui->tableBonDeLivraison->item(row, 4)->setText(nouvelle_date.toString("dd/MM/yyyy"));

    // 📌 Mise à jour dans la base de données
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.lastError().text();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("UPDATE bon_de_livraison SET date = :date_bl WHERE id_livraison = :id_bl");
    query.bindValue(":id_bl", id_bl);
    query.bindValue(":date_bl", nouvelle_date.toString("dd-MM-yyyy")); // Format adapté pour SQLite

    if (query.exec()) {
        CustomMessageBox::information(this, "Succès", "Date modifiée avec succès.");
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de la modification : " + query.lastError().text());
    }
}


void App::afficherVente(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, prix_total, date_vente, num_bon_livraison FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           );
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableVente->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableVente->insertRow(row);
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        double prix_total = queryAffichage.value(4).toDouble();
        QString date_vente = queryAffichage.value(5).toString();
        QString num_bon_livraison = queryAffichage.value(6).toString();

        ui->tableVente->setItem(row, 0, new QTableWidgetItem(id_vente));
        ui->tableVente->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableVente->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableVente->setItem(row, 4, new QTableWidgetItem(QString::number(prix_total)+" MGA"));
        ui->tableVente->setItem(row, 5, new QTableWidgetItem(date_vente));
        ui->tableVente->setItem(row, 6, new QTableWidgetItem(num_bon_livraison));

        row++;

    }

}

void App::afficherProduit(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        msgBox.showError("","Erreur lors de l'ouverture de la base de données");
        return;
    }
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT p.id_produit, p.abreviation, p.nom, p.prix_unitaire, p.prix_detail, p.prix_remise, f.nom, p.unite, p.categorie FROM produits p "
                  "INNER JOIN fournisseur f ON p.fournisseur_id = f.id_fournisseur");
    if(!query.exec()){
        msgBox.showError("","Erreur lors de la récupération des données");
        qDebug()<<query.lastError();
        return;
    }
    ui->tableStock->setRowCount(0);
    ui->tableStock->blockSignals(true);

    int row = 0;
    while(query.next()){
        ui->tableStock->insertRow(row);
        QString id_produits = query.value(0).toString();
        QString abreviation = query.value(1).toString();
        QString nom = query.value(2).toString();
        QString fournisseur = query.value(3).toString();
        QString prix_unitaire = query.value(4).toString();
        QString prix_detail = query.value(5).toString();
        QString prix_remise = query.value(6).toString();
        QString unite = query.value(7).toString();
        QString categorie = query.value(8).toString();
        //int quantite = query.value(7).toInt();

        ui->tableStock->setItem(row, 0, new QTableWidgetItem(id_produits));
        ui->tableStock->setItem(row, 1, new QTableWidgetItem(abreviation));
        ui->tableStock->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableStock->setItem(row, 3, new QTableWidgetItem(fournisseur));
        ui->tableStock->setItem(row, 4, new QTableWidgetItem(prix_unitaire));
        ui->tableStock->setItem(row, 5, new QTableWidgetItem(prix_detail));
        ui->tableStock->setItem(row, 6, new QTableWidgetItem(prix_remise));
        ui->tableStock->setItem(row, 7, new QTableWidgetItem(unite));
        ui->tableStock->setItem(row, 8, new QTableWidgetItem(categorie));
        //ui->tableStock->setItem(row, 7, new QTableWidgetItem(QString::number(quantite)));

        row++;
    }

    qDebug ()<<"Affiche du tableau";
    ui->tableStock->blockSignals(false);
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
        case 4: colonne = "prix_detail"; break;
        case 5: colonne = "prix_remise"; break;
        case 6: colonne = "fournisseur_id"; break;
        case 8: colonne = "unite"; break;
        case 9: colonne = "categorie"; break;
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

void App::supprimerVente(){
    int row = ui->tableVente->currentRow();
    if (row < 0) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une ligne à supprimer.");
        return;
    }

    // Récupérer l'ID du produit (colonne 0)
    QTableWidgetItem *celluleId = ui->tableVente->item(row, 0);
    if (!celluleId) {
        CustomMessageBox().showError("Erreur", "Impossible de trouver l'ID du vente.");
        return;
    }

    QString idVente = celluleId->text();

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
        query.prepare("DELETE FROM ligne_vente WHERE id_vente = :id");
        query.bindValue(":id", idVente);

        if (!query.exec()) {
            qDebug() << "Erreur lors de la suppression de la base de données :" << query.lastError();
            CustomMessageBox().showError("Erreur", "Échec de la suppression de la base de données.");
            return;
        }

        // Supprimer la ligne du tableau
        ui->tableVente->removeRow(row);

        // Afficher un message de succès
        CustomMessageBox().showInformation("Succès", "L'élément a été supprimé avec succès.");
        afficherVente();
        chiffreDaffaire();
    }

}

void App::chiffreDaffaire(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT SUM(prix_total) FROM ligne_vente");
    if(!query.exec()){
        qDebug()<<query.lastError();
        msgBox.showError("Erreur", "Erreur de calcul "+query.lastError().text());
        return;
    }
    if (!query.exec()) {
        qDebug() << query.lastError();
        msgBox.showError("Erreur", "Erreur de calcul: " + query.lastError().text());
        return;
    }

    if (query.next()) {  // Vérifie si la requête a renvoyé une ligne
        QString somme = query.value(0).toString();  // Récupère la somme
        ui->labelCA->setText(somme + " MGA");  // Affiche la somme avec "Ar"
    } else {
        // Cas où il n'y a pas de résultats (s'il n'y a pas de montants ou de lignes correspondantes)
        msgBox.showError("Information", "Aucun montant trouvé.");
        ui->labelCA->setText("0 MGA");
    }
}

void App::imprimerVente(){
    // Selection de plusieurs lignes
    QModelIndexList selection = ui->tableVente->selectionModel()->selectedRows();
    if(selection.isEmpty()){
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une ou plusieurs lignes à imprimer.");
        return;
    }
    //Récupérer les lignes sélectionnées
    QStringList listeIdVente;
    foreach(const QModelIndex &index, selection){
        listeIdVente.append(ui->tableVente->item(index.row(), 0)->text());
    }
    //Récuperer le nom du client, le nom du produit, la quantité, le prix unitaire, le prix total et la date de vente dans le tableau
    QString contenuVente;
    contenuVente = QString("<!DOCTYPE html>"
                           "<html>"
                           "<head>"
                           "<meta charset='UTF-8'>"
                           "<style>"
                           "table {"
                           "  width: 100%;"
                           "  border-collapse: collapse;"
                           "  margin-top: 20px; "
                           "} "
                           "th, td {"
                           "  border: 1px solid black; "
                           "  padding: 10px; "
                           "  text-align: center; "
                           "} "
                           "th {"
                           "  background-color: #f2f2f2; "
                           "  font-weight: bold; "
                           "} "
                           "</style>"
                           "</head>"
                           "<body>"
                           "<p><strong>RAHENINTSOA ALASORA</strong></p>"
                           "<p><strong>Nif : </strong></p>"
                           "<p><strong>STAT : </strong></p>"
                           "<p><strong>Adresse : </strong>ALASORA Commune en Face de Sopromer</p>"
                           "<p><strong>Contact : </strong>0347636886</p>"
                           "<div style='text-align: center; font-size: 20px;'>"
                           "<h2>FACTURE</h2>"
                           "</div>"
                           "<table>"
                           "<thead>"
                           "<tr>"
                           "<th>Client</th>"
                           "<th>Produit</th>"
                           "<th>Quantité</th>"
                           "<th>Prix Unitaire (MGA)</th>"
                           "<th>Prix Total (MGA)</th>"
                           "<th>Date de Vente</th>"
                           "</tr>"
                           "</thead>"
                           "<tbody>");
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    QSqlQuery query(sqlitedb);
    // Préparer la requête UNE SEULE FOIS avant la boucle
    query.prepare("SELECT c.nom, p.nom, quantite, p.prix_unitaire, prix_total, date_vente FROM ligne_vente l "
                  "INNER JOIN produits p ON id_produit = produit_id "
                  "INNER JOIN clients c ON id_client = client_id "
                  "WHERE id_vente = :id");

    for (const QString &idVente : listeIdVente) {
        query.bindValue(":id", idVente);

        if (!query.exec()) {
            qDebug() << "Erreur SQL:" << query.lastError();
            return;
        }

        while (query.next()) {
            QString nomClient = query.value(0).toString();
            QString nomProduit = query.value(1).toString();
            int quantite = query.value(2).toInt();
            double prixUnitaire = query.value(3).toDouble();
            double prixTotal = query.value(4).toDouble();
            QString dateVente = query.value(5).toString();

            contenuVente += QString("<tr>"
                                    "<td>%1</td>"
                                    "<td>%2</td>"
                                    "<td>%3</td>"
                                    "<td>%4 MGA</td>"
                                    "<td>%5 MGA</td>"
                                    "<td>%6</td>"
                                    "</tr>"
                                    )
                                .arg(nomClient)
                                .arg(nomProduit)
                                .arg(quantite)
                                .arg(prixUnitaire)
                                .arg(prixTotal)
                                .arg(dateVente);
        }
    }

    // Ajouter le reste du HTML après la boucle
    contenuVente += "</tbody>"
                    "</table>"
                    "</body>"
                    "</html>";

    QTextDocument document;
    document.setHtml(contenuVente);

    // Aperçu avant impression
    QPrinter previewPrinter(QPrinter::PrinterResolution);
    previewPrinter.setOutputFormat(QPrinter::PdfFormat);
    previewPrinter.setOutputFileName("facture.pdf");

    QPrintPreviewDialog previewDialog(&previewPrinter, this);
    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, [&document](QPrinter *printer) {
        document.print(printer);
    });
    previewDialog.exec();

    // Impression réelle
    QPrinter printPrinter(QPrinter::HighResolution);
    QPrintDialog printDialog(&printPrinter, this);

    if (printDialog.exec() == QDialog::Accepted) {
        document.print(&printPrinter);
    }
}

void App::rechercheBl(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEditRechercheBl->text();
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_livraison, c.nom, p.nom, quantite, date, statut, prix_total_a_payer FROM bon_de_livraison b "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id WHERE p.nom LIKE :recherche");
    queryAffichage.bindValue(":recherche", "%"+recherche+"%");
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableBonDeLivraison->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableBonDeLivraison->insertRow(row);
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString nom_produit = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        QString statut = queryAffichage.value(5).toString();
        double prix_total = queryAffichage.value(6).toDouble();

        ui->tableBonDeLivraison->setItem(row, 0, new QTableWidgetItem(id_livraison));
        ui->tableBonDeLivraison->setItem(row, 1, new QTableWidgetItem(nom_client));
        ui->tableBonDeLivraison->setItem(row, 2, new QTableWidgetItem(nom_produit));
        ui->tableBonDeLivraison->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableBonDeLivraison->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableBonDeLivraison->setItem(row, 5, new QTableWidgetItem(statut));
        ui->tableBonDeLivraison->setItem(row, 6, new QTableWidgetItem(QString::number(prix_total)));

        row++;
    }
}

void App::recherche(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEditRecherche->text();
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, prix_total, date_vente FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id WHERE p.nom LIKE :recherche");
    queryAffichage.bindValue(":recherche", "%"+recherche+"%");
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableVente->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableVente->insertRow(row);
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        double prix_total = queryAffichage.value(4).toDouble();
        QString date_vente = queryAffichage.value(5).toString();

        ui->tableVente->setItem(row, 0, new QTableWidgetItem(id_vente));
        ui->tableVente->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableVente->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableVente->setItem(row, 4, new QTableWidgetItem(QString::number(prix_total)+" MGA"));
        ui->tableVente->setItem(row, 5, new QTableWidgetItem(date_vente));
        row++;

    }
    //afficher dans labelCA le prix total des ventes filtrées
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT SUM(prix_total) FROM ligne_vente l "
                  "INNER JOIN produits p ON id_produit = produit_id "
                  "INNER JOIN clients c ON id_client = client_id WHERE p.nom LIKE :recherche");
    query.bindValue(":recherche", "%"+recherche+"%");
    if(!query.exec()){
        qDebug()<<query.lastError();
        return;
    }
    if(query.next()){
        QString somme = query.value(0).toString();
        ui->labelCA->setText(somme+" MGA");
    }
}

void App::filtrageDate(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString dateDebut = ui->dateDebut->date().toString("dd-MM-yyyy");
    QString dateFin = ui->dateFin->date().toString("dd-MM-yyyy");
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, prix_total, date_vente, num_bon_livraison FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE date_vente BETWEEN :dateDebut AND :dateFin");
    queryAffichage.bindValue(":dateDebut", dateDebut);
    queryAffichage.bindValue(":dateFin", dateFin);
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableVente->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableVente->insertRow(row);
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        double prix_total = queryAffichage.value(4).toDouble();
        QString date_vente = queryAffichage.value(5).toString();
        QString bl_id = queryAffichage.value(6).toString();

        ui->tableVente->setItem(row, 0, new QTableWidgetItem(id_vente));
        ui->tableVente->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableVente->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableVente->setItem(row, 4, new QTableWidgetItem(QString::number(prix_total)+" MGA"));
        ui->tableVente->setItem(row, 5, new QTableWidgetItem(date_vente));
        ui->tableVente->setItem(row, 6, new QTableWidgetItem(bl_id));
        row++;

    }
    //afficher dans labelCA le prix total des ventes filtrées
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT SUM(prix_total) FROM ligne_vente WHERE date_vente BETWEEN :dateDebut AND :dateFin");
    query.bindValue(":dateDebut", dateDebut);
    query.bindValue(":dateFin", dateFin);
    if(!query.exec()){
        qDebug()<<query.lastError();
        return;
    }
    if(query.next()){
        QString somme = query.value(0).toString();
        ui->labelCA->setText(somme+" MGA");
    }
}

void App::filtrageDateBonDeLivraison(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString dateDebutBl = ui->dateDebutBl->date().toString("dd-MM-yyyy");
    QString dateFinBl = ui->dateFinBl->date().toString("dd-MM-yyyy");
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_livraison, c.nom, p.nom, quantite, date, statut, prix_total_a_payer FROM bon_de_livraison b "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE date BETWEEN :dateDebutBl AND :dateFinBl");
    queryAffichage.bindValue(":dateDebutBl", dateDebutBl);
    queryAffichage.bindValue(":dateFinBl", dateFinBl);
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<queryAffichage.lastError();
        return;
    }
    ui->tableBonDeLivraison->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableBonDeLivraison->insertRow(row);
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString nom_produit = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        QString statut = queryAffichage.value(5).toString();
        double prix_total = queryAffichage.value(6).toDouble();

        ui->tableBonDeLivraison->setItem(row, 0, new QTableWidgetItem(id_livraison));
        ui->tableBonDeLivraison->setItem(row, 1, new QTableWidgetItem(nom_client));
        ui->tableBonDeLivraison->setItem(row, 2, new QTableWidgetItem(nom_produit));
        ui->tableBonDeLivraison->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableBonDeLivraison->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableBonDeLivraison->setItem(row, 5, new QTableWidgetItem(statut));
        ui->tableBonDeLivraison->setItem(row, 6, new QTableWidgetItem(QString::number(prix_total)));

        row++;
    }
}

void App::reinitialiserAffichage(){
    QDate dateActuelle = QDate::currentDate();
    ui->dateDebut->setDate(dateActuelle);  // Mettre la date de début au jour actuel
    ui->dateFin->setDate(dateActuelle);
    afficherVente();
    chiffreDaffaire();
}

void App::reinitialiserBl(){
    QDate dateActuelle = QDate::currentDate();
    ui->dateDebutBl->setDate(dateActuelle);  // Mettre la date de début au jour actuel
    ui->dateFinBl->setDate(dateActuelle);
    afficherBonDeLivraison();
}

void App::etatStock(){
    EtatStock *stock = new EtatStock(nullptr);
    stock->show();
}

void App::mouvementStock(){
    MouvementDeStock *stock = new MouvementDeStock(nullptr);
    stock->show();
}

void App::affichageFinance(){
    FinanceCompta *compta = new FinanceCompta(nullptr);
    compta->show();
}

void App::affichageTiers(){
    Tiers *tiers = new Tiers(nullptr);
    tiers->show();
}

App::~App()
{
    delete ui;
}
