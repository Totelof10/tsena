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
#include "historique.h"

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
    databasePath = "C:/db_test/test.db";
    QFileInfo fileInfo(databasePath);
    animatedNavbar = ui->animatedNavbar;
    QPushButton* venteButton = ui->btnPageVente;
    QPushButton* stockButton = ui->btnPageStock;
    QPushButton* bonButton = ui->btnPageBon;
    QPushButton* tiersButton = ui->btnTiers;
    if(animatedNavbar){
        animatedNavbar->setupButtons(venteButton, stockButton, bonButton, tiersButton);
    }
    ui->labelWorkspace->setText(fileInfo.fileName());
    ui->dateDebut->setDate(QDate::currentDate());
    ui->dateFin->setDate(QDate::currentDate());
    ui->dateDebutBl->setDate(QDate::currentDate());
    ui->dateFinBl->setDate(QDate::currentDate());
    attributionAcces();
    afficherProduit();
    afficherVente();
    chiffreDaffaire();
    afficherBonDeLivraison();
    ui->treeVente->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeBl->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableStock->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    connect(ui->btnSupprimerBonDeLivraison, &QPushButton::clicked, this, &App::supprimerBonDeLivraison);
    connect(ui->btnHistorique, &QPushButton::clicked, this, &App::afficherHistorique);
    connect(ui->btnMettreDansHistorique, &QPushButton::clicked, this, &App::mettreDansHistorique);
}

void App::tableDesOperations(){
    Operation *operation = new Operation();
    operation->show();
}

void App::afficherHistorique(){
    Historique *historique = new Historique();
    historique->show();
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
        //ui->btnModifierVente->setDisabled(true);
        ui->btnSupprimerVente->setDisabled(true);
    } else {
        ui->btnPageStock->setDisabled(false);
        //ui->btnPageFinance->setDisabled(false);
        //ui->btnModifierVente->setDisabled(false);
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

void App::afficherBonDeLivraison() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_livraison, c.nom, date, p.nom, quantite, statut, prix_total_a_payer, prix_vente FROM bon_de_livraison b "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "ORDER BY substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2) DESC");
    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeBl->clear(); // Vider l'ancien contenu
    ui->treeBl->setHeaderLabels({"Client", "Date", "Statut", "Produit", "Quantité", "Total à payer (MGA)", "Prix de vente (MGA)"}); // Entêtes principales

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Map pour stocker le total par client et date

    while (queryAffichage.next()) {
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_livraison = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        QString statut = queryAffichage.value(5).toString();
        double prix_total_a_payer = queryAffichage.value(6).toDouble();
        double prix_vente = queryAffichage.value(7).toDouble();

        QString key = nom_client + " | " + date_livraison; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeBl);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_livraison);
            clientItem->setText(2, ""); // Le statut sera mis à jour si nécessaire
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total à payer pour ce groupe
            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails du bon de livraison sous le client
        QTreeWidgetItem *blItem = new QTreeWidgetItem();
        blItem->setText(0, "➡ BL N°" + id_livraison);
        blItem->setText(3, produit);
        blItem->setText(4, QString::number(quantite));
        blItem->setText(2, statut);
        blItem->setText(5, QString::number(prix_total_a_payer) + " MGA");
        blItem->setText(6, QString::number(prix_vente) + " MGA");
        clientsMap[key]->addChild(blItem);

        // Mettre à jour le statut du parent si nécessaire
        clientsMap[key]->setText(2, statut); // Afficher le statut du dernier BL pour ce client/date

        // Accumuler le total pour ce client et cette date
        totalParClientDate[key] += prix_total_a_payer;
    }

    // Mettre à jour le total à payer pour chaque groupe parent
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        if (clientsMap.contains(it.key())) {
            clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Afficher le total dans la colonne "Total à payer" du parent
        }
    }

    //ui->treeBl->expandAll(); // Ne pas afficher les détails au départ
    ui->treeBl->collapseAll();
}
void App::supprimerBonDeLivraison() {
    // Récupérer l'élément sélectionné
    QList<QTreeWidgetItem*> selectedItems = ui->treeBl->selectedItems();
    if (selectedItems.isEmpty()) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner un bon de livraison à supprimer.");
        return;
    }

    QTreeWidgetItem* selectedItem = selectedItems.first();

    // Vérifier si l'élément sélectionné est un enfant (détail du bon de livraison)
    if (!selectedItem->parent()) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner un détail de bon de livraison à supprimer.");
        return;
    }

    QString itemText = selectedItem->text(0);
    if (!itemText.startsWith("➡ BL N°")) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner un détail de bon de livraison valide.");
        return;
    }

    QString idLivraisonStr = itemText.mid(QString("➡ BL N°").length());
    int idLivraison = idLivraisonStr.toInt();

    // Message de confirmation
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Êtes-vous sûr de vouloir supprimer ce bon de livraison ?");
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

        // Supprimer l'élément du treeView
        QTreeWidgetItem* parent = selectedItem->parent();
        if (parent) {
            parent->removeChild(selectedItem);
            // Si le parent n'a plus d'enfants, vous pouvez choisir de le supprimer aussi (optionnel)
            // if (parent->childCount() == 0) {
            //     QTreeWidgetItem* grandParent = parent->parent();
            //     if (grandParent) {
            //         grandParent->removeChild(parent);
            //     } else {
            //         ui->treeBl->takeTopLevelItem(ui->treeBl->indexOfTopLevelItem(parent));
            //     }
            // }
        }

        // Afficher un message de succès
        CustomMessageBox().showInformation("Succès", "Le bon de livraison a été supprimé avec succès.");
    }
}

