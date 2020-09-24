//
//  main.cpp
//  Database2
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include "AppProcessor.hpp"
#include "DBProcessor.hpp"
#include "SQLProcessor.hpp"
#include "Tokenizer.hpp"
#include "Errors.hpp"
#include "FolderReader.hpp"
// USE: ---------------------------------------------

static std::map<int, std::string> theErrorMessages = {
  {ECE141::illegalIdentifier, "Illegal identifier"},
  {ECE141::unknownIdentifier, "Unknown identifier"},
  {ECE141::databaseExists, "Database exists"},
  {ECE141::tableExists, "Table Exists"},
  {ECE141::syntaxError, "Syntax Error"},
  {ECE141::unknownCommand, "Unknown command"},
  {ECE141::unknownDatabase,"Unknown database"},
  {ECE141::unknownTable,   "Unknown table"},
  {ECE141::unknownAttribute, "Unknown attribute"},
  {ECE141::noDatabaseSpecified, "No database specified"},
  {ECE141::joinTypeExpected, "join Type Expected"},
  {ECE141::unknownError,   "Unknown error"}
};

void showError(ECE141::StatusResult &aResult) {
  std::string theMessage="Unknown Error";
  if(theErrorMessages.count(aResult.code)) {
    theMessage=theErrorMessages[aResult.code];
  }
  std::cout << "Error (" << aResult.code << ") " << theMessage << "\n";
}

//build a tokenizer, tokenize input, ask processors to handle...
ECE141::StatusResult handleInput(std::istream &aStream, ECE141::CommandProcessor &aProcessor) {
  ECE141::Tokenizer theTokenizer(aStream);
  
  //tokenize the input from aStream...
  ECE141::StatusResult theResult=theTokenizer.tokenize();
  while(theResult && theTokenizer.more()) {
    if(";"==theTokenizer.current().data) {
      theTokenizer.next();  //skip the ";"...
    }
    else theResult=aProcessor.processInput(theTokenizer);
  }
  return theResult;
}


//----------------------------------------------

int main(int argc, const char * argv[]) {

  /* Reader Test
  const char* path = ECE141::StorageInfo::getDefaultStoragePath();
  
  ECE141::FLTest test;
  
  ECE141::FolderReader reader(path);
  
  std::clog << "Path Exist: " << reader.exists(path) << "\n";
  reader.each(test, ".txt");
  */



  ECE141::DBProcessor       dbProcessor;
  ECE141::AppCmdProcessor   theProcessor(&dbProcessor);
  ECE141::SQLProcessor      sqlProcessor(&theProcessor);
  ECE141::StatusResult      theResult{};
    
  if(argc>1) {
    std::ifstream theStream(argv[1]);
    return handleInput(theStream, sqlProcessor);
  }
  else {
    std::string theUserInput;
    bool running=true;
    do {
      std::cout << "\n> ";
      if(std::getline(std::cin, theUserInput)) {
        if(theUserInput.length()) {
          std::stringstream theStream(theUserInput);
          theResult=handleInput(theStream, sqlProcessor);
          if(!theResult) showError(theResult);
        }
        if(ECE141::userTerminated==theResult.code)
          running=false;
      }
    }
    while (running);
  } 

  return 0;
}
