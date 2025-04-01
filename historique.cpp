#include "historique.h"
#include "ui_historique.h"
#include "databasemanager.h"

Historique::Historique(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Historique)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    afficherHistoriqueVente();
    afficherHistoriqueCharge();
    calculBeneficeNet();
    produitDansComboBox();
    ui->dateDebut->setDate(QDate::currentDate());
    ui->dateFin->setDate(QDate::currentDate());
    ui->tableHistoriqueVente->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableHistoriqueCharge->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->btnFiltrer, &QPushButton::clicked, this, &Historique::filtrageIntelligent);

}

void Historique::afficherHistoriqueVente(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryHistorique(sqlitedb);
    queryHistorique.prepare("SELECT id_historique, date_vente, nom_produit, quantite, ca, benefice FROM historique "
                            "ORDER BY substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2) DESC");
    if(!queryHistorique.exec()){
        qDebug()<<"Erreur lors de la récupération de l'historique"<<queryHistorique.lastError();
        return;
    }
    ui->tableHistoriqueVente->setRowCount(0);
    int row = 0;
    while(queryHistorique.next()){
        ui->tableHistoriqueVente->insertRow(row);
        QString id_historique = queryHistorique.value(0).toString();
        QString date = queryHistorique.value(1).toString();
        QString nom = queryHistorique.value(2).toString();
        int quantite = queryHistorique.value(3).toInt();
        double ca = queryHistorique.value(4).toDouble();
        double benefice = queryHistorique.value(5).toDouble();

        ui->tableHistoriqueVente->setItem(row, 0, new QTableWidgetItem(id_historique));
        ui->tableHistoriqueVente->setItem(row, 1, new QTableWidgetItem(date));
        ui->tableHistoriqueVente->setItem(row, 2, new QTableWidgetItem(nom));
        ui->tableHistoriqueVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
        ui->tableHistoriqueVente->setItem(row, 4, new QTableWidgetItem(QString::number(ca)));
        ui->tableHistoriqueVente->setItem(row, 5, new QTableWidgetItem(QString::number(benefice)));

        row++;
    }
    QSqlQuery queryBenefice(sqlitedb);
    queryBenefice.prepare("SELECT SUM(benefice) FROM historique");
    if(!queryBenefice.exec()){
        qDebug()<<"Erreur lors de la somme des benefices"<<queryBenefice.lastError();
        return;
    }
    if(queryBenefice.next()){
        QString benefice = queryBenefice.value(0).toString();
        ui->labelBenefice->setText(benefice + " MGA");
    }
    QSqlQuery queryCA(sqlitedb);
    queryCA.prepare("SELECT SUM(ca) FROM historique");
    if(!queryCA.exec()){
        qDebug()<<"Erreur lors du calcul du CA"<<queryCA.lastError();
        return;
    }
    if(queryCA.next()){
        QString ca = queryCA.value(0).toString();
        ui->labelCA->setText(ca + " MGA");
    }
    QSqlQuery queryQuantite(sqlitedb);
    queryQuantite.prepare("SELECT SUM(quantite) FROM historique");
    if(!queryQuantite.exec()){
        qDebug()<<"Erreur lors du calcul de la quantité"<<queryQuantite.lastError();
        return;
    }
    if(queryQuantite.next()){
        QString quantite = queryQuantite.value(0).toString();
        ui->labelNombrePot->setText(quantite + " pots");
    }

}

