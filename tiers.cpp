#include "tiers.h"
#include "ui_tiers.h"
#include "databasemanager.h"
#include "custommessagebox.h"

Tiers::Tiers(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tiers)
{
    ui->setupUi(this);
    affichageClient();
    affichageFournisseur();
    comboAffichage();
    ui->tableClient->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableFournisseur->horizontalHeader()->setSectionResizeMode((QHeaderView::Stretch));
    connect(ui->comboFournisseur, &QComboBox::currentIndexChanged, this, &Tiers::filtrageFournisseur);
    connect(ui->comboClient, &QComboBox::currentIndexChanged, this, &Tiers::filtrageClient);
    connect(ui->btnModifierClient, &QPushButton::clicked, this, &Tiers::modifierClient);
    connect(ui->btnModifierFournisseur, &QPushButton::clicked, this, &Tiers::modifierFournisseur);
    connect(ui->btnSupprimerClient, &QPushButton::clicked, this, &Tiers::supprimerClient);
    connect(ui->btnSupprimerFournisseur, &QPushButton::clicked, this, &Tiers::supprimerFournisseur);
    connect(ui->btnAjoutClient, &QPushButton::clicked, this, &Tiers::ajouterClient);
    connect(ui->btnAjoutFournisseur, &QPushButton::clicked, this, &Tiers::ajouterFournisseur);
}

void Tiers::comboAffichage(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryFournisseur(sqlitedb);
    QSqlQuery queryClient(sqlitedb);
    queryFournisseur.prepare("SELECT id_fournisseur, nom FROM fournisseur");
    if(!queryFournisseur.exec()){
        qDebug()<<"Erreur lors de la récupération des fournisseurs"<<queryFournisseur.lastError();
        return;
    }
    ui->comboFournisseur->clear();
    ui->comboFournisseur->addItem("");
    while(queryFournisseur.next()){
        int id = queryFournisseur.value(0).toInt();
        QString nom = queryFournisseur.value(1).toString();
        ui->comboFournisseur->addItem(nom, id);
    }
    queryClient.prepare("SELECT id_client, nom FROM clients");
    if (!queryClient.exec()) {
        qDebug() << "Erreur lors de la récupération des données du client" << queryClient.lastError();
        return;
    }

    ui->comboClient->clear();
    ui->comboClient->addItem("");
    while (queryClient.next()) {
        int id = queryClient.value(0).toInt();
        QString nom = queryClient.value(1).toString();
        ui->comboClient->addItem(nom, id);
    }
}

void Tiers::affichageFournisseur(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichageFournisseur(sqlitedb);
    queryAffichageFournisseur.prepare("SELECT id_fournisseur, nom, coordonnees, lieu FROM fournisseur");
    if(!queryAffichageFournisseur.exec()){
        qDebug()<<"Erreur lors de la récupération de la base données"<<queryAffichageFournisseur.lastError();
        return;
    }
    ui->tableFournisseur->setRowCount(0);
    int row = 0;
    while(queryAffichageFournisseur.next()){
        ui->tableFournisseur->insertRow(row);
        QString id_fournisseur = queryAffichageFournisseur.value(0).toString();
        QString nom = queryAffichageFournisseur.value(1).toString();
        QString coordonnees = queryAffichageFournisseur.value(2).toString();
        QString lieu = queryAffichageFournisseur.value(3).toString();

        ui->tableFournisseur->setItem(row, 0, new QTableWidgetItem(id_fournisseur));
        ui->tableFournisseur->setItem(row, 1, new QTableWidgetItem(nom));
        ui->tableFournisseur->setItem(row, 2, new QTableWidgetItem(coordonnees));
        ui->tableFournisseur->setItem(row, 3, new QTableWidgetItem(lieu));

        row++;
    }
}

void Tiers::affichageClient(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichageClient(sqlitedb);
    queryAffichageClient.prepare("SELECT id_client, nom, coordonnees, lieu FROM clients");
    if(!queryAffichageClient.exec()){
        qDebug()<<"Erreur lors de la récupération de la base données"<<queryAffichageClient.lastError();
        return;
    }
    ui->tableClient->setRowCount(0);
    int row = 0;
    while(queryAffichageClient.next()){
        ui->tableClient->insertRow(row);
        QString id_fournisseur = queryAffichageClient.value(0).toString();
        QString nom = queryAffichageClient.value(1).toString();
        QString coordonnees = queryAffichageClient.value(2).toString();
        QString lieu = queryAffichageClient.value(3).toString();

        ui->tableClient->setItem(row, 0, new QTableWidgetItem(id_fournisseur));
        ui->tableClient->setItem(row, 1, new QTableWidgetItem(nom));
        ui->tableClient->setItem(row, 2, new QTableWidgetItem(coordonnees));
        ui->tableClient->setItem(row, 3, new QTableWidgetItem(lieu));

        row++;
    }
}

