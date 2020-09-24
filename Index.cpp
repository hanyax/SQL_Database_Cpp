//
//  Index.cpp
//  Project8
//
//  Created by 徐 翰洋 on 5/24/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#include <stdio.h>
#include "Index.hpp"
#include <iostream>

namespace ECE141 {

Index::Index(){}

Index::Index(const std::string &aField, uint32_t aHashId, DataType aType)
: field(aField), type(aType), schemaId(aHashId){
  changed = false;
  blockNum = 0;
}


Index::Index(const Index& anIndex) : field(anIndex.field), type(anIndex.type), schemaId(anIndex.schemaId) { 
  changed = anIndex.changed;
  blockNum = anIndex.blockNum;
  list = anIndex.list;
}


Index::~Index(){
  list.clear();
}

Index& Index::addKeyValue(const ValueType &aKey, uint32_t aValue) {
  list.insert(std::pair<ValueType, uint32_t>(aKey, aValue));
  return *this;
}

Index& Index::removeKeyValue(const ValueType &aKey) {
  list.erase(aKey);
  return *this;
}

StatusResult Index::encode(std::ostream &aWriter) {
  changed = 0; // changed has to be zero after stored, so next time when it read in, it is unchanges
  
  aWriter << "I" << " " << schemaId << " " << blockNum << " "
  << field << " " << static_cast<char>(type) << " " << changed << " ";
  
  for(auto aPair : list) {
    std::visit(ValueWriter{aWriter}, aPair.first);
    aWriter << aPair.second << " ";
  }
  return StatusResult();
}

StatusResult Index::decode(std::istream &aReader) {
  //schemaID, blockNum is not set yet
  char charType;
  
  std::string temp1;
  aReader >> temp1;
  aReader >> schemaId >> blockNum >> field >> charType >> changed;
  type = static_cast<DataType>(charType);
  
  while(aReader.good()) {
    std::pair<ValueType, uint32_t> aPair;
    uint32_t second;
    char keyType = static_cast<char>(type);
    if('I' == keyType) {
      uint32_t val;
      aReader >> val;
      aPair.first = val;
    } else if('F' == keyType) {
      float val;
      aReader >> val;
      aPair.first = val;
    } else if ('B' == keyType) {
      bool val;
      aReader >> val;
      aPair.first = val;
    } else if ('V' == keyType) {
      std::string val;
      aReader >> val;
      aPair.first = val;
    }
    if (aReader.good()) { 
      aReader >> second;
      aPair.second = second;
      list.insert(aPair);
    }
  }
  return StatusResult();
}

bool Index::each(BlockVisitor &aVisitor) {
  for (auto aPair : list) {
    StorageBlock aBlock;
    aVisitor(aBlock, aPair.second);
  }
  return true;
}

bool Index::each(BlockVisitor &aVisitor, Filters& filter) {
  for (auto aPair : list) {
    ValueType val = aPair.first;
    if (filter.matches(val)) { 
      StorageBlock aBlock;
      aVisitor(aBlock, aPair.second);
    }
  }
  return true;
}

bool Index::canIndexBy(const std::string &aField) {
  return field == aField;
}

}
