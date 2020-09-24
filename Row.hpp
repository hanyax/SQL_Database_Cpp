//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "Value.hpp"
#include "Storage.hpp"

namespace ECE141 {

class Row : public Storable {
public:
  Row();
  Row(const Row &aRow);
  ~Row();
  Row& operator=(const Row &aRow);
  bool operator==(Row &aCopy) const;
  
  void setTableId(uint32_t id) {tableId = id;};
  StatusResult add(std::pair<std::string, ValueType> &aPair);
  StatusResult add(std::map<std::string, ValueType> anotherRow);
  StatusResult encode(std::ostream &aWriter);
  StatusResult decode(std::istream &aReader);
  
  void      setStorageBlockNumber(uint32_t aNum) {storageBlockNumber = aNum;}; //use: select row/build row collection in blockSelector
  uint32_t  getStorageBlockNumber() {return storageBlockNumber;}; //use: update and delete row/write back to file sys
  
  //void setData(std::string key, ValueType val) {data[]};
  std::map<std::string, ValueType>& getData() {return data;};
  bool      keyExists(std::string key) {return data.find(key) != data.end();}
  ValueType getVal(std::string key) {return data[key];};
  
protected:
  uint32_t tableId;
  uint32_t storageBlockNumber;
  std::map<std::string, ValueType> data;
};

class less_than {
public:
  less_than(std::string base) : basedOn(base) {}
  
  inline bool operator() (Row& row1, Row& row2) {
    // using ValueType = std::variant<uint32_t, float, bool, std::string>
    //index 0-uint_32; 1-float; 3-std::string
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
  
  std::string basedOn;
};

}

#endif /* Row_hpp */
