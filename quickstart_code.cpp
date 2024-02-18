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
            std::string schema = R"(
                                define

                                email sub attribute, value string;
                                name sub attribute, value string;
                                friendship sub relation, relates friend;
                                user sub entity,
                                    owns email @key,
                                    owns name,
                                    plays friendship:friend;
                                admin sub user;)";
            tx.query.define(schema, options).get();
            tx.commit();
        }
        {
            auto session = driver.session(dbName, TypeDB::SessionType::DATA, options);
            {
                auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
                std::string insertQuery = R"(
                                            insert
                                            $user1 isa user, has name "Alice", has email "alice@vaticle.com";
                                            $user2 isa user, has name "Bob", has email "bob@vaticle.com";
                                            $user3 isa user, has name "Charlie", has email "charlie@vaticle.com";)";
                (void) tx.query.insert(insertQuery, options);
                tx.commit();
            }
            {
                auto tx = session.transaction(TypeDB::TransactionType::READ, options);
                TypeDB::JSONIterable result = tx.query.fetch("match $u isa user, has name 'Bob'; fetch $u: name, email;", options);
                std::string res;
                for (TypeDB::JSON json : result) {
                    //std::cout << json.toString() << std::endl;
                    res.append(json.toString());
                }
                std::cout << res << std::endl;
                tx.close();
            }
        }
    } catch (TypeDB::DriverException e ) {
        std::cout << "Caught TypeDB::DriverException: " << e.code() << "\n" << e.message()  << std::endl;
        return 2;
    }
    return 0;
}


