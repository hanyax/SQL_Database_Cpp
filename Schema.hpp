//
//  Schema.hpp
//  Assignment4
//
//  Created by rick gessner on 4/18/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <stdio.h>
#include <vector>
#include "Attribute.hpp"
#include "Errors.hpp"
#include "Storage.hpp"
#include "Helpers.hpp"
#include "Value.hpp"
//#include "Row.hpp"

namespace ECE141 {
  
  struct Block;
  struct Expression;
  class  Database;
  class  Tokenizer;
    
  using AttributeList = std::vector<Attribute>;
  
  //STUDENT: If you're using the Storable interface, add that to Schema class?

  class Schema : public Storable {
  public:
                          Schema(const std::string aName);
                          Schema(const Schema &aCopy);
    
                          ~Schema();
        
    const std::string&    getName() const {return name;}
    uint32_t              getHash() {return Helpers::hashString(name.c_str()); }
      
    const AttributeList&  getAttributes() const {return attributes;}
    uint32_t              getBlockNum() const {return blockNum;}
    bool                  isChanged() {return changed;} // not used for now;
    
    Schema&               addAttribute(const Attribute &anAttribute);
    Attribute&      getAttribute(const std::string &aName); 
    
    void                  setName(std::string val) {name = val;};
    void                  setChange(bool val) {changed = val;};
    
    std::string   getPrimaryKeyName() const;
    
    
    // Implement Storage
    StatusResult  encode(std::ostream &aWriter);
    StatusResult  decode(std::istream &aReader);
    BlockType     getType() const { return BlockType::entity_block;}
    
    //STUDENT: Do you want to provide an each() method for observers?
    
    
    //friend class Database; //is this helpful?
    
  protected:
    AttributeList   attributes;
    std::string     name;
    uint32_t        blockNum;  //storage location.
    bool            changed;
  };
  
}
#endif /* Schema_hpp */
