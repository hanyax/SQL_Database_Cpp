//
//  Storage.hpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include <variant>
#include "Errors.hpp"
#include "StorageBlock.hpp"
#include "Helpers.hpp"
#include "Value.hpp"

namespace ECE141 {
  
  //first, some utility classes...
  using SchemaIdBlockMap = std::map<uint32_t, uint32_t>;

  class StorageInfo {
  public:
    static const char* getDefaultStoragePath();
  };

  class BlockVisitor {
  public:
    BlockVisitor(){};
    virtual bool operator() (StorageBlock& aBlock, uint32_t blockNum) {return false;};
  };

  struct BlockIterator {
     virtual bool each(BlockVisitor &aVisitor)=0;
     virtual bool canIndexBy(const std::string &aField) {return false;} //override this
  };
  
  struct CreateNewStorage {};
  struct OpenExistingStorage {};

  // USE: Our storage manager class...
  class Storage : public BlockIterator {
  public:
    Storage(const std::string aName, CreateNewStorage);
    Storage(const std::string aName, OpenExistingStorage);
    Storage();
    Storage(const Storage& aCopy);
    ~Storage();
    uint32_t        getTotalBlockCount();

    //high-level IO (you're not required to use this, but it may help)...
    StatusResult    save(Storable &aStorable, uint32_t& aBlockNum); //using a stream api
    StatusResult    load(Storable &aStorable, uint32_t aBlockNum); //using a stream api
    
    StatusResult    dropTable(std::string aSchemaName);
    StatusResult    dropIndex(uint32_t firstIndexBlock);
    
    StatusResult    saveToc();
    StatusResult    loadToc();
    
    StatusResult    saveIndex(Storable& aStorable, std::map<uint32_t, uint32_t>& indexBlockMap);
    StatusResult    loadIndex(Storable& aStorable, uint32_t aBlockNum);
    StatusResult    saveIndexMap(std::map<uint32_t, uint32_t>& indexBlockMap);
    StatusResult    loadIndexMap(std::map<uint32_t, uint32_t>& indexBlockMap);
    
    bool            each(BlockVisitor &aVistor);
    
    //low-level IO...
    StatusResult    readBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    StatusResult    writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber);
    
    int getNewBlockNum();
    SchemaIdBlockMap& getschemaNumMap() {return iDBlockMap;}

  protected:
    bool              isReady() const;
    std::string       name;
    int               findFreeBlockNum();
    std::fstream      stream;
    int               currentBlockNum;
    SchemaIdBlockMap  iDBlockMap;
  };

}

#endif /* Storage_hpp */
