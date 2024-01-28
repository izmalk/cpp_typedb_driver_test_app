#include <iostream>
#include <nlohmann/json.hpp>
#include <typedb_driver.hpp>

void printJson(const nlohmann::json& json, const std::string& prefix = "") {
    if (json.is_object()) {
        for (const auto& [key, value] : json.items()) {
            printJson(value, prefix + key + ":");
        }
    } else if (json.is_array()) {
        for (std::size_t i = 0; i < json.size(); ++i) {
            printJson(json[i], prefix + "[" + std::to_string(i) + "]:");
        }
    } else {
        std::cout << prefix << " " << json << std::endl;
    }
}

int main() {
    const std::string dbName = "people5";
    const std::string ServerAddr = "127.0.0.1:1729";
    try {
        TypeDB::Driver driver = TypeDB::Driver::coreDriver(ServerAddr);
        if (driver.databases.contains(dbName)) {
            driver.databases.get(dbName).deleteDatabase();
        }
        driver.databases.create(dbName);
        TypeDB::Options options;
        {
            auto session = driver.session(dbName, TypeDB::SessionType::SCHEMA, options);
            auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
            //std::string
            tx.query.define("define person sub entity;", options).get();
            tx.query.define("define name sub attribute, value string; person owns name;", options).get();
            tx.commit();
        }
        {
            auto session = driver.session(dbName, TypeDB::SessionType::DATA, options);
            {
                auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
                (void) tx.query.insert("insert $p isa person, has name 'Alice';", options);
                (void) tx.query.insert("insert $p isa person, has name 'Bob';", options);
                tx.commit();
            }
            {
                auto tx = session.transaction(TypeDB::TransactionType::READ, options);
                TypeDB::JSONIterable result = tx.query.fetch("match $p isa person; fetch $p: name;", options);
                for (TypeDB::JSON json : result) {
                    printJson(json.toString());
                }
                tx.close();
            }
        }
    } catch (TypeDB::DriverException e ) {
        std::cout << "Caught TypeDB::DriverException: " << e.code() << "\n" << e.message()  << std::endl;
        return 2;
    }
    return 0;
}


