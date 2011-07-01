/*
 * ExpressionParser.h -- This file is part of ExpressionParser
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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <exception>
#include <string>
#include <sstream>
#include <stack>
#include <cstddef> /* std::size_t */
#include <cctype>  /* std::isspace(int) */
#include <cassert>

/**
 * ExpressionParser.
 * @brief   ExpressionParser.h is a simple C++ operator precedence
 *          parser with infix notation for integer arithmetic
 *          expressions. Its eval(const std::string&) function
 *          evaluates an arithmetic expression passed as string
 *          argument, getResult() returns the corresponding result.
 * @see     http://expressionparser.googlecode.com
 * @author  Kim Walisch, <kim.walisch@gmail.com>
 * @version 1.2
 * @date    July, 01 2011
 *
 * == Supported operators ==
 *
 *     OPERATOR     NAME                     ASSOCIATIVITY     PRECEDENCE
 *
 *     or,  OR      Bitwise Inclusive OR     Left               4
 *     xor, XOR     Bitwise Exclusive OR     Left               5
 *     and, AND     Bitwise AND              Left               6
 *     not, NOT     Unary complement         Left              99
 *     shl, SHL     Shift Left               Left               9
 *     shr, SHR     Shift Right              Left               9
 *     +            Addition                 Left              10
 *     -            Subtraction              Left              10
 *     *            Multiplication           Left              20
 *     /            Division                 Left              20
 *     %            Modulo                   Left              20
 *     ^, **        Raise to power           Right             30
 *     e, E         Scientific notation      Right             40
 *
 * The operator precedence has been set according to (uses the C and
 * C++ operator precedence):
 * http://en.wikipedia.org/wiki/Order_of_operations
 * Operators with higher precedence are evaluated before operators
 * with relatively lower precedence.
 *
 * NOTE: Unary operators are set to have the highest precedence. This
 *       is not strictly correct for the power operator
 *       e.g. "-3^2" = 9 but a lot of software tools (Microsoft
 *       Excel, Bash shell, GNU bc, ...) use the same convention.
 *
 * == Examples of valid expressions ==
 *
 *     "2^16"                                  = 65536
 *     "2^16 shr 15"                           = 2
 *     "(0 + 0xdf234 - 1000) * 3 / 2 % 999"    = 828
 *     "-(2^2^2^2)"                            = -65536
 *     "(0 + not (0xDF234 and 1000) * 3) /-2"  = 817
 *     "(2^16) + (1 SHL 16) shr 0X5"           = 4096
 *     "5*-(2^(9+7))/3+5*(1 AND 0xFf123)"      = -109221
 *
 * == About the algorithm used ==
 *
 * ExpressionParser has its roots in a JavaScript parser published at:
 * http://stackoverflow.com/questions/28256/equation-expression-parser-with-precedence/114961#114961
 * The same author has also published an article about his operator
 * precedence algorithm at PerlMonks:
 * http://www.perlmonks.org/?node_id=554516
 */
template<typename T>
class ExpressionParser {
private:
  enum {
    OPERATOR_NULL,
    OPERATOR_BITWISE_OR,      /// or,  OR
    OPERATOR_BITWISE_XOR,     /// xor, XOR
    OPERATOR_BITWISE_AND,     /// and, AND
    OPERATOR_BITWISE_SHL,     /// shl, SHL
    OPERATOR_BITWISE_SHR,     /// shr, SHR
    OPERATOR_ADDITION,        /// +
    OPERATOR_SUBTRACTION,     /// -
    OPERATOR_MULTIPLICATION,  /// *
    OPERATOR_DIVISION,        /// /
    OPERATOR_MODULO,          /// %
    OPERATOR_POWER,           /// ^, **
    OPERATOR_EXPONENT         /// e, E
  };
  
  enum {
    /// default max length (32 KB of characters) for the expression string
    EXPRESSION_MAX_LENGTH = 32767
  };

  class NoAssign {
  protected:
    NoAssign() {}
    ~NoAssign() {}
  private:
    NoAssign& operator=(const NoAssign&);
  };

  class Operator : private NoAssign {
  public:
    /// Operator, one of the OPERATOR_+ enum definitions
    const int op;
    const int precedence;
    /// Associativity, 'L' = left or 'R' = right
    const int assoc;
    Operator(const Operator& op) : op(op.op), precedence(op.precedence),
        assoc(op.assoc) {
    }
    Operator(int op, int precedence, int assoc) : op(op),
        precedence(precedence), assoc(assoc) {
    }
  };

