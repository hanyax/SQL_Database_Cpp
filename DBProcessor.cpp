//
//  DBProcessor.cpp
//  assign1
//
//  Created by patrick cheng on 4/13/20.
//  Copyright © 2020 pu cheng. All rights reserved.
//

#include "DBProcessor.hpp"
#include "Tokenizer.hpp"
#include "Storage.hpp"
#include "DBStatements.hpp"
#include <fstream>
#include <filesystem>

namespace ECE141 {

  DBProcessor::DBProcessor(CommandProcessor* aNext) : CommandProcessor(aNext), curDB(nullptr){}
  DBProcessor::~DBProcessor() {
    if (curDB != nullptr) {
      delete curDB;
    }
  }

  // USE: -----------------------------------------------------
  StatusResult DBProcessor::interpret(Statement &aStatement) {
    return aStatement.run(std::cout);
  }
  
  // USE: factory to create statement based on given tokens...
  Statement* DBProcessor::getStatement(Tokenizer &aTokenizer) {
    Statement* aStatement = nullptr;
    
    if(aTokenizer.size() > 1 && Keywords::database_kw == aTokenizer.tokenAt(1).keyword){
      Token curToken = aTokenizer.tokenAt(0);
      switch(curToken.keyword) {
        case Keywords::create_kw : aStatement = new CreateDBStatement(); break;
        case Keywords::drop_kw : aStatement = new DropDBStatement(*this); break;
        case Keywords::use_kw : aStatement = new UseDBStatement(*this); break;
        case Keywords::describe_kw: aStatement = new DescribeDBStatement(*this); break;
        default: break;
      }
    }
    
    //show databases is valid while show database is not
    if(aTokenizer.size() > 1 && Keywords::show_kw == aTokenizer.tokenAt(0).keyword) {
      if (Keywords::databases_kw == aTokenizer.tokenAt(1).keyword) {
        aStatement = new ShowDBStatement();
      } else if (Keywords::indexes_kw == aTokenizer.tokenAt(1).keyword) {
        aStatement = new ShowIndexStatement(*this);
      }
    }
    
    StatusResult theResult;
    if (aStatement != nullptr) {
      theResult = aStatement->parse(aTokenizer);
      if (theResult.code == Errors::syntaxError) {
        return nullptr;
      }
    }
    
    return aStatement;
  }

  StatusResult DBProcessor::showIndex(std::ostream &anOutput) {
    Database* curDB = getActiveDatabase();
    if (curDB != nullptr) {
      return curDB->showIndex(anOutput);
    } else {
      return StatusResult(Errors::noDatabaseSpecified);
    }
  }

  Database* DBProcessor::getActiveDatabase() {
     return curDB;
  }

  void DBProcessor::setActiveDatabase(Database* DB) {
    curDB = DB;
  }

  
}
