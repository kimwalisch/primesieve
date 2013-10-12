///
/// @file  cancel_callback.h
///
/// Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
///
/// This file is distributed under the BSD License. See the COPYING
/// file in the top level directory.
///

#ifndef CANCEL_CALLBACK_H
#define CANCEL_CALLBACK_H

#include <exception>

class cancel_callback : public std::exception { };

#endif
