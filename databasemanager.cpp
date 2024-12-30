#include "databasemanager.h"

QSqlDatabase& DatabaseManager::getDatabase() {
    static QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "unique_connection_name");
    if (!db.isOpen()) {
        db.setDatabaseName("C:/db_test/login.db");
        if (!db.open()) {
            qDebug() << "Erreur : Impossible d'ouvrir la base de données";
        }
    }
    return db;
}

void DatabaseManager::closeDatabase() {
    QSqlDatabase::removeDatabase("unique_connection_name");
}