void App::livrePaye() {
    QList<QTreeWidgetItem*> selectedItems = ui->treeBl->selectedItems();
    if (selectedItems.isEmpty()) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner au moins un bon de livraison ou un groupe à modifier.");
        return;
    }

    QSet<QTreeWidgetItem*> itemsToProcess;
    for (QTreeWidgetItem* item : selectedItems) {
        if (item->parent()) {
            itemsToProcess.insert(item); // Ajouter les enfants sélectionnés
        } else {
            // Si un parent est sélectionné, ajouter tous ses enfants à la liste de traitement
            for (int i = 0; i < item->childCount(); ++i) {
                itemsToProcess.insert(item->child(i));
            }
        }
    }

    if (itemsToProcess.isEmpty()) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner au moins un bon de livraison à marquer comme payé.");
        return;
    }

    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText(itemsToProcess.size() > 1 ?
                       "Souhaitez-vous valider ces " + QString::number(itemsToProcess.size()) + " bons de livraison comme 'Payés' ?" :
                       "Souhaitez-vous valider ce bon de livraison comme 'Payé' ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret != QMessageBox::Yes) {
        return;
    }

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.transaction()) {
        qDebug() << "Erreur lors du début de la transaction :" << sqlitedb.lastError();
        CustomMessageBox().showError("Erreur", "Impossible de démarrer la transaction.");
        return;
    }

    for (QTreeWidgetItem* blItem : itemsToProcess) {
        QString itemText = blItem->text(0);
        if (!itemText.startsWith("➡ BL N°")) {
            continue; // Ignorer si l'élément sélectionné n'est pas un détail de bon de livraison
        }

        QString idLivraisonStr = itemText.mid(QString("➡ BL N°").length());
        int idLivraison = idLivraisonStr.toInt();

        QString nomProduit = blItem->text(3); // Index corrigé pour "Produit"
        int quantite = blItem->text(4).toInt(); // Index corrigé pour "Quantité"
        QString statutActuel = blItem->text(2); // Index corrigé pour "Statut"
        QString prixTotalStr = blItem->text(5); // Index corrigé pour "Total à payer (MGA)"
        prixTotalStr.chop(QString(" MGA").length());
        double prixTotal = prixTotalStr.toDouble();
        QString prixVenteStr = blItem->text(6); // Index corrigé pour "Prix de vente (MGA)"
        prixVenteStr.chop(QString(" MGA").length());
        double prixVente = prixVenteStr.toDouble();

        if (statutActuel == "Payé") {
            continue; // Ignore les lignes déjà payées
        }

        // Récupérer la date de livraison et le nom du client du parent
        QTreeWidgetItem* parentItem = blItem->parent();
        if (!parentItem) continue;
        QString dateLivraison = parentItem->text(1);
        QString nomClient = parentItem->text(0);

        // Récupération de l'ID du produit
        QSqlQuery queryProduitId(sqlitedb);
        queryProduitId.prepare("SELECT produit_id, p.prix_base FROM bon_de_livraison "
                               "INNER JOIN produits p ON id_produit = produit_id "
                               "WHERE id_livraison = :id");
        queryProduitId.bindValue(":id", idLivraison);
        if (!queryProduitId.exec() || !queryProduitId.next()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Impossible de récupérer l'ID du produit pour le BL N°" + QString::number(idLivraison));
            return;
        }
        int idProduit = queryProduitId.value(0).toInt();
        double prix_base = queryProduitId.value(1).toDouble();

        // Vérification du statut (redondant mais pour la sécurité)
        QSqlQuery queryStatut(sqlitedb);
        queryStatut.prepare("SELECT statut FROM bon_de_livraison WHERE id_livraison = :id");
        queryStatut.bindValue(":id", idLivraison);
        if (!queryStatut.exec() || !queryStatut.next()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Impossible de vérifier le statut du BL N°" + QString::number(idLivraison));
            return;
        }
        if (queryStatut.value(0).toString() == "Payé") {
            continue; // Ignore si déjà payé
        }

        // Mise à jour du statut du bon de livraison
        QSqlQuery queryUpdateBl(sqlitedb);
        queryUpdateBl.prepare("UPDATE bon_de_livraison SET statut = 'Payé' WHERE id_livraison = :id");
        queryUpdateBl.bindValue(":id", idLivraison);
        if (!queryUpdateBl.exec()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Échec de la mise à jour du statut du BL N°" + QString::number(idLivraison));
            return;
        }

        // Gestion du stock
        QSqlQuery queryStock(sqlitedb);
        queryStock.prepare("SELECT id_stock, quantite FROM stock WHERE produit_id = :id");
        queryStock.bindValue(":id", idProduit);
        if (!queryStock.exec() || !queryStock.next()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Impossible de récupérer les informations de stock pour le produit.");
            return;
        }
        int idStock = queryStock.value(0).toInt();
        int quantiteStock = queryStock.value(1).toInt();

        if (quantite > quantiteStock) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Stock insuffisant pour le produit '" + nomProduit + "'.");
            return;
        }

        QSqlQuery queryUpdateStock(sqlitedb);
        queryUpdateStock.prepare("UPDATE stock SET quantite = quantite - :quantite WHERE produit_id = :id");
        queryUpdateStock.bindValue(":quantite", quantite);
        queryUpdateStock.bindValue(":id", idProduit);
        if (!queryUpdateStock.exec()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Échec de la mise à jour du stock pour le produit '" + nomProduit + "'.");
            return;
        }

        // Enregistrement du mouvement de stock
        QSqlQuery queryMouvement(sqlitedb);
        queryMouvement.prepare("INSERT INTO mouvements_de_stock (stock_id, nom, quantite, type_mouvement, date_mouvement, vente) "
                               "VALUES (:stock_id, :nom, :quantite, 'Sortie Livraison', :date_mouvement, :vente)");
        queryMouvement.bindValue(":stock_id", idStock);
        queryMouvement.bindValue(":nom", nomProduit);
        queryMouvement.bindValue(":quantite", quantite);
        queryMouvement.bindValue(":date_mouvement", dateLivraison);
        queryMouvement.bindValue(":vente", QString("Livraison du %1 pour %2").arg(dateLivraison).arg(nomClient));
        if (!queryMouvement.exec()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Échec de l'enregistrement des mouvements de stock pour le produit '" + nomProduit + "'.");
            return;
        }

        // Enregistrement de la vente
        QSqlQuery queryInsertion(sqlitedb);
        queryInsertion.prepare("INSERT INTO ligne_vente (client_id, produit_id, quantite, date_vente, prix_total, num_bon_livraison, prix_vente, benefice) "
                               "SELECT client_id, produit_id, :quantite, :date, :prix_total, id_livraison, :prix_vente, :benefice FROM bon_de_livraison WHERE id_livraison = :id");
        queryInsertion.bindValue(":id", idLivraison);
        queryInsertion.bindValue(":quantite", quantite);
        queryInsertion.bindValue(":date", dateLivraison);
        queryInsertion.bindValue(":prix_total", prixTotal);
        queryInsertion.bindValue(":prix_vente", prixVente);
        queryInsertion.bindValue(":benefice", (prixVente - prix_base)*quantite);
        if (!queryInsertion.exec()) {
            sqlitedb.rollback();
            CustomMessageBox().showError("Erreur", "Échec de l'enregistrement de la vente pour le BL N°" + QString::number(idLivraison));
            return;
        }

        // Enregistrement de l'opération
        QSqlQuery queryOperation(sqlitedb);
        queryOperation.prepare("INSERT INTO operation (mouvement_id, stock_id, nom_produit, stock_depart, quantite_entree, quantite_sortie, stock_actuel, date_operation) "
                               "VALUES (:mouvement_id, :stock_id, :nom_produit, :stock_depart, 0, :quantite_sortie, :stock_actuel, :date_operation)");
        queryOperation.bindValue(":mouvement_id", queryMouvement.lastInsertId());
        queryOperation.bindValue(":stock_id", idStock);
        queryOperation.bindValue(":nom_produit", nomProduit);
        queryOperation.bindValue(":stock_depart", quantiteStock);
        queryOperation.bindValue(":quantite_sortie", quantite);
        queryOperation.bindValue(":stock_actuel", quantiteStock - quantite);
        queryOperation.bindValue(":date_operation", dateLivraison);
        queryOperation.exec();

        // Mise à jour de l'interface
        blItem->setText(3, "Payé");
    }

    if (!sqlitedb.commit()) {
        sqlitedb.rollback();
        CustomMessageBox().showError("Erreur", "Impossible de valider la transaction.");
        return;
    }

    CustomMessageBox().showInformation("Succès", "Les bons de livraison sélectionnés ont été marqués comme payés.");
    afficherVente();
    chiffreDaffaire();
    afficherBonDeLivraison();
}



