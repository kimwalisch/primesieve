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
#include <cctype>

ArithmeticExpression::ArithmeticExpression() : variable_("mr4tkXui6esOr"),
    maxLength_(128), result_(0), isDigits_(false) {
}

std::string ArithmeticExpression::getErrorMessage() const {
 return errorMessage_.str();
}

uint64_t ArithmeticExpression::getResult() const {
  return result_;
}

/**
 * @return true if the last expression is a plain integer.
 */
bool ArithmeticExpression::isDigits() const {
  return isDigits_;
}

/**
 * Evaluate an arithmetic expression with Parsifal's evaluator.
 * @brief readme.html and ArithmeticExpression.h contain further
 *        information.
 * @return true if expression has successfully been evaluated.
 */
bool ArithmeticExpression::evaluateParsifal(std::string expression) {
  int correctColumn = 0;
  size_t delimiter  = expression.find_first_of(";\r\n");
  size_t pos        = expression.substr(0, delimiter).find('=');
  if (pos == std::string::npos || (
      pos > 0 &&
      pos + 1 < expression.size() && (
      expression[pos - 1] == '!' ||
      expression[pos - 1] == '<' ||
      expression[pos - 1] == '>' ||
      expression[pos + 1] == '='))) {
    correctColumn = static_cast<int> (variable_.length()) + 1;
    // insert a variable if none present
    expression.insert(0, variable_ + "=");
  }
  // Parsifal's expression evaluator requires a char*
  char* expr = new char[expression.length() + 1];
  for (size_t i = 0; i < expression.length(); i++)
    expr[i] = expression[i];
  expr[expression.length()] = (char) 0;
  // evaluate the expression
  int errorFlag = evaluateExpression(expr);
  delete expr;
  // the expression is erroneous
  if (errorFlag != 0) {
    errorRecord.column -= correctColumn;
    errorRecord.message[0] = std::tolower(errorRecord.message[0]);
    errorMessage_ << errorRecord.message
                  << " at column "
                  << errorRecord.column;
    return false;
  }
  // check for unkown variables
  for (int i = 0; i < N_VARIABLES && i < nVariables; i++) {
    if (variable[i].value == UINT64_MAX &&
        variable_.compare(variable[i].name) != 0) {
      errorMessage_ << "\""
                    << variable[i].name
                    << "\" unkown variable";
      return false;
    }
  }
  // current is a pointer to the last used variable
  result_ = current->value;
  return true;
}

/**
 * Evaluate a string that holds an arithmetic expression to a 64 bit
 * unsigned integer.
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
 * "2 ** 2 ** (0+2 *2+1)"            = 4294967296
 *
 * @warning As 64 bit unsigned integers are used for all calculations
 *          one has to be careful with divisions:
 *          i.e. (10/6)*10 = 10
 *          and negative numbers:
 *          i.e. -100 = 18446744073709551516
 *          but -100+1e10 = 9999999900 is OK
 *
 * @return  true if the expression has successfully been evaluated,
 *          false if an error occurred
 */
bool ArithmeticExpression::evaluate(const std::string& expression) {
  // reset member variables
  result_ = 0;
  isDigits_ = false;
  // max length error check
  if (expression.size() > maxLength_) {
    errorMessage_ << "expression exceeds limit of "
                  << maxLength_
                  << " characters";
    return false;
  }
  // do not allow floating point numbers or floating point functions
  // with two or more arguments
  size_t pos = expression.find_first_of(".,");
  if (pos != std::string::npos) {
    errorMessage_ << "invalid character \'"
                  << expression[pos]
                  << "\' at column "
                  << pos + 1;
    return false;
  }
  // evaluate the arithmetic expression with Parsifal's evaluator
  if (!this->evaluateParsifal(expression))
    return false;
  // true if the expression contains only digits
  isDigits_ = (expression.length() > 0 &&
      expression.find_first_not_of("0123456789") == std::string::npos);
  return true;
}
