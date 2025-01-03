#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "databasemanager.h"
#include "custommessagebox.h"
#include "app.h"
#include <QCryptographicHash>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->page_3);
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
    connect(ui->btnAnnuler, &QPushButton::clicked, this, &MainWindow::inscription);
    connect(ui->btnAcceder, &QPushButton::clicked, this, &MainWindow::acceder);
    connect(ui->btnInscrire, &QPushButton::clicked, this, &MainWindow::inscrireUtilisateur);
    connect(ui->btnFermer, &QPushButton::clicked, this, &MainWindow::fermerFenetre);

    // Ouvrir la base de données une seule fois au démarrage
    if (!DatabaseManager::getDatabase().open()) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir la base de données : " + DatabaseManager::getDatabase().lastError().text());
        // Gérer l'erreur, par exemple en désactivant le bouton de connexion ou en fermant l'application
        ui->btnAcceder->setEnabled(false); // Désactive le bouton Accéder
    }
}

void MainWindow::inscrireUtilisateur() {
    CustomMessageBox msgBox;

    QString nom_utilisateur = ui->lineEdit_nouveau_nom->text();
    QString mot_de_passe = ui->lineEdit_nouveau_password->text();
    QString confirmation_mot_de_passe = ui->lineEdit_confirmation_password->text();
    QString statut_utilisateur = ui->comboBox_statut->currentText(); // Récupérer le statut depuis un QComboBox

    // Vérifications des champs
    if (nom_utilisateur.isEmpty() || mot_de_passe.isEmpty() || confirmation_mot_de_passe.isEmpty()) {
        msgBox.showError("", "Veuillez remplir tous les champs.");
        return;
    }

    if (mot_de_passe != confirmation_mot_de_passe) {
        msgBox.showError("", "Les mots de passe ne correspondent pas.");
        return;
    }

    // Vérification de l'existence de l'utilisateur
    QSqlQuery checkQuery(DatabaseManager::getDatabase());
    checkQuery.prepare("SELECT nom_user FROM users WHERE nom_user = :nom_user");
    checkQuery.bindValue(":nom_user", nom_utilisateur);
    if (!checkQuery.exec()) {
        qDebug() << "Erreur lors de la vérification de l'utilisateur : " << checkQuery.lastError();
        msgBox.showError("", "Erreur lors de la vérification de l'utilisateur.");
        return;
    }
    if (checkQuery.next()) {
        msgBox.showError("", "Ce nom d'utilisateur est déjà utilisé.");
        return;
    }


    // Hachage du mot de passe (IMPORTANT : Utiliser un algorithme plus robuste en production)
    QByteArray mot_de_passe_hache = QCryptographicHash::hash(mot_de_passe.toUtf8(), QCryptographicHash::Sha256); // A remplacer par bcrypt ou Argon2

    QSqlQuery query(DatabaseManager::getDatabase());
    query.prepare("INSERT INTO users (nom_user, password_hash, statut_user) VALUES (:nom_user, :password_hash, :statut_user)");
    query.bindValue(":nom_user", nom_utilisateur);
    query.bindValue(":password_hash", mot_de_passe_hache);
    query.bindValue(":statut_user", statut_utilisateur);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'inscription : " << query.lastError();
        msgBox.showError("", "Erreur lors de l'inscription.");
        return;
    }

    msgBox.showInformation("", "Inscription réussie !");
    // Nettoyer les champs après l'inscription
    ui->lineEdit_nouveau_nom->clear();
    ui->lineEdit_nouveau_password->clear();
    ui->lineEdit_confirmation_password->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_3);

}
void MainWindow::inscription() {
    ui->stackedWidget->setCurrentWidget(ui->page_4);
}

void MainWindow::nettoyerChamps() {
    ui->lineEdit_password->clear();
    ui->lineEdit_utilisateur->clear();
}

void MainWindow::acceder() {
    CustomMessageBox msgBox;

    QString nom_user = ui->lineEdit_utilisateur->text();
    QString password_user = ui->lineEdit_password->text();

    QSqlQuery query(DatabaseManager::getDatabase());
    query.prepare("SELECT id, nom_user, statut_user, password_hash FROM users WHERE nom_user = :nom_user");
    query.bindValue(":nom_user", nom_user);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'exécution de la requête :" << query.lastError();
        QMessageBox::critical(this, "Erreur", "Erreur de base de données.");
        nettoyerChamps();
        return;
    }

    if (query.next()) {
        int id = query.value(0).toInt();
        QString nom = query.value(1).toString();
        QString statut = query.value(2).toString();
        QByteArray storedHash = query.value(3).toByteArray();

        QByteArray inputHash = QCryptographicHash::hash(password_user.toUtf8(), QCryptographicHash::Sha256); // Hachage SHA256

        if (inputHash == storedHash) {
            msgBox.showInformation("", "Tongasoa " + nom); // Message d'accueil plus générique
            App *app = new App(id, statut, this); // Passer l'ID et le statut au constructeur de App et le MainWindow comme parent
            connect(app, &App::deconnexion, this, &MainWindow::afficher);
            app->show();
            this->hide(); // Cacher la fenêtre de connexion au lieu de la fermer
        } else {
            msgBox.showError("", "Mot de passe incorrect !");
        }
    } else {
        msgBox.showError("", "Nom d'utilisateur incorrect !");
    }

    nettoyerChamps();
}

void MainWindow::fermerFenetre(){
    this->close();
}
void MainWindow::afficher()
{
    this->show();
}

MainWindow::~MainWindow()
{
    DatabaseManager::getDatabase().close(); // Fermer la base de données à la fermeture de l'application
    delete ui;
}
