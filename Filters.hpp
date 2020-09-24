//
//  Filters.hpp
//  RGAssignment6
//
//  Created by rick gessner on 5/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <string>
#include "Errors.hpp"
#include "Value.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {
  
  class Row;
  class Schema;
  
  struct Operand {
    Operand() {}
    Operand(std::string &aName, TokenType aType, ValueType &aValue, uint32_t anId=0)
      : name(aName), value(aValue) {}
    
    TokenType   type; //so we know if it's a field, a const (number, string)...
    std::string name; //for named attr. in schema
    ValueType   value;
    uint32_t    entityId;
  };
  
  //---------------------------------------------------

  struct Expression {
    Operand     operand;
    Operators   op;
    
    Expression(Operand &anOperand, Operators anOp)
      : operand(anOperand), op(anOp) {}
    
    Expression(Operators anOp) : op(anOp) {}
    
    bool operator()(KeyValues &aList);
  };
  
  //---------------------------------------------------

  using Expressions = std::vector<Expression*>;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    Filters(const Filters &aFilters);
    ~Filters();
    
    Expression*  getExpression(int index) {return expressions[index];};
    size_t        getCount() const {return expressions.size();}
    bool          matches(KeyValues &aList);
    bool          matches(ValueType aVal);
    Filters&      add(Expression *anExpression) {expressions.push_back(anExpression); return *this;};
    
    friend class Tokenizer;
    
  protected:
    Expressions  expressions;
    
  };
   
}

#endif /* Filters_h */

