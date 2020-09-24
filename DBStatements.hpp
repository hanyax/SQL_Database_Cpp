//
//  DBStatements.hpp
//  Project2
//
//  Created by 徐 翰洋 on 4/13/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#ifndef DBStatements_hpp
#define DBStatements_hpp

#include "DBProcessor.hpp"
#include "Tokenizer.hpp"
#include "Storage.hpp"
#include <stdio.h>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include "Helpers.hpp"
#include "Database.hpp"
#include "Storage.hpp"

namespace ECE141 {
//---------------Create Database Statement----------------------
class CreateDBStatement : public Statement {
public:
  CreateDBStatement() {}
  
  StatusResult parse(Tokenizer& aTokenizer) {
    const int kMinLength = 3;
    StatusResult theResult{syntaxError};
    
    if (kMinLength <= aTokenizer.remaining()) {
      if (Keywords::database_kw == aTokenizer.peek(1).keyword) {
        if (Keywords::create_kw == aTokenizer.current().keyword) {
          aTokenizer.next();
          aTokenizer.next();
          dbName = aTokenizer.current().data;
          aTokenizer.next();
          return StatusResult();
        }
      }
    }
    return theResult;
  }

  StatusResult run(std::ostream& anOutput) {
    //creat a db file
      
    if(!Helpers::isValidName(dbName)) {
        return StatusResult(ECE141::illegalIdentifier);
    }
    std::string dbPath(ECE141::StorageInfo::getDefaultStoragePath());
    dbPath.append("/").append(dbName).append(".db");
      
    try {
        if (std::filesystem::exists(dbPath)) {
            std::cout << "database " << dbName << " already existed.\n";
            return StatusResult(ECE141::databaseExists);
        } else {
          Database db(dbName, CreateNewStorage());
          StatusResult res = db.createDatabase(dbName);
          return res;
        }
    } catch(const std::filesystem::filesystem_error& err) {
         std::cout << "filesystem error: " << err.what() << '\n';
         return StatusResult(ECE141::unknownError);
    }
    return StatusResult();
  }
  
private:
  std::string dbName;
};

class DropDBStatement : public Statement {
public:
  DropDBStatement(DBProcessor &processor) : curPro(processor) {};
  
  StatusResult parse(Tokenizer& aTokenizer) {
    const int kMinLength = 3;
    StatusResult theResult{syntaxError};
    if (kMinLength <= aTokenizer.remaining()) {
      if (Keywords::database_kw == aTokenizer.peek(1).keyword) {
        if (Keywords::drop_kw == aTokenizer.current().keyword) {
          aTokenizer.next();
          aTokenizer.next();
          dbName = aTokenizer.current().data;
          aTokenizer.next();
          return StatusResult();
        }
      }
    }
    return theResult;
  }
  
  StatusResult run(std::ostream& anOutput) {
    // delete a db file
    std::string dbFile(ECE141::StorageInfo::getDefaultStoragePath());
    dbFile.append("/").append(dbName).append(".db");
    try {
      if (std::filesystem::remove(dbFile)) {
        std::cout << "dropped database " << dbName << " (ok).\n";
      } else {
        std::cout << "database " << dbName << " not found.\n";
          return StatusResult(ECE141::unknownDatabase);
      }
    } catch(const std::filesystem::filesystem_error& err) {
       std::cout << "filesystem error: " << err.what() << '\n';
    }
    
    // if dropped db is current db, delete curDB poniter.
    // and set curPro.activeDB to nullptr
    Database* database = curPro.getActiveDatabase();
    if (database != nullptr && database->getName() == dbName) {
      delete database;
      curPro.setActiveDatabase(nullptr);
    }

    return StatusResult();
  }
  
  private:
    std::string dbName;
    DBProcessor& curPro;
};

  class UseDBStatement : public Statement {
  public:

    UseDBStatement(DBProcessor &curPro) : curPro(curPro) {}
    
    StatusResult parse(Tokenizer& aTokenizer) {
      const int kMinLength = 3;
      StatusResult theResult{syntaxError};
      
      if (kMinLength <= aTokenizer.remaining()) {
        if (Keywords::database_kw == aTokenizer.peek(1).keyword) {
          if (Keywords::use_kw == aTokenizer.current().keyword) {
            aTokenizer.next();
            aTokenizer.next();
            dbName = aTokenizer.current().data;
            aTokenizer.next();
            return StatusResult();
          }
        }
      }
      
      return theResult;
    }
    
