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

int64_t ArithmeticExpression::getResult() const {
  return result_;
}

/**
 * @return true if the expression contains only digits
 */
bool ArithmeticExpression::isPlainInteger() const {
  return (expression_.size() > 0 &&
          expression_.find_first_not_of("0123456789") == std::string::npos);
}

/**
 * Evaluates a string that holds an arithmetic expression to a 64 bit
 * integer.
 * 
 * EXAMPLES of valid expressions:
 *
 * "3+5"                            = 8
 * "2**32"                          = 4294967296
 * "1e18+1e10"                      = 1000000010000000000
 * "23*5+(7*2**32/(1e18%555))"      = 67561398
 * "x = 333"                        = 333
 * "sqrt( 10**14 )"                 = 10000000
 * "(5 < 8) ?1 :1e10+2**32"         = 1
 * 2 ** 2 ** (0+2 *2+1)"            = 4294967296
 *
 * @warning As 64 bit integers are used for all calculations one has
 *          to be careful with divisions:
 *          i.e. (10/6)*10 = 10
 * @return  true if the expression has successfully been evaluated,
 *          false if an error occurred
 */
bool ArithmeticExpression::evaluate(std::string expression) {
  if (expression.size() > 128) {
    errorMessage_ << "expression exceeds limit of 128 characters";
    return false;
  }
  // do not allow floating point numbers or floating point functions
  // with two or more arguments
  size_t pos = expression.find_first_of(".,");
  if (pos != std::string::npos) {
    errorMessage_ << "error token is \""
                  << expression.substr(pos, std::string::npos)
                  << "\""
                  << std::endl;
    return false;
  }
  bool addVariable = false;
  // save the original expression
  expression_ =  expression;
  pos = expression.find_first_of('=');
  if (pos == std::string::npos || (
      pos > 0 &&
      pos + 1 < expression.size() && (
      expression[pos - 1] == '!' ||
      expression[pos - 1] == '<' ||
      expression[pos - 1] == '>' ||
      expression[pos + 1] == '='))) {
    addVariable = true;
    // modify the copy
    expression.insert(0, "a=");
  }
  // Parsifal's expression evaluator requires a char*
  // + 2 for "a="
  // + 1 for NULL
  char expr[128 + 2 + 1];
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
  // check for uninitialized variable
  if (variable[0].value == INT64_MIN &&
      expression_.compare("-9223372036854775808") != 0) {
    // "a=" is in index 0
    errorMessage_ << "\"" << variable[1].name << "\""
                  << " unkown variable";
    return false;
  }
  // variable[0] always holds the result of the last expression
  result_ = variable[0].value;
  return true;
}
