//
//  View.hpp
//  Datatabase4
//
//  Created by rick gessner on 4/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef View_h
#define View_h

#include <iostream>
#include "Schema.hpp"
#include "Storage.hpp"
#include "Helpers.hpp"

namespace ECE141 {

  //completely generic view, which you will subclass to show information
  class View {
  public:
    virtual         ~View() {}
    virtual bool    show(std::ostream &aStream)=0;
  };

  class showTableView : View {
  public:
    showTableView(std::string aName, std::vector<Schema> aSchema) {
      tableName = aName;
      schema = aSchema;
    }
    
    bool show(std::ostream &aStream) {
      std::string name = "Tables_in_" + tableName;
      aStream << "+----------------------+\n";
      aStream << "| " << name << "       |\n";
      aStream << "+----------------------+\n";
      for (auto s : schema) {
        std::string space = " ";
        int schemaNameLen = s.getName().size();
        for (int i = 0; i < (20 - schemaNameLen); i++) {
          space = space + " ";
        }
        aStream << "| " << s.getName() << space << "|\n";
      }
      aStream << "+----------------------+\n";
      aStream << schema.size() << " rows in set\n";
      return true;
    }
    
  protected:
    std::string tableName;
    std::vector<Schema> schema;
  };

  class describeTableView : View {
  public:
    describeTableView(Schema& aSchema) : schema(aSchema) {}
    
    bool show(std::ostream &aStream) {
      aStream << "+-----------+--------------+------+-----+---------+-----------------------------+\n";
      aStream << "| Field     | Type         | Null | Key | Default | Extra                       |\n";
      aStream << "+-----------+--------------+------+-----+---------+-----------------------------+\n";
      std::vector<Attribute> attrs = schema.getAttributes();
      int attrSize = attrs.size();
      for (int i = 0; i < attrSize; i++) {
        std::string name = attrs[i].getName();
        std::string type = attrs[i].getTypeString();
        std::string nullString;
        if (attrs[i].isNullable()) {
          nullString = "YES" + Helpers::makeSpace(3, 5);
        } else {
          nullString = "NO" + Helpers::makeSpace(2, 5);
        }
        
        std::string keyString;
        if (attrs[i].isPrimaryKey()) {
          keyString = "YES ";
        } else {
          keyString = "    ";
        }
        
        std::string defaultString;
        if (attrs[i].isDefault()) {
          std::string val;
          if (attrs[i].getType() == DataType::int_type) {
            val = std::to_string(attrs[i].getValueInt());
          } else if (attrs[i].getType() == DataType::float_type) {
            val = std::to_string(attrs[i].getValueFloat());
          } else if (attrs[i].getType() == DataType::bool_type) {
            attrs[i].getValueBool();
            if (attrs[i].getValueBool()) {
              val = "true";
            } else {
              val = "false";
            }
          }
          defaultString = val + Helpers::makeSpace(val.size(), 8);
        } else {
          defaultString = "NULL    ";
        }
        
        std::string extra = "";
        if (attrs[i].isAutoIncre()) {
          extra += "auto_increment ";
        }
        if (attrs[i].isPrimaryKey()) {
          extra += "primary key ";
        }
        std::string extraString = extra + Helpers::makeSpace(extra.size(), 28);
        
        aStream << "| " << name << Helpers::makeSpace(name.size(), 10) << "| " << type << Helpers::makeSpace(type.size(), 13) << "| " << nullString << "| " << keyString << "| " << defaultString << "| " << extraString << "|\n";
      }
      aStream << "+-----------+--------------+------+-----+---------+-----------------------------+\n";
      aStream << attrs.size() << " rows in set \n";
      return true;
    }
    
  protected:
    Schema& schema;
  };

class describeDatabaseView : View {
public:
  describeDatabaseView(){};
  
  bool show(std::ostream &aStream) {
    return true;
  }
    
  bool show(std::vector<Schema>& aSchema, Storage& aStorage, std::ostream &aStream) {
    aStream << "+-----------+--------------+-----------------------------+\n";
    aStream << "| Block#    | Type         | Other                       |\n";
    aStream << "+-----------+--------------+-----------------------------+\n";
    aStream << "| 0         | meta         |                             |\n";
    int numRow = 1;
    uint32_t blockCount = aStorage.getTotalBlockCount();
    for (uint32_t i = 1; i < blockCount; i++) {
      StorageBlock aBlock;
      aStorage.readBlock(aBlock, i);
      if (aBlock.header.type != 'F') {
        if (aBlock.header.type == 'E') {
          std::string schemaName = findName(aSchema, aBlock.header.id);
          aStream << "| " << numRow << Helpers::makeSpace((std::to_string(numRow)).size(), 10) << "| schema       | " << "\"" <<
          schemaName << "\"" << Helpers::makeSpace(schemaName.size()+1, 27) << "|\n";
        } else if (aBlock.header.type == 'D') {
          int id = 0;
          std::string schemaName = findName(aSchema, aBlock.header.id);
          aStream << "| " << numRow << Helpers::makeSpace((std::to_string(numRow)).size(), 10) << "| data         | " << "id=" << id <<
          ",  \"" << schemaName << "\"" << Helpers::makeSpace(schemaName.size()+1, 20) << "|\n";
        }
        numRow++;
      }
    }
    
    aStream << "+-----------+--------------+-----------------------------+\n";
    aStream << numRow << " rows in set\n";
    return true;
  }
  
