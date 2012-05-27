//
// Copyright (c) 2012 Kim Walisch, <kim.walisch@gmail.com>.
// All rights reserved.
//
// This file is part of ExpressionParser.
// Visit: http://expressionparser.googlecode.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of the author nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.


/// @warning By default ^ is the bitwise exclusive OR operator in
///          ExpressionParser. For primesieve I modified ^ to be a
///          raise to power operator just like **.

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <exception>
#include <string>
#include <sstream>
#include <stack>
#include <cstddef>
#include <cctype>

class parser_error;

/// ExpressionParser
/// @brief   ExpressionParser.h is a simple C++ operator precedence
///          parser with infix notation for integer arithmetic
///          expressions. Its `T eval(const std::string&)' function
///          evaluates an arithmetic expression and returns the result.
///          Have a look at the homepage for a usage example.
/// @see     http://expressionparser.googlecode.com
/// @author  Kim Walisch, <kim.walisch@gmail.com>
/// @version 2.2
/// @date    May, 27 2012
///
/// == Supported operators ==
///
/// OPERATOR    NAME                     ASSOCIATIVITY    PRECEDENCE
///
/// |           Bitwise Inclusive OR    Left               4
/// &           Bitwise AND             Left               6
/// <<          Shift Left              Left               9
/// >>          Shift Right             Left               9
/// +           Addition                Left              10
/// -           Subtraction             Left              10
/// *           Multiplication          Left              20
/// /           Division                Left              20
/// %           Modulo                  Left              20
/// ^, **       Raise to power          Right             30
/// e, E        Scientific notation     Right             40
/// ~           Unary complement        Left              99
///
/// The operator precedence has been set according to (uses the C and
/// C++ operator precedence): http://en.wikipedia.org/wiki/Order_of_operations
/// Operators with higher precedence are evaluated before operators
/// with relatively lower precedence. Unary operators are set to have
/// the highest precedence, this is not strictly correct for the power
/// operator e.g. "-3**2" = 9 but a lot of software tools (Bash shell,
/// Microsoft Excel, GNU bc, ...) use the same convention.
///
/// == Examples of valid expressions ==
///
/// "65536 >> 15"                       = 2
/// "2**16"                             = 65536
/// "(0 + 0xDf234 - 1000)*3/2%999"      = 828
/// "-(2**2**2**2)"                     = -65536
/// "(0 + ~(0xDF234 & 1000) *3) /-2"    = 817
/// "(2**16) + (1 << 16) >> 0X5"        = 4096
/// "5*-(2**(9+7))/3+5*(1 & 0xFf123)"   = -109221
///
/// == About the algorithm used ==
///
/// ExpressionParser has its roots in a JavaScript parser published at:
/// http://stackoverflow.com/questions/28256/equation-expression-parser-with-precedence/114961#114961
/// The same author has also published an article about his operator
/// precedence algorithm at PerlMonks:
/// http://www.perlmonks.org/?node_id=554516
///
template <typename T = int>
class ExpressionParser {
public:
  /// Evaluate an integer arithmetic expression and return its result.
  /// @throw parser_error if parsing fails.
  ///
  T eval(const std::string& expr) {
    T result = 0;
    try {
      if (expr.size() > EXPRESSION_MAX_SIZE) {
        std::ostringstream error;
        error << "Parser error: expression exceeds limit of "
              << EXPRESSION_MAX_SIZE
              << " characters";
        throw parser_error(expr_, error.str());
      }
      index_ = 0;
      expr_ = expr;
      result = parseExpr();
      if (!isEnd()) throw parser_error(expr_, index_);
    }
    catch (parser_error& e) {
      while(!opv_.empty()) opv_.pop();
      throw e;
    }
    return result;
  }

  /// Get the integer value of a char e.g. 5 for '5'
  T eval(char c) { std::string expr(1, c); return eval(expr); }

private:
  enum {
    EXPRESSION_MAX_SIZE = 32767
  };