void App::reporterDateBl() {
    QList<QTreeWidgetItem*> selectedItems = ui->treeBl->selectedItems();
    if (selectedItems.isEmpty()) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un bon de livraison à modifier.");
        return;
    }

    QTreeWidgetItem* selectedItem = selectedItems.first(); // Prendre le premier élément sélectionné

    // Vérifier si l'élément sélectionné est un enfant (détail du bon de livraison)
    if (!selectedItem->parent()) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un détail de bon de livraison à modifier.");
        return;
    }

    QString itemText = selectedItem->text(0);
    if (!itemText.startsWith("➡ BL N°")) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un détail de bon de livraison valide.");
        return;
    }

    QString id_bl_str = itemText.mid(QString("➡ BL N°").length());
    int id_bl = id_bl_str.toInt();

    QTreeWidgetItem* parentItem = selectedItem->parent();
    QString date_bl_str = parentItem->text(1);
    QDate dateActuelle = QDate::fromString(date_bl_str, "dd-MM-yyyy"); // Assumer le format de date dans l'arbre

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
    QString nouvelle_date_str_display = nouvelle_date.toString("dd-MM-yyyy"); // Format d'affichage dans l'arbre

    // 📌 Mettre à jour la date dans l'élément parent du treeView
    if (parentItem) {
        parentItem->setText(1, nouvelle_date_str_display);
    }

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


