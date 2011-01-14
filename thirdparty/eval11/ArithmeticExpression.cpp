/*
 * ArithmeticExpression.cpp -- This file is part of primesieve
 *
 * Copyright (C) 2011 Kim Walisch, <kim.walisch@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "ArithmeticExpression.h"

ArithmeticExpression::ArithmeticExpression() : result_(0) {
}

std::string ArithmeticExpression::getExpression() const {
  return expression_;
}

std::string ArithmeticExpression::getErrorMessage() const {
 return errorMessage_.str();
}

uint64_t ArithmeticExpression::getResult() const {
  return result_;
}

/**
 * Evaluates a string that holds an arithmetic expression to a 64 bit
 * unsigned integer.
 * 
 * Examples:
 *
 * "1e10"       = 10000000000
 * "1e18+2**32" = 1000000004294967296
 *
 * @warning As 64 bit unsigned integers are used for all calculations
 *          one has to be careful with divisions and negative numbers:
 *          i.e. (10/6)*10 = 10
 */
bool ArithmeticExpression::evaluate(std::string expression) {
  if (expression.size() > 128) {
    errorMessage_ << "expression size must not exceed 128 characters!";
    return false;
  }
  // save the original expression
  expression_ =  expression;
  size_t pos = expression.find_first_of('=');
  bool addVariable = false;
  if (pos == std::string::npos) {
    addVariable = true;
    // modify the copy
    expression.insert(0, "a=");
  }
  else if (pos > 0 && pos + 1 < expression.size()) {
    // check if the first '=' belongs to a logical expression
    if (expression[pos - 1] == '!' ||
        expression[pos - 1] == '<' ||
        expression[pos - 1] == '>' ||
        expression[pos + 1] == '=') {
      addVariable = true;
      // modify the copy
      expression.insert(0, "a=");
    }
  }
  // Parsifals expression evaluator requires a char*
  // + 1 for NULL
  // + 2 for "a="
  char expr[128 + 1 + 2];
  for (size_t i = 0; i < expression.size(); i++)
    expr[i] = expression[i];
  expr[expression.size()] = (char) 0;
  // evaluate the expression
  int errorFlag = evaluateExpression(expr);
  if (errorFlag) {
    if (addVariable == true)
      errorRecord.column -= 2;
    errorMessage_ << errorRecord.message
                  << " at column "
                  << errorRecord.column;
    return false;
  }
  // check for uninitialized variables
  if (variable[0].value == UINT64_MAX) {
    // "a=" is in index 0
    errorMessage_ << "\"" << variable[1].name << "\""
                  << " has not been initialized!";
    return false;
  }
  // variable[0] always holds the result of the last expression
  result_ = variable[0].value;
  return true;
}
