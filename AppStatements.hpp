//
//  AppStatements.hpp
//  Project2
//
//  Created by 徐 翰洋 on 4/13/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#ifndef AppStatements_hpp
#define AppStatements_hpp

#include <stdio.h>
#include <iostream>
#include "AppProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>

namespace ECE141 {
  class VersionStatement : public Statement {
  public:
      VersionStatement() :  Statement(Keywords::version_kw) {}
      
      StatusResult run(std::ostream& anOutput) {
          std::cout << "ECE141b-2\n";
          return StatusResult();
      }
  };

  class HelpStatement : public Statement {
  public:
      HelpStatement() : Statement(Keywords::help_kw) {}
      
      HelpStatement(Tokenizer& aTokenizer) : Statement(Keywords::help_kw){
          while(aTokenizer.more()) {
              remainKw.push_back(aTokenizer.current().keyword);
              aTokenizer.next();
          }
      }
            
    StatusResult run(std::ostream &aStream) {
        if(0 == remainKw.size()) {
            std::cout << helpInfo;
        } else {
          switch(remainKw[0]) {
            case Keywords::select_kw : std::cout << "output some information from the database \n"; break;
            case Keywords::version_kw : std::cout << "show the current version of the application \n"; break;
            case Keywords::quit_kw : std::cout << "terminates the execution of this DB application \n"; break;
            case Keywords::create_kw :
              if (remainKw[1] == Keywords::database_kw) {
                std::cout << "create database <name> -- creates a new database\n"; break;
              } else {
                return StatusResult(ECE141::unknownCommand);
              }
            case Keywords::drop_kw :
              if (remainKw[1] == Keywords::database_kw) {
                std::cout << "drop database <name> -- drops the given database\n"; break;
              } else {
                return StatusResult(ECE141::unknownCommand);
              }
            case Keywords::use_kw :
              if (remainKw[1] == Keywords::database_kw) {
                std::cout << "use database <name>-- uses the given database\n"; break;
              } else {
                return StatusResult(ECE141::unknownCommand);
              }
            case Keywords::describe_kw :
              if (remainKw[1] == Keywords::database_kw) {
                std::cout << "describe database <name> -- describes the given database\n"; break;
              } else {
                return StatusResult(ECE141::unknownCommand);
              }
            case Keywords::show_kw :
              if (remainKw[1] == Keywords::databases_kw) {
                std::cout << "show database -- shows the list of databases available\n"; break;
              } else {
                return StatusResult(ECE141::unknownCommand);
              }
            default:
              return StatusResult(ECE141::unknownCommand);
          }
        }
        return StatusResult();
    }
    
    std::string helpInfo = std::string("help -- the available list of commands shown below: \n") +
    std::string("     -- help - shows this list of commands\n") +
    std::string("     -- version -- shows the current version of this application\n") +
    std::string("     -- quit -- terminates the execution of this DB application\n") +
    std::string("     -- create database <name> -- creates a new database\n") +
    std::string("     -- drop database <name> -- drops the given database\n") +
    std::string("     -- use database <name>  -- uses the given database\n") +
    std::string("     -- describe database <name>  -- describes the given database\n") +
    std::string("     -- show databases -- shows the list of databases available\n");

  private:
      std::vector<Keywords> remainKw;
  };

  class QuitStatement : public Statement {
  public:
    QuitStatement(AppCmdProcessor &curPro) : processor(curPro), Statement(Keywords::quit_kw) {}
  
    StatusResult run(std::ostream& anOutput) {
      Database* curDB = processor.getActiveDatabase();
      if (curDB != nullptr) {
        curDB->saveDatabase();
      }
      std::cout << "Shutting down...\n";
      return StatusResult(ECE141::userTerminated);
    }
    
  protected:
    AppCmdProcessor& processor;
  };
}
#endif /* AppStatements_hpp */
