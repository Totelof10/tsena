#include "bondelivraison.h"
#include "ui_bondelivraison.h"
#include "databasemanager.h"
#include "custommessagebox.h"

BonDeLivraison::BonDeLivraison(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BonDeLivraison)
{
    ui->setupUi(this);
    afficherInformationBl();
    ui->dateDeLivraison->setDate(QDate::currentDate());
    setWindowModality(Qt::ApplicationModal);
    connect(ui->btnAdd, &QPushButton::clicked, this, &BonDeLivraison::ajouterPanierBl);
    connect(ui->btnDelete, &QPushButton::clicked, this, &BonDeLivraison::enleverPanierBl);
    connect(ui->btnVider, &QPushButton::clicked, this, &BonDeLivraison::viderPanierBl);
    connect(ui->comboProduit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BonDeLivraison::mettreAJourPrixBl);
    connect(ui->spinBoxQuantite, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BonDeLivraison::mettreAJourTotalBl);
    connect(ui->comboProduit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BonDeLivraison::remettreAZeroBl);
    connect(ui->btnValider, &QPushButton::clicked, this, &BonDeLivraison::ajouterNouvelleBl);    
    // Connecter le signal pour filtrer les éléments existants
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

void BonDeLivraison::afficherInformationBl() {
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
        return;
    }

    // Récupération des produits
    QSqlQuery queryProduits(sqlitedb);
    queryProduits.prepare("SELECT id_produit, nom, prix_unitaire, prix_detail, prix_remise FROM produits");
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
        double prix_detail = queryProduits.value(3).toDouble();
        double prix_remise = queryProduits.value(4).toDouble();
        ui->comboProduit->addItem(nom, id);

        // Correctly create and store the PrixProduit struct
        PrixProduit prix;
        prix.prix_unitaire = prix_unitaire;
        prix.prix_detail = prix_detail;
        prix.prix_remise = prix_remise;
        mapPrixProduits[id] = prix; // Store the struct
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
}

void BonDeLivraison::ajouterPanierBl() {
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.isOpen()) {
        qDebug() << "Erreur lors de l'ouverture de la base données" << sqlitedb.rollback();
        return;
    }
    QString date = ui->dateDeLivraison->date().toString("dd-MM-yyyy");  // Date choisie
    QSqlQuery queryStock(sqlitedb);
    QString client = ui->comboClient->currentText();
    QString produit = ui->comboProduit->currentText();
    int quantite = ui->spinBoxQuantite->value();

    // Récupérer l'ID du produit actuellement sélectionné
    int idProduit = ui->comboProduit->currentData().toInt();
    int quantiteStock = -1;
    queryStock.prepare("SELECT quantite FROM stock WHERE produit_id = :produit_id");
    queryStock.bindValue(":produit_id", idProduit);
    if (queryStock.exec() && queryStock.next()) {
        quantiteStock = queryStock.value(0).toInt();
    } else {
        qDebug() << "Erreur lors de la récupération des données :" << queryStock.lastError();
        return;
    }

    if (client.isEmpty() || produit.isEmpty() || quantite <= 0) {
        msgBox.showWarning("Erreur", "Veuillez remplir tous les champs avant d'ajouter !");
        return;
    }

    // ***KEY CHANGE: Get the selected price from the combo box***
    double prixSelectionne = ui->comboBoxPrix->currentData().toDouble();

    // Calculer le total pour ce produit (USE THE SELECTED PRICE)
    double totalProduit = prixSelectionne * quantite;
    // Ajouter la date uniquement la première fois
    if (ui->listWidget->count() == 0) {
        ui->listWidget->addItem("📅 Date de livraison : " + date);
        ui->listWidget->addItem("--------------------------------------");
    }

    // Ajouter les informations du produit
    QString ligne = QString("🛒 Client: %1 | Produit: %2 | Quantité: %3 | Prix Unitaire: %5 MGA| Prix Total: %4 MGA")
                        .arg(client)
                        .arg(produit)
                        .arg(quantite)
                        .arg(QString::number(totalProduit, 'f', 2))
                        .arg(QString::number(prixSelectionne, 'f', 2));

    ui->listWidget->addItem(ligne);

    // Mettre à jour le champ total du panier
    ui->lineEditTotal->setText(QString::number(totalProduit, 'f', 2)); // Total avec 2 décimales
    double prixTotalTotal = 0.0;
    for(int i = 0; i< ui->listWidget->count(); i++){
        QString ligne = ui->listWidget->item(i)->text();
        QStringList elements = ligne.split("|");
        if(elements.size()==5){
            double prixTotal = elements[4].remove("Prix Total: ").remove("MGA").trimmed().toDouble();
            prixTotalTotal += prixTotal;
            ui->labelTotal->setText(QString::number(prixTotalTotal)+" MGA");
        }
    }
}

