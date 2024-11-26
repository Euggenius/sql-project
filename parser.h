#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <optional>
#include <vector>
#include <variant>

#include "database.h"

namespace database {

enum class QueryType {
    CREATE_TABLE,
    INSERT,
    SELECT
};

struct CreateTableQuery {
    std::string table_name;
    std::vector<Column> columns;
};

struct InsertQuery {
    std::string table_name;
    std::vector<std::optional<Value>> values;
};

struct SelectQuery {
    std::string table_name;
    std::vector<std::string> column_names;
};

using ParsedQuery = std::variant<CreateTableQuery, InsertQuery, SelectQuery>;

class Parser {
public:
    Parser(const std::string& query);

    std::optional<ParsedQuery> parse();

private:
    std::string query_;
    size_t pos_;

    void skip_whitespace();
    std::string parse_identifier();
    bool expect_char(char c);
    bool expect_keyword(const std::string& keyword);
    std::optional<Value> parse_value();
    std::optional<Column> parse_column_definition();
};

} 

#endif 
