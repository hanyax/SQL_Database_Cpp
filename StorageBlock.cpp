//
//  StorageBlock.cpp
//  Assignment3
//
//  Created by rick gessner on 4/11/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "StorageBlock.hpp"
#include <cstring>

namespace ECE141 {
  StorageBlock::StorageBlock() : header(BlockType::data_block, 0) {
    std::fill_n(data, kPayloadSize, ' ');
    data[sizeof(data)-1] = '\0';
  }

  StorageBlock::StorageBlock(BlockType aType, uint32_t anID, uint32_t nextBlock)
  : header(aType, anID, nextBlock) {
    std::fill_n(data, kPayloadSize, ' ');
    data[sizeof(data)-1] = '\0';
  }

  StorageBlock::StorageBlock(BlockType aType, uint32_t anID)
  : header(aType, anID) {
    std::fill_n(data, kPayloadSize, ' ');
    data[sizeof(data)-1] = '\0';
  }

  StorageBlock::StorageBlock(const StorageBlock &aCopy) : header(aCopy.header) {
    std::memcpy(reinterpret_cast<void*>(data), &aCopy.data, kPayloadSize);
  }

  StorageBlock& StorageBlock::operator=(const StorageBlock &aCopy) {
    header=aCopy.header;
    std::memcpy(reinterpret_cast<void*>(data), &aCopy.data, kPayloadSize);
    return *this;
  }

  //USE: save the given stream in this block data area.
  bool StorageBlock::store(std::istream &aStream) {
    std::string theResult;
    char* dataptr = data;
    int index = 0;
    aStream >> theResult;
    while(!aStream.eof()) {
      
      std::memcpy(dataptr, theResult.c_str(), theResult.length());
      index += theResult.length() ;
      data[index] = ' ';
      index += 1;
      dataptr = dataptr + theResult.length() + 1;
      aStream >> theResult;
    }
    
    return true;
  }

  bool StorageBlock::store(std::stringstream &aStrStream) {
    std::string theResult = aStrStream.str();
    std::memcpy(data, theResult.c_str(), theResult.length());
    return true;
  }
}