void BonDeLivraison::enleverPanierBl() {
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
        if (elements.size() == 5) {
            double prixTotal = elements[4].remove("Prix Total: ").remove("MGA").trimmed().toDouble();
            prixTotalTotal += prixTotal;
        }
    }

    // Mettre à jour le label total global
    ui->labelTotal->setText(QString::number(prixTotalTotal) + " MGA");
}


void BonDeLivraison::mettreAJourPrixBl(int index) {
    int idProduit = ui->comboProduit->itemData(index).toInt();
    if (mapPrixProduits.contains(idProduit)) {
        PrixProduit prices = mapPrixProduits[idProduit]; // Get the struct

        ui->comboBoxPrix->clear();

        // Use the struct members correctly
        ui->comboBoxPrix->addItem(QString::number(prices.prix_unitaire, 'f', 2), prices.prix_unitaire);
        ui->comboBoxPrix->addItem(QString::number(prices.prix_detail, 'f', 2), prices.prix_detail);
        ui->comboBoxPrix->addItem(QString::number(prices.prix_remise, 'f', 2), prices.prix_remise);

    } else {
        ui->comboBoxPrix->clear();
    }
}


void BonDeLivraison::viderPanierBl(){
    CustomMessageBox msgBox;
    if (ui->listWidget->count() == 0) {
        msgBox.showWarning("Information", "La liste est déjà vide !");
        return;
    }
    clearForm();
    msgBox.showInformation("Succès", "Tous les éléments ont été supprimés de la liste !");
}


void BonDeLivraison::remettreAZeroBl(int index)
{
    Q_UNUSED(index);

    // Réinitialiser le QSpinBox de quantité à 0
    ui->spinBoxQuantite->setValue(0);

    // Réinitialiser le champ Total à 0.00
    ui->lineEditTotal->setText("0.00");
}

void BonDeLivraison::mettreAJourTotalBl()
{
    int quantite = ui->spinBoxQuantite->value();
    int idProduit = ui->comboProduit->currentData().toInt();

    if (idProduit == 0 || quantite <= 0) {
        ui->lineEditTotal->setText("0.00");
        return;
    }

    double prixSelectionne = ui->comboBoxPrix->currentData().toDouble();
    double totalProduit = prixSelectionne * quantite;
    ui->lineEditTotal->setText(QString::number(totalProduit, 'f', 2));
}



void BonDeLivraison::clearForm(){
    ui->comboClient->setCurrentIndex(0);
    ui->comboProduit->setCurrentIndex(0);
    ui->spinBoxQuantite->clear();
    ui->comboBoxPrix->setCurrentIndex(0);
    ui->lineEditTotal->clear();
    ui->listWidget->clear();
    ui->labelTotal->setText("0");
    ui->lineEditTotal->clear();
}


#include <QPrintPreviewDialog>

