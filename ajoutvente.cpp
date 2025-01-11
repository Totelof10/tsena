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
}


void AjoutVente::enleverPanier(){
    CustomMessageBox msgBox;
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item) {
        CustomMessageBox msgBox;
        msgBox.showWarning("Erreur", "Veuillez sélectionner un produit à supprimer !");
        return;
    }

    // Récupérer le texte de l'élément (exemple : "Client: Dupont | Produit: Stylo | Quantité: 2 | Prix Total: 20.00 €")
    QString texteItem = item->text();

    // Extraire le prix total de la ligne (on suppose que c'est la dernière donnée après ": ")
    QStringList parts = texteItem.split("Prix Total: ");
    if (parts.size() == 2) {
        QString prixStr = parts.last().replace("€", "").trimmed();
        double prix = prixStr.toDouble();

        // Soustraire le prix du total cumulatif
        totalCumulatif -= prix;

        // Mettre à jour le champ Total
        ui->lineEditTotal->setText(QString::number(totalCumulatif, 'f', 2));
    }

    // Supprimer l'élément de la liste
    delete item;
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

    // Supprimer tous les éléments
    ui->listWidget->clear();
    msgBox.showInformation("Succès", "Tous les éléments ont été supprimés de la liste !");
}
void AjoutVente::ajouterNouvelleVente(){
    emit ajouterVente();
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
}





AjoutVente::~AjoutVente()
{
    delete ui;
}
