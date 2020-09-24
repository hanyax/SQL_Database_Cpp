//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <filesystem>

namespace ECE141 {
  
  class FolderListener {
  public:
    virtual bool operator()(const std::string &aName)=0;
  };
  
  class FLTest : public FolderListener {
  public:
    FLTest(){}
    
    bool operator()(const std::string &aName) {
      std::clog << "Filename is: " << aName << "\n";
      return true;
    }
    
  };

  class FolderReader {
  public:
    FolderReader(const char *aPath) : path(aPath) {}
    virtual       ~FolderReader() {}
    
    virtual bool exists(const std::string &aPath) {
      std::filesystem::file_status s = std::filesystem::file_status{};
      if(std::filesystem::status_known(s) ? std::filesystem::exists(s) : std::filesystem::exists(path)) {
        return true;
      } else {
        return false;
      }
    }
    
    virtual void each(FolderListener &aListener, const std::string &anExtension) const {
      //STUDENT: iterate db's, pass the name of each to listener
      std::string path(ECE141::StorageInfo::getDefaultStoragePath());
      for (const auto & entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
          std::filesystem::path file = entry.path();
          std::string ext(file.extension());
          if (ext.size() == 0 || ext.compare(anExtension)) {
            if(!aListener(file.stem())) break;
          }
        }
      }
    };
    
    std::string path;
  };
  
}

#endif /* FolderReader_h */
