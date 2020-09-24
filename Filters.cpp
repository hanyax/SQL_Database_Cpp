//
//  Filters.hpp
//  Assignement6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//


#include <stdio.h>
#include "Filters.hpp"
#include "Row.hpp"
#include "Schema.hpp"

namespace ECE141 {

Filters::Filters() {
  
}

Filters::~Filters() {
  expressions.clear();
}

struct equalCompVisitor {
  template<typename T>
  bool operator() (T i, T j) {
    return i == j;
  }
};

bool Filters::matches(ValueType aVal) {
    bool result = false;
    Operators op = (*expressions[0]).op;
    ValueType attrValue = aVal;
    ValueType aValue = (*expressions[0]).operand.value;
    
    if(Operators::equal_op == op) {
      result = (attrValue == aValue);
    }
    
    if(Operators::lt_op == op) {
      result = (attrValue < aValue);
    }
    
    if(Operators::gt_op == op) {
        result = (attrValue > aValue);
    }
    return result;
}

bool Filters::matches(KeyValues &aList) {
  // get OR/AND
  // get prevRes, latterRes
  //return prev OR/AND latter
  bool andOp = false;
  bool orOp = false;
  bool preResult = false;
  for(auto expre : expressions) {
    if(expre->op == Operators::and_op ) {
      andOp = true;
    }
    if(expre->op == Operators::or_op ) {
      orOp = true;
    }
  }
  size_t expressionCount = expressions.size();
  for(int i = 0; i < expressionCount; i++) {
    Operators op = (*expressions[i]).op;
    std::string attrName = (*expressions[i]).operand.name;
    ValueType attrValue = aList[attrName];
    ValueType aValue = (*expressions[i]).operand.value;
    
    if(Operators::equal_op == op) {
//      std::clog << "inside == compare\n";
//      return std::visit(equalCompVisitor(), attrValue, aValue);
      if(i == 0) {
        preResult = (attrValue == aValue);
      } else {
        if(andOp) {
          preResult = preResult && (attrValue == aValue);
        } else if(orOp) {
          preResult = preResult || (attrValue == aValue);
        } else {
          return attrValue == aValue;
        }
      }
    }
    
    if(Operators::lt_op == op) {
//      std::clog << "inside < compare\n";
      if(i == 0) {
        preResult = (attrValue < aValue);
      } else {
        if(andOp) {
          preResult = preResult && (attrValue < aValue);
        } else if(orOp) {
          preResult = preResult || (attrValue < aValue);
        } else {
          return attrValue < aValue;
        }
      }
    }
    
    if(Operators::gt_op == op) {
      if(i == 0) {
        preResult = (attrValue > aValue);
      } else {
        if(andOp) {
          preResult = preResult && (attrValue > aValue);
        } else if(orOp) {
          preResult = preResult || (attrValue > aValue);
        } else {
          return attrValue > aValue;
        }
      }
    }
    
  }
    return preResult;
  }




}



