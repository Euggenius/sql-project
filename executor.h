#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "database.h"
#include "parser.h"

namespace database {

class Executor {
public:
    Executor(Database& db);

    QueryResult execute(const std::string& query);

private:
    Database& db_;

    QueryResult execute_create_table(const CreateTableQuery& query);
    QueryResult execute_insert(const InsertQuery& query);
    QueryResult execute_select(const SelectQuery& query);
};

}

#endif
