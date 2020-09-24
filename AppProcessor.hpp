//
//  AppProcessor.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef AppProcessor_hpp
#define AppProcessor_hpp
#include <stdio.h>
#include "CommandProcessor.hpp"
#include "DBProcessor.hpp"

namespace ECE141 {


  class AppCmdProcessor : public CommandProcessor {
  public:
    
    AppCmdProcessor(CommandProcessor *aNext=nullptr);
    AppCmdProcessor(DBProcessor *aNext);
    virtual ~AppCmdProcessor();
    
    virtual Statement*    getStatement(Tokenizer &aTokenizer);
    virtual StatusResult  interpret(Statement &aStatement);
    
  };

}

#endif /* AppProcessor_hpp */