void Historique::afficherHistoriqueCharge(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryHistoriqueCharge(sqlitedb);
    queryHistoriqueCharge.prepare("SELECT id_historiqueCharge, description, personne, valeur, date FROM historique_charge "
                                 "ORDER BY substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2) DESC");
    if(!queryHistoriqueCharge.exec()){
        qDebug()<<"Erreur lors de la récupération des charges"<<queryHistoriqueCharge.lastError();
        return;
    }
    ui->tableHistoriqueCharge->setRowCount(0);
    int row = 0;
    while(queryHistoriqueCharge.next()){
        ui->tableHistoriqueCharge->insertRow(row);
        QString id_charge = queryHistoriqueCharge.value(0).toString();
        QString description = queryHistoriqueCharge.value(1).toString();
        QString personne = queryHistoriqueCharge.value(2).toString();
        double valeur = queryHistoriqueCharge.value(3).toDouble();
        QString date = queryHistoriqueCharge.value(4).toString();

        ui->tableHistoriqueCharge->setItem(row, 0, new QTableWidgetItem(id_charge));
        ui->tableHistoriqueCharge->setItem(row, 1, new QTableWidgetItem(description));
        ui->tableHistoriqueCharge->setItem(row, 2, new QTableWidgetItem(personne));
        ui->tableHistoriqueCharge->setItem(row, 3, new QTableWidgetItem(QString::number(valeur)));
        ui->tableHistoriqueCharge->setItem(row, 4, new QTableWidgetItem(date));

        row++;
    }
    QSqlQuery queryCharge(sqlitedb);
    queryCharge.prepare("SELECT SUM(valeur) FROM historique_charge");
    if(!queryCharge.exec()){
        qDebug()<<"Erreur lors de la somme des charges"<<queryCharge.lastError();
        return;
    }
    if(queryCharge.next()){
        QString charge = queryCharge.value(0).toString();
        ui->labelCharge->setText(charge + " MGA");
    }
}

void Historique::calculBeneficeNet(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QSqlQuery queryBenefice(sqlitedb);
    queryBenefice.prepare("SELECT SUM(benefice) FROM historique");
    if(!queryBenefice.exec()){
        qDebug()<<"Erreur lors de la somme des benefices"<<queryBenefice.lastError();
        return;
    }
    if(queryBenefice.next()){
        QString benefice = queryBenefice.value(0).toString();
        QSqlQuery queryCharge(sqlitedb);
        queryCharge.prepare("SELECT SUM(valeur) FROM historique_charge");
        if(!queryCharge.exec()){
            qDebug()<<"Erreur lors de la somme des charges"<<queryCharge.lastError();
            return;
        }
        if(queryCharge.next()){
            QString charge = queryCharge.value(0).toString();
            double beneficeNet = benefice.toDouble() - charge.toDouble();
            ui->labelBeneficeNet->setText(QString::number(beneficeNet) + " MGA");
        }
    }
}

void Historique::produitDansComboBox(){
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
    ui->comboProduit->clear();
    ui->comboProduit->addItem("Tous");
    while(query.next()){
        QString nom_produit = query.value(0).toString();
        ui->comboProduit->addItem(nom_produit);
    }
}

