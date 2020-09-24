//
//  SQLStatements.hpp
//  Project4
//
//  Created by 徐 翰洋 on 4/28/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#ifndef SQLStatements_hpp
#define SQLStatements_hpp

#include <stdio.h>
#include <sstream>
#include "Statement.hpp"
#include "Schema.hpp"
#include "Helpers.hpp"
#include "SQLProcessor.hpp"
#include "Row.hpp"
#include "Filters.hpp"
#include "Join.hpp"

namespace ECE141 {

  class SQLStatement : public Statement {
  public:
    SQLStatement(SQLProcessor& aProc);
    virtual StatusResult parse(Tokenizer& aTokenizer);
    virtual StatusResult run(std::ostream& anOutput);
    
  protected:
    SQLProcessor& processor;
  };

  class createTableStatement : public SQLStatement {
  public:
    createTableStatement(SQLProcessor& aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& anOutput);
    
  protected:
    // Parse input helper method
    StatusResult parseAttribute(Tokenizer& aTokenizer);
    Attribute parseRow(std::vector<Token> &row);
    DataType checkType(std::string input);
    int checkLength(std::string data);
    int findDefaultValIndex(std::vector<Token> &row);
    bool checkAutoIncrement(std::vector<Token> &row);
    bool checkPrimaryKey(std::vector<Token> &row);
    bool checkNullable(std::vector<Token> &row);
    bool checkDefault(std::vector<Token> &row);
    
    std::string name;
    std::vector<Attribute> attributes;
  };

  class useTableStatement : public SQLStatement {
  public:
    useTableStatement(SQLProcessor& aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& anOutput);
    
  protected:
    std::string name;
  };
  
  class describeTableStatement : public SQLStatement {
  public:
    describeTableStatement(SQLProcessor& aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& anOutput);
    
  protected:
    std::string name;
  };
  
  class showTableStatement : public SQLStatement {
  public:
    showTableStatement(SQLProcessor& aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& anOutput);
  };

  class dropTableStatement : public SQLStatement {
  public:
    dropTableStatement(SQLProcessor& aProcessor);
    StatusResult parse(Tokenizer& aTokenizer);
    StatusResult run(std::ostream& anOutput);
    
  protected:
    std::string name;
  };

class insertRowStatement : public SQLStatement {
public:
  insertRowStatement(SQLProcessor &aProcessor);
  StatusResult parse(Tokenizer &aTokenizer); 
  StatusResult parseAttributes(Tokenizer &aTokenizer);
  StatusResult parseRows(Tokenizer &aTokenizer);
  StatusResult run(std::ostream &anOutput);
protected:
  StatusResult getKeyword(Tokenizer &aTokenizer);
  StatusResult parseIdentifier(Tokenizer &aTokenizer);
  StatusResult getToken(Tokenizer &aTokenizer);
  StatusResult getValueLists(Tokenizer &aTokenizer);
  StatusResult buildRow(std::vector<Token> &aRow);
  int indexInAttrlist(std::string name);
  
  std::string name;
  std::vector<std::string> attrlist;
  std::vector<Row> valueList;
};

class selectSatatement : public SQLStatement {
public:
  selectSatatement(SQLProcessor &aProcessor);
  StatusResult parse(Tokenizer &aTokenizer);
  StatusResult parseAttrlist(Tokenizer &aTokenizer);
  StatusResult parseOptional(Tokenizer &aTokenizer);
  StatusResult parseCondition(Tokenizer &aTokenizer);
  StatusResult parseOrderBy(Tokenizer &aTokenier);
  StatusResult parseLimit(Tokenizer &aTokenier);
  StatusResult parseTableField(Tokenizer &aTokenier, std::string& fieldName);
  StatusResult parseJoin(Tokenizer &aTokenizer);
  StatusResult run(std::ostream &anOutput);
  
protected:
  std::string name;
  std::vector<std::string> attrlist;
  std::vector<Join> joins;
  bool selectAll = false;
  std::string orderByAttr;
  uint32_t limitNum;
  Expressions whereContent;
  StatusResult errorInfo;
};

class updateSatatement : public selectSatatement {
public:
  updateSatatement(SQLProcessor &aProcessor);
  StatusResult parse(Tokenizer &aTokenizer);
  StatusResult run(std::ostream &anOutput);
  bool parseUpdates(Tokenizer &aTokenizer);
  
protected:
  std::string tableName;
  KeyValues updates;
};

class deleteRowStatement : public selectSatatement {
public:
  deleteRowStatement(SQLProcessor &aProcessor);
  StatusResult parse(Tokenizer &aTokenizer);
  StatusResult run(std::ostream &anOutput);
  
protected:
  StatusResult getKeyword(Tokenizer &aTokenizer);
  StatusResult parseIdentifier(Tokenizer &aTokenizer);
  std::string name;
};

}

#endif /* SQLStatements_hpp */