  class OperatorValue : private NoAssign {
  public:
    const Operator op;
    const T value;
    OperatorValue(const Operator& op, T value) : op(op), value(value) {
    }
    int getPrecedence() const {
      return op.precedence;
    }
    bool isOperatorNULL() const {
      return op.op == OPERATOR_NULL;
    }
  };

  /// Expression string
  std::string expr_;
  /// Current expression offset, incremented whilst parsing
  std::size_t offset_;
  /**
   * The current operator and its left value are pushed onto the
   * stack if the operator on top of the stack has lower precedence.
   */
  std::stack<OperatorValue> opv_;
  /// Result of the evaluated expression
  T result_;
  /// Maximum length for user input
  std::size_t maxLength_;
  /// true if the last expression has been evaluated without errors
  bool isSuccess_;
  /// Error message if eval(const std::string&) failed
  std::ostringstream error_;

  /**
   * Integer pow, raise to power, x^n.
   * Code from (ported to C++ from Ruby):
   * http://en.wikipedia.org/wiki/Exponentiation_by_squaring
   */
  T pow(T x, T n) const {
    T result = 1;
    while (n != 0) {
      if (n & 1) {
        result *= x;
        n -= 1;
      }
      x *= x;
      n /= 2;
    }
    return result;
  }

  T checkZero(T value) {
    if (value == 0) {
      assert(offset_ >= 2);
      const std::string divOperators("/%");
      std::size_t division = expr_.find_last_of(divOperators, offset_-2);
      error_ << "Parser error: division by 0";
      if (division != std::string::npos)
        error_ << " (error token is \""
               << expr_.substr(division, expr_.length() - division)
               << "\")";
      throw std::exception();
    }
    return value;
  }

  /**
   * Atomic calculation with two operands and a given operator.
   * @return Result of the calculation.
   */
  T calculate(T v1, T v2, const Operator& op) {
    switch (op.op) {
      case OPERATOR_BITWISE_OR:     return v1 | v2;
      case OPERATOR_BITWISE_XOR:    return v1 ^ v2;
      case OPERATOR_BITWISE_AND:    return v1 & v2;
      case OPERATOR_BITWISE_SHL:    return v1 << v2;
      case OPERATOR_BITWISE_SHR:    return v1 >> v2;
      case OPERATOR_ADDITION:       return v1 + v2;
      case OPERATOR_SUBTRACTION:    return v1 - v2;
      case OPERATOR_MULTIPLICATION: return v1 * v2;
      case OPERATOR_DIVISION:       return v1 / checkZero(v2);
      case OPERATOR_MODULO:         return v1 % checkZero(v2);
      case OPERATOR_POWER:          return pow(v1, v2);
      case OPERATOR_EXPONENT:       return v1 * pow(10, v2);
    }
    assert(false); // never reached
    return 0;
  }

  bool isEndOfExpression() const {
    return offset_ >= expr_.length();
  }

  /**
   * @return The character at the current expression offset or 0 if
   *         the end of the expression is reached.
   */
  char getCurrentCharacter() const {
    if (!isEndOfExpression())
      return expr_[offset_];
    return (char) 0;
  }

  /**
   * Compares str at the current expression offset and eats it on
   * success.
   * @return true if str has been eaten else false.
   */
  bool eatString(const std::string& str) {
    if (offset_ + str.length() <= expr_.length() &&
        expr_.compare(offset_, str.length(), str) == 0) {
      offset_ += str.length();
      return true;
    }
    return false;
  }

  /**
   * Eat all white space characters at the current expression offset.
   */
  void eatSpaces() {
    while (std::isspace(getCurrentCharacter()) != 0)
      offset_++;
  }

