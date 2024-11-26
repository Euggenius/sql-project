#include <iostream>
#include "database.h"

int main() {
    database::Database db;

    std::string create_table_query = "create table users(id: int32, name: string[50], is_admin: bool = false)";
    auto result = db.execute_query(create_table_query);
    if (!result.is_ok()) {
        std::cerr << "Error: " << result.get_error_message() << std::endl;
        return 1;
    }

    std::string insert_query = "insert (1, \"Alice\", true) to users";
    result = db.execute_query(insert_query);
    if (!result.is_ok()) {
        std::cerr << "Error: " << result.get_error_message() << std::endl;
        return 1;
    }

    std::string insert_query2 = "insert (2, \"Bob\") to users";
    result = db.execute_query(insert_query2);
    if (!result.is_ok()) {
        std::cerr << "Error: " << result.get_error_message() << std::endl;
        return 1;
    }

    std::string select_query = "select id, name, is_admin from users";
    result = db.execute_query(select_query);
    if (!result.is_ok()) {
        std::cerr << "Error: " << result.get_error_message() << std::endl;
        return 1;
    }

    for (const auto& row : result) {
        int id = std::get<int32_t>(row.at("id"));
        std::string name = std::get<std::string>(row.at("name"));
        bool is_admin = std::get<bool>(row.at("is_admin"));
        std::cout << "ID: " << id << ", Name: " << name << ", Is Admin: " << std::boolalpha << is_admin << std::endl;
    }

    return 0;
}
