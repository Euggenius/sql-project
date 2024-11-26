#include "executor.h"

namespace database {

Executor::Executor(Database& db) : db_(db) {}

QueryResult Executor::execute(const std::string& query) {
    Parser parser(query);
    auto parsed_query_opt = parser.parse();
    if (!parsed_query_opt.has_value()) {
        return QueryResult("Failed to parse query");
    }

    auto& parsed_query = parsed_query_opt.value();

    if (std::holds_alternative<CreateTableQuery>(parsed_query)) {
        return execute_create_table(std::get<CreateTableQuery>(parsed_query));
    } else if (std::holds_alternative<InsertQuery>(parsed_query)) {
        return execute_insert(std::get<InsertQuery>(parsed_query));
    } else if (std::holds_alternative<SelectQuery>(parsed_query)) {
        return execute_select(std::get<SelectQuery>(parsed_query));
    } else {
        return QueryResult("Unsupported query type");
    }
}

QueryResult Executor::execute_create_table(const CreateTableQuery& query) {
    if (db_.tables_.find(query.table_name) != db_.tables_.end()) {
        return QueryResult("Table already exists: " + query.table_name);
    }
    Database::Table table;
    table.name = query.table_name;
    table.columns = query.columns;
    db_.tables_[query.table_name] = std::move(table);
    return QueryResult();
}

QueryResult Executor::execute_insert(const InsertQuery& query) {
    auto it = db_.tables_.find(query.table_name);
    if (it == db_.tables_.end()) {
        return QueryResult("Table not found: " + query.table_name);
    }
    auto& table = it->second;
    if (query.values.size() > table.columns.size()) {
        return QueryResult("Too many values");
    }
    Row row;
    size_t i = 0;
    for (; i < query.values.size(); ++i) {
        const auto& column = table.columns[i];
        const auto& value_opt = query.values[i];
        if (value_opt.has_value()) {
            row[column.name] = value_opt.value();
        } else if (column.default_value.has_value()) {
            row[column.name] = column.default_value.value();
        } else {
            return QueryResult("Missing value for column: " + column.name);
        }
    }
    for (; i < table.columns.size(); ++i) {
        const auto& column = table.columns[i];
        if (column.default_value.has_value()) {
            row[column.name] = column.default_value.value();
        } else {
            return QueryResult("Missing value for column: " + column.name);
        }
    }
    table.rows.push_back(std::move(row));
    return QueryResult();
}

QueryResult Executor::execute_select(const SelectQuery& query) {
    auto it = db_.tables_.find(query.table_name);
    if (it == db_.tables_.end()) {
        return QueryResult("Table not found: " + query.table_name);
    }
    auto& table = it->second;
    std::vector<Row> result_rows;
    for (const auto& row : table.rows) {
        Row result_row;
        for (const auto& col_name : query.column_names) {
            auto col_it = row.find(col_name);
            if (col_it != row.end()) {
                result_row[col_name] = col_it->second;
            } else {
                return QueryResult("Column not found: " + col_name);
            }
        }
        result_rows.push_back(std::move(result_row));
    }
    QueryResult result;
    result.set_rows(std::move(result_rows));
    return result;
}

}
