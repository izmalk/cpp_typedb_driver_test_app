#include <iostream>
#include <typedb_driver.hpp>

int main() {
    try {
        TypeDB::Driver driver = TypeDB::Driver::coreDriver("127.0.0.1:1729");
        TypeDB::Options options;
        {
            auto session = driver.session("sample_db", TypeDB::SessionType::SCHEMA, options);
            auto tx = session.transaction(TypeDB::TransactionType::WRITE, options);
            auto tag = tx.concepts.putAttributeType("tag", TypeDB::ValueType::STRING).get();
            auto root_entity = tx.concepts.getRootEntityType();
            auto entities = root_entity -> getSubtypes(tx, TypeDB::Transitivity::EXPLICIT);
            for (auto& entity : entities) {
                    //std::cout << entity -> getLabel() << " : " << std::endl;
                    if (!(entity->isAbstract())) {
                        (void) entity->setOwns(tx, tag.get());
                    }
                }
            tx.commit();
        }
    } catch (TypeDB::DriverException e ) {return 2;}
    return 0;
}


