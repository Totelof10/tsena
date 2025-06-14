#include "etatstock.h"
#include "ui_etatstock.h"
#include "databasemanager.h"
#include "custommessagebox.h"

EtatStock::EtatStock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EtatStock)
{
    ui->setupUi(this);
    affichageEtatStock();
    ui->btnAjouterQuantite->setEnabled(false);
    setWindowModality(Qt::ApplicationModal);
    ui->tableEtatStock->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->tableEtatStock, &QTableWidget::itemSelectionChanged, this, [this]() {
        ui->btnAjouterQuantite->setEnabled(ui->tableEtatStock->currentRow() >= 0);
    });

    // Gestion du clic sur le bouton "Ajouter Quantité"
    connect(ui->btnAjouterQuantite, &QPushButton::clicked, this, &EtatStock::ajouterQuantite);
    connect (ui->lineEditRecherche, &QLineEdit::textChanged, this, &EtatStock::recherche);
    connect(ui->btnImprimerStock, &QPushButton::clicked, this, &EtatStock::imprimerStock);
    connect(ui->btnRafraichir, &QPushButton::clicked, this, &EtatStock::rafraichir);
    connect(ui->btnRetirer, &QPushButton::clicked, this, &EtatStock::retirerQuantite);


}

void EtatStock::affichageEtatStock(){
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        msgBox.showError("", "Erreur lors de l'ouverture de la base de données");
        return;
    }

    QSqlQuery query(sqlitedb);
    query.prepare("SELECT s.id_stock, p.nom, s.quantite, p.prix_base FROM stock s "
                  "INNER JOIN produits p ON s.produit_id = p.id_produit");

    if(!query.exec()){
        qDebug() << "Erreur lors de la récupération de données" << query.lastError();
        msgBox.showError("", "Erreur lors de la récupération des données");
        return;
    }

    ui->tableEtatStock->setRowCount(0);
    int row = 0;
    double totalValeurStock = 0.0; // Variable pour stocker le total

    while(query.next()){
        ui->tableEtatStock->insertRow(row);
        QString id_stock = query.value(0).toString();
        QString nom = query.value(1).toString();
        int quantite = query.value(2).toInt();
        double prix_base = query.value(3).toDouble();

        // Calcul du total par produit (prix_base * quantite)
        double valeurProduit = prix_base * quantite;
        totalValeurStock += valeurProduit; // Ajout au total général

        ui->tableEtatStock->setItem(row, 0, new QTableWidgetItem(id_stock));
        ui->tableEtatStock->setItem(row, 1, new QTableWidgetItem(nom));
        ui->tableEtatStock->setItem(row, 2, new QTableWidgetItem(QString::number(quantite)));
        ui->tableEtatStock->setItem(row, 3, new QTableWidgetItem(QString::number(prix_base, 'f', 2)));
        ui->tableEtatStock->setItem(row, 4, new QTableWidgetItem(QString::number(valeurProduit, 'f', 2)));

        row++;
    }

    // Affichage du total dans un QLabel
    ui->labelTotalValeurStock->setText("Valeur totale du stock : " + QString::number(totalValeurStock, 'f', 2) + " MGA");
}

