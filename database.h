#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>

namespace database {

enum class DataType {
    INT32,
    BOOL,
    STRING,
    BYTES
};

using Value = std::variant<int32_t, bool, std::string, std::vector<uint8_t>>;

struct Column {
    std::string name;
    DataType type;
    size_t size;
    std::optional<Value> default_value;

    Column(const std::string& name, DataType type, size_t size = 0, const std::optional<Value>& default_value = std::nullopt);
};

using Row = std::unordered_map<std::string, Value>;

class QueryResult {
public:
    QueryResult() = default;
    QueryResult(const std::string& error_message);

    bool is_ok() const;
    const std::string& get_error_message() const;
    const std::vector<Row>& get_rows() const;

    std::vector<Row>::const_iterator begin() const;
    std::vector<Row>::const_iterator end() const;

    void set_rows(std::vector<Row>&& rows);

private:
    bool ok_{true};
    std::string error_message_;
    std::vector<Row> rows_;
};

class Database {
public:
    Database();
    QueryResult execute_query(const std::string& query);

private:
    struct Table {
        std::string name;
        std::vector<Column> columns;
        std::vector<Row> rows;
    };

    std::unordered_map<std::string, Table> tables_;

    friend class Executor;
};

}

#endif