void App::afficherVente() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare(
        "SELECT id_vente, c.nom, date_vente, p.nom, quantite, (prix_total / quantite) AS prix_unitaire, prix_total "
        "FROM ligne_vente l "
        "INNER JOIN produits p ON id_produit = produit_id "
        "INNER JOIN clients c ON id_client = client_id "
        "ORDER BY substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2) DESC"
        );


    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeVente->clear(); // Vider l'ancien contenu
    ui->treeVente->setHeaderLabels({"Client", "Date", "Quantite" ,"Prix unitaire", "Prix total", "Total(MGA)"}); // Entêtes principales

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Stocker les totaux par client et date

    while (queryAffichage.next()) {
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_vente = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        double prix_unitaire = queryAffichage.value(5).toDouble();
        double prix_total = queryAffichage.value(6).toDouble();

        QString key = nom_client + " | " + date_vente; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeVente);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_vente);
            clientItem->setText(2, ""); // Quantité pour le parent (laisser vide ou mettre un indicateur)
            clientItem->setText(3, ""); // Prix unitaire pour le parent
            clientItem->setText(4, ""); // Prix total pour le parent
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total

            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails de l'achat sous le client
        QTreeWidgetItem *achatItem = new QTreeWidgetItem();
        achatItem->setText(0, "➡ " + id_vente);
        achatItem->setText(1, produit);
        achatItem->setText(2, QString::number(quantite));
        achatItem->setText(3, QString::number(prix_unitaire) + " MGA");
        achatItem->setText(4, QString::number(prix_total) + " MGA");
        clientsMap[key]->addChild(achatItem);

        // Mise à jour du total d'achat pour ce client/date
        totalParClientDate[key] += prix_total;
    }

    // Mettre à jour les totaux dans l'affichage principal
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Utiliser l'index correct (5)
    }

    ui->treeVente->collapseAll(); // Ne pas afficher les détails au départ
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