void EtatStock::ajouterQuantite() {
    int row = ui->tableEtatStock->currentRow();
    if (row < 0) return; // Vérification : aucune ligne sélectionnée

    // Récupérer la cellule de la quantité initiale (colonne 2)
    QTableWidgetItem *celluleQuantiteInitiale = ui->tableEtatStock->item(row, 2);
    if (!celluleQuantiteInitiale || celluleQuantiteInitiale->text().isEmpty()) {
        qDebug() << "La cellule de quantité initiale est vide ou inexistante.";
        return;
    }

    // Récupérer la valeur initiale
    int quantiteInitiale = celluleQuantiteInitiale->text().toInt();

    // Créer une nouvelle cellule pour la quantité à ajouter (colonne 3)
    QTableWidgetItem *celluleQuantiteAjoutee = ui->tableEtatStock->item(row, 5);
    if (!celluleQuantiteAjoutee) {
        celluleQuantiteAjoutee = new QTableWidgetItem();
        ui->tableEtatStock->setItem(row, 5, celluleQuantiteAjoutee);
    }

    // Activer le mode édition pour la cellule
    ui->tableEtatStock->editItem(celluleQuantiteAjoutee);

    // Connexion pour traiter les modifications de la cellule
    connect(ui->tableEtatStock, &QTableWidget::itemChanged, this, [this, row, quantiteInitiale, celluleQuantiteInitiale](QTableWidgetItem *item) {
        if (item->row() == row && item->column() == 5) {
            bool ok;
            int valeurAjoutee = item->text().toInt(&ok); // Conversion en entier
            if (!ok || valeurAjoutee <= 0) {
                // Réinitialiser si l'entrée est invalide ou négative
                item->setText("");
                return;
            }

            // Confirmation via CustomMessageBox
            CustomMessageBox msgBox;
            msgBox.setWindowTitle("Confirmation");
            msgBox.setText("Souhaitez-vous ajouter cette quantité ?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Yes) {
                // Mettre à jour la base de données
                QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
                QSqlQuery query(sqlitedb);
                query.prepare("UPDATE stock SET quantite = quantite + :valeurAjoutee , date_du_dernier_entree= :date_du_dernier_entree WHERE id_stock = :id");
                query.bindValue(":valeurAjoutee", valeurAjoutee);
                query.bindValue(":id", ui->tableEtatStock->item(row, 0)->text());
                query.bindValue(":date_du_dernier_entree", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                if (!query.exec()) {
                    qDebug() << "Erreur lors de la mise à jour de la quantité :" << query.lastError();
                    CustomMessageBox().showError("Erreur", "Échec de la mise à jour de la base de données.");
                    item->setText(""); // Réinitialiser la cellule ajoutée en cas d'erreur
                }
                // Mise à jour réussie : modifier la valeur affichée
                celluleQuantiteInitiale->setText(QString::number(quantiteInitiale + valeurAjoutee));
                item->setText(""); // Vider la cellule ajoutée après validation
                QSqlQuery queryMouvement(sqlitedb);
                queryMouvement.prepare("INSERT INTO mouvements_de_stock (stock_id, nom, quantite, type_mouvement, date_mouvement) "
                                       "VALUES (:stock_id, :nom, :quantite, :type_mouvement, :date_mouvement)");
                queryMouvement.bindValue(":stock_id", ui->tableEtatStock->item(row, 0)->text());
                queryMouvement.bindValue(":nom", ui->tableEtatStock->item(row, 1)->text()); // Récupérer l'ID du produit (à adapter selon votre structure)
                queryMouvement.bindValue(":type_mouvement", QStringLiteral("Entrée"));
                queryMouvement.bindValue(":quantite", valeurAjoutee);
                queryMouvement.bindValue(":date_mouvement", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                if (!queryMouvement.exec()) {
                    qDebug()<<"Erreur lors de l'insertion des données mouvements"<<queryMouvement.lastError();
                }
                QSqlQuery queryOperation(sqlitedb);
                queryOperation.prepare("INSERT INTO operation (mouvement_id, stock_id, nom_produit, stock_depart, quantite_entree, quantite_sortie, stock_actuel, date_operation) "
                                       "VALUES (:mouvement_id, :stock_id, :nom_produit, :stock_depart, :quantite_entree, :quantite_sortie, :stock_actuel, :date_operation)");
                queryOperation.bindValue(":mouvement_id", queryMouvement.lastInsertId());
                queryOperation.bindValue(":stock_id", ui->tableEtatStock->item(row, 0)->text());
                queryOperation.bindValue(":nom_produit", ui->tableEtatStock->item(row, 1)->text());
                queryOperation.bindValue(":stock_depart", quantiteInitiale);
                queryOperation.bindValue(":quantite_sortie", 0);
                queryOperation.bindValue(":quantite_entree", valeurAjoutee);
                queryOperation.bindValue(":stock_actuel", quantiteInitiale + valeurAjoutee);
                queryOperation.bindValue(":date_operation", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                if (!queryOperation.exec()) {
                    qDebug()<<"Erreur lors de l'insertion des données opérations"<<queryOperation.lastError();
                }

            } else {
                // Annuler et vider la cellule
                item->setText("");
            }

            // Déconnecter ce signal pour éviter des boucles infinies
            disconnect(ui->tableEtatStock, &QTableWidget::itemChanged, nullptr, nullptr);
        }
    });
}

void EtatStock::retirerQuantite() {
    int row = ui->tableEtatStock->currentRow();
    if (row < 0) return; // Vérification : aucune ligne sélectionnée

    // Récupérer la cellule de la quantité actuelle (colonne 2)
    QTableWidgetItem *celluleQuantiteInitiale = ui->tableEtatStock->item(row, 2);
    if (!celluleQuantiteInitiale || celluleQuantiteInitiale->text().isEmpty()) {
        qDebug() << "La cellule de quantité initiale est vide ou inexistante.";
        return;
    }

    // Récupérer la valeur actuelle du stock
    int quantiteInitiale = celluleQuantiteInitiale->text().toInt();

    // Créer une nouvelle cellule pour la quantité à retirer (colonne 4)
    QTableWidgetItem *celluleQuantiteRetiree = ui->tableEtatStock->item(row, 5);
    if (!celluleQuantiteRetiree) {
        celluleQuantiteRetiree = new QTableWidgetItem();
        ui->tableEtatStock->setItem(row, 5, celluleQuantiteRetiree);
    }

    // Activer le mode édition pour la cellule
    ui->tableEtatStock->editItem(celluleQuantiteRetiree);

    // Connexion pour traiter les modifications de la cellule
    connect(ui->tableEtatStock, &QTableWidget::itemChanged, this, [this, row, quantiteInitiale, celluleQuantiteInitiale](QTableWidgetItem *item) {
        if (item->row() == row && item->column() == 5) {
            bool ok;
            int valeurRetiree = item->text().toInt(&ok); // Conversion en entier
            if (!ok || valeurRetiree <= 0 || valeurRetiree > quantiteInitiale) {
                // Réinitialiser si l'entrée est invalide ou dépasse le stock disponible
                item->setText("");
                return;
            }

            // Confirmation via CustomMessageBox
            CustomMessageBox msgBox;
            msgBox.setWindowTitle("Confirmation");
            msgBox.setText("Souhaitez-vous retirer cette quantité ?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Yes) {
                // Mettre à jour la base de données
                QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
                QSqlQuery query(sqlitedb);
                query.prepare("UPDATE stock SET quantite = quantite - :valeurRetiree , date_du_dernier_entree = :date_du_dernier_entree WHERE id_stock = :id");
                query.bindValue(":valeurRetiree", valeurRetiree);
                query.bindValue(":id", ui->tableEtatStock->item(row, 0)->text());
                query.bindValue(":date_du_dernier_entree", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                if (!query.exec()) {
                    qDebug() << "Erreur lors de la mise à jour de la quantité :" << query.lastError();
                    CustomMessageBox().showError("Erreur", "Échec de la mise à jour de la base de données.");
                    item->setText(""); // Réinitialiser la cellule en cas d'erreur
                } else {
                    // Mise à jour réussie : modifier la valeur affichée
                    celluleQuantiteInitiale->setText(QString::number(quantiteInitiale - valeurRetiree));
                    item->setText(""); // Vider la cellule après validation

                    // Insérer dans la table mouvements_de_stock
                    QSqlQuery queryMouvement(sqlitedb);
                    queryMouvement.prepare("INSERT INTO mouvements_de_stock (stock_id, nom, quantite, type_mouvement, date_mouvement) "
                                           "VALUES (:stock_id, :nom, :quantite, :type_mouvement, :date_mouvement)");
                    queryMouvement.bindValue(":stock_id", ui->tableEtatStock->item(row, 0)->text());
                    queryMouvement.bindValue(":nom", ui->tableEtatStock->item(row, 1)->text());
                    queryMouvement.bindValue(":type_mouvement", QStringLiteral("Sortie Retrait"));
                    queryMouvement.bindValue(":quantite", valeurRetiree);
                    queryMouvement.bindValue(":date_mouvement", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                    if (!queryMouvement.exec()) {
                        qDebug() << "Erreur lors de l'insertion des données mouvements" << queryMouvement.lastError();
                    }

                    // Insérer dans la table operation
                    QSqlQuery queryOperation(sqlitedb);
                    queryOperation.prepare("INSERT INTO operation (mouvement_id, stock_id, nom_produit, stock_depart, quantite_entree, quantite_sortie, stock_actuel, date_operation) "
                                           "VALUES (:mouvement_id, :stock_id, :nom_produit, :stock_depart, :quantite_entree, :quantite_sortie, :stock_actuel, :date_operation)");
                    queryOperation.bindValue(":mouvement_id", queryMouvement.lastInsertId());
                    queryOperation.bindValue(":stock_id", ui->tableEtatStock->item(row, 0)->text());
                    queryOperation.bindValue(":nom_produit", ui->tableEtatStock->item(row, 1)->text());
                    queryOperation.bindValue(":stock_depart", quantiteInitiale);
                    queryOperation.bindValue(":quantite_entree", 0);
                    queryOperation.bindValue(":quantite_sortie", valeurRetiree);
                    queryOperation.bindValue(":stock_actuel", quantiteInitiale - valeurRetiree);
                    queryOperation.bindValue(":date_operation", QDateTime::currentDateTime().toString("dd-MM-yyyy"));
                    if (!queryOperation.exec()) {
                        qDebug() << "Erreur lors de l'insertion des données opérations" << queryOperation.lastError();
                    }
                }
            } else {
                // Annuler et vider la cellule
                item->setText("");
            }

            // Déconnecter ce signal pour éviter des boucles infinies
            disconnect(ui->tableEtatStock, &QTableWidget::itemChanged, nullptr, nullptr);
        }
    });
}


void EtatStock::recherche(){
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base de données"<<sqlitedb.rollback();
        return;
    }
    QString recherche = ui->lineEditRecherche->text();
    QSqlQuery query(sqlitedb);
    query.prepare("SELECT s.id_stock, p.nom, s.quantite, s.date_du_dernier_entree FROM stock s "
                  "INNER JOIN produits p ON s.produit_id = p.id_produit WHERE p.nom LIKE :recherche");
    query.bindValue(":recherche", "%"+recherche+"%");
    if(!query.exec()){
        qDebug()<<"Erreur lors de la récupération des données"<<query.lastError();
        return;
    }
    ui->tableEtatStock->setRowCount(0);
    int row = 0;
    while(query.next()){
        ui->tableEtatStock->insertRow(row);
        QString id_stock = query.value(0).toString();
        QString nom = query.value(1).toString();
        int quantite = query.value(2).toInt();

        ui->tableEtatStock->setItem(row, 0, new QTableWidgetItem(id_stock));
        ui->tableEtatStock->setItem(row, 1, new QTableWidgetItem(nom));
        ui->tableEtatStock->setItem(row, 2, new QTableWidgetItem(QString::number(quantite)));

        row++;
    }
}

void EtatStock::imprimerStock(){
    //Récupérer les données des lignes dans le tableau et les imprimer
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);
    if(printDialog.exec() == QDialog::Accepted){
        QPainter painter(&printer);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 12));

        //Entête
        painter.drawText(100, 100, "ID Stock");
        painter.drawText(200, 100, "Nom");
        painter.drawText(300, 100, "Quantité");
        painter.drawText(400, 100, "Date du dernier entrée");

        //Affichage des données
        for(int i=0; i<ui->tableEtatStock->rowCount(); i++){
            for(int j=0; j<ui->tableEtatStock->columnCount(); j++){
                painter.drawText(100+j*100, 150+i*50, ui->tableEtatStock->item(i,j)->text());
            }
        }
    }
}

void EtatStock::rafraichir(){
    affichageEtatStock();
}



EtatStock::~EtatStock()
{
    delete ui;
}
