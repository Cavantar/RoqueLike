#include "SimpleParser.h"
#include <algorithm>
#include <assert.h>
#include <string>
#include "Types.h"

bool SimpleParser::isExpressionCorrect(std::string expression, StringList& validVariables)
{
  SimpleParser sp;
  EntryList reversePolish = sp.getReversePolish(expression);

  if(reversePolish.front().type == ET_INVALID)
  {
    std::cout << "Invalid Atom starting at: " << reversePolish.front().value  << std::endl;
    std::cout << "In Expression: " << expression  << std::endl << std::endl;

    return false;
  }

  // Checking for variables

  for(auto it = reversePolish.begin(); it != reversePolish.end(); it++)
  {
    const Entry& entry = *it;
    if(entry.type == ET_VARIABLE)
    {
      if(std::find(std::begin(validVariables), std::end(validVariables), entry.value) == std::end(validVariables))
	return false;
    }
  }

  return true;
}

bool SimpleParser::isNumberOrItsPart(char c) const
{
  if((c >= '0' && c <= '9') || c == '.') return true;
  return false;
}

ENTRY_TYPE SimpleParser::getEntryType(char c) const
{
  if(isNumberOrItsPart(c)) return ET_NUMBER;

  if((c >= 'a' && c <= 'z') ||
     (c >= 'A' && c <= 'Z')) return ET_VARIABLE;

  if(c == '(' || c == ')') return ET_PARENTHESIS;

  if(c == '+' || c == '-' ||
     c == '*' || c == '/' ||
     c == '^' || c == '%') return ET_OPERATOR;

  if(c == ',') return ET_SEPARATOR;

  return ET_INVALID;
}

Entry SimpleParser::popEntry(std::string& expression) const
{
  Entry resultEntry;
  int endIndex = 0;

  resultEntry.type = getEntryType(expression[0]);

  // assert(resultEntry.type != ET_INVALID);

  if(resultEntry.type == ET_INVALID)
  {
    resultEntry.value = expression;
    return resultEntry;
  }

  resultEntry.value = expression[endIndex++];

  // Operators and parenthesis are only one character long
  switch(resultEntry.type)
  {
  case ET_OPERATOR:
  case ET_PARENTHESIS:
  case ET_SEPARATOR:
    expression.erase(0, endIndex);
    return resultEntry;
  case ET_VARIABLE:
    {
      if(endIndex >= expression.size()) break;

      ENTRY_TYPE nextEntryType = getEntryType(expression[endIndex]);
      while(endIndex < expression.size() &&
	    (nextEntryType == ET_VARIABLE || nextEntryType == ET_NUMBER))
      {
	resultEntry.value += expression[endIndex++];

	nextEntryType = getEntryType(expression[endIndex]);
      }

      // We get function characters just like variables so we can just check if current variable name is function name
      if(resultEntry.value == "min" || resultEntry.value == "max" ||
	 resultEntry.value == "sin" || resultEntry.value == "cos" ||
	 resultEntry.value == "abs" || resultEntry.value == "mod" ||
	 resultEntry.value == "floor" || resultEntry.value == "ceil" ||
	 resultEntry.value == "blend")
      {
	resultEntry.type = ET_FUNCTION;
      }

    } break;
  case ET_NUMBER:
    {
      if(endIndex >= expression.size()) break;
      ENTRY_TYPE nextEntryType = getEntryType(expression[endIndex]);
      while(endIndex < expression.size() && nextEntryType == ET_NUMBER)
      {
	resultEntry.value += expression[endIndex++];
	nextEntryType = getEntryType(expression[endIndex]);
      }
    } break;
  }

  expression.erase(0, endIndex);
  return resultEntry;
}

EntryList SimpleParser::getReversePolish(std::string expression) const
{
  expression.erase(std::remove(expression.begin(), expression.end(), ' '), expression.end());

  EntryList result;
  int stackIndex = 0;
  EntryList operationStack[maxOpStackSize];

  while(expression.size() != 0)
  {
    Entry entry = popEntry(expression);

    //std::cout << stackIndex << std::endl;

    switch(entry.type)
    {
    case ET_NUMBER:
    case ET_VARIABLE:
      {
	result.push_back(entry);
      } break;
    case ET_PARENTHESIS:
      {
	if(entry.value == "(") ++stackIndex;
	else if(entry.value == ")")
	{
	  // Getting All Operators from Current Stack
	  while(operationStack[stackIndex].size())
	  {
	    result.push_back(operationStack[stackIndex].back());
	    operationStack[stackIndex].pop_back();
	  }

	  --stackIndex;
	  if(operationStack[stackIndex].size() != 0)
	  {
	    result.push_back(operationStack[stackIndex].back());
	    operationStack[stackIndex].pop_back();
	  }
	}
      } break;
    case ET_SEPARATOR:
      {
	if(entry.value == ",")
	{
	  while(operationStack[stackIndex].size())
	  {
	    result.push_back(operationStack[stackIndex].back());
	    operationStack[stackIndex].pop_back();
	  }

	}
      } break;
    case ET_FUNCTION:
    case ET_OPERATOR:
      {
	operationStack[stackIndex].push_back(entry);
      } break;
    case ET_INVALID:
      {
	result.clear();
	result.push_back(entry);
	return result;
      } break;

    }
  }

  if(stackIndex != 0)
  {
    result.clear();
    Entry invalidEntry = { ET_INVALID, "Probably forgot closing parenthesis"};
    result.push_back(invalidEntry);
    return result;
  }

  while(operationStack[stackIndex].size() != 0)
  {
    result.push_back(operationStack[stackIndex].back());
    operationStack[stackIndex].pop_back();
  }

#ifdef DEBUG_PARSER
  {
    std::cout << "\nReverse Polish: \n";
    std::cout << "-----------------\n\n";
    for(auto i = result.begin(); i != result.end(); i++)
    {
      std::cout << i->value << std::endl;
    }
    std::cout << "\n-----------------\n\n";
  }
#endif

  return result;
};

