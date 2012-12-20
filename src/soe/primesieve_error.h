#ifndef PRIMESIEVE_ERROR_H
#define PRIMESIEVE_ERROR_H

#include <stdexcept>
#include <string>

/// PrimeSieve objects throw primesieve_error exceptions for
/// invalid arguments like start > stop.
///
class primesieve_error : public std::runtime_error {
public:
  primesieve_error(const std::string& message)
    : std::runtime_error(message)
  { }
};

#endif
