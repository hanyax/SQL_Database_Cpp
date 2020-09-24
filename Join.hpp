//
//  Join.hpp
//  Project9
//
//  Created by 徐 翰洋 on 6/3/20.
//  Copyright © 2020 ECE141A. All rights reserved.
//

#ifndef Join_hpp
#define Join_hpp

#include <stdio.h>
#include "keywords.hpp"

namespace ECE141 {

struct Join {
  Join(const std::string &aTable, Keywords aType, const std::string &aLHS, const std::string &aRHS)
    : joinType(aType), table(aTable), onLeft(aLHS), onRight(aRHS) {}
  
  Keywords    joinType;
  std::string table;
  std::string  onLeft;
  std::string  onRight;
};

}

#endif /* Join_hpp */
