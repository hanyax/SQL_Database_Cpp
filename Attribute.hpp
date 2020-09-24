//
//  Attribute.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include <variant>
#include "Value.hpp"

namespace ECE141 {
    
  class Attribute {
  protected:
    std::string   name;
    DataType      type;
    int           length;
    bool          hasDefault;
    bool          autoIncre;
    bool          primaryKey;
    bool          nullable;
    bool          valid;
    ValueType     value;
    uint32_t      nextAutoIncreVal;
    
  public:
    Attribute(DataType aType=DataType::no_type, bool isValid=false);
    Attribute(std::string aName, DataType aType);
    Attribute(const Attribute &aCopy);
    ~Attribute();
    
    bool operator==(const Attribute &other) const {
      return name == other.name && type == other.type;
    }
    
    Attribute&  setName(std::string &aName) {name = aName; return *this;};
    Attribute&  setType(DataType aType) {type = aType; return *this;};
    Attribute&  setLength(int len) {length = len; return *this;}
    Attribute&  setAutoIncrement(bool val) {autoIncre = val; nextAutoIncreVal = 0; return *this;};
    Attribute&  setPrimaryKey(bool val) {primaryKey = val; return *this;};
    Attribute&  setNull(bool val) {nullable = val; return *this;};
    Attribute&  setValid(bool val) {valid = val; return *this;};
    Attribute&  setDefault(bool val) {hasDefault = val; return *this;};
    Attribute&  setValueInt(uint32_t val) {value = val; return *this;};
    Attribute&  setValueFloat(float val) {value = val; return *this;};
    Attribute&  setValueBool(bool val) {value = val; return *this;};
    Attribute&  setAutoIncreVal(uint32_t val) {nextAutoIncreVal = val; return *this;};
    
    const std::string&  getName() {return name;}
    DataType            getType() {return type;}
    std::string         getTypeString();
    int                 getLength() {return length;};
    ValueType           getValue() {return value;};
    uint32_t            getValueInt(){return std::get<uint32_t>(value);};
    float               getValueFloat(){return std::get<float>(value);};
    bool                getValueBool(){return std::get<bool>(value);};
    uint32_t            getAutoIncreVal() {return nextAutoIncreVal;};
    uint32_t            getNextAutoIncre() {nextAutoIncreVal+=1; return nextAutoIncreVal;};

    bool                isDefault() {return hasDefault;};
    bool                isAutoIncre() {return autoIncre;};
    bool                isPrimaryKey() {return primaryKey;};
    bool                isNullable() {return nullable;};
    bool                isValid() {return valid;};
  };

}

#endif /* Attribute_hpp */
