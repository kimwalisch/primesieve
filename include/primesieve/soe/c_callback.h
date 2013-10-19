/**
 *  @file   c_callback.h
 *  @brief  Callback types with extern "C" linkage.
 * 
 *  Copyright (C) 2013 Kim Walisch, <kim.walisch@gmail.com>
 * 
 *  This file is distributed under the BSD License. See the COPYING
 *  file in the top level directory.
 */

#ifndef C_CALLBACK_H
#define C_CALLBACK_H

#include <stdint.h>

extern "C"
{
  typedef void (*c_callback_t)(uint64_t);
  typedef void (*c_callback_tn_t)(uint64_t, int);
}

#endif
