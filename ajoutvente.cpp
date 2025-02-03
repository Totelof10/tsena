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
    connect(ui->comboClient->lineEdit(), &QLineEdit::textEdited, this, [=](const QString &text) {
        for (int i = 0; i < ui->comboClient->count(); ++i) {
            bool match = ui->comboClient->itemText(i).contains(text, Qt::CaseInsensitive);
            ui->comboClient->setItemData(i, match ? QVariant() : QVariant(0), Qt::UserRole - 1);
        }
    });
    connect(ui->comboClient, &QComboBox::currentIndexChanged, this, [=]() {
        for (int i = 0; i < ui->comboClient->count(); ++i) {
            ui->comboClient->setItemData(i, QVariant(), Qt::UserRole - 1);
        }
    });
    // Connecter le signal pour filtrer les éléments existants
    connect(ui->comboProduit->lineEdit(), &QLineEdit::textEdited, this, [=](const QString &text) {
        for (int i = 0; i < ui->comboProduit->count(); ++i) {
            bool match = ui->comboProduit->itemText(i).contains(text, Qt::CaseInsensitive);
            ui->comboProduit->setItemData(i, match ? QVariant() : QVariant(0), Qt::UserRole - 1);
        }
    });
    connect(ui->comboProduit, &QComboBox::currentIndexChanged, this, [=]() {
        for (int i = 0; i < ui->comboProduit->count(); ++i) {
            ui->comboProduit->setItemData(i, QVariant(), Qt::UserRole - 1);
        }
    });


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
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if(!sqlitedb.isOpen()){
        qDebug()<<"Erreur lors de l'ouverture de la base données"<<sqlitedb.rollback();
    }
    QSqlQuery queryStock(sqlitedb);
    QString client = ui->comboClient->currentText();
    QString produit = ui->comboProduit->currentText();
    int quantite = ui->spinBoxQuantite->value();

    // Récupérer l'ID du produit actuellement sélectionné dans le QComboBox
    int idProduit = ui->comboProduit->currentData().toInt();
    int quantiteStock = -1;
    queryStock.prepare("SELECT quantite FROM stock WHERE produit_id = :produit_id");
    queryStock.bindValue(":produit_id", idProduit);
    if (queryStock.exec() && queryStock.next()) {
        quantiteStock = queryStock.value(0).toInt();  // Supposons que "quantite" est la première colonne.
    } else {
        qDebug() << "Erreur lors de la récupération des données :" << queryStock.lastError();
        return;
    }

    if(quantiteStock <= 0 || quantiteStock < quantite){
        msgBox.showWarning("","Votre stock de "+produit+" est insuffisant, stock actuel : " +QString::number(quantiteStock)+ " Quantité demandé : "+QString::number(quantite));
        return;
    }


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
        msgBox.showWarning("Information", "La liste est déjà vide !");
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


#include <QPrintPreviewDialog>

