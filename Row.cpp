//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"

namespace ECE141 {
  
  //STUDENT: You need to fully implement these methods...

Row::Row() {}
Row::Row(const Row &aRow) : tableId(aRow.tableId), storageBlockNumber(aRow.storageBlockNumber), data(aRow.data) {}
Row::~Row() {}

Row& Row::operator=(const Row &aRow) { tableId = aRow.tableId; storageBlockNumber = aRow.storageBlockNumber; data = aRow.data; return *this; }
bool Row::operator==(Row &aCopy) const {return false;}

bool less_than (Row& row1, Row& row2, std::string basedOn) {
    int index = row1.getVal(basedOn).index();
    if (index == 0) {
      uint32_t x = std::get<uint32_t>(row1.getVal(basedOn));
      uint32_t y = std::get<uint32_t>(row2.getVal(basedOn));
      return x < y;
    } else if (index == 1) {
      float x = std::get<float>(row1.getVal(basedOn));
      float y = std::get<float>(row2.getVal(basedOn));
      return x < y;
    } else if (index == 3) {
      std::string x = std::get<std::string>(row1.getVal(basedOn));
      std::string y = std::get<std::string>(row2.getVal(basedOn));
      return x < y;
    }
    return false;
}

StatusResult Row::encode(std::ostream &aWriter) {
  aWriter << 'D' << " " << tableId << " ";
  
  static std::map<int, DataType> types = {
    {0, DataType::int_type},
    {1, DataType::float_type},
    {2, DataType::bool_type},
    {3, DataType::varchar_type},
  };
  
  for (auto pair : data) {
    aWriter << pair.first << " ";
    aWriter << static_cast<char>(types[pair.second.index()]) << " ";
    std::visit(ValueWriter{aWriter}, pair.second);
  }
  
  return StatusResult();
}

StatusResult Row::decode(std::istream &aReader) {
  char blcokType;
  aReader >> blcokType >> tableId;
  while (!aReader.eof()) {
    std::pair<std::string, ValueType> aPair;
    std::string name;
    char dataType;
    ValueType val;
    aReader >> name;
    if (!aReader.eof()) {
      aReader >> dataType;
      aPair.first = name;
      if('I' == dataType) {
        uint32_t val;
        aReader >> val;
        aPair.second = val;
      } else if ('F' == dataType) {
        float val;
        aReader >> val;
        aPair.second = val;
      } else if ('B' == dataType) {
        bool val;
        aReader >> val;
        aPair.second = val;
      } else if ('V' == dataType) {
        std::string val;
        aReader >> val;
        aPair.second = val;
      }
      data.insert(aPair);
    }
  }
  
  return StatusResult();
}

StatusResult Row::add(std::pair<std::string, ValueType> &aPair) {
  data.insert(aPair);
  return StatusResult();
}

StatusResult Row::add(std::map<std::string, ValueType> anotherRow) {
  data.insert(anotherRow.begin(), anotherRow.end());
  return StatusResult();
}

}
