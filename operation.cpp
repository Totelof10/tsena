#include "operation.h"
#include "ui_operation.h"
#include "databasemanager.h"
#include "custommessagebox.h"

Operation::Operation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Operation)
{
    ui->setupUi(this);
    nomProduitDansComboBox();
    ui->tableOperation->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWindowModality(Qt::ApplicationModal);
    // Connexion pour afficher les opérations filtrées
    connect(ui->comboProduit, &QComboBox::currentIndexChanged, this, &Operation::affichageOperationParFiltrageComboBox);

}

void Operation::nomProduitDansComboBox(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT nom FROM produits");
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<query.lastError();
        return;
    }
    while(query.next()){
        QString nom_produit = query.value(0).toString();
        ui->comboProduit->addItem(nom_produit);
    }
}

void Operation::affichageOperationParFiltrageComboBox(){
    CustomMessageBox messageBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString nomProduit = ui->comboProduit->currentText();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT o.id_operation, o.mouvement_id, o.nom_produit, o.stock_depart, o.quantite_entree, o.quantite_sortie, o.stock_actuel, o.date_operation FROM operation o "
                  "INNER JOIN stock s ON o.stock_id = s.id_stock "
                  "INNER JOIN produits p ON s.produit_id = p.id_produit "
                  "WHERE p.nom = :nom_produit");
    query.bindValue(":nom_produit", nomProduit);
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<query.lastError();
        messageBox.showError("Erreur", "Erreur lors de la récupération des données");
        return;
    }
    ui->tableOperation->setRowCount(0);
    int row = 0;
    while(query.next()){
        ui->tableOperation->insertRow(row);
        QString id_operation = query.value(0).toString();
        QString mouvement_id = query.value(1).toString();
        QString nom_produit = query.value(2).toString();
        int stock_depart = query.value(3).toInt();
        int quantite_entree = query.value(4).toInt();
        int quantite_sortie = query.value(5).toInt();
        int stock_actuel = query.value(6).toInt();
        QString date_operation = query.value(7).toString();

        ui->tableOperation->setItem(row, 0, new QTableWidgetItem(id_operation));
        ui->tableOperation->setItem(row, 1, new QTableWidgetItem(mouvement_id));
        ui->tableOperation->setItem(row, 2, new QTableWidgetItem(nom_produit));
        ui->tableOperation->setItem(row, 3, new QTableWidgetItem(QString::number(stock_depart)));
        ui->tableOperation->setItem(row, 4, new QTableWidgetItem("(+) "+QString::number(quantite_entree)));
        ui->tableOperation->setItem(row, 5, new QTableWidgetItem("(-) "+QString::number(quantite_sortie)));
        ui->tableOperation->setItem(row, 6, new QTableWidgetItem(QString::number(stock_actuel)));
        ui->tableOperation->setItem(row, 7, new QTableWidgetItem(date_operation));

        row++;
    }
}


Operation::~Operation()
{
    delete ui;
}
