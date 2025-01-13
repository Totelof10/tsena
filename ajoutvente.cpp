#include "ajoutvente.h"
#include "ui_ajoutvente.h"
#include "databasemanager.h"
#include "custommessagebox.h"

AjoutVente::AjoutVente(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AjoutVente)
{
    ui->setupUi(this);
    afficherInformation();
    connect(ui->btnAdd, &QPushButton::clicked, this, &AjoutVente::ajouterPanier);
    connect(ui->btnDelete, &QPushButton::clicked, this, &AjoutVente::enleverPanier);
    connect(ui->btnVider, &QPushButton::clicked, this, &AjoutVente::viderPanier);
    connect(ui->comboProduit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AjoutVente::mettreAJourPrix);
    connect(ui->spinBoxQuantite, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AjoutVente::mettreAJourTotal);
    connect(ui->comboProduit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AjoutVente::remettreAZero);
    connect(ui->btnValider, &QPushButton::clicked, this, &AjoutVente::ajouterNouvelleVente);

}

void AjoutVente::afficherInformation(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    // Récupération des produits
    QSqlQuery queryProduits(sqlitedb);
    queryProduits.prepare("SELECT id_produit, nom, prix_unitaire FROM produits");
    if (!queryProduits.exec()) {
        qDebug() << "Erreur lors de la récupération des données du produit" << queryProduits.lastError();
        return;
    }

    ui->comboProduit->clear();
    ui->comboProduit->addItem("");
    while (queryProduits.next()) {
        int id = queryProduits.value(0).toInt();
        QString nom = queryProduits.value(1).toString();
        double prix_unitaire = queryProduits.value(2).toDouble();
        ui->comboProduit->addItem(nom, id);
        mapPrixProduits[id] = prix_unitaire;
    }

    // Récupération des clients
    QSqlQuery queryClients(sqlitedb);
    queryClients.prepare("SELECT id_client, nom FROM clients");
    if (!queryClients.exec()) {
        qDebug() << "Erreur lors de la récupération des données du client" << queryClients.lastError();
        return;
    }

    ui->comboClient->clear();
    ui->comboClient->addItem("");
    while (queryClients.next()) {
        int id = queryClients.value(0).toInt();
        QString nom = queryClients.value(1).toString();
        ui->comboClient->addItem(nom, id);
    }
};

void AjoutVente::ajouterPanier() {
    CustomMessageBox msgBox;
    QString client = ui->comboClient->currentText();
    QString produit = ui->comboProduit->currentText();
    int quantite = ui->spinBoxQuantite->value();

    // Récupérer l'ID du produit actuellement sélectionné dans le QComboBox
    int idProduit = ui->comboProduit->currentData().toInt();

    // Vérifier que les champs ne sont pas vides ou invalides
    if (client.isEmpty() || produit.isEmpty() || quantite <= 0) {
        msgBox.showWarning("Erreur", "Veuillez remplir tous les champs avant d'ajouter !");
        return;
    }

    // Récupérer le prix unitaire du produit
    double prixUnitaire = 0.0;
    if (mapPrixProduits.contains(idProduit)) {
        prixUnitaire = mapPrixProduits[idProduit];
    }

    // Calculer le total pour ce produit
    double totalProduit = prixUnitaire * quantite;

    // Ajouter les données au listWidget
    QString ligne = QString("Client: %1 | Produit: %2 | Quantité: %3 | Prix Total: %4 MGA")
                        .arg(client)
                        .arg(produit)
                        .arg(quantite)
                        .arg(QString::number(totalProduit, 'f', 2));
    ui->listWidget->addItem(ligne);
    // Mettre à jour le champ total du panier
    ui->lineEditTotal->setText(QString::number(totalProduit, 'f', 2)); // Total avec 2 décimales
    double prixTotalTotal = 0.0;
    for(int i = 0; i< ui->listWidget->count(); i++){
        QString ligne = ui->listWidget->item(i)->text();
        QStringList elements = ligne.split("|");
        if(elements.size()==4){
            double prixTotal = elements[3].remove("Prix Total: ").remove("MGA").trimmed().toDouble();
            prixTotalTotal += prixTotal;
            ui->labelTotal->setText(QString::number(prixTotalTotal)+" MGA");
        }
    }
}


void AjoutVente::enleverPanier() {
    CustomMessageBox msgBox;
    QListWidgetItem *item = ui->listWidget->currentItem();

    if (!item) {
        msgBox.showWarning("Erreur", "Veuillez sélectionner un produit à supprimer !");
        return;
    }

    delete item;

    // Recalculer le total global après suppression de l'élément
    double prixTotalTotal = 0.0;
    for(int i = 0; i < ui->listWidget->count(); i++) {
        QString ligne = ui->listWidget->item(i)->text();
        QStringList elements = ligne.split("|");
        if (elements.size() == 4) {
            double prixTotal = elements[3].remove("Prix Total: ").remove("MGA").trimmed().toDouble();
            prixTotalTotal += prixTotal;
        }
    }

    // Mettre à jour le label total global
    ui->labelTotal->setText(QString::number(prixTotalTotal) + " MGA");
}


