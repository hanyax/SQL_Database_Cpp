//
//  Database.cpp
//  Database1
//
//  Created by rick gessner on 4/12/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include <sstream>
#include <map>
#include "Database.hpp"
#include "View.hpp"
#include "Storage.hpp"

//this class represents the database object.
//This class should do actual database related work,
//we called upon by your db processor or commands

namespace ECE141 {
    
  Database::Database(const std::string aName, CreateNewStorage)
    : dirty(false), name(aName), storage(aName, CreateNewStorage{}) {
  }
  
  Database::Database(const std::string aName, OpenExistingStorage)
    : dirty(false), name(aName), storage(aName, OpenExistingStorage{}) {
      storage.loadIndexMap(indexBlockMap);
      for (auto pair : indexBlockMap) {
        uint32_t tableId = pair.first;
        Index index;
        storage.loadIndex(index, indexBlockMap[tableId]);
        indexes.insert(std::pair<uint32_t, Index>(tableId, index));
      }
  }
  
  Database::~Database() {
    schemas.clear();
  }

  StatusResult Database::createDatabase(std::string &aName) {
    std::cout << "create new "<< aName <<".db file\n";
    StorageBlock aBlock(BlockType::meta_block, 0);
    storage.writeBlock(aBlock, 0);
    return StatusResult();
  }

  StatusResult Database::describeDatabase(std::ostream &anOutput) {
    describeDatabaseView view;
    view.show(schemas, storage, anOutput);
    return StatusResult();
  }

  StatusResult Database::dropDatabase(std::string &aName) {
    return StatusResult();
  }

  /*
  use: will be called in quit and use(switch to another db) command
  save TOC block and each schema block
   */
  StatusResult Database::saveDatabase() {
    for (auto schema : schemas) {
      //storage.getschemaNumMap(): return schemaIdBlockMap, to get blockNum
      storage.save(schema, storage.getschemaNumMap()[schema.getHash()]);
    }
    
    for (auto index : indexes) {
      if (index.second.isChanged()) {
        storage.saveIndex(index.second, indexBlockMap);
      }
    }
    
    // write TOC
    storage.saveToc();
    storage.saveIndexMap(indexBlockMap);
    
    return StatusResult();
  }

  StatusResult Database::createTable(Schema *aSchema) {
    if (!getEntityNamebyId(aSchema->getHash())) {
      aSchema->setChange(false);
      //-1 here is a dummy blockNum, a valid blockNum will be assigned in save();
      uint32_t placeHolder = -1;
      StatusResult theResult = storage.save(*aSchema, placeHolder);
      if (theResult) {
        dirty = true;
        
        // Add Index to Database
        std::string primaryKey;
        primaryKey = aSchema->getPrimaryKeyName();
        if (primaryKey == noPrimaryKeyProvided) {
          Attribute attr("auto_id", DataType::int_type);
          attr.setPrimaryKey(true);
          attr.setAutoIncrement(true);
          attr.setDefault(false);
          primaryKey = "auto_id";
          aSchema->addAttribute(attr);
        }

        Index index = Index(primaryKey, aSchema->getHash(), aSchema->getAttribute(primaryKey).getType());
        index.setChanged();
        indexes.insert(std::make_pair(aSchema->getHash(), index));
        // Add table to database
        schemas.push_back(*aSchema);
        ids.push_back(aSchema->getHash());
        delete aSchema;
      }
      return theResult;
    }
    return StatusResult{tableExists};
  }

  StatusResult Database::showTables(std::ostream &anOutput) {
    showTableView view(name, schemas);
    view.show(anOutput);
    return StatusResult();
  }

  StatusResult Database::dropTable(std::string aTableName) {
    schemas.erase(schemas.begin()+getSchemaIndexByName(aTableName));
    uint32_t id = Helpers::hashString(aTableName.c_str());
    ids.erase(ids.begin()+getIdIndexById(id));
    storage.dropTable(aTableName); //mark schema relevant blocks as free
    // delete index from storage
    storage.dropIndex(indexBlockMap[id]);
    indexes.erase(id);
    indexBlockMap.erase(id);
    return StatusResult();
  }

  StatusResult Database::describeTable(std::string tableName, std::ostream &anOutput) {
    int index = getSchemaIndexByName(tableName);
    if (index != -1) {
      describeTableView view(schemas[index]);
      view.show(anOutput);
      return StatusResult();
    }
    return StatusResult{unknownTable};
  }

  StatusResult Database::loadTablesFromToc() {
    SchemaIdBlockMap map = storage.getschemaNumMap();
    for (auto pair : map) {
      Schema s("temp");
      StorageBlock aBlock;
      storage.readBlock(aBlock, pair.second);
      std::string res(aBlock.data);
      std::stringstream aStream(res);
      s.decode(aStream);
      schemas.push_back(s);
      ids.push_back(s.getHash());
    }
    return StatusResult();
  }

