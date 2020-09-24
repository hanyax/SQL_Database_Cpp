//
//  Database.hpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "Storage.hpp"
#include "Schema.hpp"
#include "Row.hpp"
#include "Index.hpp"
#include "Filters.hpp"
#include "View.hpp"
#include "Timer.hpp"
#include "Join.hpp"

namespace ECE141 {
using SchemaList = std::vector<Schema>;
using IndexList = std::map<uint32_t, Index>;
using HashList = std::vector<uint32_t>;
using IndexBlockMap = std::map<uint32_t, uint32_t>; 
using joinsList = std::vector<Join>;

class BlockSelector : public BlockVisitor {
public:
  BlockSelector(std::vector<Row> &aRowCollection, Filters &aFilter, uint32_t aId, Storage& aStorage) : id(aId), rowCollection(aRowCollection), filter(aFilter), storage(aStorage) {};
  
  bool operator()(StorageBlock& aBlock, uint32_t blockNum) {
    //where id > 24 ? where score > 90
    //expression.operator.name == index.field should be faster
    //
    // pass block to row
    storage.readBlock(aBlock, blockNum);
    if (aBlock.header.id == id && aBlock.header.type == 'D') {
      Row aRow;
      aRow.setTableId(aBlock.header.id);
      std::string res(aBlock.data);
      std::stringstream aStream(res);
      aRow.decode(aStream);
      aRow.setStorageBlockNumber(blockNum);
      
      if (filter.getCount() > 0 ) {
        KeyValues rowData = aRow.getData();
        if (filter.matches(rowData)) {
          rowCollection.push_back(aRow);
        }
      } else {
        rowCollection.push_back(aRow);
      }
    }
    return false;
  }
  
private:
  uint32_t id;
  std::vector<Row> &rowCollection;
  Filters &filter;
  Storage& storage;
};

class BlockAdder : public BlockVisitor {
public:
  BlockAdder(std::vector<Row> &aRowCollection, uint32_t aId, Storage& aStorage) : id(aId), rowCollection(aRowCollection), storage(aStorage) {};
  
  bool operator()(StorageBlock& aBlock, uint32_t blockNum) {
    // pass block to row
    storage.readBlock(aBlock, blockNum);
    if (aBlock.header.id == id && aBlock.header.type == 'D') {
      Row aRow;
      aRow.setTableId(aBlock.header.id);
      std::string res(aBlock.data);
      std::stringstream aStream(res);
      aRow.decode(aStream);
      aRow.setStorageBlockNumber(blockNum);
      rowCollection.push_back(aRow);
    }
    return false;
  }
  
private:
  uint32_t id;
  std::vector<Row> &rowCollection;
  Storage& storage;
};

class Database {
public:
  
  Database(const std::string aName, CreateNewStorage);
  Database(const std::string aName, OpenExistingStorage);
  ~Database();
  
  StatusResult createDatabase(std::string &aName);
  StatusResult describeDatabase(std::ostream &anOutput);
  StatusResult dropDatabase(std::string &aName);
  StatusResult saveDatabase();
  
  StatusResult createTable(Schema* aSchema);
  StatusResult showTables(std::ostream &anOutput);
  StatusResult describeTable(std::string tableNmae, std::ostream &anOutput);
  StatusResult dropTable(std::string tableName);
  StatusResult loadTablesFromToc();
  
  StatusResult insertRow(std::string tableName, std::vector<Row> &valueList);
  StatusResult deleteRow(std::string aName, double& time, int& numRow, Expressions& whereContent);
  StatusResult selectRow(std::string aName, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent, joinsList &joins, std::ostream &anOutput);
  StatusResult buildRowCollection (std::vector<Row> &rowCollection, std::string aName, double& time, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent);
  StatusResult buildJoinRowCollection (std::vector<Row> &rowCollection, std::string firstTableName, std::vector<Join> joins, double& time, std::vector<std::string> attrlist, bool selectAll, std::string orderByAttr, uint32_t limitNum, Expressions& whereContent);
  StatusResult constructJoinRows(std::vector<Row>& joiner, std::vector<Row>& joinee, std::vector<Row>& rowCollection, std::vector<Join>& joins);
  StatusResult applyOptional(std::vector<Row> &rowCollection, std::vector<std::string> schemaAttrName, std::string orderByAttr, uint32_t limitNum);
  StatusResult updateRow(std::string aName, double& time, int& numRow, KeyValues& updates, Expressions& whereContent);
  
  StatusResult showIndex(std::ostream &anOutput);
  
  Storage&     getStorage() {return storage;}
  std::string& getName() {return name;}
  Schema&      getSchema(int index);
      
  bool  getEntityNamebyId(uint32_t aId);
  int   getSchemaIndexByName(std::string name);
  int   getIdIndexById(uint32_t aId);
  int   getAutoIncre(std::string tableName, std::string attrName);
  
protected:
  bool            dirty; // not much use for now??
  std::string     name;
  Storage         storage;
  SchemaList      schemas;
  IndexList       indexes; //tableIdtoIndexObjMap
  HashList        ids; //tableIdList
  IndexBlockMap   indexBlockMap;
};
}

#endif /* Database_hpp */