void Historique::filtrageIntelligent(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString dateDebut = ui->dateDebut->date().toString("dd-MM-yyyy");
    QString dateFin = ui->dateFin->date().toString("dd-MM-yyyy");
    QString nom_produit = ui->comboProduit->currentText();
    if(nom_produit == "Tous"){
        QSqlQuery query(sqlitedb);
        query.prepare("SELECT id_historique, date_vente, nom_produit, quantite, ca, benefice FROM historique "
                      "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                      "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                      "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        query.bindValue(":dateDebut", dateDebut);
        query.bindValue(":dateFin", dateFin);
        if(!query.exec()){
            qDebug()<<"Erreur lors de la récupération des données"<<query.lastError();
            return;
        }
        ui->tableHistoriqueVente->setRowCount(0);
        int row = 0;
        while(query.next()){
            ui->tableHistoriqueVente->insertRow(row);
            QString id_historique = query.value(0).toString();
            QString date = query.value(1).toString();
            QString nom = query.value(2).toString();
            int quantite = query.value(3).toInt();
            double ca = query.value(4).toDouble();
            double benefice = query.value(5).toDouble();

            ui->tableHistoriqueVente->setItem(row, 0, new QTableWidgetItem(id_historique));
            ui->tableHistoriqueVente->setItem(row, 1, new QTableWidgetItem(date));
            ui->tableHistoriqueVente->setItem(row, 2, new QTableWidgetItem(nom));
            ui->tableHistoriqueVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
            ui->tableHistoriqueVente->setItem(row, 4, new QTableWidgetItem(QString::number(ca)));
            ui->tableHistoriqueVente->setItem(row, 5, new QTableWidgetItem(QString::number(benefice)));

            row++;
        }
        QSqlQuery queryBenefice(sqlitedb);
        queryBenefice.prepare("SELECT SUM(benefice) FROM historique "
                              "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                              "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                              "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryBenefice.bindValue(":dateDebut", dateDebut);
        queryBenefice.bindValue(":dateFin", dateFin);
        if(!queryBenefice.exec()){
            qDebug()<<"Erreur lors de la somme des benefices"<<queryBenefice.lastError();
            return;
        }
        if(queryBenefice.next()){
            double benefice = queryBenefice.value(0).toDouble();
            ui->labelBenefice->setText(QString::number(benefice, 'f', 0)+" MGA");
        }
        QSqlQuery queryCA(sqlitedb);
        queryCA.prepare("SELECT SUM(ca) FROM historique "
                        "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                        "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                        "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryCA.bindValue(":dateDebut", dateDebut);
        queryCA.bindValue(":dateFin", dateFin);
        if(!queryCA.exec()){
            qDebug()<<"Erreur lors du calcul du CA"<<queryCA.lastError();
            return;
        }
        if(queryCA.next()){
            double ca = queryCA.value(0).toDouble();
            ui->labelCA->setText(QString::number(ca, 'f', 0)+" MGA");
        }
        QSqlQuery queryQuantite(sqlitedb);
        queryQuantite.prepare("SELECT SUM(quantite) FROM historique "
                              "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                              "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                              "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryQuantite.bindValue(":dateDebut", dateDebut);
        queryQuantite.bindValue(":dateFin", dateFin);
        if(!queryQuantite.exec()){
            qDebug()<<"Erreur lors du calcul de la quantité"<<queryQuantite.lastError();
            return;
        }
        if(queryQuantite.next()){
            QString quantite = queryQuantite.value(0).toString();
            ui->labelNombrePot->setText(quantite + " pots");
        }
        //calcul du benefice net
        QSqlQuery queryBeneficeNet(sqlitedb);
        queryBeneficeNet.prepare("SELECT SUM(benefice) FROM historique "
                                 "WHERE (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                                 "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                                 "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryBeneficeNet.bindValue(":dateDebut", dateDebut);
        queryBeneficeNet.bindValue(":dateFin", dateFin);
        if(!queryBeneficeNet.exec()){
            qDebug()<<"Erreur lors de la somme des benefices"<<queryBeneficeNet.lastError();
            return;
        }
        if(queryBeneficeNet.next()){
            QString benefice = queryBeneficeNet.value(0).toString();
            QSqlQuery queryCharge(sqlitedb);
            queryCharge.prepare("SELECT SUM(valeur) FROM historique_charge "
                                "WHERE (substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2)) "
                                "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                                "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
            queryCharge.bindValue(":dateDebut", dateDebut);
            queryCharge.bindValue(":dateFin", dateFin);
            if(!queryCharge.exec()){
                qDebug()<<"Erreur lors de la somme des charges"<<queryCharge.lastError();
                return;
            }
            if(queryCharge.next()){
                QString charge = queryCharge.value(0).toString();
                double beneficeNet = benefice.toDouble() - charge.toDouble();
                ui->labelBeneficeNet->setText(QString::number(beneficeNet) + " MGA");
            }
        }
    }else{
        QSqlQuery query(sqlitedb);
        query.prepare("SELECT id_historique, date_vente, nom_produit, quantite, ca, benefice FROM historique "
                      "WHERE nom_produit = :nom_produit "
                      "AND (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                      "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                      "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        query.bindValue(":nom_produit", nom_produit);
        query.bindValue(":dateDebut", dateDebut);
        query.bindValue(":dateFin", dateFin);
        if(!query.exec()){
            qDebug()<<"Erreur lors de la récupération des données"<<query.lastError();
            return;
        }
        ui->tableHistoriqueVente->setRowCount(0);
        int row = 0;
        while(query.next()){
            ui->tableHistoriqueVente->insertRow(row);
            QString id_historique = query.value(0).toString();
            QString date = query.value(1).toString();
            QString nom = query.value(2).toString();
            int quantite = query.value(3).toInt();
            double ca = query.value(4).toDouble();
            double benefice = query.value(5).toDouble();

            ui->tableHistoriqueVente->setItem(row, 0, new QTableWidgetItem(id_historique));
            ui->tableHistoriqueVente->setItem(row, 1, new QTableWidgetItem(date));
            ui->tableHistoriqueVente->setItem(row, 2, new QTableWidgetItem(nom));
            ui->tableHistoriqueVente->setItem(row, 3, new QTableWidgetItem(QString::number(quantite)));
            ui->tableHistoriqueVente->setItem(row, 4, new QTableWidgetItem(QString::number(ca)));
            ui->tableHistoriqueVente->setItem(row, 5, new QTableWidgetItem(QString::number(benefice)));

            row++;
        }
        QSqlQuery queryBenefice(sqlitedb);
        queryBenefice.prepare("SELECT SUM(benefice) FROM historique "
                              "WHERE nom_produit = :nom_produit "
                              "AND (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                              "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                              "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryBenefice.bindValue(":nom_produit", nom_produit);
        queryBenefice.bindValue(":dateDebut", dateDebut);
        queryBenefice.bindValue(":dateFin", dateFin);
        if(!queryBenefice.exec()){
            qDebug()<<"Erreur lors de la somme des benefices"<<queryBenefice.lastError();
            return;
        }
        if(queryBenefice.next()){
            double benefice = queryBenefice.value(0).toDouble();
            ui->labelBenefice->setText(QString::number(benefice, 'f', 0)+" MGA");
        }
        QSqlQuery queryCA(sqlitedb);
        queryCA.prepare("SELECT SUM(ca) FROM historique "
                        "WHERE nom_produit = :nom_produit "
                        "AND (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                        "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                        "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryCA.bindValue(":nom_produit", nom_produit);
        queryCA.bindValue(":dateDebut", dateDebut);
        queryCA.bindValue(":dateFin", dateFin);
        if(!queryCA.exec()){
            qDebug()<<"Erreur lors du calcul du CA"<<queryCA.lastError();
            return;
        }
        if(queryCA.next()){
            double ca = queryCA.value(0).toDouble();
            ui->labelCA->setText(QString::number(ca, 'f', 0)+" MGA");
        }
        QSqlQuery queryQuantite(sqlitedb);
        queryQuantite.prepare("SELECT SUM(quantite) FROM historique "
                              "WHERE nom_produit = :nom_produit "
                              "AND (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                              "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                              "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryQuantite.bindValue(":nom_produit", nom_produit);
        queryQuantite.bindValue(":dateDebut", dateDebut);
        queryQuantite.bindValue(":dateFin", dateFin);
        if(!queryQuantite.exec()){
            qDebug()<<"Erreur lors du calcul de la quantité"<<queryQuantite.lastError();
            return;
        }
        if(queryQuantite.next()){
            QString quantite = queryQuantite.value(0).toString();
            ui->labelNombrePot->setText(quantite + " pots");
        }
        //Calcul du benefice net
        QSqlQuery queryBeneficeNet(sqlitedb);
        queryBeneficeNet.prepare("SELECT SUM(benefice) FROM historique "
                                 "WHERE nom_produit = :nom_produit "
                                 "AND (substr(date_vente, 7, 4) || '-' || substr(date_vente, 4, 2) || '-' || substr(date_vente, 1, 2)) "
                                 "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                                 "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
        queryBeneficeNet.bindValue(":nom_produit", nom_produit);
        queryBeneficeNet.bindValue(":dateDebut", dateDebut);
        queryBeneficeNet.bindValue(":dateFin", dateFin);
        if(!queryBeneficeNet.exec()){
            qDebug()<<"Erreur lors de la somme des benefices"<<queryBeneficeNet.lastError();
            return;
        }
        if(queryBeneficeNet.next()){
            QString benefice = queryBeneficeNet.value(0).toString();
            QSqlQuery queryCharge(sqlitedb);
            queryCharge.prepare("SELECT SUM(valeur) FROM historique_charge "
                                "WHERE (substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2)) "
                                "BETWEEN (substr(:dateDebut, 7, 4) || '-' || substr(:dateDebut, 4, 2) || '-' || substr(:dateDebut, 1, 2)) "
                                "AND (substr(:dateFin, 7, 4) || '-' || substr(:dateFin, 4, 2) || '-' || substr(:dateFin, 1, 2))");
            queryCharge.bindValue(":dateDebut", dateDebut);
            queryCharge.bindValue(":dateFin", dateFin);
            if(!queryCharge.exec()){
                qDebug()<<"Erreur lors de la somme des charges"<<queryCharge.lastError();
                return;
            }
            if(queryCharge.next()){
                QString charge = queryCharge.value(0).toString();
                double beneficeNet = benefice.toDouble() - charge.toDouble();
                ui->labelBeneficeNet->setText(QString::number(beneficeNet) + " MGA");
            }
        }
    }
}



Historique::~Historique()
{
    delete ui;
}
