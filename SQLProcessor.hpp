//
//  SQLProcessor.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CommandProcessor.hpp"
#include "Database.hpp"
#include "Tokenizer.hpp"
#include "Schema.hpp"
#include "Filters.hpp"

class Statement;
class Database; //define this later...

namespace ECE141 {

  class SQLProcessor : public CommandProcessor {
  public:
    
    SQLProcessor(CommandProcessor *aNext=nullptr);
    virtual ~SQLProcessor();
    
    virtual Statement*    getStatement(Tokenizer &aTokenizer);
    virtual StatusResult  interpret(Statement &aStatement);
    
    StatusResult createTable(Schema* aSchema, std::ostream &anOutput);
    StatusResult useTable(std::string &aName, std::ostream &anOutput);
    StatusResult dropTable(std::string &aName, std::ostream &anOutput);
    StatusResult describeTable(std::string &aName, std::ostream &anOutput);
    StatusResult showTables(std::ostream &anOutput);
    
    StatusResult insertRow(std::string tableName, std::vector<Row>& valueList, std::ostream &anOutput);
    StatusResult deleteRow(std::string aName, Expressions& whereContent, std::ostream &anOutput);
    StatusResult selectRow(std::string name, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent, joinsList &joins, std::ostream &anOutput);
    StatusResult updateRow(std::string aName, KeyValues& updates, Expressions& whereContent, std::ostream &anOutput);

  };

}
#endif /* SQLProcessor_hpp */