void App::supprimerVente() {
    QTreeWidgetItem *selectedItem = ui->treeVente->currentItem();
    if (!selectedItem) {
        CustomMessageBox().showError("Erreur", "Veuillez sélectionner une vente à supprimer.");
        return;
    }

    QString idToDelete;
    QTreeWidgetItem *itemToRemove = selectedItem;
    QTreeWidgetItem *parentItem = selectedItem->parent();

    // Récupérer l'ID de l'élément sélectionné (colonne 0)
    QString selectedItemId = selectedItem->text(0);

    if (parentItem) {
        // Cas où un enfant (détail de la vente) est sélectionné
        if (selectedItemId.startsWith("➡ ")) {
            idToDelete = selectedItemId.mid(2); // Extraire l'ID après "➡ "
        } else {
            CustomMessageBox().showError("Erreur", "Impossible de trouver l'ID de la vente à supprimer.");
            return;
        }
    } else {
        // Cas où l'élément parent (client et date) est sélectionné - Gérer si nécessaire
        CustomMessageBox().showWarning("Avertissement", "La suppression d'une vente groupée n'est pas encore entièrement prise en charge. Veuillez sélectionner un détail de vente à supprimer.");
        return;
        // Si vous souhaitez supprimer toute la vente (parent et enfants), vous devrez récupérer tous les id_vente des enfants
        // et les supprimer un par un, puis supprimer le parent.
    }

    // Demande de confirmation
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
        query.bindValue(":id", idToDelete);

        if (!query.exec()) {
            qDebug() << "Erreur lors de la suppression de la base de données :" << query.lastError();
            CustomMessageBox().showError("Erreur", "Échec de la suppression de la base de données.");
            return;
        }

        // Supprimer l'élément de l'arbre
        if (parentItem) {
            parentItem->removeChild(itemToRemove);

            // Mise à jour du total du parent
            double newTotal = 0.0;
            for (int i = 0; i < parentItem->childCount(); ++i) {
                QString prixTotalStr = parentItem->child(i)->text(4);
                prixTotalStr.chop(4); // Supprimer " MGA"
                newTotal += prixTotalStr.toDouble();
            }
            parentItem->setText(3, QString::number(newTotal) + " MGA");

            delete itemToRemove; // Libérer la mémoire après suppression
        } else {
            // Si pour une raison quelconque un élément de premier niveau est sélectionné et que l'ID est valide
            // (ce cas devrait être géré par l'avertissement ci-dessus), vous pouvez le supprimer ici.
            int index = ui->treeVente->indexOfTopLevelItem(itemToRemove);
            if (index != -1) {
                ui->treeVente->takeTopLevelItem(index);
                delete itemToRemove;
            }
        }

        // Afficher un message de succès
        CustomMessageBox().showInformation("Succès", "L'élément a été supprimé avec succès.");
        chiffreDaffaire(); // Mettre à jour le chiffre d'affaires
    }
}


void App::chiffreDaffaire(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT SUM(prix_total) FROM ligne_vente");
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

}

void App::rechercheBl() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEditRechercheBl->text().trimmed(); // Récupérer le texte de recherche et supprimer les espaces inutiles
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_livraison, c.nom, date, p.nom, quantite, statut, prix_total_a_payer, prix_vente FROM bon_de_livraison b "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE p.nom LIKE :recherche OR c.nom LIKE :recherche OR statut LIKE :recherche OR date LIKE :recherche ORDER BY date DESC"); // Ajouter la recherche par date et trier par date
    queryAffichage.bindValue(":recherche", "%" + recherche + "%");

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeBl->clear(); // Vider l'ancien contenu
    ui->treeBl->setHeaderLabels({"Client", "Date", "Statut", "Produit", "Quantité", "Total à payer (MGA)", "Prix de vente (MGA)"}); // Définir tous les en-têtes ici

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Map pour stocker le total par client et date

    while (queryAffichage.next()) {
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_livraison = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        QString statut = queryAffichage.value(5).toString();
        double prix_total_a_payer = queryAffichage.value(6).toDouble();
        double prix_vente = queryAffichage.value(7).toDouble();

        QString key = nom_client + " | " + date_livraison; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeBl);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_livraison);
            clientItem->setText(2, ""); // Le statut sera mis à jour si nécessaire
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total à payer pour ce groupe
            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails du bon de livraison sous le client
        QTreeWidgetItem *blItem = new QTreeWidgetItem();
        blItem->setText(0, "➡ BL N°" + id_livraison);
        blItem->setText(3, produit);
        blItem->setText(4, QString::number(quantite));
        blItem->setText(2, statut);
        blItem->setText(5, QString::number(prix_total_a_payer) + " MGA");
        blItem->setText(6, QString::number(prix_vente) + " MGA");
        clientsMap[key]->addChild(blItem);

        // Mettre à jour le statut du parent si nécessaire
        clientsMap[key]->setText(2, statut); // Afficher le statut du dernier BL pour ce client/date

        // Accumuler le total pour ce client et cette date
        totalParClientDate[key] += prix_total_a_payer;
    }

    // Mettre à jour le total à payer pour chaque groupe parent
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        if (clientsMap.contains(it.key())) {
            clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Afficher le total dans la colonne "Total à payer" du parent
        }
    }

    ui->treeBl->expandAll(); // Afficher les détails par défaut après la recherche
}