  StatusResult Database::insertRow(std::string tableName, std::vector<Row>& valueList) {
    if (getSchemaIndexByName(tableName) != -1) {
      for (auto row : valueList) {
        uint32_t tableId = Helpers::hashString(tableName.c_str());
        row.setTableId(tableId);
        // 1) get block number
        uint32_t blockNumberSaved = -1;
        storage.save(row, blockNumberSaved); // save row to storage
        
        // 2) get row's primary and its value
        std::string primaryKey = indexes[tableId].getFieldName();
        ValueType val = row.getVal(primaryKey);
        
        // 3) get its index object and add a pair into the object
        indexes[tableId].addKeyValue(val, blockNumberSaved);
        indexes[tableId].setChanged();
        
      }
      return StatusResult();
    } else {
      return StatusResult(unknownTable);
    }
  }

  StatusResult Database::deleteRow(std::string aName, double& time, int& numRow, Expressions& whereContent) {
    if (getSchemaIndexByName(aName) != -1) {
      // Implement here
      std::vector<Row> rowCollection;
      std::vector<std::string> placeholder;
      
      buildRowCollection(rowCollection, aName, time, placeholder, true, "", -1, whereContent);
      
      for (auto row : rowCollection) {
        numRow++;
        // write a empty row to block
        StorageBlock aBlock;
        aBlock.header.type = 'F';
        StatusResult result = storage.writeBlock(aBlock, row.getStorageBlockNumber());
        
        // delete from index
        uint32_t tableId = Helpers::hashString(aName.c_str());
        ValueType primaryKey = row.getVal(indexes[tableId].getFieldName());
        indexes[tableId].removeKeyValue(primaryKey);
        indexes[tableId].setChanged();
        
        if (result.code != noError) {
          return result;
        }
      }
      return StatusResult();
    } else {
      return StatusResult(unknownTable);
    }
  }

  StatusResult Database::selectRow(std::string aName, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent, joinsList &joins, std::ostream &anOutput) {
    if (getSchemaIndexByName(aName) != -1) {
      // Construct filter first, then row, then pass row and filters to lisenter, add lisenter adds to row

      std::vector<Row> rowCollection;
      double time = 0.0;
      // call buildJoinRowCollection()
      StatusResult result;
      if(joins.size() == 0) {
        result = buildRowCollection(rowCollection, aName, time, attrlist, selectAll, orderByAttr, limitNum, whereContent);
      } else {
        result = buildJoinRowCollection(rowCollection, aName, joins, time, attrlist, selectAll, orderByAttr, limitNum, whereContent);
      }
      
      if (result.code != Errors::noError) {
        return result;
      }
      
      //move applyOptional here??
      
      for (auto attrString : attrlist) {
        Attribute attr = schemas[getSchemaIndexByName(aName)].getAttribute(attrString);
        if (joins.size() != 0) {
          if (!attr.isValid()) {
            attr = schemas[getSchemaIndexByName(joins[0].table)].getAttribute(attrString);
          }
          if (!attr.isValid()) {
            return StatusResult(Errors::unknownAttribute);
          }
        } else {
          if (!attr.isValid()) {
            return StatusResult(Errors::unknownAttribute);
          }
        }
      }
      
      if (selectAll) {
        AttributeList list = getSchema(getSchemaIndexByName(aName)).getAttributes();
        for (auto attr : list) {
          attrlist.push_back(attr.getName());
        }
        //add joinee attrs to attrlist, check if same name
        if(joins.size() > 0) {
          AttributeList joineeList = getSchema(getSchemaIndexByName(joins[0].table)).getAttributes();
          for (auto attr : joineeList) {
            if(!std::count(attrlist.begin(), attrlist.end(), attr.getName())) {
              attrlist.push_back(attr.getName());
            }
          }
        }
      }
      
      if (joins.size() == 0) {
        tableView view(attrlist, rowCollection, time);
        view.show(anOutput);
      } else {
        joinView view(attrlist, rowCollection, time);
        view.show(anOutput);
      }
      
      return StatusResult();
    }
    return StatusResult(Errors::unknownTable);
  }

StatusResult Database::buildRowCollection(std::vector<Row> &rowCollection, std::string aName, double& time, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent) {

    // filter
    Filters filter;
    for (auto expre : whereContent) {
      // fill expresion
      if (expre->op != Operators::and_op && expre->op != Operators::or_op) {
        Attribute attr = schemas[getSchemaIndexByName(aName)].getAttribute(expre->operand.name);
        if (attr.isValid()) {  // attr is valid if the attr is in the table
          DataType type = attr.getType();
          if (type == DataType::int_type) {
            expre->operand.value = static_cast<uint32_t>(std::stoi(std::get<std::string>(expre->operand.value)));
          } else if (type == DataType::float_type) {
            expre->operand.value = std::stof(std::get<std::string>(expre->operand.value));
          } else if (type == DataType::bool_type) {
            std::string result = std::get<std::string>(expre->operand.value);
            if (Helpers::caseInSensStringCompare(result, "true") || result == "1") {
              expre->operand.value = true;
            } else {
              expre->operand.value = false;
            }
          }
          filter.add(expre);
        } else {
          return StatusResult(Errors::unknownAttribute);
        }
      } else {
        filter.add(expre); // No operand, only AND / OR
      }
    }
    
    uint32_t tableId = Helpers::hashString(aName.c_str());
    
    Timer timer;
    timer.start();
    std::string condition;
    if (filter.getCount() > 0) {
      condition = filter.getExpression(0)->operand.name;
    }
    if (indexes[tableId].canIndexBy(condition)) {
      BlockAdder blockadder(rowCollection, tableId, storage);
      indexes[tableId].each(blockadder, filter);
    } else {
      BlockSelector blockSelector(rowCollection, filter, tableId, storage);
      indexes[tableId].each(blockSelector);
    }
  
    timer.stop();
    time = timer.elapsed();
    
    // clear filer
    for (auto f : whereContent) {
      delete f;  // Expressions = std::vector<Expression*>;
    }
    whereContent.clear();
    
    //orderby
    AttributeList list = getSchema(getSchemaIndexByName(aName)).getAttributes();
    std::vector<std::string> schemaAttrName;
    for (auto attr : list) {
      schemaAttrName.push_back(attr.getName());
    }
    
    applyOptional(rowCollection, schemaAttrName, orderByAttr, limitNum);

    return StatusResult();
  }

