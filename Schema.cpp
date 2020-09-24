//
//  Schema.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Schema.hpp"
#include "Helpers.hpp"

namespace ECE141 {

//STUDENT: Implement the Schema class here...
Schema::Schema(const std::string aName) : name(aName), changed(true) {}

Schema::Schema(const Schema &aCopy) {
  name = aCopy.name;
  blockNum = aCopy.blockNum;
  changed = aCopy.changed;
  attributes = aCopy.attributes;
}

Schema::~Schema(){}

Schema& Schema::addAttribute(const Attribute &anAttribute) {
  attributes.push_back(anAttribute);
  return *this;
}

Attribute& Schema::getAttribute(const std::string &aName) {
  int index = 0;
  for (Attribute attr : attributes) {
    if (attr.getName() == aName) {
      return attributes[index];
    }
    index += 1;
  }
  
  Attribute* invalid = new Attribute(DataType::no_type, false);
  return *invalid;
}

std::string Schema::getPrimaryKeyName() const {
  for (auto attr : attributes) {
    if (attr.isPrimaryKey()) {
      return attr.getName();
    }
  }
  return "No Primary Key Provided";
}

StatusResult Schema::encode(std::ostream &aWriter) {
    aWriter << 'E' << " " << Helpers::hashString(name.c_str()) << " " <<  name << " ";
    for(auto attr : attributes) {
      aWriter << attr.getName() << " " <<  static_cast<char>(attr.getType())
        << " " << attr.isAutoIncre() << " "
        << attr.isPrimaryKey() << " " << attr.isNullable() << " "
        << attr.isValid() << " " << attr.getLength() << " "
        << attr.isDefault() << " " << attr.getAutoIncreVal() << " ";
        
      //read default value
      if (attr.isDefault()) {
        std::visit(ValueWriter{aWriter}, attr.getValue());
      }
    }
  return StatusResult();
}

StatusResult Schema::decode(std::istream &aReader) {
    char blockType;
    uint32_t id;
//    std::string name;
  
  std::string tableName;
  aReader >> blockType >> id >> tableName;
  setName(tableName);
  
  while (!aReader.eof()) {
    std::string name;
    char type;
    bool isAutoIncre;
    bool isPrimaryKey;
    bool isNullable;
    bool isValid;
    int length;
    bool isDefault;
    uint32_t nextAutoIncreVal;
    aReader >> name >> type >> isAutoIncre >>
    isPrimaryKey >> isNullable >> isValid >> length >> isDefault >> nextAutoIncreVal;
    if (!aReader.eof()) {
      Attribute attr(static_cast<DataType>(type), isValid);
      attr.setName(name);
      attr.setAutoIncrement(isAutoIncre);
      attr.setPrimaryKey(isPrimaryKey);
      attr.setNull(isNullable);
      attr.setLength(length);
      attr.setDefault(isDefault);
      attr.setAutoIncreVal(nextAutoIncreVal);
      if (isDefault) {
        if('I' == type) {
          int val;
          aReader >> val;
          attr.setValueInt(val);
        } else if ('F' == type) {
          float val;
          aReader >> val;
          attr.setValueFloat(val);
        } else if ('B' ==type) {
          bool val;
          aReader >> val;
          attr.setValueBool(val);
        }
      }
      addAttribute(attr);
    }
  }
    
  return StatusResult();
}
}