void App::recherche() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEditRecherche->text().trimmed(); // Récupérer le texte de recherche et supprimer les espaces inutiles
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, c.nom, date_vente, p.nom, quantite, (prix_total / quantite) AS prix_unitaire, prix_total "
                           "FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE p.nom LIKE :recherche OR p.categorie LIKE :recherche OR c.nom LIKE :recherche OR date_vente LIKE :recherche"); // Ajouter la recherche par date
    queryAffichage.bindValue(":recherche", "%" + recherche + "%");

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeVente->clear(); // Vider l'ancien contenu
    ui->treeVente->setHeaderLabels({"Client", "Date", "Quantite" ,"Prix unitaire", "Prix total", "Total(MGA)"}); // Entêtes principales

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Stocker les totaux par client et date

    while (queryAffichage.next()) {
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_vente = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        double prix_unitaire = queryAffichage.value(5).toDouble();
        double prix_total = queryAffichage.value(6).toDouble();

        QString key = nom_client + " | " + date_vente; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeVente);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_vente);
            clientItem->setText(2, ""); // Quantité pour le parent (laisser vide ou mettre un indicateur)
            clientItem->setText(3, ""); // Prix unitaire pour le parent
            clientItem->setText(4, ""); // Prix total pour le parent
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total

            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails de l'achat sous le client
        QTreeWidgetItem *achatItem = new QTreeWidgetItem();
        achatItem->setText(0, "➡ " + id_vente);
        achatItem->setText(1, produit);
        achatItem->setText(2, QString::number(quantite));
        achatItem->setText(3, QString::number(prix_unitaire) + " MGA");
        achatItem->setText(4, QString::number(prix_total) + " MGA");
        clientsMap[key]->addChild(achatItem);

        // Mise à jour du total d'achat pour ce client/date
        totalParClientDate[key] += prix_total;
    }

    // Mettre à jour les totaux dans l'affichage principal
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Utiliser l'index correct (5)
    }

    ui->treeVente->expandAll(); // Afficher les détails par défaut après la recherche

    // Calculer et afficher le chiffre d'affaires des ventes filtrées
    QSqlQuery queryCA(sqlitedb);
    queryCA.prepare("SELECT SUM(prix_total) FROM ligne_vente l "
                    "INNER JOIN produits p ON id_produit = produit_id "
                    "INNER JOIN clients c ON id_client = client_id "
                    "WHERE p.nom LIKE :recherche OR p.categorie LIKE :recherche OR c.nom LIKE :recherche OR date_vente LIKE :recherche");
    queryCA.bindValue(":recherche", "%" + recherche + "%");
    if (!queryCA.exec()) {
        qDebug() << "Erreur lors du calcul du CA filtré" << queryCA.lastError();
        return;
    }
    if (queryCA.next()) {
        QString somme = queryCA.value(0).toString();
        ui->labelCA->setText(somme + " MGA");
    } else {
        ui->labelCA->setText("0.0 MGA"); // Si aucune vente filtrée
    }
}