  StatusResult Database::buildJoinRowCollection (std::vector<Row> &rowCollection, std::string firstTableName, std::vector<Join> joins, double& time, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent) {
    
    
    // Construct filters
    Filters filter;
    for (auto expre : whereContent) {
      // fill expresion
      if (expre->op != Operators::and_op && expre->op != Operators::or_op) {
        Attribute attrFirstTable = schemas[getSchemaIndexByName(firstTableName)].getAttribute(expre->operand.name);
        Attribute attrSecondTable = schemas[getSchemaIndexByName(joins[0].table)].getAttribute(expre->operand.name);
        Attribute attr;
        if (attrFirstTable.isValid()) {
          attr = attrFirstTable;
        } else if (attrSecondTable.isValid()) {
          attr = attrSecondTable;
        } else {
          return StatusResult(Errors::unknownAttribute);
        }
    
        DataType type = attr.getType();
        if (type == DataType::int_type) {
          expre->operand.value = static_cast<uint32_t>(std::stoi(std::get<std::string>(expre->operand.value)));
        } else if (type == DataType::float_type) {
          expre->operand.value = std::stof(std::get<std::string>(expre->operand.value));
        } else if (type == DataType::bool_type) {
          std::string result = std::get<std::string>(expre->operand.value);
          if (Helpers::caseInSensStringCompare(result, "true") || result == "1") {
            expre->operand.value = true;
          } else {
            expre->operand.value = false;
          }
        }
        filter.add(expre);
      } else {
        filter.add(expre); // No operand, only AND / OR
      }
    }
    
    Timer timer;
    timer.start();
    
    uint32_t firstTableId = Helpers::hashString(firstTableName.c_str());
    uint32_t secondTableId = Helpers::hashString(joins[0].table.c_str());
    
    // Read in all rows
    std::vector<Row> firstTableRows;
    std::vector<Row> secondTableRows;
    
    BlockAdder readFirstTable(firstTableRows, firstTableId, storage);
    indexes[firstTableId].each(readFirstTable);
    
    BlockAdder readSecondTable(secondTableRows, secondTableId, storage);
    indexes[secondTableId].each(readSecondTable);
    
    StatusResult result;
    if (joins[0].joinType == Keywords::left_kw) {
    // Go through rows and check join condition
      result = constructJoinRows(firstTableRows, secondTableRows, rowCollection, joins);
    } else {
      result = constructJoinRows(secondTableRows, firstTableRows, rowCollection, joins);
    }
    if (result.code != Errors::noError) {
      return result;
    }
    
    timer.stop();
    time = timer.elapsed();
    
    std::vector<Row> rowCollectionFinal;
    for (auto row : rowCollection) {
      if (filter.matches(row.getData())) {
        rowCollectionFinal.push_back(row);
      }
    }
    
    // clear filer
    for (auto f : whereContent) {
      delete f;  // Expressions = std::vector<Expression*>;
    }
    whereContent.clear();
    
    AttributeList list;
    AttributeList attrsSecondTable;
    //orderby
    if (joins[0].joinType == Keywords::left_kw) {
      list = getSchema(getSchemaIndexByName(firstTableName)).getAttributes();
      attrsSecondTable = getSchema(getSchemaIndexByName(joins[0].table)).getAttributes();
    } else {
      list = getSchema(getSchemaIndexByName(joins[0].table)).getAttributes();
      attrsSecondTable = getSchema(getSchemaIndexByName(firstTableName)).getAttributes();
    }
    
    list.insert(list.end(), attrsSecondTable.begin(), attrsSecondTable.end());
    
    if (joins[0].onLeft == joins[0].onRight) {
      for (std::vector<Attribute>::iterator it = list.begin() ; it != list.end(); ++it) {
        if (it->getName() == joins[0].onRight) {
          list.erase(it);
          break;
        }
      }
    }
    
    std::vector<std::string> schemaAttrName;
    for (auto attr : list) {
      schemaAttrName.push_back(attr.getName());
    }
    
    applyOptional(rowCollectionFinal, schemaAttrName, orderByAttr, limitNum);
    
    return StatusResult();
  }