StringList SimpleParser::getListOfVariables(const EntryList& reversePolish)
{
  StringList result;
  for(auto it = reversePolish.begin(); it != reversePolish.end(); it++)
  {
    const Entry& entry = *it;
    if(entry.type == ET_VARIABLE)
      result.push_back(entry.value);
  }
  return result;
}

real32 SimpleParser::evaluateExpression(EntryList& reversePolish) const
{
  Entry entry = reversePolish.back();
  reversePolish.pop_back();

  switch(entry.type)
  {
  case ET_NUMBER:
    {
      return std::stof(entry.value);
    } break;
  case ET_VARIABLE:
    {
      const std::string& variableName = entry.value;

      // for debugging purposes I allow null pointers;
      // I'm gonna have to assume that the variable exist - for efficiency
      if(variableMap && variableMap->count(variableName))
      {
	return *variableMap->at(variableName);
      }
      std::cout << "Coulnd find variable: " << variableName << std::endl;
      return 0;
    } break;
  case ET_OPERATOR:
    {
      // This pops things
      real32 right = evaluateExpression(reversePolish);

      // And this too
      real32 left = evaluateExpression(reversePolish);

#ifdef DEBUG_PARSER
      {
	std::cout << "Left: " << left << std::endl;
	std::cout << "Right: "<< right << std::endl;
	std::cout << "Operator: "<< entry.value[0] << std::endl;
      }
#endif

      char operatorC = entry.value[0];
      switch(operatorC)
      {
      case '+': return left + right;
      case '-': return left - right;
      case '*': return left * right;
      case '/': return left / right;
      case '^': return pow(left, right);
      case '%': return fmod(left, right);
      }
    } break;
  case ET_FUNCTION:
    {
      const std::string& function = entry.value;

      if(function == "blend")
      {
	real32 blendRange = evaluateExpression(reversePolish);
	real32 t = evaluateExpression(reversePolish);
	real32 val2 = evaluateExpression(reversePolish);
	real32 val1 = evaluateExpression(reversePolish);

	//return blendRange + t + val2 + val1;
	return blendSmooth(val1, val2, t, blendRange);
      }
      // Two argument functions
      else if(function == "min" || function == "max" || function == "mod")
      {
	// This pops things
	real32 right = evaluateExpression(reversePolish);

	// And this too
	real32 left = evaluateExpression(reversePolish);

	if(function == "min") return std::min(left, right);
	else if(function == "max") return std::max(left, right);
	else if(function == "mod") return fmod(left, right);
      }
      // One Argument functions
      else
      {
	real32 argument = evaluateExpression(reversePolish);


	if(function == "sin") return sin(argument);
	else if(function == "cos") return cos(argument);
	else if(function == "abs") return abs(argument);
	else if(function == "floor") return floor(argument);
	else if(function == "ceil") return ceil(argument);
	else std::cout << "Function: " << function << " doesn't exist" << std::endl;
      }
    } break;
  }

  // It's a value
  // ^^^^^^
  // Whaaaaat ?
  return std::stof(entry.value);
}

real32 SimpleParser::evaluateExpression(std::string expression) const
{
  EntryList reversePolish = getReversePolish(expression);

  if(reversePolish.front().type == ET_INVALID)
  {
    std::cout << "Invalid Atom starting at: " << reversePolish.front().value  << std::endl;
    std::cout << "In Expression: " << expression  << std::endl << std::endl;

    return 0;
  }

  real32 result;
  result = evaluateExpression(reversePolish);
  return result;
}

real32
blendSmooth(real32 val1, real32 val2, real32 t, real32 blendRange)
{
  real32 result = 0;
  // Value is in interpolated range
  if(t > (0.5f - blendRange) && t < (0.5f + blendRange))
  {
    // Calculating t value
    real32 resultT = ((t - (0.5f - blendRange)) / (blendRange * 2.0f));
    result = val1 + (val2 - val1) * resultT;
  }
  // Value is lower then blend region
  else if(t < 0.5f) result = val1;
  // Value is higer then blend region
  else result = val2;

  return result;
}
