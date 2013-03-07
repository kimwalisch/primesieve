///
/// @file   ExpressionParser.h
/// @brief  Simple C++ operator precedence parser with infix notation
///         for integer arithmetic expressions.
/// @see    http://expressionparser.googlecode.com
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <stdexcept>
#include <string>
#include <sstream>
#include <stack>
#include <cstddef>
#include <cctype>

/// ExpressionParser throws a parser_error if it fails
/// to evaluate the expression string.
///
class parser_error : public std::runtime_error {
public:
  parser_error(const std::string& expr, const std::string& message)
    : std::runtime_error(message),
      expr_(expr)
  { }
  ~parser_error() throw() { }
  std::string expression() const {
    return expr_;
  }
private:
  std::string expr_;
};

/// @class   ExpressionParser<T>
/// @brief   ExpressionParser is a simple C++ operator precedence
///          parser with infix notation for integer arithmetic
///          expressions. Its `T eval(const std::string&)' method
///          evaluates an arithmetic expression and returns the result.
///          Have a look at the homepage for a usage example.
/// @author  Kim Walisch, <kim.walisch@gmail.com>
/// @version 2.5 patched: `^' is raise to power instead of XOR.
/// @date    March 07, 2013
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
    index_ = 0;
    expr_ = expr;
    try {
      result = parseExpr();
      if (!isEnd())
        unexpected();
    }
    catch (parser_error&) {
      while(!stack_.empty()) stack_.pop();
      throw;
    }
    return result;
  }

  /// Get the integer value of a character.
  T eval(char c) {
    std::string expr(1, c);
    return eval(expr);
  }

private:
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
    /// 'L' = left or 'R' = right
    int associativity;
    Operator(int opr, int precedence, int associativity) :
      op(opr),
      precedence(precedence),
      associativity(associativity)
    { }
  };

  struct OperatorValue {
    Operator op;
    T value;
    OperatorValue(const Operator& opr, T value) :
      op(opr),
      value(value)
    { }
    int getPrecedence() const {
      return op.precedence;
    }
    bool isNull() const {
      return op.op == OPERATOR_NULL;
    }
  };

  /// Expression string
  std::string expr_;
  /// Current expression index, incremented whilst parsing
  std::size_t index_;
  /// The current operator and its left value
  /// are pushed onto the stack if the operator on
  /// top of the stack has lower precedence.
  std::stack<OperatorValue> stack_;

  /// Exponentiation by squaring, x^n.
  static T pow(T x, T n) {
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

  T checkZero(T value) const {
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
  T calculate(T v1, T v2, const Operator& op) const {
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
      default:                      return 0;
    }
  }

  bool isEnd() const {
    return index_ >= expr_.size();
  }

  /// Returns the character at the current expression index or
  /// 0 if the end of the expression is reached.
  ///
  char getCharacter() const {
    if (!isEnd())
      return expr_[index_];
    return 0;
  }

  /// Parse str at the current expression index.
  /// @throw parser_error if parsing fails.
  ///
  void expect(const std::string& str) {
    if (expr_.compare(index_, str.size(), str) != 0)
      unexpected();
    index_ += str.size();
  }

  void unexpected() const {
    std::ostringstream error;
    error << "Syntax error: unexpected token \""
          << expr_.substr(index_, expr_.size() - index_)
          << "\" at index "
          << index_;
    throw parser_error(expr_, error.str());
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
      default :               return Operator(OPERATOR_NULL,            0, 'L');
    }
  }

  static T toInteger(char c) {
    if (c >= '0' && c <= '9') return c -'0';
    if (c >= 'a' && c <= 'f') return c -'a' + 0xA;
    if (c >= 'A' && c <= 'F') return c -'A' + 0xA;
    T noDigit = 0xF + 1;
    return noDigit;
  }

  T parseDecimal() {
    T value = 0;
    for (T d; (d = toInteger(getCharacter())) <= 9; index_++)
      value = value * 10 + d;
    return value;
  }

  T parseHex() {
    index_ = index_ + 2;
    T value = 0;
    for (T h; (h = toInteger(getCharacter())) <= 0xf; index_++)
      value = value * 16 + h;
    return value;
  }

  bool isHex() const {
    if (index_ + 2 < expr_.size()) {
      char x = expr_[index_ + 1];
      char h = expr_[index_ + 2];
      return (std::tolower(x) == 'x' && toInteger(h) <= 0xf);
    }
    return false;
  }

  /// Parse an integer value at the current expression index.
  /// The unary `+', `-' and `~' operators and opening
  /// parentheses `(' cause recursion.
  ///
  T parseValue() {
    T val = 0;
    eatSpaces();
    switch (getCharacter()) {
      case '0': if (isHex()) {
                  val = parseHex();
                  break;
                }
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
                val = parseDecimal();
                break;
      case '(': index_++;
                val = parseExpr();
                eatSpaces();
                if (getCharacter() != ')') {
                  if (!isEnd())
                    unexpected();
                  throw parser_error(expr_, "Syntax error: `)' expected at end of expression");
                }
                index_++; break;
      case '~': index_++; val = ~parseValue(); break;
      case '+': index_++; val =  parseValue(); break;
      case '-': index_++; val =  parseValue() * static_cast<T>(-1);
                break;
      default : if (!isEnd())
                  unexpected();
                throw parser_error(expr_, "Syntax error: value expected at end of expression");
    }
    return val;
  }

  /// Parse all operations of the current parenthesis
  /// level and the levels above, when done
  /// return the result (value).
  ///
  T parseExpr() {
    stack_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
    // first parse value on the left
    T value = parseValue();

    while (!stack_.empty()) {
      // parse an operator (+, -, *, ...)
      Operator op(parseOp());
      while (op.precedence  < stack_.top().getPrecedence() || (
             op.precedence == stack_.top().getPrecedence() &&
             op.associativity == 'L')) {
        // end reached
        if (stack_.top().isNull()) {
          stack_.pop();
          return value;
        }
        // do the calculation ("reduce"), producing a new value
        value = calculate(stack_.top().value, value, stack_.top().op);
        stack_.pop();
      }

      // store on stack_ and continue parsing ("shift")
      stack_.push(OperatorValue(op, value));
      // parse value on the right
      value = parseValue();
    }
    return 0;
  }
};

#endif
