#ifndef TOSTRING_PRIMESIEVE_H
#define TOSTRING_PRIMESIEVE_H

#include <string>
#include <sstream>

namespace soe {

template <typename T>
inline std::string toString(T t)
{
  std::ostringstream oss;
  oss << t;
  return oss.str();
}

} // namespace soe

#endif
