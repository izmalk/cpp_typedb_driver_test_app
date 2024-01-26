#include <iostream>
#include <typedb_driver.hpp>

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
                TypeDB::ConceptMapIterable result = tx.query.get("match $p isa person, has name $n; get $n;", options);
                for (TypeDB::ConceptMap& cm : result) {
                    std::unique_ptr<TypeDB::Concept> t = cm.get("n");
                    std::cout << t->asAttribute()->getType()->getLabel() << " : " << (t->asAttribute()->getValue()->asString()) << std::endl;
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