void AjoutVente::mettreAJourPrix(int index) {
    // Récupérer l'identifiant du produit à partir de l'élément sélectionné
    int idProduit = ui->comboProduit->itemData(index).toInt();

    // Vérifier si l'identifiant existe dans le QMap
    if (mapPrixProduits.contains(idProduit)) {
        // Mettre à jour le champ de prix
        double prix = mapPrixProduits[idProduit];
        ui->lineEditPrix->setText(QString::number(prix, 'f', 2)); // Affichage formaté avec 2 décimales
    } else {
        // Si aucun produit n'est sélectionné, effacer le champ de prix
        ui->lineEditPrix->clear();
    }
}


void AjoutVente::viderPanier(){
    CustomMessageBox msgBox;
    if (ui->listWidget->count() == 0) {
        QMessageBox::information(this, "Information", "La liste est déjà vide !");
        return;
    }
    clearForm();
    msgBox.showInformation("Succès", "Tous les éléments ont été supprimés de la liste !");
}


void AjoutVente::remettreAZero(int index)
{
    Q_UNUSED(index);

    // Réinitialiser le QSpinBox de quantité à 0
    ui->spinBoxQuantite->setValue(0);

    // Réinitialiser le champ Total à 0.00
    ui->lineEditTotal->setText("0.00");
}

void AjoutVente::mettreAJourTotal()
{
    // Récupérer la quantité depuis le QSpinBox
    int quantite = ui->spinBoxQuantite->value();

    // Récupérer l'ID du produit actuellement sélectionné dans le QComboBox
    int idProduit = ui->comboProduit->currentData().toInt();

    // Vérifier si un produit est sélectionné et si la quantité est valide
    if (idProduit == 0 || quantite <= 0) {
        ui->lineEditTotal->setText("0.00"); // Réinitialiser à zéro si aucun produit n'est sélectionné ou quantité non valide
        return;
    }

    // Récupérer le prix unitaire correspondant à l'ID
    double prixUnitaire = mapPrixProduits.value(idProduit, 0.0);

    // Calculer le total pour le produit sélectionné
    double totalProduit = prixUnitaire * quantite;

    // Mettre à jour le champ Total avec le total du produit sélectionné
    ui->lineEditTotal->setText(QString::number(totalProduit, 'f', 2)); // Affichage avec 2 décimales
}



void AjoutVente::clearForm(){
    ui->comboClient->setCurrentIndex(0);
    ui->comboProduit->setCurrentIndex(0);
    ui->spinBoxQuantite->clear();
    ui->lineEditPrix->clear();
    ui->lineEditTotal->clear();
    ui->listWidget->clear();
    ui->labelTotal->setText("0");
    ui->lineEditTotal->clear();
}


void AjoutVente::ajouterNouvelleVente(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.open()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
    }
    for(int i = 0; i< ui->listWidget->count(); i++){
        QListWidgetItem *item = ui->listWidget->item(i);
        QString ligne = item->text();

        QStringList elements = ligne.split("|");

        if (elements.size() == 4) {
            // Extraire les valeurs sans les préfixes pour Client et Produit
            QString nomClient = elements[0].remove("Client: ").trimmed();

            QString nomProduit = elements[1].remove("Produit: ").trimmed();

            // Nettoyer la quantité et le prix total
            QString quantiteStr = elements[2].remove("Quantité: ").trimmed();
            int quantite = quantiteStr.toInt();  // Convertir en entier

            QString prixTotalStr = elements[3].remove("Prix Total: ").remove("MGA").trimmed();

            double prixTotal = prixTotalStr.toDouble();  // Convertir en flottant

            // Créer les requêtes pour rechercher les ID
            QSqlQuery queryProduit(sqlitedb);
            QSqlQuery queryClient(sqlitedb);

            // Rechercher l'ID du client
            queryClient.prepare("SELECT id_client FROM clients WHERE nom = ?");
            queryClient.addBindValue(nomClient);
            if (!queryClient.exec() || !queryClient.next()) {
                qDebug() << "Erreur lors de la recherche du client : " << queryClient.lastError().text();
                continue; // Passer à la ligne suivante si le client n'est pas trouvé
            }
            int clientId = queryClient.value(0).toInt();

            // Rechercher l'ID du produit
            queryProduit.prepare("SELECT id_produit FROM produits WHERE nom = ?");
            queryProduit.addBindValue(nomProduit);
            if (!queryProduit.exec() || !queryProduit.next()) {
                qDebug() << "Erreur lors de la recherche du produit : " << queryProduit.lastError().text();
                continue; // Passer à la ligne suivante si le produit n'est pas trouvé
            }

            int produitId = queryProduit.value(0).toInt();

            // Créer la requête d'insertion dans la table ligne_vente
            QSqlQuery queryInsertion(sqlitedb);
            queryInsertion.prepare("INSERT INTO ligne_vente (produit_id, client_id, quantite, prix_total, date_vente) "
                                   "VALUES (?, ?, ?, ?, ?)");
            queryInsertion.addBindValue(produitId);
            queryInsertion.addBindValue(clientId);
            queryInsertion.addBindValue(quantite);
            queryInsertion.addBindValue(prixTotal);
            queryInsertion.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));  // Date de la vente

            // Exécuter l'insertion
            if (!queryInsertion.exec()) {
                qDebug() << "Erreur lors de l'insertion dans la table ligne_vente : " << queryInsertion.lastError().text();
            }
        } else {
            qDebug() << "La ligne ne contient pas 4 éléments valides.";
        }

    }
    CustomMessageBox msgBox;
    msgBox.showInformation("", "Vente effectuée");
    emit ajouterVente();
    clearForm();
}


AjoutVente::~AjoutVente()
{
    delete ui;
}
