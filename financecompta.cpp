#include "financecompta.h"
#include "ui_financecompta.h"
#include "databasemanager.h"
//#include "custommessagebox.h"

FinanceCompta::FinanceCompta(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FinanceCompta)
{
    ui->setupUi(this);
    calcul();
    ui->dateDebut->setDate(QDate::currentDate());
    ui->dateFin->setDate(QDate::currentDate());
    ui->tableCompta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableCharge->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    affichageDesVentes();
    affichageDesCharges();
    connect(ui->btnAjouterCharge, &QPushButton::clicked, this, &FinanceCompta::ajouterUneCharge);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &FinanceCompta::recherche);
    connect(ui->btnFiltrer, &QPushButton::clicked, this, &FinanceCompta::filtrerParDate);
    connect(ui->btnReinitialiser, &QPushButton::clicked, this, &FinanceCompta::reinitialiser);
    setWindowModality(Qt::ApplicationModal);
}

void FinanceCompta::affichageDesVentes(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, prix_vente "
                           "FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "ORDER BY substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2) DESC");
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
        double prix_vente = queryAffichage.value(6).toDouble();
        double benefice = (prix_vente-prix_base)*quantite;
        totalBenefices += benefice;

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base)+" MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_vente)+ " MGA"));
        ui->tableCompta->setItem(row, 7, new QTableWidgetItem(QString::number(benefice)+ " MGA"));

        row++;
    }
    ui->labelBenefice->setText(QString::number(totalBenefices) + " MGA");
}

void FinanceCompta::recherche(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEdit->text();
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, prix_vente FROM ligne_vente l "
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
        double prix_vente = queryAffichage.value(6).toDouble();
        double benefice = (prix_vente-prix_base)*quantite;
        totalBenefices += benefice;

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base)+" MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_vente)+ " MGA"));
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
    queryAffichage.prepare("SELECT id_vente, p.nom, c.nom, quantite, date_vente, p.prix_base, prix_vente, benefice FROM ligne_vente l "
                           "INNER JOIN produits p ON id_produit = produit_id "
                           "INNER JOIN clients c ON id_client = client_id "
                           "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                           "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                           "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
    queryAffichage.bindValue(":dateDebut", dateDebut);
    queryAffichage.bindValue(":dateFin", dateFin);

    if (!queryAffichage.exec()) {
        qDebug() << "Erreur lors de la récupération des données" << queryAffichage.lastError();
        return;
    }

    // Réinitialiser le tableau
    ui->tableCompta->setRowCount(0);
    int row = 0;
    // Remplir les données du tableau
    while(queryAffichage.next()){
        ui->tableCompta->insertRow(row);
        QString identifiant = queryAffichage.value(0).toString();
        QString nom_produit = queryAffichage.value(1).toString();
        QString nom_client = queryAffichage.value(2).toString();
        int quantite = queryAffichage.value(3).toInt();
        QString date = queryAffichage.value(4).toString();
        double prix_base = queryAffichage.value(5).toDouble();
        double prix_vente = queryAffichage.value(6).toDouble();
        double benefice = queryAffichage.value(7).toDouble();

        ui->tableCompta->setItem(row, 0, new QTableWidgetItem(identifiant));
        ui->tableCompta->setItem(row, 1, new QTableWidgetItem(nom_produit));
        ui->tableCompta->setItem(row, 2, new QTableWidgetItem(nom_client));
        ui->tableCompta->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableCompta->setItem(row, 4, new QTableWidgetItem(date));
        ui->tableCompta->setItem(row, 5, new QTableWidgetItem(QString::number(prix_base)+" MGA"));
        ui->tableCompta->setItem(row, 6, new QTableWidgetItem(QString::number(prix_vente)+ " MGA"));
        ui->tableCompta->setItem(row, 7, new QTableWidgetItem(QString::number(benefice)+ " MGA"));

        row++;
    }
    QSqlQuery queryBenefice(sqlitedb);
    queryBenefice.prepare("SELECT SUM(benefice) FROM ligne_vente");
    if(!queryBenefice.exec()){
        qDebug()<<"Erreur lors de la somme des benefices"<<queryBenefice.lastError();
        return;
    }
    if(queryBenefice.next()){
        QString benefice = queryBenefice.value(0).toString();
        ui->labelBenefice->setText(benefice);
    }
}