void AjoutVente::ajouterNouvelleVente() {
    texte = new QTextEdit();
    QString totalPayer = ui->labelTotal->text();
    QString dateActuelle = QDate::currentDate().toString("dd-MM-yyyy");
    QString nom_client = ui->comboClient->currentText();
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.open()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
    }
    QString contenuFacture;
    contenuFacture = QString("<!DOCTYPE html>"
                             "<html>"
                             "<head>"
                             "<meta charset='UTF-8'>"
                             "<style>"
                             "table {"
                             "  width: 100%;"
                             "  border-collapse: collapse;"
                             "  margin-top: 20px;"
                             "}"
                             "th, td {"
                             "  border: 1px solid black;"
                             "  padding: 10px;"
                             "  text-align: center;"
                             "}"
                             "th {"
                             "  background-color: #f2f2f2;"
                             "  font-weight: bold;"
                             "}"
                             "</style>"
                             "</head>"
                             "<body>"
                             "<p>RAHENINTSOA ALASORA</p>"
                             "<p><strong>Nif : </strong></p>"
                             "<p><strong>STAT : </strong></p>"
                             "<p><strong>Adresse : </strong>ALASORA Commune en Face de Sopromer</p>"
                             "<p><strong>Contact : </strong>0347636886</p>"
                             "<div style='text-align: center; font-size: 20px;'>"
                             "<h2>FACTURE</h2>"
                             "</div>"
                             "<h2>Client : %1</h2>"
                             "<table>"
                             "<thead>"
                             "<tr>"
                             "<th>Désignation</th>"
                             "<th>Quantité</th>"
                             "<th>Prix Unitaire (MGA)</th>"
                             "<th>Prix Total (MGA)</th>"
                             "</tr>"
                             "</thead>"
                             "<tbody>").arg(nom_client);


    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QString ligne = item->text();

        QStringList elements = ligne.split("|");

        // Récupération de la date et de l'heure actuelle
        QString currentDate = QDate::currentDate().toString("dd-MM-yyyy");
        QString currentDateTime = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss");

        // Parcourir les éléments
        if (elements.size() == 4) {
            // Extraire les valeurs
            QString nomClient = elements[0].remove("Client: ").trimmed();
            QString nomProduit = elements[1].remove("Produit: ").trimmed();
            int quantite = elements[2].remove("Quantité: ").trimmed().toInt();
            double prixTotal = elements[3].remove("Prix Total: ").remove("MGA").trimmed().toDouble();

            // Requêtes SQL
            QSqlQuery queryProduit(sqlitedb);
            QSqlQuery queryClient(sqlitedb);
            QSqlQuery queryUpdateQuantite(sqlitedb);
            QSqlQuery queryMouvement(sqlitedb);
            QSqlQuery queryStock(sqlitedb);
            QSqlQuery operation(sqlitedb);

            // Rechercher l'ID du client
            queryClient.prepare("SELECT id_client FROM clients WHERE nom = ?");
            queryClient.addBindValue(nomClient);
            if (!queryClient.exec() || !queryClient.next()) {
                qDebug() << "Erreur lors de la recherche du client : " << queryClient.lastError().text();
                continue; // Passer à l'élément suivant
            }
            int clientId = queryClient.value(0).toInt();

            // Rechercher l'ID du produit
            queryProduit.prepare("SELECT id_produit, prix_unitaire FROM produits WHERE nom = ?");
            queryProduit.addBindValue(nomProduit);
            if (!queryProduit.exec() || !queryProduit.next()) {
                qDebug() << "Erreur lors de la recherche du produit : " << queryProduit.lastError().text();
                continue;
            }
            int produitId = queryProduit.value(0).toInt();
            double prixUnitaire = queryProduit.value(1).toDouble();

            // Rechercher l'ID du stock
            queryStock.prepare("SELECT id_stock, quantite FROM stock WHERE produit_id = :produit_id");
            queryStock.bindValue(":produit_id", produitId);
            if (!queryStock.exec() || !queryStock.next()) {
                qDebug() << "Erreur lors de la récupération de l'ID du stock : " << queryStock.lastError().text();
                continue;
            }
            int id_stock = queryStock.value(0).toInt();
            int quantiteStock = queryStock.value(1).toInt();

            // Insertion dans ligne_vente
            QSqlQuery queryInsertion(sqlitedb);
            queryInsertion.prepare("INSERT INTO ligne_vente (produit_id, client_id, quantite, prix_total, date_vente) "
                                   "VALUES (?, ?, ?, ?, ?)");
            queryInsertion.addBindValue(produitId);
            queryInsertion.addBindValue(clientId);
            queryInsertion.addBindValue(quantite);
            queryInsertion.addBindValue(prixTotal);
            queryInsertion.addBindValue(currentDate); // Date de la vente

            if (!queryInsertion.exec()) {
                qDebug() << "Erreur lors de l'insertion dans ligne_vente : " << queryInsertion.lastError().text();
                continue;
            }

            // Mise à jour du stock
            queryUpdateQuantite.prepare("UPDATE stock SET quantite = quantite - :quantite WHERE produit_id = :id");
            queryUpdateQuantite.bindValue(":quantite", quantite);
            queryUpdateQuantite.bindValue(":id", produitId);
            if (!queryUpdateQuantite.exec()) {
                qDebug() << "Erreur lors de la mise à jour du stock : " << queryUpdateQuantite.lastError().text();
                continue;
            }

            // Insertion dans mouvements_de_stock
            queryMouvement.prepare("INSERT INTO mouvements_de_stock (stock_id, nom, quantite, type_mouvement, date_mouvement, vente) "
                                   "VALUES (:stock_id, :nom, :quantite, :type_mouvement, :date_mouvement, :vente)");
            queryMouvement.bindValue(":stock_id", id_stock);
            queryMouvement.bindValue(":nom", nomProduit);
            queryMouvement.bindValue(":quantite", quantite);
            queryMouvement.bindValue(":type_mouvement", "Sortie Vente");
            queryMouvement.bindValue(":date_mouvement", currentDateTime);
            queryMouvement.bindValue(":vente", QString("Achat de %1").arg(nomClient));
            if (!queryMouvement.exec()) {
                qDebug() << "Erreur lors de l'insertion dans mouvements_de_stock : " << queryMouvement.lastError().text();
            }

            QSqlQuery queryOperation(sqlitedb);
            queryOperation.prepare("INSERT INTO operation (mouvement_id, stock_id, nom_produit, stock_depart, quantite_entree, quantite_sortie, stock_actuel, date_operation) "
                                   "VALUES (:mouvement_id, :stock_id, :nom_produit, :stock_depart, :quantite_entree, :quantite_sortie, :stock_actuel, :date_operation)");
            queryOperation.bindValue(":mouvement_id", queryMouvement.lastInsertId());
            queryOperation.bindValue(":stock_id", id_stock);
            queryOperation.bindValue(":nom_produit", nomProduit);
            queryOperation.bindValue(":stock_depart", quantiteStock);
            queryOperation.bindValue(":quantite_sortie", quantite);
            queryOperation.bindValue(":quantite_entree", 0);
            queryOperation.bindValue(":stock_actuel", quantiteStock - quantite);
            queryOperation.bindValue(":date_operation", currentDateTime);
            if (!queryOperation.exec()) {
                qDebug() << "Erreur lors de l'insertion des données" << queryOperation.lastError();
            }

            // Ajouter à la facture
            contenuFacture += QString("<tr>"
                                      "<td>%1</td>"
                                      "<td>%2</td>"
                                      "<td>%3 MGA</td>"
                                      "<td>%4 MGA</td>"
                                      "</tr>"
                                      )
                                  .arg(nomProduit)
                                  .arg(quantite)
                                  .arg(prixUnitaire)
                                  .arg(prixTotal);
        } else {
            qDebug() << "La ligne ne contient pas 4 éléments valides.";
        }
    }

    totalPayer=totalPayer.remove(" MGA").trimmed();
    long long totalPayerInt = totalPayer.toLongLong();
    QString totalEnLettres = convertirNombreEnLettres(totalPayerInt);
    contenuFacture += QString(
                          "</tbody></table>"  // Fermeture du tableau
                          "<br><br>"  // Ajoute des espaces pour séparer les infos du tableau

                          // Texte pour l'arrêté de la somme
                          "<p><strong>Arrêtée la facture à la somme de :</strong> %1</p>"

                          // Zone de total bien centrée
                          "<div style='text-align: right; font-size: 18px; margin-top: 20px;'>"
                          "<h2>TOTAL À PAYER : %2 MGA</h2>"
                          "<h2>DATE DU : %3</h2>"
                          "</div>"

                          // Ajout de la signature en bas
                          "<div style='text-align: right; margin-top: 50px;'>"
                          "<p>Signature :</p>"
                          "<p>______________________</p>"
                          "</div>"

                          "</body></html>"  // Fermeture du HTML
                          ).arg(totalEnLettres)  // Convertit en lettres
                          .arg(totalPayer)
                          .arg(dateActuelle);
    qDebug()<<convertirNombreEnLettres(totalPayer.toLongLong());

    QTextDocument document;
    document.setHtml(contenuFacture);

    // Assurez-vous que le contenu est correct
    qDebug() << document.toPlainText();

    // Configurez l'impression pour l'aperçu
    QPrinter previewPrinter(QPrinter::PrinterResolution);
    previewPrinter.setOutputFormat(QPrinter::PdfFormat);
    previewPrinter.setOutputFileName("facture.pdf");

    // Aperçu avant impression
    QPrintPreviewDialog previewDialog(&previewPrinter, this);
    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, [&document](QPrinter *printer) {
        document.print(printer);
    });
    previewDialog.exec();

    // Création d'un nouvel objet QPrinter pour l'impression réelle
    QPrinter printPrinter(QPrinter::HighResolution);
    QPrintDialog printDialog(&printPrinter, this);

    if (printDialog.exec() == QDialog::Accepted) {
        document.print(&printPrinter);
    }


    qDebug() << contenuFacture;
    msgBox.showInformation("", "Vente effectuée");
    emit ajouterVente();
    emit CA();
    clearForm();
}



AjoutVente::~AjoutVente()
{
    delete ui;
}