void App::filtrageDate() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }
    QString dateDebut = ui->dateDebut->date().toString("dd-MM-yyyy"); // Format de date compatible avec SQLite
    QString dateFin = ui->dateFin->date().toString("dd-MM-yyyy");   // Format de date compatible avec SQLite
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, c.nom, date_vente, p.nom, quantite, (prix_total / quantite) AS prix_unitaire, prix_total "
                           "FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                           "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                           "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))"); // Utiliser la fonction date() pour la comparaison
    queryAffichage.bindValue(":dateDebut", dateDebut);
    queryAffichage.bindValue(":dateFin", dateFin);

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeVente->clear(); // Vider l'ancien contenu
    ui->treeVente->setHeaderLabels({"Client", "Date", "Quantite" ,"Prix unitaire", "Prix total", "Total(MGA)"}); // Entêtes principales

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Stocker les totaux par client et date

    while (queryAffichage.next()) {
        QString id_vente = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_vente = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        double prix_unitaire = queryAffichage.value(5).toDouble();
        double prix_total = queryAffichage.value(6).toDouble();

        QString key = nom_client + " | " + date_vente; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeVente);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_vente);
            clientItem->setText(2, ""); // Quantité pour le parent (laisser vide ou mettre un indicateur)
            clientItem->setText(3, ""); // Prix unitaire pour le parent
            clientItem->setText(4, ""); // Prix total pour le parent
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total

            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails de l'achat sous le client
        QTreeWidgetItem *achatItem = new QTreeWidgetItem();
        achatItem->setText(0, "➡ " + id_vente);
        achatItem->setText(1, produit);
        achatItem->setText(2, QString::number(quantite));
        achatItem->setText(3, QString::number(prix_unitaire) + " MGA");
        achatItem->setText(4, QString::number(prix_total) + " MGA");
        clientsMap[key]->addChild(achatItem);

        // Mise à jour du total d'achat pour ce client/date
        totalParClientDate[key] += prix_total;
    }

    // Mettre à jour les totaux dans l'affichage principal
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Utiliser l'index correct (5)
    }
    ui->treeVente->expandAll(); // Afficher les détails par défaut après le filtrage

    // Calculer et afficher le chiffre d'affaires des ventes filtrées par date
    QSqlQuery queryCA(sqlitedb);
    queryCA.prepare("SELECT SUM(prix_total) FROM ligne_vente "
                    "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                    "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                    "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
    queryCA.bindValue(":dateDebut", dateDebut);
    queryCA.bindValue(":dateFin", dateFin);
    if (!queryCA.exec()) {
        qDebug() << "Erreur lors du calcul du CA filtré par date" << queryCA.lastError();
        return;
    }
    if (queryCA.next()) {
        QString somme = queryCA.value(0).toString();
        ui->labelCA->setText(somme + " MGA");
    } else {
        ui->labelCA->setText("0.0 MGA"); // Si aucune vente filtrée par date
    }
}

