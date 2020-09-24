//
//  StorageBlock.hpp
//  Assignment3
//
//  Created by rick gessner on 4/11/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef StorageBlock_hpp
#define StorageBlock_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <variant>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include "Attribute.hpp"

namespace ECE141 {

  //a "storage" file is comprised of fixed-sized blocks (defined below)

//=======
  const size_t kPayloadSize = 1004; //1004 = 1024-(4+8+8) area reserved in storage-block for user data...

  

  enum class BlockType {
    meta_block='M',
    data_block='D',
    entity_block='E',
    free_block='F',
    index_block='I',
    unknown_block='V',
  };

  using NamedBlockNums = std::map<std::string, uint32_t>;

  struct BlockHeader {
   
    BlockHeader(BlockType aType=BlockType::data_block, uint32_t aId=0, uint32_t aNextBlock=-1 )
      : type(static_cast<char>(aType)), id(aId), nextBlock(aNextBlock) {}
   
    BlockHeader(const BlockHeader &aCopy) {
      type = aCopy.type;
      id = aCopy.id;
      nextBlock = aCopy.nextBlock;
    }
   
    BlockHeader& operator=(const BlockHeader &aCopy) {
      type=aCopy.type;
      id=aCopy.id;
      nextBlock = aCopy.nextBlock;
      return *this;
    }
    
    void setBlockID(uint32_t anID) {
        id = anID;
    }
   
    char      type;     //char version of block type {[D]ata, [F]ree... }
    uint32_t  id;       //use this anyway you like
    uint32_t  nextBlock;
  };
 
  struct StorageBlock {
    StorageBlock();
    StorageBlock(BlockType aType, uint32_t aID);
    StorageBlock(BlockType aType, uint32_t aID, uint32_t nextBlock);
    StorageBlock(const StorageBlock &aCopy);
    StorageBlock& operator=(const StorageBlock &aCopy);

    bool  store(std::istream &aStream);
    bool  store(std::stringstream &aStrStream);
    void  setNextBlock(uint32_t aBlockNum) {header.nextBlock = aBlockNum;};
    
    //we use attributes[0] as table name...
    BlockHeader   header;
    char          data[kPayloadSize];
  };
}

#endif /* StorageBlock_hpp */
