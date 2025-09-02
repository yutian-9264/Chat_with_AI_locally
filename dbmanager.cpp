#include "dbmanager.h"

DBManager::DBManager()
{

}

DBManager& DBManager::instance() {
    static DBManager inst;
    return inst;
}
