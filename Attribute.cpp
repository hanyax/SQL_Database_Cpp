//
//  Attribute.cpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Attribute.hpp"

namespace ECE141 {
  //STUDENT: Implement the attribute class here..
Attribute::Attribute(DataType aType, bool isValid) : type(aType), valid(isValid), nextAutoIncreVal(0) {}

Attribute::Attribute(std::string aName, DataType aType) : name(aName), type(aType), valid(true), nextAutoIncreVal(0) {}

Attribute::Attribute(const Attribute &aCopy) : name(aCopy.name), type(aCopy.type),length(aCopy.length), hasDefault(aCopy.hasDefault), autoIncre(aCopy.autoIncre), primaryKey(aCopy.primaryKey), nullable(aCopy.nullable), valid(aCopy.valid), value(aCopy.value), nextAutoIncreVal(aCopy.nextAutoIncreVal) {}

Attribute::~Attribute(){}

std::string Attribute::getTypeString() {
  if (type == DataType::int_type) {
    return "integer";
  } else if (type == DataType::float_type) {
    return "float";
  } else if (type == DataType::datetime_type) {
    return "date";
  } else if (type == DataType::bool_type) {
    return "boolean";
  } else if (type == DataType::varchar_type) {
    return "varchar(" + std::to_string(length) + ")";
  } else {
    return "noType";
  }
}

}