void FinanceCompta::reinitialiser(){

        // Réinitialiser les dates à la date du jour
        QDate dateActuelle = QDate::currentDate();
        ui->dateDebut->setDate(dateActuelle);  // Mettre la date de début au jour actuel
        ui->dateFin->setDate(dateActuelle);    // Mettre la date de fin au jour actuel

        affichageDesVentes();
}

void FinanceCompta::ajouterUneCharge(){
    bool ok;
    QString description = QInputDialog::getText(this, "Ajouter la description de la charge", "Description:", QLineEdit::Normal, "", &ok);
    if(!ok || description.isEmpty())return;
    QString personne = QInputDialog::getText(this, "Ajouter le nom de la personne associé à la charge", "Nom:", QLineEdit::Normal, "", &ok);
    if(!ok || personne.isEmpty())return;
    double prix = QInputDialog::getDouble(this, "Ajouter la valeur de la charge", "Valeur:", 0, 0, 99999999, 2, &ok);
    if(!ok)return;

    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery query(sqlitedb);
    query.prepare("INSERT INTO charge(description, personne, valeur, date) "
                  "VALUES(:description, :personne, :valeur, :date)");
    query.bindValue(":description", description);
    query.bindValue(":personne", personne);
    query.bindValue(":valeur", prix);
    query.bindValue(":date", QDate::currentDate().toString("dd-MM-yyyy"));
    if(!query.exec()){
        qDebug()<<"Erreur lors de l'insertion de la charge"<<query.lastError();
        return;
    }
    affichageDesCharges();
    calcul();
}

void FinanceCompta::affichageDesCharges(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryAffichage(sqlitedb);
    queryAffichage.prepare("SELECT id_charge, description, personne, valeur, date FROM charge "
                           "ORDER BY substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2) DESC");
    if(!queryAffichage.exec()){
        qDebug()<<"Erreur lors de la récupération des charges"<<queryAffichage.lastError();
        return;
    }
    ui->tableCharge->setRowCount(0);
    int row = 0;
    while(queryAffichage.next()){
        ui->tableCharge->insertRow(row);
        QString id_charge = queryAffichage.value(0).toString();
        QString description = queryAffichage.value(1).toString();
        QString personne = queryAffichage.value(2).toString();
        double valeur = queryAffichage.value(3).toDouble();
        QString date = queryAffichage.value(4).toString();

        ui->tableCharge->setItem(row, 0, new QTableWidgetItem(id_charge));
        ui->tableCharge->setItem(row, 1, new QTableWidgetItem(description));
        ui->tableCharge->setItem(row, 2, new QTableWidgetItem(personne));
        ui->tableCharge->setItem(row, 3, new QTableWidgetItem(QString::number(valeur)+ " MGA"));
        ui->tableCharge->setItem(row, 4, new QTableWidgetItem(date));

        row++;
    }
    QSqlQuery queryCharge(sqlitedb);
    queryCharge.prepare("SELECT SUM(valeur) FROM charge");
    if(!queryCharge.exec()){
        qDebug()<<"Erreur lors de la somme des charges"<<queryCharge.lastError();
        return;
    }
    if(queryCharge.next()){
        QString charge = queryCharge.value(0).toString();
        ui->label_charge->setText(charge);
    }
}

void FinanceCompta::calcul(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryCharge(sqlitedb);
    queryCharge.prepare("SELECT SUM(valeur) FROM charge");
    if(!queryCharge.exec() || !queryCharge.next()){
        qDebug()<<"Erreur lors du calcul de la somme des charges"<<queryCharge.lastError();
    }
    double charge = queryCharge.value(0).toDouble();
    QSqlQuery queryBenefice(sqlitedb);
    queryBenefice.prepare("SELECT SUM(benefice) FROM ligne_vente");
    if(!queryBenefice.exec() || !queryBenefice.next()){
        qDebug()<<"Erreur lors du calcul de la somme des bénéfices"<<queryBenefice.lastError();
        return;
    }
    double benefice = queryBenefice.value(0).toDouble();
    double beneficeCharge = benefice - charge;
    ui->labelBeneficeCharge->setText(QString::number(beneficeCharge)+ " MGA");
}

FinanceCompta::~FinanceCompta()
{
    delete ui;
}
