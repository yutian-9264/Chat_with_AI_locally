#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QString>


class DBManager
{
public:
    DBManager();

    static DBManager& instance();
};

#endif // DBMANAGER_H
