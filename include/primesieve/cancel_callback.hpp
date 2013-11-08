///
/// @file  cancel_callback.hpp
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CANCEL_CALLBACK_HPP
#define CANCEL_CALLBACK_HPP

#include <exception>

namespace primesieve {

class cancel_callback : public std::exception { };

} // namespace primesieve

#endif
