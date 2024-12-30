#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager
{
public:
    static QSqlDatabase& getDatabase();
    static void closeDatabase();
    static void enregistrerLivre(const QString &titre, const QString &genre, const QString &auteur, const QString &maison_edition, const QString &proprietes, int quantite, const QString &armoire, const QString &identifiant, const QString &date);
};

#endif // DATABASEMANAGER_H