void Tiers::filtrageFournisseur() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    QString fournisseur = ui->comboFournisseur->currentText();
    QSqlQuery queryFournisseur(sqlitedb);

    // Gestion du cas "Tous"
    if (fournisseur == "") {
        queryFournisseur.prepare("SELECT id_fournisseur, nom, coordonnees, lieu FROM fournisseur");
    } else {
        queryFournisseur.prepare("SELECT id_fournisseur, nom, coordonnees, lieu FROM fournisseur WHERE nom = :nom_fournisseur");
        queryFournisseur.bindValue(":nom_fournisseur", fournisseur);
    }

    if (!queryFournisseur.exec()) {
        qDebug() << "Erreur lors de la récupération des données du fournisseur" << queryFournisseur.lastError();
        return;
    }

    // Mise à jour de la table
    ui->tableFournisseur->setRowCount(0);
    int row = 0;
    while (queryFournisseur.next()) {
        ui->tableFournisseur->insertRow(row);

        ui->tableFournisseur->setItem(row, 0, new QTableWidgetItem(queryFournisseur.value(0).toString()));
        ui->tableFournisseur->setItem(row, 1, new QTableWidgetItem(queryFournisseur.value(1).toString()));
        ui->tableFournisseur->setItem(row, 2, new QTableWidgetItem(queryFournisseur.value(2).toString()));
        ui->tableFournisseur->setItem(row, 3, new QTableWidgetItem(queryFournisseur.value(3).toString()));

        row++;
    }
}


void Tiers::filtrageClient() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    QString client = ui->comboClient->currentText();
    QSqlQuery queryClient(sqlitedb);

    // Gestion du cas ""
    if (client == "") {
        queryClient.prepare("SELECT id_client, nom, coordonnees, lieu FROM clients");
    } else {
        queryClient.prepare("SELECT id_client, nom, coordonnees, lieu FROM clients WHERE nom = :nom_client");
        queryClient.bindValue(":nom_client", client);
    }

    if (!queryClient.exec()) {
        qDebug() << "Erreur lors de la récupération des données du client" << queryClient.lastError();
        return;
    }

    // Mise à jour de la table
    ui->tableClient->setRowCount(0);
    int row = 0;
    while (queryClient.next()) {
        ui->tableClient->insertRow(row);

        ui->tableClient->setItem(row, 0, new QTableWidgetItem(queryClient.value(0).toString()));
        ui->tableClient->setItem(row, 1, new QTableWidgetItem(queryClient.value(1).toString()));
        ui->tableClient->setItem(row, 2, new QTableWidgetItem(queryClient.value(2).toString()));
        ui->tableClient->setItem(row, 3, new QTableWidgetItem(queryClient.value(3).toString()));

        row++;
    }
}

void Tiers::modifierClient() {
    int row = ui->tableClient->currentRow();
    if (row < 0) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un client à modifier.");
        return;
    }

    QString id_client = ui->tableClient->item(row, 0)->text();
    QString nom = ui->tableClient->item(row, 1)->text();
    QString coordonnees = ui->tableClient->item(row, 2)->text();
    QString lieu = ui->tableClient->item(row, 3)->text();

    bool ok;
    nom = QInputDialog::getText(this, "Modifier Client", "Nom:", QLineEdit::Normal, nom, &ok);
    if (!ok || nom.isEmpty()) return;

    coordonnees = QInputDialog::getText(this, "Modifier Client", "Coordonnées:", QLineEdit::Normal, coordonnees, &ok);
    if (!ok || coordonnees.isEmpty()) return;

    lieu = QInputDialog::getText(this, "Modifier Client", "Lieu:", QLineEdit::Normal, lieu, &ok);
    if (!ok || lieu.isEmpty()) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("UPDATE clients SET nom = :nom, coordonnees = :coordonnees, lieu = :lieu WHERE id_client = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":coordonnees", coordonnees);
    query.bindValue(":lieu", lieu);
    query.bindValue(":id", id_client);

    if (query.exec()) {
        ui->tableClient->item(row, 1)->setText(nom);
        ui->tableClient->item(row, 2)->setText(coordonnees);
        ui->tableClient->item(row, 3)->setText(lieu);
        CustomMessageBox::information(this, "Succès", "Client modifié avec succès.");
        comboAffichage();
        affichageClient();
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de la modification.");
        qDebug()<<query.lastError();
    }
}

void Tiers::supprimerClient() {
    int row = ui->tableClient->currentRow();
    if (row < 0) {
        CustomMessageBox::warning(this, "Suppression", "Veuillez sélectionner un client à supprimer.");
        return;
    }

    QString id_client = ui->tableClient->item(row, 0)->text();
    int reponse = QMessageBox::question(this, "Suppression", "Voulez-vous vraiment supprimer ce client ?");
    if (reponse != QMessageBox::Yes) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("DELETE FROM clients WHERE id_client = :id");
    query.bindValue(":id", id_client);

    if (query.exec()) {
        ui->tableClient->removeRow(row);
        CustomMessageBox::information(this, "Succès", "Client supprimé.");
        affichageClient();
        comboAffichage();
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de la suppression.");
    }
}

