//
//  SQLStatements.cpp
//  Project4
//
//  Created by 徐 翰洋 on 4/28/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#include "SQLStatements.hpp"

namespace ECE141 {
// SQL Statement
SQLStatement::SQLStatement(SQLProcessor& aProc) : processor(aProc) {}

StatusResult SQLStatement::parse(Tokenizer& aTokenizer){
  return StatusResult();
}

StatusResult SQLStatement::run(std::ostream& anOutput){
  return StatusResult();
}

// create table statement //////////////////////////////////////////////////////////////////////////////
// create table statement //////////////////////////////////////////////////////////////////////////////
// create table statement //////////////////////////////////////////////////////////////////////////////

createTableStatement::createTableStatement(SQLProcessor& aProc) : SQLStatement(aProc) {}

StatusResult createTableStatement::parse(Tokenizer& aTokenizer){
  const int kMinLength = 8;
  StatusResult theResult{syntaxError};
  
  if (kMinLength <= aTokenizer.remaining()) {
    if (Keywords::create_kw == aTokenizer.current().keyword) {
      aTokenizer.next();
      if (Keywords::table_kw == aTokenizer.current().keyword) {
        aTokenizer.next();
        name = aTokenizer.current().data;
        aTokenizer.next();
        if (aTokenizer.current().type == TokenType::punctuation &&  aTokenizer.current().data == "(" ) {
          return parseAttribute(aTokenizer);
        }
      }
    }
  }
  return theResult;
}

// Parsing statement detail//////////////////////////////////////////////////////////////////////////////
// Parsing statement detail//////////////////////////////////////////////////////////////////////////////
// Parsing statement detail//////////////////////////////////////////////////////////////////////////////

StatusResult createTableStatement::parseAttribute(Tokenizer &aTokenizer) {
  std::vector<Token> row;
  aTokenizer.next(); // eat left-parn
  while (aTokenizer.remaining() > 1) {
    while (aTokenizer.more() && aTokenizer.current().data != ",") {
      row.push_back(aTokenizer.current());
      aTokenizer.next();
    }
    Attribute attr = parseRow(row);
    attributes.push_back(attr);
    row.clear();
    aTokenizer.next();
  }
  aTokenizer.next(); // eat right-parn
  return StatusResult();
}

Attribute createTableStatement::parseRow(std::vector<Token> &row) {
  std::string name = row[0].data;
  DataType type = checkType(row[1].data);
  Attribute attr(name, type);
  attr.setNull(checkNullable(row));
  attr.setPrimaryKey(checkPrimaryKey(row));
  attr.setAutoIncrement(checkAutoIncrement(row));
  attr.setDefault(checkDefault(row));
  
  if (type == DataType::varchar_type) {
    attr.setLength(checkLength(row[3].data));
  }
  
  if (attr.isDefault()) {
    int index = findDefaultValIndex(row);
    if (attr.getType() == DataType::int_type) {
      attr.setValueInt(std::stoi(row[index].data.c_str()));
    } else if (attr.getType() == DataType::float_type) {
      attr.setValueFloat(std::stof(row[index].data.c_str()));
    } else if (attr.getType() == DataType::bool_type) {
      if (Helpers::caseInSensStringCompare(row[index].data, "true")) {
        attr.setValueBool(true);
      } else {
        attr.setValueBool(false);
      }
    }
  }
  return attr;
}

DataType createTableStatement::checkType(std::string input) {
  if (Helpers::caseInSensStringCompare(input, "int")) {
    return DataType::int_type;
  } else if (Helpers::caseInSensStringCompare(input, "bool") || Helpers::caseInSensStringCompare(input, "boolean")) {
    return DataType::bool_type;
  } else if (Helpers::caseInSensStringCompare(input, "timestamp")) {
    return DataType::datetime_type;
  } else if (Helpers::caseInSensStringCompare(input, "float")) {
    return DataType::float_type;
  } else if (Helpers::caseInSensStringCompare(input.substr(0, 7), "varchar")) {
    return DataType::varchar_type;
  } else {
    return DataType::no_type;
  }
}

int createTableStatement::checkLength(std::string data) {
  return std::stoi(data);
}

bool createTableStatement::checkAutoIncrement(std::vector<Token> &row) {
  size_t rowSize = row.size();
  for (int i = 2; i < rowSize; i++) {
    if (Helpers::caseInSensStringCompare(row[i].data, "auto_increment")) {
      return true;
    }
  }
  return false;
}

bool createTableStatement::checkPrimaryKey(std::vector<Token> &row) {
  size_t rowSize = row.size();
  for (int i = 2; i < rowSize; i++) {
    if ((i < row.size() - 1) && Helpers::caseInSensStringCompare(row[i].data, "primary")) {
      if (Helpers::caseInSensStringCompare(row[i+1].data, "key")) {
        return true;
      }
    }
  }
  return false;
}

bool createTableStatement::checkNullable(std::vector<Token> &row) {
  size_t rowSize = row.size();
  for (int i = 2; i < rowSize; i++) {
    if ((i < row.size() - 1) && Helpers::caseInSensStringCompare(row[i].data, "not")) {
      if (Helpers::caseInSensStringCompare(row[i+1].data, "null")) {
        return false;
      }
    }
  }
  return true;
}

bool createTableStatement::checkDefault(std::vector<Token> &row) {
  size_t rowSize = row.size();
  for (int i = 2; i < rowSize; i++) {
    if ((i < row.size() - 1) && Helpers::caseInSensStringCompare(row[i].data, "default")) {
      return true;
    }
  }
  return false;
}

int createTableStatement::findDefaultValIndex(std::vector<Token> &row) {
  size_t rowSize = row.size();
  for (int i = 2; i < rowSize; i++) {
    if ((i < row.size() - 1) && Helpers::caseInSensStringCompare(row[i].data, "default")) {
      return i+1;
    }
  }
  return -1;
}

StatusResult createTableStatement::run(std::ostream& anOutput) {
  Schema* newSchema = new Schema(name);
  for (auto attr : attributes) {
    newSchema->addAttribute(attr); 
  }
  return processor.createTable(newSchema, anOutput);
}
 
// use table statement //////////////////////////////////////////////////////////////////////////////
// use table statement //////////////////////////////////////////////////////////////////////////////
// use table statement //////////////////////////////////////////////////////////////////////////////

useTableStatement::useTableStatement(SQLProcessor& aProc) : SQLStatement(aProc) {}

StatusResult useTableStatement::parse(Tokenizer& aTokenizer) {
  const int kMinLength = 3;
  StatusResult theResult{syntaxError};
  
  if (kMinLength <= aTokenizer.remaining()) {
    if (Keywords::table_kw == aTokenizer.peek(1).keyword) {
      if (Keywords::use_kw == aTokenizer.current().keyword) {
        aTokenizer.next();
        aTokenizer.next();
        name = aTokenizer.current().data;
        aTokenizer.next();
        return StatusResult();
      }
    }
  }
  return theResult;
}
 
StatusResult useTableStatement::run(std::ostream& anOutput){
  return processor.useTable(name, anOutput);
}

// show table statement
showTableStatement::showTableStatement(SQLProcessor& aProc) : SQLStatement(aProc) {}

StatusResult showTableStatement::parse(Tokenizer& aTokenizer) {
  aTokenizer.next(); aTokenizer.next();
  return StatusResult();
}
 
StatusResult showTableStatement::run(std::ostream& anOutput){
  return processor.showTables(anOutput);
}

// describe table statement
describeTableStatement::describeTableStatement(SQLProcessor& aProc) : SQLStatement(aProc) {}

StatusResult describeTableStatement::parse(Tokenizer& aTokenizer){
  const int kMinLength = 2;
  StatusResult theResult{syntaxError};
  
  if (kMinLength <= aTokenizer.remaining()) {
    if (Keywords::describe_kw == aTokenizer.current().keyword) {
      aTokenizer.next();
      name = aTokenizer.current().data;
      aTokenizer.next();
      return StatusResult();
    }
  }
  
  return theResult;
}
 
StatusResult describeTableStatement::run(std::ostream& anOutput){
  return processor.describeTable(name, anOutput);
}

// drop table statement//////////////////////////////////////////////////////////////////////////////
// drop table statement//////////////////////////////////////////////////////////////////////////////
// drop table statement//////////////////////////////////////////////////////////////////////////////

dropTableStatement::dropTableStatement(SQLProcessor& aProc) : SQLStatement(aProc) {}

StatusResult dropTableStatement::parse(Tokenizer& aTokenizer){
    const int kMinLength = 3;
    StatusResult theResult{syntaxError};
    
    if (kMinLength <= aTokenizer.remaining()) {
      if (Keywords::table_kw == aTokenizer.peek(1).keyword) {
        if (Keywords::drop_kw == aTokenizer.current().keyword) {
          aTokenizer.next();
          aTokenizer.next();
          name = aTokenizer.current().data;
          aTokenizer.next();
          return StatusResult();
        }
      }
    }
    return theResult;
}

StatusResult dropTableStatement::run(std::ostream& anOutput){
  return processor.dropTable(name, anOutput);
}

// insertStatement//////////////////////////////////////////////////////////////////////////////
// insertStatement//////////////////////////////////////////////////////////////////////////////
// insertStatement//////////////////////////////////////////////////////////////////////////////

insertRowStatement::insertRowStatement(SQLProcessor &aProcessor) : SQLStatement(aProcessor){}

StatusResult insertRowStatement::parse(Tokenizer &aTokenizer) {
  if (getKeyword(aTokenizer)) {
    if(parseIdentifier(aTokenizer)) {
      name = aTokenizer.current().data;
      aTokenizer.next();
      if (getToken(aTokenizer)) {
        if (aTokenizer.current().keyword == Keywords::values_kw) {
          aTokenizer.next();
          return parseRows(aTokenizer);
        }
      }
    }
  }
  return StatusResult(syntaxError);
}

StatusResult insertRowStatement::getKeyword(Tokenizer &aTokenizer) {
  if (aTokenizer.current().keyword == Keywords::insert_kw) {
    aTokenizer.next();
    if (aTokenizer.current().keyword == Keywords::into_kw) {
      aTokenizer.next();
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult insertRowStatement::parseIdentifier(Tokenizer &aTokenizer) {  
  if (aTokenizer.current().keyword != Keywords::values_kw) {
    return StatusResult();
  } else {
    return StatusResult(syntaxError);
  }
}

StatusResult insertRowStatement::getToken(Tokenizer &aTokenizer) {
  if("(" == aTokenizer.current().data) {
      aTokenizer.next();
      while(")" != aTokenizer.current().data) {
          attrlist.push_back(aTokenizer.current().data);
          aTokenizer.next();
          if("," == aTokenizer.current().data)
              aTokenizer.next();
      }
      aTokenizer.next();
      //validate attrlist each token/field must be in schema TBD
      return StatusResult();
  }
  return StatusResult(syntaxError);
}

StatusResult insertRowStatement::parseRows(Tokenizer &aTokenizer) {
  std::vector<Token> aRow;
  while (aTokenizer.more()) {
    if (aTokenizer.current().data == "(") {
      aTokenizer.next();
      
      // Parse and add a row
      while (aTokenizer.current().data != ")") {
        if (aTokenizer.current().data == "") {
          return StatusResult(syntaxError);
        } else {
          if (aTokenizer.current().data != ",") {
            aRow.push_back(aTokenizer.current());
          }
          aTokenizer.next();
        }
      }
      aTokenizer.next();
      StatusResult result = buildRow(aRow);
      
      if (result.code != Errors::noError) {
        return result;
      }
      
    }
    aTokenizer.next();
    aRow.clear();
  }
  return StatusResult();
}

using ValidationFactory = StatusResult (*) (Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList);

StatusResult validateInt(Token& aToken, Attribute& attr, Row& aRow,  std::vector<Row> &valueList) {
  if (aToken.type == TokenType::number) {
    if (aToken.data.find('.') == std::string::npos) {
      std::pair<std::string, ValueType> aPair;
      aPair.first = attr.getName();
      aPair.second = static_cast<uint32_t>(std::stoi(aToken.data));
      aRow.add(aPair);
      //valueList.push_back(aRow);
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult validateFloat(Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList) {
  if (aToken.type == TokenType::number) {
    if (aToken.data.find('.') != std::string::npos) {
      std::pair<std::string, ValueType> aPair;
      aPair.first = attr.getName();
      aPair.second = std::stof(aToken.data);
      aRow.add(aPair);
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult validateString(Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList) {
  if (aToken.type == TokenType::string || aToken.type == TokenType::identifier) {
    if (aToken.data.size() <= attr.getLength()) {
      std::pair<std::string, ValueType> aPair;
      aPair.first = attr.getName();
      aPair.second = aToken.data;
      aRow.add(aPair);
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult validateBool(Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList) {
  if (aToken.type == TokenType::identifier) {
    if (aToken.data == "true" || aToken.data == "false" ||
        aToken.data == "TRUE" || aToken.data == "FALSE") {
      std::pair<std::string, ValueType> aPair;
      aPair.first = attr.getName();
      if (aToken.data == "true" || aToken.data == "TRUE") {
        aPair.second = true;
      } else {
        aPair.second = false;
      }
      aRow.add(aPair);
      return StatusResult();
    }
  }
  return StatusResult(invalidAttribute);
}

StatusResult validateDatetime(Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList) {
  return StatusResult(invalidAttribute);
}

StatusResult validateNoType(Token& aToken, Attribute& attr, Row& aRow, std::vector<Row> &valueList) {
  return StatusResult(invalidAttribute);
}

static std::map<DataType, ValidationFactory> validationFactories = {
  {DataType::int_type, validateInt},
  {DataType::float_type, validateFloat},
  {DataType::datetime_type, validateDatetime},
  {DataType::varchar_type, validateString},
  {DataType::bool_type, validateBool},
  {DataType::no_type, validateNoType},
};

StatusResult insertRowStatement::buildRow(std::vector<Token> &aTokenRow) {
  Database* curDB = processor.getActiveDatabase();
  if (curDB != nullptr) {
    Row row;
    int index = curDB->getSchemaIndexByName(name);
    if (index != -1) {
      Schema curSchema = curDB->getSchema(index);
      std::vector<Attribute> attrs = curSchema.getAttributes();
      StatusResult result;
      size_t attrSize = attrs.size();
      for (int i = 0; i < attrSize; i++) {
        int index = indexInAttrlist(attrs[i].getName());
        if (index != -1) {
          // parse this token and add it to row
          result = validationFactories[attrs[i].getType()](aTokenRow[index], attrs[i], row, valueList);
          if (result.code != noError) {
            return result;
          }
        } else {
          // check if it is auto increment or default
          // adds to row if so
          if (attrs[i].isDefault()) {
            std::pair<std::string, ValueType> aPair;
            aPair.first = attrs[i].getName();
            switch (attrs[i].getType()) {
              case DataType::bool_type:
                aPair.second = attrs[i].getValueBool(); break;
              case DataType::int_type:
                aPair.second = attrs[i].getValueInt(); break;
              case DataType::float_type:
                aPair.second = attrs[i].getValueFloat(); break;
              default:
                return StatusResult(invalidAttribute);
            }
            row.add(aPair);
          } else if (attrs[i].isAutoIncre()) {
            uint32_t num = curDB->getAutoIncre(curSchema.getName(), attrs[i].getName());
            std::pair<std::string, ValueType> aPair(attrs[i].getName(), num);
            row.add(aPair);
          } else {
            return StatusResult(invalidAttribute);
          }
        }
      }
      valueList.push_back(row);
    }
  }
  
  return StatusResult();
}

int insertRowStatement::indexInAttrlist(std::string aName) {
  size_t attrlistSize = attrlist.size();
  for (int i = 0; i < attrlistSize; i++) {
    if (attrlist[i] == aName) {
      return i;
    }
  }
  return -1;
}

StatusResult insertRowStatement::run(std::ostream &anOutput) {
  return processor.insertRow(name, valueList, anOutput);
}

// delete statement//////////////////////////////////////////////////////////////////////////////
// delete statement//////////////////////////////////////////////////////////////////////////////
// delete statement//////////////////////////////////////////////////////////////////////////////

deleteRowStatement::deleteRowStatement(SQLProcessor &aProcessor) : selectSatatement(aProcessor){}

StatusResult deleteRowStatement::parse(Tokenizer &aTokenizer) {
  StatusResult theResult = getKeyword(aTokenizer);
  if (theResult.code == Errors::noError) {
    theResult = parseIdentifier(aTokenizer);
    if(theResult.code == Errors::noError) {
      name = aTokenizer.current().data;
      aTokenizer.next();
      return parseCondition(aTokenizer);
    }
  }
  return theResult;
}

StatusResult deleteRowStatement::getKeyword(Tokenizer& aTokenizer) {
  if (aTokenizer.current().keyword == Keywords::delete_kw) {
    aTokenizer.next();
    if (aTokenizer.current().keyword == Keywords::from_kw) {
      aTokenizer.next();
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult deleteRowStatement::parseIdentifier(Tokenizer& aTokenizer) {
  return StatusResult();
}

StatusResult deleteRowStatement::run(std::ostream &anOutput) {
  return processor.deleteRow(name, whereContent, anOutput);
}

//---------------------select statement---------------------------
//---------------------select statement---------------------------
//---------------------select statement---------------------------

selectSatatement::selectSatatement(SQLProcessor &aProcesor) : SQLStatement(aProcesor), limitNum(-1) {}

StatusResult selectSatatement::parse(Tokenizer &aTokenizer) {
  //select fist_name from Users where age<50 order by last_name limit 3
  if(aTokenizer.skipIf(Keywords::select_kw)) {
    if(parseAttrlist(aTokenizer)) {
      if (aTokenizer.skipIf(Keywords::from_kw)) {
        if (aTokenizer.more()) {
          name = aTokenizer.current().data;
          aTokenizer.next();
          StatusResult result;
          
          // Parse Join
          if (Helpers::in_vector<Keywords>(gJoinTypes, aTokenizer.current().keyword)) {
            result = parseJoin(aTokenizer);
            if (result.code != Errors::noError) {
              return result;
            }
          } else if(aTokenizer.more() && Keywords::join_kw == aTokenizer.current().keyword) {
            errorInfo = StatusResult(joinTypeExpected);
          }
          
          // Parse optional
          std::vector<Keywords> optionalType = {Keywords::where_kw, Keywords::order_kw, Keywords::limit_kw};
          if (std::find(optionalType.begin(), optionalType.end(), aTokenizer.current().keyword) != optionalType.end()) {
            errorInfo = parseOptional(aTokenizer);
//            result = parseOptional(aTokenizer);
            result = errorInfo;
          }
          
          return result;
        }
      }
    }
  }
  return StatusResult(syntaxError);
}

StatusResult selectSatatement::parseJoin(Tokenizer &aTokenizer) {
  Token &theToken = aTokenizer.current();
  StatusResult theResult{joinTypeExpected}; //add joinTypeExpected to your errors file if missing...

  Keywords theJoinType{Keywords::join_kw}; //could just be a plain join
  if(Helpers::in_vector<Keywords>(gJoinTypes, theToken.keyword)) {
    theJoinType=theToken.keyword;
    aTokenizer.next(1); //yank the 'join-type' token (e.g. left, right)
    if(aTokenizer.skipIf(Keywords::join_kw)) {
      std::string secondTableName = aTokenizer.current().data;
      aTokenizer.next();
      theResult.code=keywordExpected; //on...
      if(aTokenizer.skipIf(Keywords::on_kw)) { //LHS field = RHS field
        Join theJoin(secondTableName, theJoinType, std::string(""),std::string(""));
        if((theResult=parseTableField(aTokenizer, theJoin.onLeft))) {
          if(aTokenizer.current().type == TokenType::operators && aTokenizer.current().data == "=") {
            aTokenizer.next();
            if((theResult=parseTableField(aTokenizer, theJoin.onRight))) {
              joins.push_back(theJoin);
            }
          }
        }
      }
    }
  }
  return theResult;
}

StatusResult selectSatatement::parseTableField(Tokenizer &aTokenizer, std::string& fieldName) {
  aTokenizer.next(2);
  fieldName = aTokenizer.current().data;
  aTokenizer.next();
  return StatusResult();
}

StatusResult selectSatatement::parseOptional(Tokenizer &aTokenizer) {
  while(aTokenizer.more()) {
    switch(aTokenizer.current().keyword) {
      case Keywords::where_kw: parseCondition(aTokenizer); break;
      case Keywords::order_kw: parseOrderBy(aTokenizer); break;
      case Keywords::limit_kw: parseLimit(aTokenizer); break;
      default:return StatusResult(unknownCommand);
    }
  }
  return StatusResult();
}

StatusResult selectSatatement::parseAttrlist(Tokenizer &aTokenizer) {
  // select all attributes
  if("*" == aTokenizer.current().data) {
    selectAll = true;
    aTokenizer.next();
    return StatusResult();
  }
  
  if (Keywords::from_kw == aTokenizer.current().keyword) {
    return StatusResult(syntaxError);
  }
  
  //parse until from; need validation;
  while(Keywords::from_kw != aTokenizer.current().keyword) {
    if("," == aTokenizer.current().data) {
      aTokenizer.next();
    }
    attrlist.push_back(aTokenizer.current().data);
    aTokenizer.next();
  }
  
  return StatusResult();
}

StatusResult selectSatatement::parseCondition(Tokenizer &aTokenizer) {
  if(Keywords::where_kw == aTokenizer.current().keyword) {
    aTokenizer.next();
    while(aTokenizer.more() && Keywords::order_kw != aTokenizer.current().keyword && Keywords::limit_kw != aTokenizer.current().keyword ) {
      // parse expression: op1 oprator op2
      if (aTokenizer.current().type != TokenType::keyword) {
        Operand operand;
        operand.name = aTokenizer.current().data;
        aTokenizer.next();
        std::string anOperator = aTokenizer.current().data;
        Operators op = aTokenizer.current().op;
        op = gOperators[anOperator];
        aTokenizer.next();
        
        operand.value = aTokenizer.current().data;
        aTokenizer.next();
        Expression* expre = new Expression(operand, op);
        whereContent.push_back(expre);
      } else {
        std::string anOperator = aTokenizer.current().data;
        Operators op = aTokenizer.current().op;
        op = gOperators[anOperator];
        aTokenizer.next();
        Expression* expre = new Expression(op);
        whereContent.push_back(expre);
      }
    }
    return StatusResult();
  }
  return StatusResult(syntaxError);
}

StatusResult selectSatatement::parseOrderBy(Tokenizer &aTokenizer) {
  if(Keywords::order_kw == aTokenizer.current().keyword) {
    aTokenizer.next();
    if(Keywords::by_kw == aTokenizer.current().keyword) {
      aTokenizer.next();
      orderByAttr = aTokenizer.current().data;
      aTokenizer.next();
      return StatusResult();
    }
  }
  return StatusResult(syntaxError);
}

StatusResult selectSatatement::parseLimit(Tokenizer &aTokenizer) {
  if(Keywords::limit_kw == aTokenizer.current().keyword) {
    aTokenizer.next();
    if(TokenType::number == aTokenizer.current().type) {
      limitNum = std::stoi(aTokenizer.current().data);
      aTokenizer.next();
      return StatusResult();
    }
  } 
  return StatusResult(syntaxError);
}

StatusResult selectSatatement::run(std::ostream &anOutput) {
  if(noError != errorInfo.code) {
    return errorInfo;
  }
  return processor.selectRow(name, attrlist, selectAll, orderByAttr, limitNum, whereContent, joins, anOutput); 
}

//---------------------update statement---------------------------
//---------------------update statement---------------------------
//---------------------update statement---------------------------

updateSatatement::updateSatatement(SQLProcessor &aProcesor) : selectSatatement(aProcesor) {}

StatusResult updateSatatement::parse(Tokenizer &aTokenizer) {
  //update Names set first_name="Chandhini" where id=3
  if(Keywords::update_kw == aTokenizer.current().keyword) {
    aTokenizer.next();
    tableName = aTokenizer.current().data;
    aTokenizer.next();
    if (parseUpdates(aTokenizer)) {
      return parseCondition(aTokenizer);
    }
  }
  return StatusResult(syntaxError);
}

bool updateSatatement::parseUpdates(Tokenizer &aTokenizer) {
  //set name = "xx", email = "yy@zz" where ...
  if(Keywords::set_kw == aTokenizer.current().keyword) {
    aTokenizer.next();
    while (aTokenizer.current().keyword != Keywords::where_kw) {
      
      if("," == aTokenizer.current().data){
        aTokenizer.next();
      }
      std::string attrName = aTokenizer.current().data;
      aTokenizer.next();
      
      if (gOperators[aTokenizer.current().data] == Operators::equal_op) {
        aTokenizer.next();
        ValueType val = aTokenizer.current().data;
        aTokenizer.next();
        updates.insert(std::make_pair(attrName, val));
      } else {
        return false;
      }
    }
    return true;
  }
  return false;
}

StatusResult updateSatatement::run(std::ostream &anOutput) {
  return processor.updateRow(tableName, updates, whereContent, anOutput);
}

}