  std::string findName(std::vector<Schema>& aSchema, uint32_t id) {
    for (auto table : aSchema) {
      if (table.getHash() == id) {
        return table.getName();
      }
    }
    return "";
  }
};

class tableView : View {
public:
  tableView(std::vector<std::string> aAttrlist,
            std::vector<Row> aRowCollection, double aTime) : attrlist(aAttrlist), rowCollection(aRowCollection), time(aTime) {};
  
  bool show(std::ostream &aStream) {
    std::string seperator = "";
    std::string attrRow = "";
    for (auto attr : attrlist) {
      seperator += Helpers::buildSeperator(14);
      attrRow += Helpers::buildAttrView(attr);
    }
    seperator += "+\n";
    attrRow += "|\n";
    
    aStream << seperator;
    aStream << attrRow;
    aStream << seperator;

    for (Row aRow : rowCollection) {
      std::map<std::string, ValueType> values = aRow.getData();
      std::string rowString = "";
      for (auto attr : attrlist) {
        int index = values[attr].index();
        rowString += "| ";
        std::string val;
        if (index == 0) {
          val = std::to_string(std::get<uint32_t>(values[attr]));
          
        } else if (index == 1) {
          val = std::to_string(std::get<float>(values[attr]));
          
        } else if (index == 2) {
          val = std::to_string(std::get<bool>(values[attr]));
          
        } else if (index == 3) {
          val = std::get<std::string>(values[attr]);
          
        } else {
          aStream << "Variant is wrong\n";
          return false;
        }
        rowString += val;
        rowString += Helpers::makeSpace(val.size(), 14);
      }
      rowString += "|\n";
      aStream << rowString;
    }
    
    aStream << seperator;
    aStream << rowCollection.size() << " rows in set (" << time << " ms.)\n";
    return true;
  }
protected:
  std::vector<std::string> attrlist;
  std::vector<Row> rowCollection;
  double time;
  
};


class IndexView : View {
public:
  IndexView(std::vector<std::pair<std::string, std::string>> aTablePrimaryKey) : tablePrimaryKey(aTablePrimaryKey) {};
  
  bool show(std::ostream &aStream) {
    std::string seperator = "+-----------------+-----------------+\n";
    aStream << seperator;
    aStream << "| table           | field           | \n";
    aStream << seperator;
    
    for (auto aPair : tablePrimaryKey) {
      std::string out = "| ";
      out += aPair.first;
      out += Helpers::makeSpace(aPair.first.size(), 16);
      out += "| ";
      out += aPair.second;
      out += Helpers::makeSpace(aPair.second.size(), 16);
      out += "|\n";
      aStream << out;
      aStream << seperator;
    }
    aStream << tablePrimaryKey.size() << " rows in set\n";
    return true;
  }
  
protected:
  std::vector<std::pair<std::string, std::string>> tablePrimaryKey;
};

class joinView : View {
public:
  joinView(std::vector<std::string> aAttrlist,
            std::vector<Row> aRowCollection, double aTime) : attrlist(aAttrlist), rowCollection(aRowCollection), time(aTime) {};
  
  bool show(std::ostream &aStream) {
    std::string seperator = "";
    std::string attrRow = "";
    for (auto attr : attrlist) {
      seperator += Helpers::buildSeperator(14);
      attrRow += Helpers::buildAttrView(attr);
    }
    seperator += "+\n";
    attrRow += "|\n";
    
    aStream << seperator;
    aStream << attrRow;
    aStream << seperator;

    for (Row aRow : rowCollection) {
      std::map<std::string, ValueType> values = aRow.getData();
      std::string rowString = "";
      for (auto attr : attrlist) {
        std::string val;
        rowString += "| ";
        if (values.find(attr) != values.end()) {
          int index = values[attr].index();
          if (index == 0) {
            val = std::to_string(std::get<uint32_t>(values[attr]));
            
          } else if (index == 1) {
            val = std::to_string(std::get<float>(values[attr]));
            
          } else if (index == 2) {
            val = std::to_string(std::get<bool>(values[attr]));
            
          } else if (index == 3) {
            val = std::get<std::string>(values[attr]);
            
          } else {
            aStream << "Variant is wrong\n";
            return false;
          }
        } else {
          val = "NULL";
        }
        rowString += val;
        rowString += Helpers::makeSpace(val.size(), 14);
      }
      rowString += "|\n";
      aStream << rowString;
    }
    
    aStream << seperator;
    aStream << rowCollection.size() << " rows in set (" << time << " ms.)\n";
    return true;
  }
protected:
  std::vector<std::string> attrlist;
  std::vector<Row> rowCollection;
  double time;
  
};

}

#endif /* View_h */
