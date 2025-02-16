#include "financecompta.h"
#include "ui_financecompta.h"
#include "databasemanager.h"
//#include "custommessagebox.h"

FinanceCompta::FinanceCompta(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FinanceCompta)
{
    ui->setupUi(this);
    ui->tableCompta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    affichageDesVentes();
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &FinanceCompta::recherche);
    connect(ui->btnFiltrer, &QPushButton::clicked, this, &FinanceCompta::filtrerParDate);
    connect(ui->btnReinitialiser, &QPushButton::clicked, this, &FinanceCompta::reinitialiser);
}

void FinanceCompta::affichageDesVentes(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
    }
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, p.prix_unitaire FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id");
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération de la base de données"<<queryAffichage.lastError();
    }
    ui->tableCompta->setRowCount(0);
    int row = 0;
    double totalBenefices = 0.0;
    while(queryAffichage.next()){
        ui->tableCompta->insertRow(row);
        QString identifiant = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        double prix_base = queryAffichage.value(5).toDouble();
        double prix_unitaire = queryAffichage.value(6).toDouble();
        double benefice = (prix_unitaire-prix_base)*quantite;
        totalBenefices += benefice;

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base)+" MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_unitaire)+ " MGA"));
        ui->tableCompta->setItem(row, 7, new QTableWidgetItem(QString::number(benefice)+ " MGA"));

        row++;
    }
    ui->labelBenefice->setText(QString::number(totalBenefices) + " MGA");
}

void FinanceCompta::recherche(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
    }
    QString recherche = ui->lineEdit->text();
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, p.prix_unitaire FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE p.nom LIKE :recherche");
    queryAffichage.bindValue(":recherche", "%"+recherche+"%");

    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération de la base de données"<<queryAffichage.lastError();
    }
    ui->tableCompta->setRowCount(0);
    int row = 0;
    double totalBenefices = 0.0;
    while(queryAffichage.next()){
        ui->tableCompta->insertRow(row);
        QString identifiant = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        double prix_base = queryAffichage.value(5).toDouble();
        double prix_unitaire = queryAffichage.value(6).toDouble();
        double benefice = (prix_unitaire-prix_base)*quantite;
        totalBenefices += benefice;

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base)+" MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_unitaire)+ " MGA"));
        ui->tableCompta->setItem(row, 7, new QTableWidgetItem(QString::number(benefice)+ " MGA"));

        row++;
    }
    ui->labelBenefice->setText(QString::number(totalBenefices) + " MGA");
}

void FinanceCompta::filtrerParDate() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    // Récupérer les dates à partir des QDateEdit
    QString dateDebut = ui->dateDebut->date().toString("dd-MM-yyyy");
    QString dateFin = ui->dateFin->date().toString("dd-MM-yyyy");

    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, p.prix_unitaire FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE date_vente BETWEEN :dateDebut AND :dateFin");
    queryAffichage.bindValue(":dateDebut", dateDebut);
    queryAffichage.bindValue(":dateFin", dateFin);

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    // Réinitialiser le tableau
    ui->tableCompta->setRowCount(0);
    int row = 0;
    double totalBenefices = 0.0;

    // Remplir les données du tableau
    while (queryAffichage.next()) {
        ui->tableCompta->insertRow(row);
        QString identifiant = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        double prix_base = queryAffichage.value(5).toDouble();
        double prix_unitaire = queryAffichage.value(6).toDouble();
        double benefice = (prix_unitaire - prix_base) * quantite;
        totalBenefices += benefice;

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base) + " MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_unitaire) + " MGA"));
        ui->tableCompta->setItem(row, 7, new QTableWidgetItem(QString::number(benefice) + " MGA"));

        row++;
    }

    // Mettre à jour le label des bénéfices totaux
    ui->labelBenefice->setText(QString::number(totalBenefices) + " MGA");
}

void FinanceCompta::reinitialiser(){

        // Réinitialiser les dates à la date du jour
        QDate dateActuelle = QDate::currentDate();
        ui->dateDebut->setDate(dateActuelle);  // Mettre la date de début au jour actuel
        ui->dateFin->setDate(dateActuelle);    // Mettre la date de fin au jour actuel

        affichageDesVentes();
}


FinanceCompta::~FinanceCompta()
{
    delete ui;
}
