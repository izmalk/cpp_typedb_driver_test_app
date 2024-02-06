#include <iostream>
#include <typedb_driver.hpp>

int main() {
    const std::string dbName = "sample_db";
    const std::string ServerAddr = "127.0.0.1:1729";
    try {
        TypeDB::Driver driver = TypeDB::Driver::coreDriver(ServerAddr);
        TypeDB::Options options;
        {
            auto session = driver.session(dbName, TypeDB::SessionType::SCHEMA, options);
            auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
            auto tag = tx.concepts.putAttributeType("tag", TypeDB::ValueType::STRING).get();
            auto root_entity = tx.concepts.getRootEntityType();
            auto entities = root_entity -> getSubtypes(tx, TypeDB::Transitivity::EXPLICIT);
            for (auto& entity : entities) {
                    //std::cout << entity -> getLabel() << " : " << std::endl;
                    entity->setOwns(tx, tag.get());
                }
            tx.commit();
        }
    } catch (TypeDB::DriverException e ) {
        std::cout << "Caught TypeDB::DriverException: " << e.code() << "\n" << e.message()  << std::endl;
        return 2;
    }
    return 0;
}