  /**
   * Consume a binary operator at the current expression offset.
   * @return Operator object with precedence and associativity.
   * Order precedence set according to:
   * http://en.wikipedia.org/wiki/Order_of_operations
   */
  Operator parseOp() {
    eatSpaces();
    switch (getCurrentCharacter()) {
      case 'o': if (eatString("or"))
                  return Operator(OPERATOR_BITWISE_OR,       4, 'L');
                throw std::exception();
      case 'O': if (eatString("OR"))
                  return Operator(OPERATOR_BITWISE_OR,       4, 'L');
                throw std::exception();
      case 'x': if (eatString("xor"))
                  return Operator(OPERATOR_BITWISE_XOR,      5, 'L');
                throw std::exception();
      case 'X': if (eatString("XOR"))
                  return Operator(OPERATOR_BITWISE_XOR,      5, 'L');
                throw std::exception();
      case 'a': if (eatString("and"))
                  return Operator(OPERATOR_BITWISE_AND,      6, 'L');
                throw std::exception();
      case 'A': if (eatString("AND"))
                  return Operator(OPERATOR_BITWISE_AND,      6, 'L');
                throw std::exception();
      case 's': if (eatString("shl"))
                  return Operator(OPERATOR_BITWISE_SHL,      9, 'L');
                if (eatString("shr"))
                  return Operator(OPERATOR_BITWISE_SHR,      9, 'L');
                throw std::exception();
      case 'S': if (eatString("SHL"))
                  return Operator(OPERATOR_BITWISE_SHL,      9, 'L');
                if (eatString("SHR"))
                  return Operator(OPERATOR_BITWISE_SHR,      9, 'L');
                throw std::exception();
      case '+': offset_++;
                return Operator(OPERATOR_ADDITION,          10, 'L');
      case '-': offset_++;
                return Operator(OPERATOR_SUBTRACTION,       10, 'L');
      case '/': offset_++;
                return Operator(OPERATOR_DIVISION,          20, 'L');
      case '%': offset_++;
                return Operator(OPERATOR_MODULO,            20, 'L');
      case '*': offset_++;
                if (getCurrentCharacter() != '*')
                  return Operator(OPERATOR_MULTIPLICATION,  20, 'L');
                offset_++;
                return Operator(OPERATOR_POWER,             30, 'R');
      case '^': offset_++;
                return Operator(OPERATOR_POWER,             30, 'R');
      case 'e': offset_++;
                return Operator(OPERATOR_EXPONENT,          40, 'R');
      case 'E': offset_++;
                return Operator(OPERATOR_EXPONENT,          40, 'R');
    }
    // Operator NULL used for:
    //
    // 1. Closing parentheses `)'
    // 2. End of expression (char) 0
    // 3. Other characters not handled in switch (errors)
    //
    return Operator(OPERATOR_NULL, 0, 'L');
  }

  /**
   * @return The integer value of the character c, e.g. 9 for '9' or
   *         15 for f (hexadecimal number).
   *         If c is not a decimal or hexadecimal character a
   *         value > 0xf is returned.
   */
  T toInteger(char c) const {
    switch(c) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        return static_cast<T> (c -'0');
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        return static_cast<T> (0xa + c -'A');
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        return static_cast<T> (0xa + c -'a');
    }
    return 0xf + 1;
  }

  T parseDecimal() {
    assert(toInteger(getCurrentCharacter()) <= 9);
    T value = 0;
    T d = 0;
    for (;(d = toInteger(getCurrentCharacter())) <= 9; offset_++)
      value = value * 10 + d;
    return value;
  }

  /**
   * Parse a hexadecimal number, i.e. 0x7fff.
   * @pre "0x" or "0X" has already been consumed.
   */
  T parseHexadecimal() {
    assert(toInteger(getCurrentCharacter()) <= 0xf);
    T value = 0;
    T h = 0;
    for (;(h = toInteger(getCurrentCharacter())) <= 0xf; offset_++)
      value = value * 16 + h;
    return value;
  }

  /**
   * Parse an integer value (hex or decimal) at the current
   * expression offset. Also handles the unary `+', `-' and "NOT"
   * operators and opening parentheses `(' using recursion.
   */
  T parseVal() {
    eatSpaces();
    T value = 0;
    switch (getCurrentCharacter()) {
      case '0': if (offset_ + 2 < expr_.length() &&
                    (expr_[offset_ + 1] == 'x' ||
                     expr_[offset_ + 1] == 'X') &&
                    toInteger(expr_[offset_ + 2]) <= 0xf) {
                  offset_ += 2;
                  return parseHexadecimal();
                }
      case '1': case '2': case '3': case '4': case '5': case '6':
      case '7': case '8': case '9':
                return parseDecimal();
      case '(': offset_++;
                value = parseExpr();
                eatSpaces();
                if (getCurrentCharacter() != ')') {
                 if (isEndOfExpression())
                   error_ << "Syntax error: `)' expected at end of expression";
                  throw std::exception();
                }
                offset_++; return  value;
      case '+': offset_++; return  parseVal();
                // cast used to silence warning C4146 (MSVC)
      case '-': offset_++; return static_cast<T> (-1) * parseVal();
      case 'n': if (eatString("not"))
                  return ~parseVal();
                throw std::exception();
      case 'N': if (eatString("NOT"))
                  return ~parseVal();
                throw std::exception();
      default:
        if (isEndOfExpression())
          error_ << "Syntax error: value expected at end of expression";
        throw std::exception();
    }
    assert(false); // never reached
    return 0;
  }

  /**
   * Parse all operations of the current parenthesis level and the
   * levels above (parseVal() causes recursion).
   * @return The combined result of the parsed operations.
   */
  T parseExpr() {
    opv_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
    // first value on the left
    T value = parseVal();
    for (;;) {
      Operator op(parseOp());
      while (op.precedence < opv_.top().getPrecedence() || (
             op.precedence == opv_.top().getPrecedence() &&
             op.assoc == 'L')) {
        // end reached
        if (opv_.top().isOperatorNULL()) {
          opv_.pop();
          return value;
        }
        // do the calculation ("reduce"), producing a new value
        value = calculate(opv_.top().value, value, opv_.top().op);
        opv_.pop();
      }
      // store on opv_ and continue parsing ("shift")
      opv_.push(OperatorValue(op, value));
      // value on the right
      value = parseVal();
    }
    assert(false); // never reached
    return 0;
  }