void BonDeLivraison::ajouterNouvelleBl() {
    texte = new QTextEdit();
    QString totalPayer = ui->labelTotal->text();
    QString date = ui->dateDeLivraison->date().toString("dd-MM-yyyy");
    QString nom_client = ui->comboClient->currentText();
    CustomMessageBox msgBox;
    QSqlDatabase sqlitedb = DatabaseManager::getDatabase();
    if (!sqlitedb.open()) {
        qDebug() << "Erreur lors de l'ouverture de la base de données" << sqlitedb.rollback();
    }
    int id_facture = ui->spinBox->value();
    QString contenuFacture;
    contenuFacture = QString("<!DOCTYPE html>"
                             "<html>"
                             "<head>"
                             "<meta charset='UTF-8'>"
                             "<style>"
                             "table {"
                             "  width: 100%;"
                             "  border-collapse: collapse;"
                             "  margin-top: 20px; "
                             "} "
                             "th, td {"
                             "  border: 1px solid black; "
                             "  padding: 10px; "
                             "  text-align: center; "
                             "} "
                             "th {"
                             "  background-color: #f2f2f2; "
                             "  font-weight: bold; "
                             "} "
                             "</style>"
                             "</head>"
                             "<body>"
                             "<p>RAHENINTSOA ALASORA</p>"
                             "<p><strong>Nif : </strong></p>"
                             "<p><strong>STAT : </strong></p>"
                             "<p><strong>Adresse : </strong>ALASORA Commune en Face de Sopromer</p>"
                             "<p><strong>Contact : </strong>0347636886</p>"
                             "<div style='text-align: center; font-size: 20px;'>"
                             "<h2>BON DE LIVRAISON</h2>"
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

        // Parcourir les éléments du produit
        QStringList elements = ligne.split("|");
        if (elements.size() >= 4) {
            // Extraire les valeurs
            QString nomClient = elements[0].remove("🛒 Client: ").trimmed();
            QString nomProduit = elements[1].remove("Produit: ").trimmed();
            int quantite = elements[2].remove("Quantité: ").trimmed().toInt();
            double prixUnitaire = elements[3].remove("Prix Unitaire: ").remove("MGA").trimmed().toDouble(); // Get unit price from listWidget
            double prixTotal = elements[4].remove("Prix Total: ").remove("MGA").trimmed().toDouble(); // Get t

            // Requêtes SQL
            QSqlQuery queryProduit(sqlitedb);
            QSqlQuery queryClient(sqlitedb);

            // Rechercher l'ID du client
            queryClient.prepare("SELECT id_client FROM clients WHERE nom = ?");
            queryClient.addBindValue(nomClient);
            if (!queryClient.exec() || !queryClient.next()) {
                qDebug() << "Erreur lors de la recherche du client : " << queryClient.lastError().text();
                continue; // Passer à l'élément suivant
            }
            int clientId = queryClient.value(0).toInt();

            // Rechercher l'ID du produit
            queryProduit.prepare("SELECT id_produit, prix_base FROM produits WHERE nom = ?"); // Only get the ID
            queryProduit.addBindValue(nomProduit);
            if (!queryProduit.exec() || !queryProduit.next()) {
                qDebug() << "Erreur lors de la recherche du produit : " << queryProduit.lastError().text();
                continue;
            }
            int produitId = queryProduit.value(0).toInt();
            double prix_base = queryProduit.value(1).toDouble();


            // Insertion dans ligne_vente
            QSqlQuery queryInsertion(sqlitedb);
            queryInsertion.prepare("INSERT INTO bon_de_livraison (client_id, produit_id, quantite, date, statut, prix_vente,prix_total_a_payer, benefice) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
            queryInsertion.addBindValue(clientId);
            queryInsertion.addBindValue(produitId);
            queryInsertion.addBindValue(quantite);
            queryInsertion.addBindValue(date);
            // Date de la vente
            QString statut = "Non Payé";
            queryInsertion.addBindValue(statut);
            queryInsertion.addBindValue(prixUnitaire);
            queryInsertion.addBindValue(prixTotal);
            queryInsertion.addBindValue((prixUnitaire - prix_base)*quantite);
            if (!queryInsertion.exec()) {
                qDebug() << "Erreur lors de l'insertion dans ligne_vente : " << queryInsertion.lastError().text();
                continue;
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

    totalPayer = totalPayer.remove(" MGA").trimmed();
    long long totalPayerInt = totalPayer.toLongLong();
    QString totalEnLettres = convertirNombreEnLettres(totalPayerInt);
    contenuFacture += QString(
                          "</tbody></table>"  // Fermeture du tableau
                          "<br><br>"  // Ajoute des espaces pour séparer les infos du tableau
                          "<p><strong>Arrêtée la facture à la somme de :</strong> %1</p>"
                          "<div style='text-align: right; font-size: 18px; margin-top: 20px;'>"
                          "<h2>TOTAL À PAYER : %2 MGA</h2>"
                          "<h2>DATE DU : %3</h2>"
                          "</div>"
                          "<div style='text-align: right; margin-top: 50px;'>"
                          "<p>Signature :</p>"
                          "<p>______________________</p>"
                          "</div>"
                          "</body></html>"  // Fermeture du HTML
                          ).arg(totalEnLettres)  // Convertit en lettres
                          .arg(totalPayer)
                          .arg(date);

    qDebug() << "Facture générée : " << contenuFacture;

    QTextDocument document;
    document.setHtml(contenuFacture);

    // Aperçu avant impression
    QPrinter previewPrinter(QPrinter::PrinterResolution);
    previewPrinter.setOutputFormat(QPrinter::PdfFormat);
    previewPrinter.setOutputFileName("facture.pdf");

    QPrintPreviewDialog previewDialog(&previewPrinter, this);
    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, [&document](QPrinter *printer) {
        document.print(printer);
    });
    previewDialog.exec();

    // Impression réelle
    QPrinter printPrinter(QPrinter::HighResolution);
    QPrintDialog printDialog(&printPrinter, this);

    if (printDialog.exec() == QDialog::Accepted) {
        document.print(&printPrinter);
    }

    msgBox.showInformation("", "Bon de livraison effectué");
    emit ajouterBonDeLivraison();
    clearForm();
}



BonDeLivraison::~BonDeLivraison()
{
    delete ui;
}