void App::filtrageDateBonDeLivraison() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }
    QString dateDebutBl = ui->dateDebutBl->date().toString("dd-MM-yyyy");  // Garde le format dd-MM-yyyy
    QString dateFinBl = ui->dateFinBl->date().toString("dd-MM-yyyy");      // Garde le format dd-MM-yyyy

    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare(
        "SELECT id_livraison, c.nom, date, p.nom, quantite, statut, prix_total_a_payer, prix_vente "
        "FROM bon_de_livraison b "
        "INNER JOIN produits p ON id_produit = produit_id "
        "INNER JOIN clients c ON id_client = client_id "
        "WHERE (substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2)) "
        "BETWEEN (substr(:dateDebutBl, 7, 4) || '-' || substr(:dateDebutBl, 4, 2) || '-' || substr(:dateDebutBl, 1, 2)) "
        "AND (substr(:dateFinBl, 7, 4) || '-' || substr(:dateFinBl, 4, 2) || '-' || substr(:dateFinBl, 1, 2))"
        );

    queryAffichage.bindValue(":dateDebutBl", dateDebutBl);
    queryAffichage.bindValue(":dateFinBl", dateFinBl);

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    ui->treeBl->clear(); // Vider l'ancien contenu
    ui->treeBl->setHeaderLabels({"Client", "Date", "Statut", "Produit", "Quantité", "Total à payer (MGA)", "Prix de vente (MGA)"}); // Définir tous les en-têtes ici

    QMap<QString, QTreeWidgetItem*> clientsMap; // Map pour stocker les clients/dates
    QMap<QString, double> totalParClientDate; // Map pour stocker le total par client et date

    while (queryAffichage.next()) {
        QString id_livraison = queryAffichage.value(0).toString();
        QString nom_client = queryAffichage.value(1).toString();
        QString date_livraison = queryAffichage.value(2).toString();
        QString produit = queryAffichage.value(3).toString();
        int quantite = queryAffichage.value(4).toInt();
        QString statut = queryAffichage.value(5).toString();
        double prix_total_a_payer = queryAffichage.value(6).toDouble();
        double prix_vente = queryAffichage.value(7).toDouble();

        QString key = nom_client + " | " + date_livraison; // Clé unique (client + date)

        // Vérifier si le client/date a déjà une entrée
        if (!clientsMap.contains(key)) {
            QTreeWidgetItem *clientItem = new QTreeWidgetItem(ui->treeBl);
            clientItem->setText(0, nom_client);
            clientItem->setText(1, date_livraison);
            clientItem->setText(2, ""); // Le statut sera mis à jour si nécessaire
            clientItem->setText(5, "0.0 MGA"); // Initialiser le total à payer pour ce groupe
            clientsMap[key] = clientItem;
            totalParClientDate[key] = 0.0;
        }

        // Ajouter les détails du bon de livraison sous le client
        QTreeWidgetItem *blItem = new QTreeWidgetItem();
        blItem->setText(0, "➡ BL N°" + id_livraison);
        blItem->setText(3, produit);
        blItem->setText(4, QString::number(quantite));
        blItem->setText(2, statut);
        blItem->setText(5, QString::number(prix_total_a_payer) + " MGA");
        blItem->setText(6, QString::number(prix_vente) + " MGA");
        clientsMap[key]->addChild(blItem);

        // Mettre à jour le statut du parent si nécessaire
        clientsMap[key]->setText(2, statut); // Afficher le statut du dernier BL pour ce client/date

        // Accumuler le total pour ce client et cette date
        totalParClientDate[key] += prix_total_a_payer;
    }

    // Mettre à jour le total à payer pour chaque groupe parent
    for (auto it = totalParClientDate.begin(); it != totalParClientDate.end(); ++it) {
        if (clientsMap.contains(it.key())) {
            clientsMap[it.key()]->setText(5, QString::number(it.value()) + " MGA"); // Afficher le total dans la colonne "Total à payer" du parent
        }
    }

    ui->treeBl->expandAll(); // Afficher les détails par défaut après le filtrage
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
void App::mettreDansHistorique(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    CustomMessageBox msgBox;
    msgBox.setWindowTitle("Confirmation");
    msgBox.setText("Voulez-vous vraiment mettre les ventes et charges dans l'historique? ATTENTION : Cette action "
                   "est irréversible");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes){
        QSqlQuery query(sqlitedb);
        query.prepare("SELECT id_vente, p.nom, date_vente, quantite, prix_total, benefice from ligne_vente l "
                      "INNER JOIN produits p ON id_produit = produit_id");
        if(!query.exec()){
            qDebug()<<"Erreur lors de la récupération des données pour l'ajout dans historique"
                        "!"<<query.lastError();
            return;
        }
        while(query.next()){
            QString  nom = query.value(1).toString();
            QString date = query.value(2).toString();
            int quantite = query.value(3).toInt();
            double prix_total = query.value(4).toDouble();
            double benefice = query.value(5).toDouble();

            QSqlQuery queryInsertion(sqlitedb);
            queryInsertion.prepare("INSERT INTO historique(date_vente, nom_produit, quantite, ca, benefice) "
                                   "VALUES(:date, :nom, :quantite, :prix_total, :benefice)");
            queryInsertion.bindValue(":date", date);
            queryInsertion.bindValue(":nom", nom);
            queryInsertion.bindValue(":quantite", quantite);
            queryInsertion.bindValue(":prix_total", prix_total);
            queryInsertion.bindValue(":benefice", benefice);
            if(!queryInsertion.exec()){
                qDebug()<<"Erreur lors de l'insertion de l'historique"<<queryInsertion.lastError();
                return;
            }
        }
        QSqlQuery queryDelete(sqlitedb);
        //supprimer tous les lignes dans ligne_vente
        queryDelete.prepare("DELETE FROM ligne_vente");
        if(!queryDelete.exec()){
            qDebug()<<"Erreur lors de la suppression des ventes"<<queryDelete.lastError();
            return;
        }
        QSqlQuery queryHistoriqueCharge(sqlitedb);
        queryHistoriqueCharge.prepare("SELECT description, personne, valeur, date FROM charge");
        if(!queryHistoriqueCharge.exec()){
            qDebug()<<"Erreur lors de récupération des charges pour l'historique"<<queryHistoriqueCharge.lastError();
            return;
        }
        while(queryHistoriqueCharge.next()){
            QString description = queryHistoriqueCharge.value(0).toString();
            QString personne = queryHistoriqueCharge.value(1).toString();
            double valeur = queryHistoriqueCharge.value(2).toDouble();
            QString date = queryHistoriqueCharge.value(3).toString();

            QSqlQuery queryInsertionCharge(sqlitedb);
            queryInsertionCharge.prepare("INSERT INTO historique_charge(description, personne, valeur, date) "
                                         "VALUES (:description, :personne, :valeur, :date)");
            queryInsertionCharge.bindValue(":description", description);
            queryInsertionCharge.bindValue(":personne", personne);
            queryInsertionCharge.bindValue(":valeur", valeur);
            queryInsertionCharge.bindValue(":date", date);
            if(!queryInsertionCharge.exec()){
                qDebug()<<"Erreur lors de l'insertion des données dans historique_charge"<<queryInsertionCharge.lastError();
            }
        }
        QSqlQuery deleteCharge(sqlitedb);
        deleteCharge.prepare("DELETE FROM charge");
        if(!deleteCharge.exec()){
            qDebug()<<"Erreur lors de la suppression des charges"<<deleteCharge.lastError();
            return;
        }
        CustomMessageBox msgBox;
        msgBox.showInformation("Succes", "Ajout des historiques réussi");
        afficherVente();
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
