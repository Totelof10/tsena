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
    connect(ui->btnImprimerMouvement, &QPushButton::clicked, this, &MouvementDeStock::imprimerMouvement);
    connect(ui->btnFiltrageMouvement, &QPushButton::clicked, this, &MouvementDeStock::filtrageDateMouvement);
    connect(ui->btnReinitialiserAffichageMouvement, &QPushButton::clicked, this, &MouvementDeStock::reinitialiserAffichage);
    setWindowModality(Qt::ApplicationModal);

}

void MouvementDeStock::afficherMouvement(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base données"<<sqlitedb.rollback();
        return;
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
        return;
    }
    QString recherche = ui->lineEditRecherche->text();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, vente FROM mouvements_de_stock WHERE nom LIKE :recherche");
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

void MouvementDeStock::filtrageMouvement(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    QString combo = ui->comboMouvement->currentText();
    QSqlQuery query(sqlitedb);
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de la récupération des données"<<sqlitedb.rollback();
    }
    if(combo == "Tous"){
        query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, vente FROM mouvements_de_stock");
    }else{
        query.prepare("SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, vente FROM mouvements_de_stock WHERE type_mouvement = :type_mouvement");
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


void MouvementDeStock::imprimerMouvement(){
    //Récupérer les données des lignes dans le tableau et les imprimer
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);
    if(printDialog.exec() == QDialog::Accepted){
        QPainter painter(&printer);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 12));

        //Entête
        painter.drawText(100, 100, "ID Mouvement");
        painter.drawText(200, 100, "Stock ID");
        painter.drawText(300, 100, "Nom");
        painter.drawText(400, 100, "Quantité");
        painter.drawText(500, 100, "Type Mouvement");
        painter.drawText(600, 100, "Date Mouvement");
        painter.drawText(700, 100, "Vente");

        //Affichage des données
        for(int i=0; i<ui->tableWidgetMouvement->rowCount(); i++){
            for(int j=0; j<ui->tableWidgetMouvement->columnCount(); j++){
                painter.drawText(100+j*100, 150+i*50, ui->tableWidgetMouvement->item(i,j)->text());
            }
        }
    }
}

void MouvementDeStock::filtrageDateMouvement() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de la récupération des données" << sqlitedb.rollback();
        return;
    }

    QDate dateDebut = ui->dateDebutMouvement->date();
    QDate dateFin = ui->dateFinMouvement->date();

    QSqlQuery query(sqlitedb);
    QString queryString = "SELECT id_mouvement, stock_id, nom, quantite, type_mouvement, date_mouvement, vente FROM mouvements_de_stock";

    // Ajout du filtre par date uniquement si elles sont valides
    if (dateDebut.isValid() && dateFin.isValid()) {
        queryString += " WHERE date_mouvement BETWEEN :dateDebut AND :dateFin";
    }

    query.prepare(queryString);

    // Liaison des valeurs des dates
    if (dateDebut.isValid() && dateFin.isValid()) {
        query.bindValue(":dateDebut", dateDebut.toString("dd-MM-yyyy"));
        query.bindValue(":dateFin", dateFin.toString("dd-MM-yyyy"));
    }

    if (!query.exec()) {
        qDebug() << "Erreur lors de la récupération des mouvements de stock" << query.lastError();
        return;
    }

    // Mise à jour du tableau des mouvements
    ui->tableWidgetMouvement->setRowCount(0);
    int row = 0;
    while (query.next()) {
        ui->tableWidgetMouvement->insertRow(row);
        ui->tableWidgetMouvement->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidgetMouvement->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidgetMouvement->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->tableWidgetMouvement->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->tableWidgetMouvement->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        ui->tableWidgetMouvement->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
        ui->tableWidgetMouvement->setItem(row, 6, new QTableWidgetItem(query.value(6).toString()));

        row++;
    }
}


void MouvementDeStock::reinitialiserAffichage(){
    QDate dateActuelle = QDate::currentDate();
    ui->dateDebutMouvement->setDate(dateActuelle);  // Mettre la date de début au jour actuel
    ui->dateFinMouvement->setDate(dateActuelle);
    afficherMouvement();
}

MouvementDeStock::~MouvementDeStock()
{
    delete ui;
}