    StatusResult run(std::ostream& anOutput) {
      // use a db file
      std::string dbFile(ECE141::StorageInfo::getDefaultStoragePath());
      dbFile.append("/").append(dbName).append(".db");
      try {
        Database* curDB = curPro.getActiveDatabase();
        if (curDB != nullptr && dbName == curDB->getName()) {
         std::cout << "Using database " << dbName << " \n";
          return StatusResult();
        } else if (std::filesystem::exists(dbFile)){
          if (curDB != nullptr) {
            // 1) Save current DB
            curDB->saveDatabase();
            // 2) Delete Current DB
            delete curDB;
          }
          // 3) Load new DB from file
          Database* newDB = new Database(dbName, OpenExistingStorage());
          newDB->loadTablesFromToc();
          
          // 4) update curDB;
          curPro.setActiveDatabase(newDB);
        } else {
              std::cout << "database " << dbName << " not found.\n";
              return StatusResult(ECE141::unknownDatabase);
          }
      }
      catch(const std::filesystem::filesystem_error& err) {
          std::cout << "filesystem error: " << err.what() << '\n';
          return StatusResult(ECE141::unknownError);
      }
      anOutput << "Using database " << dbName << "\n";
      return StatusResult();
    }
    
  private:
    std::string dbName;
    DBProcessor& curPro;
  };

  class ShowDBStatement : public Statement {
  public:
    ShowDBStatement(){}
    
    StatusResult parse(Tokenizer& aTokenizer) {
      const int kMinLength = 2;
      StatusResult theResult{syntaxError};
      
      if (kMinLength <= aTokenizer.remaining()) {
        if (Keywords::databases_kw == aTokenizer.peek(1).keyword) {
          if (Keywords::show_kw == aTokenizer.current().keyword) {
            aTokenizer.next();
            aTokenizer.next();
            return StatusResult();
          }
        }
      }
      
      return theResult;
    }
  
    StatusResult run(std::ostream& anOutput) {
      //print out all dbs
      //std::cout << "dbs: \n";
      std::string path(ECE141::StorageInfo::getDefaultStoragePath());
      for (const auto & entry : std::filesystem::directory_iterator(path)) {
        std::string file = entry.path();
        if (file.substr(file.length()-3,file.length()) == ".db") {
            std::cout << file.substr(path.length()+1,file.length()) << "\n";
        }
      }
      return StatusResult();
    }
  };

  class DescribeDBStatement : public Statement {
  public:
    DescribeDBStatement(DBProcessor& processor) : curPro(processor) {};
    
    StatusResult parse(Tokenizer& aTokenizer) {
      const int kMinLength = 3;
      StatusResult theResult{syntaxError};
      
      if (kMinLength <= aTokenizer.remaining()) {
        if (Keywords::database_kw == aTokenizer.peek(1).keyword) {
          if (Keywords::describe_kw == aTokenizer.current().keyword) {
            aTokenizer.next();
            aTokenizer.next();
            dbName = aTokenizer.current().data;
            aTokenizer.next();
            return StatusResult();
          }
        }
      }
      
      return theResult;
    }
    
    StatusResult run(std::ostream& anOutput) {
      //std::cout << "This is a non-standard command, that we use for testing\n";
      std::string dbFile(ECE141::StorageInfo::getDefaultStoragePath());
      dbFile.append("/").append(dbName).append(".db");
      try {
        if (std::filesystem::exists(dbFile)) {
          Database* db = curPro.getActiveDatabase();
          StatusResult res;
          if (db != nullptr && db->getName() == dbName) {
            res = db->describeDatabase(anOutput);
          } else {
            Database* curDB = new Database(dbName, OpenExistingStorage());
            curDB->loadTablesFromToc();
            res = curDB->describeDatabase(anOutput);
            delete curDB;
          }
          return res;
        }
        else {
            std::cout << "database " << dbName << " not found.\n";
            return StatusResult(ECE141::unknownDatabase);
        }
      }
      catch(const std::filesystem::filesystem_error& err) {
          std::cout << "filesystem error: " << err.what() << '\n';
          return StatusResult(ECE141::unknownError);
      }
    }
  private:
    std::string dbName;
    DBProcessor& curPro;
  };

class ShowIndexStatement : public Statement {
public:
  ShowIndexStatement(DBProcessor& processor) : curPro(processor) {};
  
  StatusResult parse(Tokenizer& aTokenizer) {
    const int kMinLength = 2;
    StatusResult theResult{syntaxError};
    
    if (kMinLength <= aTokenizer.remaining()) {
      if (Keywords::indexes_kw == aTokenizer.peek(1).keyword) {
        if (Keywords::show_kw == aTokenizer.current().keyword) {
          aTokenizer.next();
          aTokenizer.next();
          return StatusResult();
        }
      }
    }
    
    return theResult;
  }

  StatusResult run(std::ostream& anOutput) {
    return curPro.showIndex(anOutput);
  }
  
private:
  DBProcessor& curPro;
};
}
#endif /* DBStatements_hpp */
