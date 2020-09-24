//
//  Storage.cpp
//  Assignment2
//
//  Created by rick gessner on 4/5/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//
#include "Storage.hpp"
#include <cmath>
#include <vector>

namespace ECE141 {
const int dataStart = 2;

// USE: Our main class for managing storage...
const char* StorageInfo::getDefaultStoragePath() {
  //STUDENT -- MAKE SURE TO SET AN ENVIRONMENT VAR for DB_PATH!
  //           This lets us change the storage location during autograder testing

  //WINDOWS USERS:  Use forward slash (/) not backslash (\) to separate paths.
  //                (Windows convert forward slashes for you)
  
  const char* thePath = std::getenv("DB_PATH");
  return thePath;
}

//----------------------------------------------------------

//path to the folder where you want to store your DB's...
std::string getDatabasePath(const std::string &aDBName) {
  std::string thePath;
  
  //build a full path (in default storage location) to a given db file..
    thePath = StorageInfo::getDefaultStoragePath();
    thePath.append("/").append(aDBName).append(".db");
  return thePath;
}

// USE: ctor ---------------------------------------
Storage::Storage(const std::string aName, CreateNewStorage) : name(aName), currentBlockNum(dataStart) {
  std::string thePath = getDatabasePath(name);
  //try to create a new db file in known storage location.
  //throw error if it fails...
    try {
        if (std::filesystem::exists(thePath)) {
          std::cout << "database " << name << " already existed.\n";
          //return StatusResult(ECE141::databaseExists);
        } else {
            stream.open(thePath.c_str(), std::fstream::out);
            stream.close();
            stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
        }
    } catch(const std::filesystem::filesystem_error& err) {
       std::cout << "filesystem error: " << err.what() << '\n';
       //return StatusResult(ECE141::unknownError);
    }
}

// USE: ctor ---------------------------------------
Storage::Storage(const std::string aName, OpenExistingStorage) : name(aName), currentBlockNum(dataStart) {
  std::string thePath = getDatabasePath(aName);
  //try to OPEN a db file a given storage location
  //if it fails, throw an error
    try {
        if (!std::filesystem::exists(thePath)) {
          std::cout << "database " << name << " does not exist.\n";
          //return StatusResult(ECE141::databaseExists);
        } else {
          stream.open(thePath.c_str(), std::fstream::in);
          stream.close();
          stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
          loadToc();
        }
    } catch(const std::filesystem::filesystem_error& err) {
       std::cout << "filesystem error: " << err.what() << '\n';
       //return StatusResult(ECE141::unknownError);
    }
}

Storage::Storage() {}

Storage::Storage(const Storage& aCopy) {
  name = aCopy.name;
  currentBlockNum = aCopy.currentBlockNum;
  iDBlockMap = aCopy.iDBlockMap;
}

// USE: dtor ---------------------------------------
Storage::~Storage() {
  stream.close();
}

// USE: validate we're open and ready ---------------------------------------
bool Storage::isReady() const {
  return stream.is_open();
}

// USE: count blocks in file ---------------------------------------
uint32_t Storage::getTotalBlockCount() {
  return currentBlockNum;
}

StatusResult Storage::save(Storable &aStorable, uint32_t& aBlockNum) {
  //High-level IO: save a storable object (like a table row)...
  std::stringstream aStream;
  StatusResult theResult = aStorable.encode(aStream);
  char blockType;
  uint32_t id;
  aStream >> blockType >> id;
    
  StorageBlock aBlock(static_cast<BlockType>(blockType), id);
  aBlock.store(aStream);
  uint32_t blockNum;
  //if blockNums is -1/not specified, assign a new blockNum;
  //  if we have a free blockNum, then use that one;
  //  otherwise allocate a new blockNum;
  //otherwise use passed value;
  if (aBlockNum == -1) {
    blockNum = getNewBlockNum();
    aBlockNum = blockNum;
  } else {
    blockNum = aBlockNum;
  }
  
  if (blockType == 'E') {
    iDBlockMap.insert(std::pair<uint32_t,uint32_t>(id,blockNum));
  }
  writeBlock(aBlock, blockNum);
  saveToc();
  
  return theResult;
}

StatusResult Storage::load(Storable &aStorable, uint32_t aBlockNum) {
  StorageBlock aBlock;
  readBlock(aBlock, aBlockNum);
  std::string res(aBlock.data);
  std::stringstream aStream(res);
  return aStorable.decode(aStream);
}

StatusResult Storage::saveIndex(Storable &aStorable, std::map<uint32_t, uint32_t>& indexBlockMap) {
  //High-level IO: save a storable object (like a table row)...
  std::stringstream aStream;
  StatusResult theResult = aStorable.encode(aStream);
  
  char blockType;
  uint32_t id;
  aStream >> blockType >> id;
  
  // Delete previous index in storage
  dropIndex(indexBlockMap[id]);

  //calculate how many blocks we need
  long streamLen = aStream.tellp();
  int blockNeed = ceil(static_cast<double>(streamLen) / (kPayloadSize-1));
  std::vector<uint32_t> blockNumSet;
  for(int i = 0; i < blockNeed; i++) {
    blockNumSet.push_back(getNewBlockNum());
  }
  
  //write index object into file
  uint32_t curBlockNum;
  uint32_t nextBlockNum = -1;
  std::string res = aStream.str();

  for(int i = 0; i < blockNeed; i++) {
    curBlockNum = blockNumSet[i];
    if (i == 0) {
      indexBlockMap[id] = curBlockNum;
    }
    
    if(i+1 < blockNeed) {
      nextBlockNum = blockNumSet[i+1];
    }
    
    StorageBlock aBlock(static_cast<BlockType>(blockType), id, nextBlockNum);
    nextBlockNum = -1;
    std::string subStr;
    if (i == blockNeed - 1) {
      subStr = res.substr(i*(kPayloadSize-1));
    } else {
      subStr = res.substr(i*(kPayloadSize-1), (kPayloadSize-1));
    }
    std::stringstream subStream(subStr);
    aBlock.store(subStream);
    writeBlock(aBlock, curBlockNum);
  }
  return theResult;
}

StatusResult Storage::loadIndex(Storable& index, uint32_t aBlockNum) {
  StorageBlock aBlock;
  readBlock(aBlock, aBlockNum);
  std::string res(aBlock.data);
  while(aBlock.header.nextBlock != -1) {
    uint32_t blockNum = aBlock.header.nextBlock;
    readBlock(aBlock, blockNum);
    std::string nextRes(aBlock.data);
    res.append(nextRes);
  }
  std::stringstream aStream(res);
  return index.decode(aStream);
}

StatusResult Storage::saveToc() {
  StorageBlock Toc(BlockType::meta_block, 0);
  std::stringstream aStream;
  // Fill the stream
  aStream << currentBlockNum << " ";
  for (auto pair : iDBlockMap) {
    aStream << pair.first << " " << pair.second << " ";
  }
  Toc.store(aStream);
  return writeBlock(Toc, 0);
}

StatusResult Storage::loadToc() {
  StorageBlock Toc(BlockType::meta_block, 0);
  readBlock(Toc, 0);
  
  std::string res(Toc.data);
  std::stringstream ss(res);
  ss >> currentBlockNum;
  while (!ss.eof()) {
    std::string firstString;
    std::string secondString;
    ss >> firstString;
    ss >> secondString;
    
    if (!ss.eof()) {
      std::pair<uint32_t, uint32_t> aPair;
      aPair.first = static_cast<uint32_t>(std::stoul(firstString));
      aPair.second = static_cast<uint32_t>(std::stoul(secondString));
      
      iDBlockMap.insert(aPair);
    }
  }

  return StatusResult();
}

StatusResult Storage::saveIndexMap(std::map<uint32_t, uint32_t>& indexBlockMap) {
  StorageBlock index(BlockType::meta_block, 1);
  std::stringstream aStream;
  // Fill the stream
  for (auto pair : indexBlockMap) {
    aStream << pair.first << " " << pair.second << " ";
  }
  index.store(aStream);
  return writeBlock(index, 1);
}

StatusResult Storage::loadIndexMap(std::map<uint32_t, uint32_t>& indexBlockMap) {
  StorageBlock index(BlockType::meta_block, 1); //1?
  readBlock(index, 1);
  
  std::string res(index.data);
  std::stringstream ss(res);
  while (!ss.eof()) {
    std::string firstString;
    std::string secondString;
    ss >> firstString;
    ss >> secondString;

    if (!ss.eof()) {
      std::pair<uint32_t, uint32_t> aPair;
      aPair.first = static_cast<uint32_t>(std::stoul(firstString));
      aPair.second = static_cast<uint32_t>(std::stoul(secondString));
      indexBlockMap.insert(aPair);
    }
  }
  return StatusResult();
}

//mark schema block as free
//read schema related block(Entity/Data) and change header,
//then write back in place
StatusResult Storage::dropTable(std::string aTableName) {
  uint32_t id = Helpers::hashString(aTableName.c_str());
  iDBlockMap.erase(id);
  for (int i = 0; i < currentBlockNum; i++) {
    StorageBlock aBlock;
    readBlock(aBlock, i);
    if (aBlock.header.id == id) {
      aBlock.header.type = 'F';
      writeBlock(aBlock, i);
    }
  }
  return StatusResult();
}

StatusResult Storage::dropIndex(uint32_t firstIndexBlock) {
  uint32_t curBlock = firstIndexBlock;
  if (curBlock != 0) {
    StorageBlock aBlock;
    readBlock(aBlock, curBlock);
    uint32_t nextBlock = aBlock.header.nextBlock;
    while (curBlock != -1) {
      StorageBlock block;
      block.header.type = 'F';
      writeBlock(block, curBlock);
      curBlock = nextBlock;
      if (curBlock != -1) {
        readBlock(aBlock, curBlock);
        nextBlock = aBlock.header.nextBlock;
      }
    }
  }
  return StatusResult();
}

bool Storage::each(BlockVisitor &aVisitor) {
  for (int i = 1; i < currentBlockNum; i++) {
    StorageBlock aBlock;
    readBlock(aBlock, i);
    aVisitor(aBlock, i);
  }
  return true;
}

// USE: write data a given block (after seek)
StatusResult Storage::writeBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
  //STUDENT: Implement this; this is your low-level block IO...
  stream.clear();
  stream.seekp(aBlockNumber * sizeof(aBlock), std::ios::beg);
  stream.write((char*)(&aBlock), sizeof(aBlock));
  return StatusResult();
}

// USE: read data from a given block (after seek)
StatusResult Storage::readBlock(StorageBlock &aBlock, uint32_t aBlockNumber) {
  //STUDENT: Implement this; this is your low-level block IO...
  stream.clear();
  stream.seekg(aBlockNumber * sizeof(aBlock), std::ios::beg);
  stream.read((char*)&aBlock, sizeof(aBlock));
  return StatusResult();
}

int Storage::getNewBlockNum() {
  int freeBlockNum = findFreeBlockNum();
  if(-1 != freeBlockNum) {
    return freeBlockNum;
  }
  int temp = currentBlockNum;
  currentBlockNum++;
  return temp;
}

// Call this to locate a free block in this storage file.
// If you can't find a free block, then append a new block and return its blocknumber
int Storage::findFreeBlockNum() {
  for (int i = dataStart; i < currentBlockNum; i++) {
    StorageBlock aBlock;
    readBlock(aBlock, i);
    if (aBlock.header.type == 'F') {
      aBlock.header.type = 'V';
      writeBlock(aBlock, i);
      return i;
    }
  }
  return -1;
}

}
