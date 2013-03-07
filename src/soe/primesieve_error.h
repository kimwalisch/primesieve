///
/// @file   primesieve_error.h
/// @brief  The primesieve_error class is used for all exceptions
///         within primesieve.
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef PRIMESIEVE_ERROR_H
#define PRIMESIEVE_ERROR_H

#include <stdexcept>
#include <string>

class primesieve_error : public std::runtime_error {
public:
  primesieve_error(const std::string& message)
    : std::runtime_error(message)
  { }
};

#endif
