///
/// @file      calculator.hpp
/// @brief     calculator::eval(const std::string&) evaluates an integer
///            arithmetic expression and returns the result. If an error
///            occurs a calculator::error exception is thrown.
///            <https://github.com/kimwalisch/calculator>
/// @author    Kim Walisch, <kim.walisch@gmail.com>
/// @copyright Copyright (C) 2013-2025 Kim Walisch
/// @license   BSD 2-Clause, https://opensource.org/licenses/BSD-2-Clause
/// @version   2.0
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
/// C++ operator precedence): https://en.wikipedia.org/wiki/Order_of_operations
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
/// calculator::eval(std::string&) relies on the ExpressionParser
/// class which is a simple C++ operator precedence parser with infix
/// notation for integer arithmetic expressions.
/// ExpressionParser has its roots in a JavaScript parser published
/// at: http://stackoverflow.com/questions/28256/equation-expression-parser-with-precedence/114961#114961
/// The same author has also published an article about his operator
/// precedence algorithm at PerlMonks:
/// http://www.perlmonks.org/?node_id=554516
///

#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace calculator
{

/// calculator::eval() throws a calculator::error if it fails
/// to evaluate the expression string.
///
class error : public std::runtime_error
{
public:
  error(const std::string& msg)
    : std::runtime_error(msg)
  { }
};

template <typename T>
class ExpressionParser
{
public:
  /// Evaluate an integer arithmetic expression and return its result.
  /// @throw calculator::error if parsing fails.
  ///
  T eval(const std::string& expr)
  {
    // Prevent denial of service attacks
    if (expr.size() >= 10000)
      throw calculator::error("Error: math expression string exceeds 10000 characters!");

    expr_ = expr;
    index_ = 0;
    T result = parseExpr();

    if (!isEnd())
      throw_unexpected();

    return result;
  }

private:
  enum
  {
    OPERATOR_NULL,
    OPERATOR_BITWISE_OR,     /// |
    OPERATOR_BITWISE_AND,    /// &
    OPERATOR_BITWISE_SHL,    /// <<
    OPERATOR_BITWISE_SHR,    /// >>
    OPERATOR_ADDITION,       /// +
    OPERATOR_SUBTRACTION,    /// -
    OPERATOR_MULTIPLICATION, /// *
    OPERATOR_DIVISION,       /// /
    OPERATOR_MODULO,         /// %
    OPERATOR_POWER,          /// ^, **
    OPERATOR_EXPONENT        /// e, E
  };

  struct Operator
  {
    /// Operator, one of the OPERATOR_* enum definitions
    int op;
    int precedence;
    /// 'L' = left or 'R' = right
    int associativity;
    Operator(int opr, int prec, int assoc) :
      op(opr),
      precedence(prec),
      associativity(assoc)
    { }
  };

  struct OperatorValue
  {
    Operator op;
    T value;
    OperatorValue(const Operator& opr, T val) :
      op(opr),
      value(val)
    { }
    int getPrecedence() const
    {
      return op.precedence;
    }
    bool isNull() const
    {
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

  void throw_unexpected() const
  {
    std::ostringstream msg;
    msg << "Syntax error: unexpected token '"
        << expr_.substr(index_, expr_.size() - index_)
        << "' at index " << index_
        << " of math expression '" << expr_ << "'";
    throw calculator::error(msg.str());
  }

  void throw_integer_underflow_error() const
  {
    std::ostringstream msg;
    msg << "Error: " << numBits() << "-bit "
        << (is_signed() ? "signed" : "unsigned")
        << " integer underflow detected ";

    if (expr_.find_first_not_of("0123456789 \t\n\r\f\v") == std::string::npos)
      msg << "in string to integer conversion of '" << expr_ << "'";
    else
      msg << "at index " << index_ << " of math expression '" << expr_ << "'";

    throw calculator::error(msg.str());
  }

  void throw_integer_overflow_error() const
  {
    std::ostringstream msg;
    msg << "Error: " << numBits() << "-bit "
        << (is_signed() ? "signed" : "unsigned")
        << " integer overflow detected ";

    if (expr_.find_first_not_of("0123456789 \t\n\r\f\v") == std::string::npos)
      msg << "in string to integer conversion of '" << expr_ << "'";
    else
      msg << "at index " << index_ << " of math expression '" << expr_ << "'";

    throw calculator::error(msg.str());
  }

  void throw_division_by_0_error() const
  {
    std::ostringstream msg;
    msg << "Error: division by 0 at index " << index_ << " of math expression '" << expr_ << "'";
    throw calculator::error(msg.str());
  }

  void throw_modulo_by_0_error() const
  {
    std::ostringstream msg;
    msg << "Error: modulo by 0 at index " << index_ << " of math expression '" << expr_ << "'";
    throw calculator::error(msg.str());
  }

  /// Same as std::is_unsigned<T>::value
  /// but also works with __uint128_t.
  static constexpr bool is_unsigned()
  {
    // Second cast required for sizeof(T) < sizeof(int)
    // due to C/C++ integer promotion rules.
    return T(~T(0)) > 0;
  }

  /// Same as std::is_signed<T>::value
  /// but also works with __int128_t.
  static constexpr bool is_signed()
  {
    return !is_unsigned();
  }

  /// Same as std::numeric_limits<T>::digit
  /// but also works with __int128_t.
  static constexpr std::size_t numBits()
  {
    return sizeof(T) * CHAR_BIT;
  }

  /// Same as std::numeric_limits<T>::min
  /// but also works with __uint128_t.
  template <typename TT = T>
  static typename std::enable_if<is_unsigned(), TT>::type
  minValue()
  {
    return 0;
  }

  /// Same as std::numeric_limits<T>::min
  /// but also works with __int128_t.
  template <typename TT = T>
  static typename std::enable_if<is_signed(), TT>::type
  minValue()
  {
    T halfMagnitude = T(1) << (numBits() - 2);
    return -halfMagnitude - halfMagnitude;
  }

  /// Same as std::numeric_limits<T>::max
  /// but also works with __uint128_t.
  template <typename TT = T>
  static typename std::enable_if<is_unsigned(), TT>::type
  maxValue()
  {
    return T(~T(0));
  }

  /// Same as std::numeric_limits<T>::max
  /// but also works with __int128_t.
  template <typename TT = T>
  static typename std::enable_if<is_signed(), TT>::type
  maxValue()
  {
    T halfMagnitude = T(1) << (numBits() - 2);
    return halfMagnitude | (halfMagnitude - T(1));
  }

  template <typename TT = T>
  typename std::enable_if<is_unsigned(), TT>::type
  checked_add(T x, T y) const
  {
    if (x > maxValue() - y)
      throw_integer_overflow_error();

    return x + y;
  }

  template <typename TT = T>
  typename std::enable_if<is_signed(), TT>::type
  checked_add(T x, T y) const
  {
    if (x > 0 && y > 0) {
      if (x > maxValue() - y)
        throw_integer_overflow_error();
    }
    else if (x < 0 && y < 0) {
      if (x < minValue() - y)
        throw_integer_underflow_error();
    }

    return x + y;
  }

  template <typename TT = T>
  typename std::enable_if<is_unsigned(), TT>::type
  checked_sub(T x, T y) const
  {
    if (x < y)
      throw_integer_underflow_error();

    return x - y;
  }

  template <typename TT = T>
  typename std::enable_if<is_signed(), TT>::type
  checked_sub(T x, T y) const
  {
    if (x > 0 && y < 0) {
      if (x > maxValue() + y)
        throw_integer_overflow_error();
    }
    else if (x < 0 && y > 0) {
      if (x < minValue() + y)
        throw_integer_underflow_error();
    }

    return x - y;
  }

  template <typename TT = T>
  typename std::enable_if<is_unsigned(), TT>::type
  checked_mul(T x, T y) const
  {
    // Prevent division by 0
    if (y == 0)
      return 0;

    if (x > maxValue() / y)
      throw_integer_overflow_error();

    return x * y;
  }

  template <typename TT = T>
  typename std::enable_if<is_signed(), TT>::type
  checked_mul(T x, T y) const
  {
    // Prevent division by 0
    if (x == 0 || y == 0)
      return 0;

    if (x > 0)
    {
      if (y > 0) {
        if (x > maxValue() / y)
          throw_integer_overflow_error();
      }
      else { // x > 0 && y < 0
        if (y < minValue() / x)
          throw_integer_underflow_error();
      }
    }
    else // x < 0
    {
      if (y > 0) {
        if (x < minValue() / y)
          throw_integer_underflow_error();
      }
      else // x < 0 && y < 0
      {
        // INT_MIN * -1 causes integer overflow
        if (x == -1 && y == minValue())
          throw_integer_overflow_error();
        if (y == -1 && x == minValue())
          throw_integer_overflow_error();

        if (x < maxValue() / y)
          throw_integer_overflow_error();
      }
    }

    return x * y;
  }

  template <typename TT = T>
  typename std::enable_if<is_unsigned(), TT>::type
  checked_div(T x, T y) const
  {
    if (y == 0)
      throw_division_by_0_error();

    return x / y;
  }

  template <typename TT = T>
  typename std::enable_if<is_signed(), TT>::type
  checked_div(T x, T y) const
  {
    if (y == 0)
      throw_division_by_0_error();

    if (x == minValue() && y == -1)
      throw_integer_overflow_error();

    return x / y;
  }

  template <typename TT = T>
  typename std::enable_if<is_unsigned(), TT>::type
  checked_modulo(T x, T y) const
  {
    if (y == 0)
      throw_modulo_by_0_error();

    return x % y;
  }

  template <typename TT = T>
  typename std::enable_if<is_signed(), TT>::type
  checked_modulo(T x, T y) const
  {
    if (y == 0)
      throw_modulo_by_0_error();

    if (x == minValue() && y == -1)
      throw_integer_overflow_error();

    return x % y;
  }

  /// Calculate x^n using an exponentiation by
  /// squaring algorithm for integers.
  ///
  T ipow(T x, T n) const
  {
    // For 0^0 we use the same convention as
    // std::pow(0, 0) which returns 1.
    if (x == 1 || n == 0)
      return 1;

    if (x == 0)
    {
      if (n > 0)
        return 0;
      // 0^-n = 1/0^n = 1/0
      if (is_signed() && n <= T(-1))
        throw_division_by_0_error();
    }

    // Handle -1^n and x^-n
    if (is_signed())
    {
      if (x == T(-1))
        return (n % 2 == 0) ? 1 : T(-1);
      // Here x != -1, 0, 1
      if (n <= T(-1))
        return 0;
    }

    T res = 1;

    while (n > 0)
    {
      if (n % 2 != 0)
      {
        res = checked_mul(res, x);
        n -= 1;
      }
      n /= 2;

      if (n > 0)
        x = checked_mul(x, x);
    }

    return res;
  }

  T calculate(T v1, T v2, const Operator& op) const
  {
    switch (op.op)
    {
      case OPERATOR_BITWISE_OR:     return v1 | v2;
      case OPERATOR_BITWISE_AND:    return v1 & v2;
      case OPERATOR_BITWISE_SHL:    return v1 << v2;
      case OPERATOR_BITWISE_SHR:    return v1 >> v2;
      case OPERATOR_ADDITION:       return checked_add(v1, v2);
      case OPERATOR_SUBTRACTION:    return checked_sub(v1, v2);
      case OPERATOR_MULTIPLICATION: return checked_mul(v1, v2);
      case OPERATOR_DIVISION:       return checked_div(v1, v2);
      case OPERATOR_MODULO:         return checked_modulo(v1, v2);
      case OPERATOR_POWER:          return ipow(v1, v2);
      case OPERATOR_EXPONENT:       return checked_mul(v1, ipow(10, v2));
      default:                      return 0;
    }
  }

  bool isEnd() const
  {
    return index_ >= expr_.size();
  }

  /// Returns the character at the current expression index or
  /// 0 if the end of the expression is reached.
  ///
  char getCharacter() const
  {
    if (!isEnd())
      return expr_[index_];
    return 0;
  }

  /// Parse str at the current expression index.
  /// @throw error if parsing fails.
  ///
  void expect(const std::string& str)
  {
    if (expr_.compare(index_, str.size(), str) != 0)
      throw_unexpected();
    index_ += str.size();
  }

  /// Eat all white space characters at the
  /// current expression index.
  ///
  void eatSpaces()
  {
    while (std::isspace(getCharacter()) != 0)
      index_++;
  }

  /// Parse a binary operator at the current expression index.
  /// @return Operator with precedence and associativity.
  ///
  Operator parseOp()
  {
    eatSpaces();
    switch (getCharacter())
    {
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
                index_++;     return Operator(OPERATOR_POWER,          30, 'R');
      case '^': index_++;     return Operator(OPERATOR_POWER,          30, 'R');
      case 'e': index_++;     return Operator(OPERATOR_EXPONENT,       40, 'R');
      case 'E': index_++;     return Operator(OPERATOR_EXPONENT,       40, 'R');
      default :               return Operator(OPERATOR_NULL,            0, 'L');
    }
  }

  static T toInteger(char c)
  {
    if (c >= '0' && c <= '9') return c -'0';
    if (c >= 'a' && c <= 'f') return c -'a' + 0xa;
    if (c >= 'A' && c <= 'F') return c -'A' + 0xa;
    T noDigit = 0xf + 1;
    return noDigit;
  }

  T getInteger() const
  {
    return toInteger(getCharacter());
  }

  T parseDecimal()
  {
    T value = 0;

    for (T d; (d = getInteger()) <= 9; index_++)
    {
      value = checked_mul(value, 10);
      value = checked_add(value, d);
    }

    return value;
  }

  T parseHex()
  {
    index_ = index_ + 2;
    T value = 0;

    for (T h; (h = getInteger()) <= 0xf; index_++)
    {
      value = checked_mul(value, 0x10);
      value = checked_add(value, h);
    }

    return value;
  }

  bool isHex() const
  {
    if (index_ + 2 < expr_.size())
    {
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
  T parseValue()
  {
    T val = 0;
    eatSpaces();
    switch (getCharacter())
    {
      case '0': if (isHex())
                  val = parseHex();
                else
                  val = parseDecimal();
                break;
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
                val = parseDecimal();
                break;
      case '(': index_++;
                val = parseExpr();
                eatSpaces();
                if (getCharacter() != ')')
                {
                  if (!isEnd())
                    throw_unexpected();
                  throw calculator::error("Syntax error: `)' expected at end of math expression '" + expr_ + "'");
                }
                index_++; break;
      case '~': index_++; val = ~parseValue(); break;
      case '+': index_++; val =  parseValue(); break;
      case '-': index_++;
                // For e.g. uint64_t x = 100
                // -x = 18446744073709551516
                // If we would later use this value to e.g.
                // calculate -100+200 we would trigger
                // an integer overflow exception due to:
                // 18446744073709551516 + 200 > 2^64-1
                if (is_unsigned())
                  throw_integer_underflow_error();

                val =  parseValue();

                // For e.g. val = min(int64_t):
                // -min(int64_t) = -(-2^63) = 2^63
                // but 2^63 > max(int64_t)
                if (is_signed() && val == minValue())
                  throw_integer_overflow_error();
                else
                   val *= T(-1);

                break;
      default : if (!isEnd())
                  throw_unexpected();
                throw calculator::error("Syntax error: value expected at end of math expression '" + expr_ + "'");
    }
    return val;
  }

  /// Parse all operations of the current parenthesis
  /// level and the levels above, when done
  /// return the result (value).
  ///
  T parseExpr()
  {
    stack_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
    // first parse value on the left
    T value = parseValue();

    while (!stack_.empty())
    {
      // parse an operator (+, -, *, ...)
      Operator op(parseOp());
      while (op.precedence  < stack_.top().getPrecedence() || (
             op.precedence == stack_.top().getPrecedence() &&
             op.associativity == 'L'))
      {
        // end reached
        if (stack_.top().isNull())
        {
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

template <typename T>
inline T eval(const std::string& expression)
{
  ExpressionParser<T> parser;
  return parser.eval(expression);
}

inline std::int64_t eval(const std::string& expression)
{
  return eval<std::int64_t>(expression);
}

} // namespace calculator

#endif
