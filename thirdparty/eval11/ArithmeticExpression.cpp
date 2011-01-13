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
#include <algorithm>

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
 * unsigned integer. Examples:
 *
 * "1e10"  = 10000000000
 * "2**32" = 4294967296
 *
 * @warning As 64 bit unsigned integers for all calculations one has
 *          to be careful with divisions:
 *
 *          (10/6)*10000 = 10000
 */
bool ArithmeticExpression::evaluate(std::string expression) {
  expression_ =  expression;
  // modify the copy rather than the expression_ member variable
  expression.insert(0,"x=");
  // convert the expression to char*
  char* writable = new char[expression.size() + 1];
  std::copy(expression.begin(), expression.end(), writable);
  writable[expression.size()] = '\0';
  // evaluate the expression
  int errorFlag = evaluateExpression(writable);
  delete writable;
  if (errorFlag) {
    errorMessage_ << errorRecord.message
                  << " at column "
                  << errorRecord.column - 2;
    return false;
  }
  result_ = variable[0].value;
  return true;
}
