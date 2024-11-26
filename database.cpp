#include "database.h"
#include "executor.h"

namespace database {

Column::Column(const std::string& name, DataType type, size_t size, const std::optional<Value>& default_value)
    : name(name), type(type), size(size), default_value(default_value) {}

QueryResult::QueryResult(const std::string& error_message)
    : ok_(false), error_message_(error_message) {}

bool QueryResult::is_ok() const {
    return ok_;
}

const std::string& QueryResult::get_error_message() const {
    return error_message_;
}

const std::vector<Row>& QueryResult::get_rows() const {
    return rows_;
}

std::vector<Row>::const_iterator QueryResult::begin() const {
    return rows_.begin();
}

std::vector<Row>::const_iterator QueryResult::end() const {
    return rows_.end();
}

void QueryResult::set_rows(std::vector<Row>&& rows) {
    rows_ = std::move(rows);
}

Database::Database() = default;

QueryResult Database::execute_query(const std::string& query) {
    Executor executor(*this);
    return executor.execute(query);
}

}