  enum {
    OPERATOR_NULL,
    OPERATOR_BITWISE_OR,     /// |
    OPERATOR_BITWISE_XOR,    /// ^
    OPERATOR_BITWISE_AND,    /// &
    OPERATOR_BITWISE_SHL,    /// <<
    OPERATOR_BITWISE_SHR,    /// >>
    OPERATOR_ADDITION,       /// +
    OPERATOR_SUBTRACTION,    /// -
    OPERATOR_MULTIPLICATION, /// *
    OPERATOR_DIVISION,       /// /
    OPERATOR_MODULO,         /// %
    OPERATOR_POWER,          /// **
    OPERATOR_EXPONENT        /// e, E
  };

  struct Operator {
    /// Operator, one of the OPERATOR_* enum definitions
    int op;
    int precedence;
    /// Associativity, 'L' = left or 'R' = right
    int assoc;
    Operator(int op, int precedence, int assoc) :
      op(op),
      precedence(precedence),
      assoc(assoc) { }
  };

  struct OperatorValue {
    Operator op;
    T value;
    OperatorValue(const Operator& op, T value) :
      op(op),
      value(value) { }
    int getPrecedence()   const { return op.precedence; }
    bool isOperatorNULL() const { return op.op == OPERATOR_NULL; }
  };

  /// Expression string
  std::string expr_;
  /// Current expression index, incremented whilst parsing
  std::size_t index_;
  /// The current operator and its left value are pushed onto the
  /// stack if the operator on top of the stack has lower precedence.
  std::stack<OperatorValue> opv_;

  /// Exponentiation by squaring, x^n.
  T pow(T x, T n) const {
    T result = 1;
    while (n != 0) {
      if ((n & 1) != 0) {
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
      std::string divOperators("/%");
      std::size_t division = expr_.find_last_of(divOperators, index_ - 2);
      std::ostringstream error;
      error << "Parser error: division by 0";
      if (division != std::string::npos)
        error << " (error token is \""
              << expr_.substr(division, expr_.size() - division)
              << "\")";
      throw parser_error(expr_, error.str());
    }
    return value;
  }

  /// Atomic calculation with two operands and a given operator.
  /// @return Result (int, long, ...) of the calculation.
  ///
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
    return 0;
  }

  bool isEnd() const {
    return index_ >= expr_.size();
  }

  /// Returns the character at the current expression index or
  /// 0 if the end of the expression is reached.
  ///
  char getCharacter() const {
    if (!isEnd()) return expr_[index_];
    return 0;
  }

  /// Parse str at the current expression index.
  /// @throw parser_error if parsing fails.
  ///
  void expect(const std::string str) {
    if (expr_.compare(index_, str.size(), str) != 0)
      throw parser_error(expr_, index_);
    index_ += str.size();
  }

  /// Eat all white space characters at the current expression index.
  void eatSpaces() {
    while (std::isspace(getCharacter()) != 0)
      index_++;
  }

  /// Parse a binary operator at the current expression index.
  /// @return Operator with precedence and associativity.
  ///
  Operator parseOp() {
    eatSpaces();
    switch (getCharacter()) {
      case '|': index_++;     return Operator(OPERATOR_BITWISE_OR,      4, 'L');
      case '&': index_++;     return Operator(OPERATOR_BITWISE_AND,     6, 'L');
      case '<': expect("<<"); return Operator(OPERATOR_BITWISE_SHL,     9, 'L');
      case '>': expect(">>"); return Operator(OPERATOR_BITWISE_SHR,     9, 'L');
      case '+': index_++;     return Operator(OPERATOR_ADDITION,       10, 'L');
      case '-': index_++;     return Operator(OPERATOR_SUBTRACTION,    10, 'L');
      case '/': index_++;     return Operator(OPERATOR_DIVISION,       20, 'L');
      case '%': index_++;     return Operator(OPERATOR_MODULO,         20, 'L');
      case '*': index_++; if (getCharacter() != '*')
                              return Operator(OPERATOR_MULTIPLICATION, 20, 'L');
      case '^': index_++;     return Operator(OPERATOR_POWER,          30, 'R');
      case 'e': index_++;     return Operator(OPERATOR_EXPONENT,       40, 'R');
      case 'E': index_++;     return Operator(OPERATOR_EXPONENT,       40, 'R');
    }
    // OPERATOR_NULL used for:
    // 1. End of expression (char) 0
    // 2. Closing parentheses `)'
    // 3. Invalid characters (errors)
    return Operator(OPERATOR_NULL, 0, 'L');
  }