  StatusResult Database::constructJoinRows(std::vector<Row>& joiner, std::vector<Row>& joinee, std::vector<Row>& rowCollection, std::vector<Join>& joins) {
    for (auto joinerRow : joiner) {
      bool found = false;
      for (auto joineeRow : joinee) {
        if (joinerRow.keyExists(joins[0].onLeft) && joineeRow.keyExists(joins[0].onRight)) {
          Row result;
          if (joinerRow.getVal(joins[0].onLeft) == joineeRow.getVal(joins[0].onRight)) {
            result.add(joinerRow.getData());
            result.add(joineeRow.getData());
            rowCollection.push_back(result);
            found = true;
          }
        } else {
          return StatusResult(Errors::unknownAttribute);
        }
      }
      if (!found) {
        Row result;
        result.add(joinerRow.getData());
        rowCollection.push_back(result);
      }
    }
    return StatusResult();
  }

  
  StatusResult Database::applyOptional(std::vector<Row> &rowCollection, std::vector<std::string> schemaAttrName, std::string orderByAttr, uint32_t limitNum) {
    
    if(orderByAttr != "") {
      if (Helpers::findIndex<std::string>(schemaAttrName, orderByAttr) != -1) {
        std::sort(rowCollection.begin(), rowCollection.end(), less_than(orderByAttr));
      } else{
        return StatusResult(Errors::unknownAttribute);
      }
    }
    
    // limit
    if (limitNum != -1 && limitNum < rowCollection.size()) {
      rowCollection.resize(limitNum);
    }
    
    return StatusResult();
  }


  StatusResult Database::updateRow(std::string aName, double& time, int& numRow, KeyValues& updates, Expressions& whereContent) {
    std::vector<Row> rowCollection;
    std::vector<std::string> placeholder;
    
    buildRowCollection(rowCollection, aName, time, placeholder, true, "", -1, whereContent);
    
    for (auto row : rowCollection) {
      std::map<std::string, ValueType>& data = row.getData();
      for (auto pair : updates) {
        if (data.find(pair.first) != data.end()) {
          data[pair.first] = pair.second;
          numRow++;
        } else {
          return StatusResult(Errors::invalidAttribute);
        }
      }
      uint32_t placeholder = row.getStorageBlockNumber();
      StatusResult result = storage.save(row, placeholder);
      if (result.code != noError) {
        return result;
      }
    }
    return StatusResult();
  }

  StatusResult Database::showIndex(std::ostream &anOutput) {
    std::vector<std::pair<std::string, std::string>> tablePrimaryKey;
    for (auto schema : schemas) {
      tablePrimaryKey.push_back(std::make_pair(schema.getName(), schema.getPrimaryKeyName()));
    }
    IndexView view(tablePrimaryKey);
    view.show(anOutput);
    return StatusResult();
  }

  Schema& Database::getSchema(int index) {
    return schemas[index];
  }

  bool Database::getEntityNamebyId(uint32_t aId) {
    //use vector find()??
    for (uint32_t id : ids) {
      if (id == aId) {
        return true;
      }
    }
    return false;
  }

  int Database::getSchemaIndexByName(std::string name) {
    size_t schemaSize = schemas.size();
    for (int i = 0; i < schemaSize; i++) {
      if (schemas[i].getName() == name) {
        return i;
      }
    }
    return -1;
  }

  int Database::getIdIndexById(uint32_t aId) {
    size_t schemaSize = schemas.size();
    for (int i = 0; i < schemaSize; i++) {
      if (ids[i] == aId) {
        return i;
      }
    }
    return -1;
  }

  int Database::getAutoIncre(std::string tableName, std::string attrName) {
    int index = getSchemaIndexByName(tableName);
    return schemas[index].getAttribute(attrName).getNextAutoIncre();
  }

}
