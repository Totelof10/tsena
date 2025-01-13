#include "mouvementdestock.h"
#include "ui_mouvementdestock.h"
#include "databasemanager.h"

MouvementDeStock::MouvementDeStock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MouvementDeStock)
{
    ui->setupUi(this);
    ui->tableWidgetMouvement->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    afficherMouvement();
    connect(ui->lineEditRecherche, &QLineEdit::textChanged,this, &MouvementDeStock::recherche);
    connect(ui->comboMouvement, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MouvementDeStock::filtrageMouvement);

}

void MouvementDeStock::afficherMouvement(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base données"<<sqlitedb.rollback();
    }
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, vente FROM mouvements_de_stock");
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération de données"<<query.lastError();
    }
    ui->tableWidgetMouvement->setRowCount(0);
    int row = 0;
    while(query.next()){
        ui->tableWidgetMouvement->insertRow(row);
        QString id_mouvement = query.value(0).toString();
        QString stock_id = query.value(1).toString();
        QString nom = query.value(2).toString();
        int quantite = query.value(3).toInt();
        QString type_mouvement = query.value(4).toString();
        QString date_mouvement = query.value(5).toString();
        QString vente = query.value(6).toString();

        ui->tableWidgetMouvement->setItem(row, 0, new QTableWidgetItem(id_mouvement));
        ui->tableWidgetMouvement->setItem(row, 1, new QTableWidgetItem(stock_id));
        ui->tableWidgetMouvement->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableWidgetMouvement->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableWidgetMouvement->setItem(row, 4, new QTableWidgetItem(type_mouvement));
        ui->tableWidgetMouvement->setItem(row, 5, new QTableWidgetItem(date_mouvement));
        ui->tableWidgetMouvement->setItem(row, 6, new QTableWidgetItem(vente));

        row++;
    }
}

void MouvementDeStock::recherche(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de la récupération des données"<<sqlitedb.rollback();
    }
    QString recherche = ui->lineEditRecherche->text();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, commande_id FROM mouvements_de_stock WHERE nom LIKE :recherche");
    query.bindValue(":recherche", "%"+recherche+"%");
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération de données"<<query.lastError();
    }
    ui->tableWidgetMouvement->setRowCount(0);
    int row = 0;
    while(query.next()){
        ui->tableWidgetMouvement->insertRow(row);
        QString id_mouvement = query.value(0).toString();
        QString stock_id = query.value(1).toString();
        QString nom = query.value(2).toString();
        int quantite = query.value(3).toInt();
        QString type_mouvement = query.value(4).toString();
        QString date_mouvement = query.value(5).toString();
        QString commande_id = query.value(6).toString();

        ui->tableWidgetMouvement->setItem(row, 0, new QTableWidgetItem(id_mouvement));
        ui->tableWidgetMouvement->setItem(row, 1, new QTableWidgetItem(stock_id));
        ui->tableWidgetMouvement->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableWidgetMouvement->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableWidgetMouvement->setItem(row, 4, new QTableWidgetItem(type_mouvement));
        ui->tableWidgetMouvement->setItem(row, 5, new QTableWidgetItem(date_mouvement));
        ui->tableWidgetMouvement->setItem(row, 6, new QTableWidgetItem(commande_id));

        row++;

    }
}

void MouvementDeStock::filtrageMouvement(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    QString combo = ui->comboMouvement->currentText();
    QSqlQuery query(sqlitedb);
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de la récupération des données"<<sqlitedb.rollback();
    }
    if(combo == "Tous"){
        query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, commande_id FROM mouvements_de_stock");
    }else{
        query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, commande_id FROM mouvements_de_stock WHERE type_mouvement = :type_mouvement");
        query.bindValue(":type_mouvement", combo);
    }
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération de données"<<query.lastError();
    }
    ui->tableWidgetMouvement->setRowCount(0);
    int row = 0;
    while(query.next()){
        ui->tableWidgetMouvement->insertRow(row);
        QString id_mouvement = query.value(0).toString();
        QString stock_id = query.value(1).toString();
        QString nom = query.value(2).toString();
        int quantite = query.value(3).toInt();
        QString type_mouvement = query.value(4).toString();
        QString date_mouvement = query.value(5).toString();
        QString commande_id = query.value(6).toString();

        ui->tableWidgetMouvement->setItem(row, 0, new QTableWidgetItem(id_mouvement));
        ui->tableWidgetMouvement->setItem(row, 1, new QTableWidgetItem(stock_id));
        ui->tableWidgetMouvement->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableWidgetMouvement->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableWidgetMouvement->setItem(row, 4, new QTableWidgetItem(type_mouvement));
        ui->tableWidgetMouvement->setItem(row, 5, new QTableWidgetItem(date_mouvement));
        ui->tableWidgetMouvement->setItem(row, 6, new QTableWidgetItem(commande_id));

        row++;
    }

}

MouvementDeStock::~MouvementDeStock()
{
    delete ui;
}
