#include "parser.h"
#include <cctype>
#include <sstream>

namespace database {

Parser::Parser(const std::string& query) : query_(query), pos_(0) {}

void Parser::skip_whitespace() {
    while (pos_ < query_.size() && isspace(static_cast<unsigned char>(query_[pos_]))) {
        ++pos_;
    }
}

std::string Parser::parse_identifier() {
    skip_whitespace();
    size_t start = pos_;
    if (pos_ < query_.size() && (isalpha(query_[pos_]) || query_[pos_] == '_')) {
        ++pos_;
        while (pos_ < query_.size() && (isalnum(query_[pos_]) || query_[pos_] == '_')) {
            ++pos_;
        }
        return query_.substr(start, pos_ - start);
    }
    return "";
}

bool Parser::expect_char(char c) {
    skip_whitespace();
    if (pos_ < query_.size() && query_[pos_] == c) {
        ++pos_;
        return true;
    }
    return false;
}

bool Parser::expect_keyword(const std::string& keyword) {
    skip_whitespace();
    size_t len = keyword.size();
    if (query_.substr(pos_, len) == keyword) {
        pos_ += len;
        return true;
    }
    return false;
}

std::optional<Value> Parser::parse_value() {
    skip_whitespace();
    if (pos_ >= query_.size()) {
        return std::nullopt;
    }

    if (query_[pos_] == '"') {
        ++pos_;
        std::string value;
        while (pos_ < query_.size() && query_[pos_] != '"') {
            value += query_[pos_++];
        }
        if (pos_ < query_.size() && query_[pos_] == '"') {
            ++pos_;
            return value;
        } else {
            return std::nullopt;
        }
    }

    if (isdigit(query_[pos_]) || query_[pos_] == '-' || query_[pos_] == '+') {
        size_t start = pos_;
        if (query_[pos_] == '-' || query_[pos_] == '+') {
            ++pos_;
        }
        while (pos_ < query_.size() && isdigit(query_[pos_])) {
            ++pos_;
        }
        int32_t value = std::stoi(query_.substr(start, pos_ - start));
        return value;
    }

    std::string word = parse_identifier();
    if (word == "true") {
        return true;
    } else if (word == "false") {
        return false;
    }

    return std::nullopt;
}

std::optional<Column> Parser::parse_column_definition() {
    skip_whitespace();
    std::string name = parse_identifier();
    if (name.empty()) {
        return std::nullopt;
    }
    if (!expect_char(':')) {
        return std::nullopt;
    }
    std::string type_str = parse_identifier();
    DataType type;
    size_t size = 0;

    if (type_str == "int32") {
        type = DataType::INT32;
    } else if (type_str == "bool") {
        type = DataType::BOOL;
    } else if (type_str == "string") {
        type = DataType::STRING;
        if (expect_char('[')) {
            std::string size_str;
            while (pos_ < query_.size() && isdigit(query_[pos_])) {
                size_str += query_[pos_++];
            }
            if (!expect_char(']')) {
                return std::nullopt;
            }
            size = std::stoul(size_str);
        }
    } else if (type_str == "bytes") {
        type = DataType::BYTES;
        if (expect_char('[')) {
            std::string size_str;
            while (pos_ < query_.size() && isdigit(query_[pos_])) {
                size_str += query_[pos_++];
            }
            if (!expect_char(']')) {
                return std::nullopt;
            }
            size = std::stoul(size_str);
        }
    } else {
        return std::nullopt;
    }

    std::optional<Value> default_value = std::nullopt;
    if (expect_char('=')) {
        default_value = parse_value();
        if (!default_value.has_value()) {
            return std::nullopt;
        }
    }

    return Column{name, type, size, default_value};
}

std::optional<ParsedQuery> Parser::parse() {
    skip_whitespace();
    std::string command = parse_identifier();
    if (command == "create") {
        if (!expect_keyword("table")) {
            return std::nullopt;
        }
        std::string table_name = parse_identifier();
        if (table_name.empty()) {
            return std::nullopt;
        }
        if (!expect_char('(')) {
            return std::nullopt;
        }
        std::vector<Column> columns;
        while (true) {
            auto column = parse_column_definition();
            if (!column.has_value()) {
                return std::nullopt;
            }
            columns.push_back(column.value());
            if (expect_char(')')) {
                break;
            }
            if (!expect_char(',')) {
                return std::nullopt;
            }
        }
        return CreateTableQuery{table_name, columns};
    } else if (command == "insert") {
        if (!expect_char('(')) {
            return std::nullopt;
        }
        std::vector<std::optional<Value>> values;
        while (true) {
            auto value = parse_value();
            values.push_back(value);
            if (expect_char(')')) {
                break;
            }
            if (!expect_char(',')) {
                return std::nullopt;
            }
        }
        if (!expect_keyword("to")) {
            return std::nullopt;
        }
        std::string table_name = parse_identifier();
        if (table_name.empty()) {
            return std::nullopt;
        }
        return InsertQuery{table_name, values};
    } else if (command == "select") {
        std::vector<std::string> column_names;
        while (true) {
            std::string column_name = parse_identifier();
            if (column_name.empty()) {
                return std::nullopt;
            }
            column_names.push_back(column_name);
            if (!expect_char(',')) {
                break;
            }
        }
        if (!expect_keyword("from")) {
            return std::nullopt;
        }
        std::string table_name = parse_identifier();
        if (table_name.empty()) {
            return std::nullopt;
        }
        return SelectQuery{table_name, column_names};
    }

    return std::nullopt;
}

}
