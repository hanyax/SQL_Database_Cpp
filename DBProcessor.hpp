//
//  DBProcessor.hpp
//  assign1
//
//  Created by patrick cheng on 4/13/20.
//  Copyright © 2020 pu cheng. All rights reserved.
//

#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "Storage.hpp"
#include "CommandProcessor.hpp"
#include "Statement.hpp"
#include "Database.hpp"


namespace ECE141 {
  class DBProcessor : public CommandProcessor {
      
  public:
    DBProcessor(CommandProcessor* aNext = nullptr);
    virtual ~DBProcessor();
        
    Database* getActiveDatabase();
    void setActiveDatabase(Database* DB);
    
    virtual Statement*    getStatement(Tokenizer &aTokenizer);
    virtual StatusResult  interpret(Statement &aStatement);
    
    StatusResult showIndex(std::ostream& anOutput);
    
  protected:
    ECE141::Database *curDB;
  };
}

#endif /* DBProcessor_hpp */
