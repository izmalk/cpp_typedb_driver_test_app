#include <iostream>

#include <typedb.hpp>

int main() {
    try {
        TypeDB::Driver driver = TypeDB::Driver::cloudDriver({"127.0.0.1:1729"}, TypeDB::Credential("admin", "password", false));
        std::string dbName = "test_cpp";
        if (driver.databases.contains(dbName)) {
            std::cout << "The DB " + dbName + " exists already. Deleting..." << std::endl;
            driver.databases.get(dbName).deleteDatabase();
            std::cout << "DB deleted." << std::endl;
        }
        if (driver.databases.contains(dbName)) return 1;
        driver.databases.create(dbName);
        std::cout << "Created a new DB." << std::endl;
        if (!driver.databases.contains(dbName)) return 1;
        std::cout << "A new DB is ready." << std::endl;
        TypeDB::Options options;
        {
            auto session = driver.session(dbName, TypeDB::SessionType::SCHEMA, options);
            auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
            tx.query.define("define\n"
                            "\n"
                            "id sub attribute, value long;\n"
                            "email sub attribute, value string;\n"
                            "full-name sub attribute, value string;\n"
                            "\n"
                            "user sub entity,\n"
                            "    owns id @key,\n"
                            "    owns email,\n"
                            "    abstract;\n"
                            "\n"
                            "person sub user, owns full-name;", options).get();
            tx.commit();
            std::cout << "Schema is defined." << std::endl;
        }

        {
            std::cout << "Inserting initial data." << std::endl;
            auto session = driver.session(dbName, TypeDB::SessionType::DATA, options);
            auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
            TypeDB::ConceptMapIterable result = tx.query.insert("insert "
                            "$p1 isa person, has id 1, has full-name \"Masako Holley\", has email \"masako.holley@vaticle.com\";\n"
                            "$p2 isa person, has id 2, has full-name \"Pearle Goodman\", has email \"pearle.goodman@vaticle.com\";\n"
                            "$p3 isa person, has id 3, has full-name \"Kevin Morrison\", has email \"kevin.morrison@vaticle.com\";", options);
            std::cout << "Inserted people:\n" << std::endl;
            for (TypeDB::ConceptMap& cm : result) {
                std::unique_ptr<TypeDB::Concept> v1 = cm.get("_1");
                std::cout << "Full name #1: " << (v1->asAttribute()->getValue()->asString()) << std::endl;
                std::unique_ptr<TypeDB::Concept> v2 = cm.get("_4");
                std::cout << "Full name #2: " << (v2->asAttribute()->getValue()->asString()) << std::endl;
                std::unique_ptr<TypeDB::Concept> v3 = cm.get("_7");
                std::cout << "Full name #3: " << (v3->asAttribute()->getValue()->asString()) << std::endl;
            }
            tx.commit();
        }

        {
            std::cout << "Reading data: get all string value-type attributes of all people in a database" << std::endl;
            auto session = driver.session(dbName, TypeDB::SessionType::DATA, options);
            auto tx = session.transaction(TypeDB::TransactionType::READ, options);

            TypeDB::ConceptMapGroupIterable result = tx.query.getGroup("match $p isa person, has $a; $a isa $t; $t value string; get; group $p;", options);
            for (TypeDB::ConceptMapGroup& cmg : result) {
                std::cout << "\nFound a person with the following string attributes:" << std::endl;
                for(TypeDB::ConceptMap& cm : cmg.conceptMaps()){
                    std::unique_ptr<TypeDB::Concept> t = cm.get("a");
                    std::cout << t->asAttribute()->getType()->getLabel() << " : " << (t->asAttribute()->getValue()->asString()) << std::endl;
                }
            }
            tx.close();
        }

    } catch (TypeDB::DriverException e ) {
        std::cout << "Caught TypeDB::DriverException: " << e.code() << "\n" << e.message()  << std::endl;
        return 2;
    }
    return 0;
}
