#pragma once

#include <iostream>
#include <list>
#include <unordered_map>

#include "jpb.h"
#include "types.h"

// #define DEBUG_PARSER

enum ENTRY_TYPE{
  ET_NUMBER,
  ET_VARIABLE,
  ET_FUNCTION,
  ET_PARENTHESIS,
  ET_SEPARATOR, // [,]
  ET_OPERATOR,
  ET_INVALID
};

struct Entry {
  ENTRY_TYPE type;
  std::string value;
};

typedef std::list<std::string> StringList;
typedef std::list<Entry> EntryList;
typedef std::unordered_map<std::string, float*> VariableMap;

class DllExport SimpleParser {
 public:
  static bool isExpressionCorrect(std::string expression, StringList& validVariables);

  float evaluateExpression(std::string expression) const;
  float evaluateExpression(EntryList& reversePolish) const;

  void setVariableMap(const VariableMap* variableMap) { this->variableMap = variableMap; }
  EntryList getReversePolish(std::string expression) const;

  static StringList getListOfVariables(const EntryList& reversePolish);
 private:
  static const int maxOpStackSize = 16;
  const VariableMap* variableMap = NULL;

  bool isNumberOrItsPart(char c) const;
  ENTRY_TYPE getEntryType(char c) const;
  Entry popEntry(std::string& expression)const;
};


DllExport real32
blendSmooth(real32 val1, real32 val2, real32 t, real32 blendRange);