  /// Returns the integer value of the character c e.g. 9 for '9' or
  /// 15 for f (hexadecimal number). If c is not a decimal or
  /// hexadecimal character a value > 0xf is returned.
  ///
  T toInteger(char c) const {
    if (c >= '0' && c <= '9') return static_cast<T> (c -'0');
    if (c >= 'a' && c <= 'f') return static_cast<T> (c -'a' + 0xa);
    if (c >= 'A' && c <= 'F') return static_cast<T> (c -'A' + 0xa);
    return 0xf + 1;
  }

  T parseDecimal() {
    T value = 0;
    for (T d; (d = toInteger(getCharacter())) <= 9; index_++)
      value = value * 10 + d;
    return value;
  }

  /// Parse a hexadecimal number, i.e. 0x7fff.
  /// @pre "0x" has already been consumed.
  ///
  T parseHexadecimal() {
    T value = 0;
    for (T h; (h = toInteger(getCharacter())) <= 0xf; index_++)
      value = value * 16 + h;
    return value;
  }

  /// Parse an integer value (hex or decimal) at the current
  /// expression index. Also handles the unary `+', `-' and `~'
  /// operators and opening parentheses `(' using recursion.
  ///
  T parseVal() {
    T val = 0;
    eatSpaces();
    switch (getCharacter()) {
      case '0': if (index_ + 2 < expr_.size() &&
                    std::tolower(expr_[index_ + 1]) == 'x' &&
                       toInteger(expr_[index_ + 2]) <= 0xf) {
                  index_ += 2;
                  return parseHexadecimal();
                }
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
                return parseDecimal();
      case '(': index_++;
                val = parseExpr();
                eatSpaces();
                if (getCharacter() != ')') {
                  if (!isEnd())
                    throw parser_error(expr_, index_);
                  throw parser_error(expr_, "Syntax error: `)' expected at end of expression");
                }
                index_++; return  val;
      case '+': index_++; return  parseVal();
      case '-': index_++; return  parseVal() * static_cast<T> (-1);
      case '~': index_++; return ~parseVal();
      default:
        if (!isEnd())
          throw parser_error(expr_, index_);
        throw parser_error(expr_, "Syntax error: value expected at end of expression");
    }
    return 0;
  }

  /// Parse all operations of the current parenthesis level and
  /// the levels above (parseVal() causes recursion).
  /// @return The combined result of the parsed operations.
  ///
  T parseExpr() {
    opv_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
    // first value on the left
    T value = parseVal();
    while (!opv_.empty()) {
      // parse an operator (+, -, *, ...)
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
    return 0;
  }
};

/// ExpressionParser throws a parser_error if it fails
/// to evaluate the expression string.
///
class parser_error : public std::exception {
public:
  parser_error(const std::string& expr,
               const std::string& error) :
    expr_(expr), error_(error) { }
  parser_error(const std::string& expr, std::size_t index) :
    expr_(expr)
  {
    std::ostringstream error;
    error << "Syntax error: unexpected token \""
          << expr.substr(index, expr.size() - index)
          << "\" at index " << index;
    error_ = error.str();
  }
  virtual ~parser_error() throw() { }
  virtual const char* what() const throw() { return error_.c_str(); }
  std::string expression() const { return expr_; }
private:
  std::string expr_;
  std::string error_;
};

#endif
