//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include "AppProcessor.hpp"
#include "Tokenizer.hpp"
#include "AppStatements.hpp"
#include <iostream>
#include <memory>

namespace ECE141 {

  //.....................................
/*
  AppCmdProcessor::AppCmdProcessor(CommandProcessor *aNext) : CommandProcessor(aNext) {
  }
 */
  AppCmdProcessor::AppCmdProcessor(DBProcessor *aNext) : CommandProcessor(aNext) {}
  AppCmdProcessor::~AppCmdProcessor() {}
  
  // USE: -----------------------------------------------------
  StatusResult AppCmdProcessor::interpret(Statement &aStatement) {
    //STUDENT: write code related to given statement
      
      switch(aStatement.getType()) {
          case Keywords::quit_kw :
          case Keywords::version_kw :
          case Keywords::help_kw :
            return aStatement.run(std::cout);
          default: break;
      }
    return StatusResult();
  }
  
  // USE: factory to create statement based on given tokens...
  Statement* AppCmdProcessor::getStatement(Tokenizer &aTokenizer) {
    //STUDENT: Analyze tokens in tokenizer, see if they match one of the
    //         statements you are supposed to handle. If so, create a
    //         statement object of that type on heap and return it.
    
    //         If you recognize the tokens, consider using a factory
    //         to construct a custom statement object subclass.
      if(aTokenizer.more()) {
          Token curToken = aTokenizer.current();
          switch(curToken.keyword) {
              case Keywords::quit_kw :  aTokenizer.next();
              return new QuitStatement(*this);
              case Keywords::version_kw :  aTokenizer.next(); return new VersionStatement();
              case Keywords::help_kw :  aTokenizer.next(); return new HelpStatement(aTokenizer);
              default: break;
          }
      }

    return nullptr;
  }
  
}