public:
  ExpressionParser() :
    result_(0), maxLength_(EXPRESSION_MAX_LENGTH), isSuccess_(true) {
  }

  ExpressionParser(const std::string& expr) :
    result_(0), maxLength_(EXPRESSION_MAX_LENGTH) {
    isSuccess_ = this->eval(expr);
  }

  ExpressionParser(const ExpressionParser& parser) {
    if (this != &parser) {
      expr_      = parser.expr_;
      result_    = parser.result_;
      maxLength_ = parser.maxLength_;
      isSuccess_ = parser.isSuccess_;
      error_.clear();
      error_.str(parser.error_.str());
    }
  }

  ExpressionParser& operator=(const ExpressionParser& parser) {
    if (this != &parser) {
      expr_      = parser.expr_;
      result_    = parser.result_;
      maxLength_ = parser.maxLength_;
      isSuccess_ = parser.isSuccess_;
      error_.clear();
      error_.str(parser.error_.str());
    }
    return *this;
  }

  /**
   * Get the last evaluated expression.
   */
  std::string getExpression() const {
    return expr_;
  }

  /**
   * True if the last expression has been evaluated without errors
   * else false.
   */
  bool isSuccess() const {
    return isSuccess_;
  }
  /**
   * Result of the last expression if eval(const std::string&) has
   * been successful else 0.
   */
  T getResult() const {
    return result_;
  }

  /**
   * Error message of the last expression if eval(const std::string&)
   * failed or !isSuccess().
   */
  std::string getErrorMessage() const {
    return error_.str();
  }

  /**
   * Get the maximum expression length.
   */
  std::size_t getMaxLength() const {
    return maxLength_;
  }

  /**
   * Set the maximum expression length.
   */
  void setMaxLength(std::size_t maxLength) {
    maxLength_ = maxLength;
  }

  /**
   * Evaluate an integer expression.
   * @return true if expr has correctly been evaluated false if an
   *         error occurred.
   *
   * == Examples of valid expressions ==
   *
   *     "2^16"                                  = 65536
   *     "2^16 shr 15"                           = 2
   *     "(0 + 0xdf234 - 1000) * 3 / 2 % 999"    = 828
   *     "-(2^2^2^2)"                            = -65536
   *     "(0 + not (0xDF234 and 1000) * 3) /-2"  = 817
   *     "(2^16) + (1 SHL 16) shr 0X5"           = 4096
   *     "5*-(2^(9+7))/3+5*(1 AND 0xFf123)"      = -109221
   */
  bool eval(const std::string& expr) {
    try {
      error_.clear();
      error_.str(std::string());
      offset_ = 0;
      if (expr.length() > maxLength_) {
        expr_ = "";
        error_ << "Parser error: expression exceeds limit of "
               << maxLength_
               << " characters";
        throw std::exception();
      }
      expr_ = expr;
      // evaluate the expression
      result_ = parseExpr();
      if (isEndOfExpression() == false)
        throw std::exception();
      // stack is empty here i.e. all operators have been consumed
      assert(opv_.size() == 0);
      isSuccess_ = true;
    }
    catch(...) {
      // clear the stack for next usage
      while(!opv_.empty()) opv_.pop();
      result_ = 0;
      if (error_.str().length() == 0)
        error_ << "Syntax error: unexpected token \""
               << expr_.substr(offset_, expr_.length() - offset_)
               << "\" at index "
               << offset_;
      isSuccess_ = false;
    }
    return isSuccess_;
  }
};

#endif /* EXPRESSIONPARSER_H */
