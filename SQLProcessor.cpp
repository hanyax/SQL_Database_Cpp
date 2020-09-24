//
//  SQLProcessor.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "SQLProcessor.hpp"
#include "SQLStatements.hpp"

namespace ECE141 {
  using StatementFactory = Statement* (*) (SQLProcessor& processor);

  SQLProcessor::SQLProcessor(CommandProcessor* aNext) : CommandProcessor(aNext) {}

  SQLProcessor::~SQLProcessor() {}

  // USE: -----------------------------------------------------
  StatusResult SQLProcessor::interpret(Statement& aStatement) {
    return aStatement.run(std::cout);
  }
  
  Statement* createTableStatementFactory(SQLProcessor& aProcessor) {
    return new createTableStatement(aProcessor);
  }

  Statement* useTableStatementFactory(SQLProcessor& aProcessor) {
    return new useTableStatement(aProcessor);
  }

  Statement* describeTableStatementFactory(SQLProcessor& aProcessor) {
    return new describeTableStatement(aProcessor);
  }

  Statement* showTableStatementFactory(SQLProcessor& aProcessor) {
    return new showTableStatement(aProcessor);
  }

  Statement* dropTableStatementFactory(SQLProcessor& aProcessor) {
    return new dropTableStatement(aProcessor);
  }

  Statement* insertRowStatementFactory(SQLProcessor &aProcessor) {
    return new insertRowStatement(aProcessor);
  }

  Statement* deleteRowStatementFactory(SQLProcessor &aProcessor) {
    return new deleteRowStatement(aProcessor);
  }

  Statement* selectStatementFactory(SQLProcessor &aProcessor) {
    return new selectSatatement(aProcessor);
  }

  Statement* updateStatementFactory(SQLProcessor &aProcessor) {
    return new updateSatatement(aProcessor);
  }
  
  // USE: factory to create statement based on given tokens...
  Statement* SQLProcessor::getStatement(Tokenizer& aTokenizer) {
    static std::map<Keywords, StatementFactory> factories = {
      {Keywords::create_kw, createTableStatementFactory},
      {Keywords::use_kw, useTableStatementFactory},
      {Keywords::describe_kw, describeTableStatementFactory},
      {Keywords::show_kw, showTableStatementFactory},
      {Keywords::drop_kw, dropTableStatementFactory},
      {Keywords::insert_kw, insertRowStatementFactory},
      {Keywords::delete_kw, deleteRowStatementFactory},
      {Keywords::select_kw, selectStatementFactory},
      {Keywords::update_kw, updateStatementFactory},
    };
        
    if (aTokenizer.size()) {
      Token theToken = aTokenizer.tokenAt(0);
      // Handle database commands
      if ((factories.count(theToken.keyword) && (aTokenizer.tokenAt(1).keyword == Keywords::table_kw || aTokenizer.tokenAt(1).keyword == Keywords::tables_kw)) ||
          (aTokenizer.tokenAt(0).keyword == Keywords::describe_kw && aTokenizer.tokenAt(1).keyword != Keywords::database_kw) ||
          (aTokenizer.tokenAt(0).keyword == Keywords::insert_kw && aTokenizer.tokenAt(1).keyword == Keywords::into_kw) ||
          (aTokenizer.tokenAt(0).keyword == Keywords::delete_kw && aTokenizer.tokenAt(1).keyword == Keywords::from_kw) ||
          (aTokenizer.tokenAt(0).keyword == Keywords::select_kw) ||
          (aTokenizer.tokenAt(0).keyword == Keywords::update_kw) ) {
        if (Statement* theStatement = factories[theToken.keyword](*this)) {
          if (StatusResult theResule = theStatement->parse(aTokenizer)) {
            return theStatement;
          }
        }
      }
    }
    return nullptr;
  }

  StatusResult SQLProcessor::createTable(Schema* aSchema, std::ostream &anOutput) {
    if (Database* curDB = getActiveDatabase()) {
      StatusResult theResult = curDB->createTable(aSchema);
      if (theResult) {
        anOutput << "Created Table " << aSchema->getName() << "\n";
        return theResult;
      }
      delete aSchema;
      return theResult;
    }
    return StatusResult(noDatabaseSpecified);
  }


  StatusResult SQLProcessor::dropTable(std::string &aName, std::ostream &anOutput) {
    if (Database* curDB = getActiveDatabase()) {
      if (curDB->getSchemaIndexByName(aName) != -1) {
        StatusResult result = curDB->dropTable(aName);
        if (result.code == Errors::noError) {
          anOutput << "Table " << aName << " dropped\n";
        }
        return result;
      }
      return StatusResult(unknownTable);
    }
    return StatusResult(noDatabaseSpecified);
  }


  StatusResult SQLProcessor::describeTable(std::string &aName, std::ostream &anOutput) {
    if (Database* curDB = getActiveDatabase()) {
      StatusResult theResult = curDB->describeTable(aName, anOutput);
      return theResult;
    } else {
      return StatusResult(noDatabaseSpecified);
    }
  }

  StatusResult SQLProcessor::showTables(std::ostream &anOutput) {
    if (Database* curDB = getActiveDatabase()) {
      StatusResult theResult = curDB->showTables(anOutput);
      return theResult;
    } else {
      return StatusResult(noDatabaseSpecified);
    }
  }

  StatusResult SQLProcessor::useTable(std::string &aName, std::ostream &anOutput) {
    return StatusResult();
  }

  StatusResult SQLProcessor::insertRow(std::string tableName, std::vector<Row>& valueList, std::ostream &anOutput) {
    StatusResult result;
    Database* curDB = getActiveDatabase();
    if (curDB != nullptr) {
      result = curDB->insertRow(tableName, valueList);
    } else {
      return StatusResult(Errors::noDatabaseSpecified);
    }
    if (result.code == Errors::noError) {
      anOutput << "Insert Completed\n";
    }
    return result;
  }

  StatusResult SQLProcessor::deleteRow(std::string aName, Expressions& whereContent, std::ostream &anOutput) { 
    StatusResult result;
    Database* curDB = getActiveDatabase();
    if (curDB != nullptr) {
      double time = 0.0;
      int numRow = 0;
      result = curDB->deleteRow(aName, time, numRow, whereContent);
      if (result.code == Errors::noError) {
        anOutput << numRow << " rows affected (" << time << " ms.)\n";
      }
      return result;
    } else {
      return StatusResult(Errors::noDatabaseSpecified);
    }
  }

  StatusResult SQLProcessor::selectRow(std::string name, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent, joinsList &joins, std::ostream &anOutput) {
    Database* curDB = getActiveDatabase();
    if (curDB != nullptr) {
      if(selectAll) {
        std::clog << "in SQLProcessor: selectAll is true \n";
      }
      return curDB->selectRow(name, attrlist, selectAll, orderByAttr, limitNum, whereContent, joins, anOutput);
    } else {
      return StatusResult(Errors::noDatabaseSpecified);
    }
  }

  StatusResult SQLProcessor::updateRow(std::string aName, KeyValues& updates, Expressions& whereContent, std::ostream &anOutput) {
    Database* curDB = getActiveDatabase();
    if (curDB != nullptr) {
      double time = 0.0;
      int numRow = 0 ;
      StatusResult result = curDB->updateRow(aName, time, numRow, updates, whereContent);
      if (result.code == Errors:: noError) {
        anOutput << numRow << " rows affected (" << time << " ms.)\n";
      }
      return result;
    } else {
      return StatusResult(Errors::noDatabaseSpecified);
    }
  }
}
