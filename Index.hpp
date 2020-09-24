//
//  Index.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/17/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Index_h
#define Index_h

#include "keywords.hpp"
#include "Storage.hpp"
#include "Filters.hpp"

namespace ECE141 {

using IntOpt = std::optional<uint32_t>;

struct LessKey {
  bool operator()(const ValueType& anLHS, const ValueType& aRHS) const {
    uint32_t index = anLHS.index();
    if (index == 0) {
      uint32_t x = std::get<uint32_t>(anLHS);
      uint32_t y = std::get<uint32_t>(aRHS);
      return x < y;
    } else if (index == 1) {
      float x = std::get<float>(anLHS);
      float y = std::get<float>(aRHS);
      return x < y;
    } else if (index == 3) {
      std::string x = std::get<std::string>(anLHS);
      std::string y = std::get<std::string>(aRHS);
      return x < y;
    }
    return false;
  }
};

class Index : public BlockIterator, public Storable {
  
public:
  using ValueMap = std::map<ValueType, uint32_t, LessKey>;
  Index();
  Index(const std::string &aField, uint32_t aHashId, DataType aType);
  Index(const Index& anIndex);
  
  virtual ~Index();
  
  ValueMap&           getList() {return list;}
  void                setChanged(bool aValue=true) {changed=aValue;}
  bool                isChanged() {return changed;}
  const std::string&  getFieldName() const {return field;}
  uint32_t            getBlockNum() const {return blockNum;}
  
    //manage keys/values in the index...
  Index& addKeyValue(const ValueType &aKey, uint32_t aValue);

  Index& removeKeyValue(const ValueType &aKey);
  //bool contains(const ValueType &aKey);
  //uint32_t getValue(const ValueType &aKey);
  
  //don't forget to support the storable interface IF you want it...

  //void initBlock(StorageBlock &aBlock);
  StatusResult encode(std::ostream &aWriter) override;
  StatusResult decode(std::istream &aReader) override; 

  //and the blockIterator interface...
  
  bool each(BlockVisitor &aVisitor) override;
  bool each(BlockVisitor &aVisitor, Filters& filters);
  bool canIndexBy(const std::string &aField) override;
  
protected:
  std::string   field; //what field are we indexing?
  DataType      type;
  uint32_t      schemaId;
  bool          changed;
  uint32_t      blockNum;  //storage block# of index...
  ValueMap      list;
};

}
#endif /* Index_h */
