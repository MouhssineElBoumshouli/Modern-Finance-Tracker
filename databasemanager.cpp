#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initialize()
{
    db = QSqlDatabase::addDatabase("QSQLITE");

    // Get the application directory path
    QString appDir = QCoreApplication::applicationDirPath();
    QString dbPath = appDir + "/finance_tracker.db";

    // Create the directory if it doesn't exist
    QDir dir(appDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    qDebug() << "Database path:" << dbPath;  // Debug line to see the path
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
        qDebug() << "Error details:" << db.lastError().text();
        return false;
    }

    if (!createTables()) {
        qDebug() << "Error: failed to create tables";
        return false;
    }

    qDebug() << "Database initialized successfully";
    return true;
}
bool DatabaseManager::deleteTransaction(const QString& datetime, double amount, const QString& description)
{
    QSqlQuery query;
    query.prepare("DELETE FROM transactions WHERE datetime = :datetime AND amount = :amount AND description = :description");
    query.bindValue(":datetime", datetime);
    query.bindValue(":amount", amount);
    query.bindValue(":description", description);

    if (!query.exec()) {
        qDebug() << "Error deleting transaction:" << query.lastError().text();
        return false;
    }

    return true;
} //The deleteTransaction method in the DatabaseManager class deletes a specific transaction from the database based on its datetime, amount, and description. It prepares an SQL DELETE query with placeholders, binds the input values to prevent SQL injection, and executes the query. If the deletion succeeds, it returns true; otherwise, it logs an error and returns false. This method ensures precise and secure deletion of transactions while providing clear error handling for debugging.
bool DatabaseManager::createTables()
{
    QSqlQuery query;
    QString createTableQuery =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type INTEGER NOT NULL,"
        "amount REAL NOT NULL,"
        "description TEXT,"
        "category TEXT,"
        "datetime TEXT NOT NULL"
        ")";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Tables created successfully";
    return true;
}

bool DatabaseManager::addTransaction(const Transaction& transaction)
{
    QSqlQuery query;
    query.prepare("INSERT INTO transactions (type, amount, description, category, datetime) "
                  "VALUES (:type, :amount, :description, :category, :datetime)");

    query.bindValue(":type", transaction.type());
    query.bindValue(":amount", transaction.amount());
    query.bindValue(":description", transaction.description());
    query.bindValue(":category", transaction.category());
    query.bindValue(":datetime", transaction.datetime().toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Error adding transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

QVector<Transaction> DatabaseManager::getAllTransactions()
{
    QVector<Transaction> transactions;
    QSqlQuery query("SELECT * FROM transactions ORDER BY datetime DESC");

    while (query.next()) {
        Transaction::Type type = static_cast<Transaction::Type>(query.value("type").toInt());
        double amount = query.value("amount").toDouble();
        QString description = query.value("description").toString();
        QString category = query.value("category").toString();
        QDateTime datetime = QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate);

        transactions.append(Transaction(type, amount, description, category, datetime));
    }

    return transactions;
}

double DatabaseManager::getTotalBalance()
{
    QSqlQuery query;
    query.exec("SELECT SUM(CASE WHEN type = 0 THEN amount ELSE -amount END) FROM transactions");

    if (query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double DatabaseManager::getTotalIncome()
{
    QSqlQuery query;
    query.exec("SELECT SUM(amount) FROM transactions WHERE type = 0");

    if (query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double DatabaseManager::getTotalExpenses()
{
    QSqlQuery query;
    query.exec("SELECT SUM(amount) FROM transactions WHERE type = 1");

    if (query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}