void Tiers::modifierFournisseur() {
    int row = ui->tableFournisseur->currentRow();
    if (row < 0) {
        CustomMessageBox::warning(this, "Modification", "Veuillez sélectionner un fournisseur à modifier.");
        return;
    }

    QString id_fournisseur = ui->tableFournisseur->item(row, 0)->text();
    QString nom = ui->tableFournisseur->item(row, 1)->text();
    QString coordonnees = ui->tableFournisseur->item(row, 2)->text();
    QString lieu = ui->tableFournisseur->item(row, 3)->text();

    bool ok;
    nom = QInputDialog::getText(this, "Modifier Fournisseur", "Nom:", QLineEdit::Normal, nom, &ok);
    if (!ok || nom.isEmpty()) return;

    coordonnees = QInputDialog::getText(this, "Modifier Fournisseur", "Coordonnées:", QLineEdit::Normal, coordonnees, &ok);
    if (!ok || coordonnees.isEmpty()) return;

    lieu = QInputDialog::getText(this, "Modifier Fournisseur", "Lieu:", QLineEdit::Normal, lieu, &ok);
    if (!ok || lieu.isEmpty()) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("UPDATE fournisseur SET nom = :nom, coordonnees = :coordonnees, lieu = :lieu WHERE id_fournisseur = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":coordonnees", coordonnees);
    query.bindValue(":lieu", lieu);
    query.bindValue(":id", id_fournisseur);

    if (query.exec()) {
        ui->tableFournisseur->item(row, 1)->setText(nom);
        ui->tableFournisseur->item(row, 2)->setText(coordonnees);
        ui->tableFournisseur->item(row, 3)->setText(lieu);
        CustomMessageBox::information(this, "Succès", "Fournisseur modifié avec succès.");
        comboAffichage();
        affichageFournisseur();
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de la modification.");
    }
}

void Tiers::supprimerFournisseur() {
    int row = ui->tableFournisseur->currentRow();
    if (row < 0) {
        CustomMessageBox::warning(this, "Suppression", "Veuillez sélectionner un fournisseur à supprimer.");
        return;
    }

    QString id_fournisseur = ui->tableFournisseur->item(row, 0)->text();
    int reponse = QMessageBox::question(this, "Suppression", "Voulez-vous vraiment supprimer ce fournisseur ?");
    if (reponse != QMessageBox::Yes) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("DELETE FROM fournisseur WHERE id_fournisseur = :id");
    query.bindValue(":id", id_fournisseur);

    if (query.exec()) {
        ui->tableFournisseur->removeRow(row);
        CustomMessageBox::information(this, "Succès", "Fournisseur supprimé.");
        affichageFournisseur();
        comboAffichage();
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de la suppression.");
    }
}

void Tiers::ajouterClient() {
    bool ok;

    QString nom = QInputDialog::getText(this, "Ajouter Client", "Nom:", QLineEdit::Normal, "", &ok);
    if (!ok || nom.isEmpty()) return;

    QString coordonnees = QInputDialog::getText(this, "Ajouter Client", "Coordonnées:", QLineEdit::Normal, "", &ok);
    if (!ok || coordonnees.isEmpty()) return;

    QString lieu = QInputDialog::getText(this, "Ajouter Client", "Lieu:", QLineEdit::Normal, "", &ok);
    if (!ok || lieu.isEmpty()) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("INSERT INTO clients (nom, coordonnees, lieu) VALUES (:nom, :coordonnees, :lieu)");
    query.bindValue(":nom", nom);
    query.bindValue(":coordonnees", coordonnees);
    query.bindValue(":lieu", lieu);

    if (query.exec()) {
        CustomMessageBox::information(this, "Succès", "Client ajouté avec succès.");
        affichageClient();
        comboAffichage();
        // Rafraîchir la table des clients si nécessaire
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de l'ajout du client.");
    }
}

void Tiers::ajouterFournisseur() {
    bool ok;

    QString nom = QInputDialog::getText(this, "Ajouter Fournisseur", "Nom:", QLineEdit::Normal, "", &ok);
    if (!ok || nom.isEmpty()) return;

    QString coordonnees = QInputDialog::getText(this, "Ajouter Fournisseur", "Coordonnées:", QLineEdit::Normal, "", &ok);
    if (!ok || coordonnees.isEmpty()) return;

    QString lieu = QInputDialog::getText(this, "Ajouter Fournisseur", "Lieu:", QLineEdit::Normal, "", &ok);
    if (!ok || lieu.isEmpty()) return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur ouverture BD" << sqlitedb.rollback();
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("INSERT INTO fournisseur (nom, coordonnees, lieu) VALUES (:nom, :coordonnees, :lieu)");
    query.bindValue(":nom", nom);
    query.bindValue(":coordonnees", coordonnees);
    query.bindValue(":lieu", lieu);

    if (query.exec()) {
        CustomMessageBox::information(this, "Succès", "Fournisseur ajouté avec succès.");
        affichageFournisseur();
        comboAffichage();
        // Rafraîchir la table des fournisseurs si nécessaire
    } else {
        CustomMessageBox::critical(this, "Erreur", "Échec de l'ajout du fournisseur.");
    }
}



Tiers::~Tiers()
{
    delete ui;
}
