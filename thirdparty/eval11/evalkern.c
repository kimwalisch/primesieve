/**
 * @brief This file has been modified for use in primesieve
 * <http://primesieve.googlecode.com>.
 * @author Kim Walisch <kim.walisch@gmail.com>
 * Last updated: January 2011
 *
 * CHANGES:
 *
 * 1. Use of extern "C" for usage in C++ project
 * 2. double changed to uint64_t (better precision near 1e19) type
 *    from stdint.h
 * 3. Uninitialized variables are set to UINT64_MAX instead
 *    of 0
 * 4. Removed use of strdup (not ANSI) and sprintf (causes 
 *    unsafe warnings)
 * 5. Added a current pointer which points to the last used
 *    variable
 * 6. Added (char*) cast for strings to silence warnings
 * 7. Unused file evaldemo.c has been renamed to evaldemo.c.unused
 *
 * NOTE:
 *
 * The original source archive can be obtained from:
 * http://www.parsifalsoft.com/examples/evalexpression/index.html
 */
 
 #ifdef __cplusplus
extern "C" {
#endif

/*
 EVALKERN.SYN  Version 1.1

 evaluateExpression: A Simple Expression Evaluator
 Copyright (c) 1996 - 1999 Parsifal Software, All
 Rights Reserved.

 evaluateExpression is an example program for evaluating
 C-style expressions which demonstrates some of the
 capabilities of the AnaGram parser generator.

 EVALKERN.SYN is the kernel of the example, consisting
 of the definition of the expression parser itself.
 Support functions are defined in EVALWRAP.C. A test
 program is defined in EVALDEMO.C. Global declarations
 are contained in EVALDEFS.H.

 EVALKERN.SYN is an AnaGram syntax (.SYN) file which
 describes, in this case, the syntax of expressions. It
 is written in the AnaGram language, which is a variang of
 BNF (Backus-Naur Form) with additional constructs to
 allow more concise descriptions and greater analytical
 power. AnaGram analyzes a syntax file containing a
 grammar and produces a parser in C or C++.

 The vital part of a syntax file consists of its
 productions, each of which takes the form:
        p -> p1, p2, p3, ... pn
 Productions describe the grammatical elements to be
 recognized by the parser. This rule says that the
 grammatical element p consists of a single instance
 each of the elements p1, p2, p3, ... pn in order.

 You will notice that a production may have some C code
 at its right end following an equals sign. This code
 constitutes a "reduction procedure", which will be
 executed automatically when the grammatical element
 specified by the production is identified in the input
 to the parser.

 AnaGram analyzes EVALKERN.SYN and produces the parser
 file EVALKERN.C and a header file EVALKERN.H.

 The parse function defined in EVALKERN.SYN is called
 evalKernel. All communication with evalKernel is via
 the parser control block (PCB). The wrapper function,
 evaluateExpression, defined in EVALWRAP.C, provides
 a more convenient interface for the parse function.

 The expression syntax is borrowed from C but with the
 addition of the FORTRAN exponentiation operator (**).

 The cast, increment, and decrement operators are not
 implemented, nor are the following operations that
 are defined only for integers:
   Bitwise logical operators:   &, |, ^, ~, &=, |=, ^=
   Remainder operators:         %, %=
   Shift operators:             <<, >>, >>=, <<=

 The supported operations are:
   Assignment operators:        =, +=, -=, *=, /=
   Conditional expressions:     ? :
   Logical operators:           !, &&, ||
   Comparison operators:        ==, !=, <, <=, >, >=
   Binary arithmetic operators: +, -, *, /
   Exponentiation:              **
   Unary arithmetic operators:  +, -
   Parentheses
   Function calls

 All arithmetic is double precision floating point.

 Input strings may contain any number of expressions, separated by
 commas or semicolons. White space may be used freely, including
 both C and C++ style comments.

 evalKernel() makes the following external calls:
 ------------------------------------------------
   void pushChar(int character);
     Push the specified character onto a character stack.

   double *locateVariable(int nameLength);
     Pop the last nameLength characters from the character stack
     and, treating them as the name of a variable, return a pointer
     to the location where the value of the variable is stored.

   void pushArg(double value);
     Push the specified value onto an argument stack.

   double callFunction(nameLength, int argCount);
     Pop the last nameLength characters from the character stack
     and, treating them as the name of a function, identify the
     function and invoke it with argCount arguments popped from
     the argument stack.

   double checkZero(double value);
     Verify that value is not zero.

 Overrides for macros defined by AnaGram, such as SYNTAX_ERROR
 should be included in EVALDEFS.H

 For information about AnaGram, contact
   Parsifal Software
   http://www.parsifalsoft.com
   info@parsifalsoft.com
   1-800-879-2755, Voice/Fax 1-508-358-2564
   P.O. Box 219
   Wayland, MA 01778
*/

#include <stdint.h>
#include <math.h>
#include "evaldefs.h"                  // defines external interface


/*

 AnaGram Parsing Engine
 Copyright (c) 1993-1999, Parsifal Software.
 All Rights Reserved.

 Serial number 2P17253U
 Registered to:
   AnaGram 2.01 Release Version
   Parsifal Software

*/

#ifndef EVALKERN_H
#include "evalkern.h"
#endif

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RULE_CONTEXT (&((PCB).cs[(PCB).ssx]))
#define ERROR_CONTEXT ((PCB).cs[(PCB).error_frame_ssx])
#define CONTEXT ((PCB).cs[(PCB).ssx])



evalKernel_pcb_type evalKernel_pcb;
#define PCB evalKernel_pcb

#ifndef CONVERT_CASE
#define CONVERT_CASE(c) (c)
#endif
#ifndef TAB_SPACING
#define TAB_SPACING 8
#endif

/* used instead of pow(double, double) from math.h which has a
   poor integer precision past 1e15 */

static uint64_t ipow(uint64_t x, int64_t n) 
{
  uint64_t result = 1;
  if (n < 0)
    return 0;
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

#define ag_rp_1(k, x) (*locateVariable(k)  = x)

#define ag_rp_2(k, x) (*locateVariable(k) += x)

#define ag_rp_3(k, x) (*locateVariable(k) -= x)

#define ag_rp_4(k, x) (*locateVariable(k) *= x)

#define ag_rp_5(k, x) (*locateVariable(k) /= x)

#define ag_rp_6(c, x, y) (c?x:y)

#define ag_rp_7(x, y) (x||y)

#define ag_rp_8(x, y) (x&&y)

#define ag_rp_9(x, y) (x==y)

#define ag_rp_10(x, y) (x!=y)

#define ag_rp_11(x, y) (x<y)

#define ag_rp_12(x, y) (x<=y)

#define ag_rp_13(x, y) (x>y)

#define ag_rp_14(x, y) (x>=y)

#define ag_rp_15(x, y) (x+y)

#define ag_rp_16(x, y) (x-y)

#define ag_rp_17(x, y) (x*y)

#define ag_rp_18(x, y) (x/checkZero(y))

#define ag_rp_19(x) (x)

/* #define ag_rp_20(x) (-x) */

/* does the same but does not cause warnings */
#define ag_rp_20(x) ((uint64_t)(-1*(int64_t)x))

#define ag_rp_21(x) (x)

/* #define ag_rp_22(x, y) (pow(x,y)) */

/* Causes errors:
  mingw/g++ 4.5.1 x64, i.e. x=100000000; y=x**2; y = 10000000000000034 
  #define ag_rp_22(x, y) ((uint64_t)pow((double)x,(double)y)) */
  
#define ag_rp_22(x, y) (ipow(x,(int64_t)y))

#define ag_rp_23(k) (*locateVariable(k))

#define ag_rp_24(x) (x)

#define ag_rp_25(x) (!x)

#define ag_rp_26(k, n) (callFunction(k,n))

#define ag_rp_27() (0)

#define ag_rp_28(x) (pushArg(x), 1)

#define ag_rp_29(k, x) (pushArg(x), k+1)

/* #define ag_rp_30(x, e) (x*pow(10,e)) */

/* Causes errors:
  mingw/g++ 4.5.1 x64, i.e. 1e16 = 10000000000000034 
  #define ag_rp_30(x, e) (x*(uint64_t)pow(10.0,(double)e)) */

#define ag_rp_30(x, e) (x*ipow(10,(int64_t)e))

/* #define ag_rp_31(x, e) (x*pow(10,-e)) */

#define ag_rp_31(x, e) (x*ipow(10,-1*(int64_t)e))

#define ag_rp_32(i, f) (i+f)

#define ag_rp_33(f) (f)

#define ag_rp_34(d) (d-'0')

#define ag_rp_35(x, d) (10*x + d-'0')

/* #define ag_rp_36(d) ((d-'0')/10.) */

#define ag_rp_36(d) ((uint64_t)((d-'0')/10.))

/* #define ag_rp_37(d, f) ((d-'0' + f)/10.) */

#define ag_rp_37(d, f) ((uint64_t)((d-'0' + f)/10.))

#define ag_rp_38(d) (d-'0')

#define ag_rp_39(x, d) (10*x + d-'0')

#define ag_rp_40(c) (pushChar(c), 1)

#define ag_rp_41(k, c) (pushChar(c), k+1)


#define READ_COUNTS 
#define WRITE_COUNTS 
#undef V
#define V(i,t) (*(t *) (&(PCB).vs[(PCB).ssx + i]))
#undef VS
#define VS(i) (PCB).vs[(PCB).ssx + i]

#ifndef GET_CONTEXT
#define GET_CONTEXT CONTEXT = (PCB).input_context
#endif

typedef enum {
  ag_action_1,
  ag_action_2,
  ag_action_3,
  ag_action_4,
  ag_action_5,
  ag_action_6,
  ag_action_7,
  ag_action_8,
  ag_action_9,
  ag_action_10,
  ag_action_11,
  ag_action_12
} ag_parser_action;

#ifndef NULL_VALUE_INITIALIZER
#define NULL_VALUE_INITIALIZER = { 0 }
#endif


static evalKernel_vs_type const ag_null_value NULL_VALUE_INITIALIZER;

static const unsigned char ag_rpx[] = {
    0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  0,  6,  0,  7,  0,  8,
    0,  9, 10,  0, 11, 12, 13, 14,  0, 15, 16,  0, 17, 18, 19, 20, 21,  0,
   22,  0, 23, 24, 25, 26, 27,  0, 28, 29,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 30, 31, 32,  0,  0,  0, 33, 34, 35, 36, 37, 38,
   39, 40, 41
};

static const unsigned char ag_key_itt[] = {
 0
};

static const unsigned short ag_key_pt[] = {
0
};

static const unsigned char ag_key_ch[] = {
    0, 42, 47,255, 47,255, 42,255, 42, 61,255, 42, 47, 61,255, 33, 38, 42,
   43, 45, 47, 60, 61, 62,124,255, 42, 47,255, 33, 38, 42, 47, 60, 61, 62,
  124,255, 33, 38, 42, 60, 61, 62,124,255, 33, 38, 60, 61, 62,124,255, 33,
   38, 61,124,255, 38,124,255,124,255, 42, 61,255, 33, 38, 42, 43, 45, 47,
   60, 61, 62,124,255
};

static const unsigned char ag_key_act[] = {
  0,0,0,4,2,4,3,4,0,0,4,0,0,0,4,3,3,2,3,3,2,3,3,3,3,4,0,0,4,3,3,3,2,3,3,
  3,3,4,3,3,3,3,3,3,3,4,3,3,3,3,3,3,4,3,3,3,3,4,3,3,4,3,4,0,0,4,3,3,2,3,
  3,3,3,3,3,3,4
};

static const unsigned char ag_key_parm[] = {
    0, 47, 52,  0,  0,  0, 51,  0, 94, 78,  0, 47, 52, 79,  0, 85, 83,  0,
   76, 77,  0, 87, 84, 89, 82,  0, 47, 52,  0, 85, 83, 94,  0, 87, 84, 89,
   82,  0, 85, 83, 94, 87, 84, 89, 82,  0, 85, 83, 87, 84, 89, 82,  0, 85,
   83, 84, 82,  0, 83, 82,  0, 82,  0, 94, 78,  0, 85, 83,  0, 76, 77, 79,
   87, 84, 89, 82,  0
};

static const unsigned char ag_key_jmp[] = {
    0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  8,
    6,  8, 11, 10, 12, 14, 16,  0,  0,  0,  0, 18, 20, 22, 26, 24, 26, 28,
   30,  0, 32, 34, 36, 38, 40, 42, 44,  0, 46, 48, 50, 52, 54, 56,  0, 58,
   60, 62, 64,  0, 66, 68,  0, 70,  0,  0,  0,  0, 72, 74, 63, 76, 78, 80,
   82, 84, 86, 88,  0
};

static const unsigned char ag_key_index[] = {
    4,  0,  6, 15,  0,  0,  0,  6,  6,  0, 29, 29,  4,  4, 29,  0,  0,  4,
    4, 38,  0,  0, 46, 46, 46, 53, 58, 15, 61, 66,  0, 29, 29,  0, 38,  0,
    4,  0,  4,  0,  4,  0,  0,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,  0,
    4,  0,  4,  0,  4,  0,  4,  0,  0,  4,  0,  4,  0,  4,  0,  4,  0,  4,
    0,  4,  4,  0,  0,  0, 29, 46, 46, 46, 46, 46, 46, 46, 46, 53, 58,  0,
    0,  0, 29, 29,  4,  0,  0
};

static const unsigned char ag_key_ends[] = {
47,0, 61,0, 38,0, 61,0, 61,0, 61,0, 61,0, 61,0, 124,0, 
61,0, 38,0, 42,0, 61,0, 61,0, 61,0, 124,0, 61,0, 38,0, 42,0, 
61,0, 61,0, 61,0, 124,0, 61,0, 38,0, 61,0, 61,0, 61,0, 124,0, 
61,0, 38,0, 61,0, 124,0, 38,0, 124,0, 124,0, 61,0, 38,0, 
61,0, 61,0, 61,0, 61,0, 61,0, 61,0, 124,0, 
};

#define AG_TCV(x) ag_tcv[(x)]

static const unsigned char ag_tcv[] = {
    6, 70, 70, 70, 70, 70, 70, 70, 70, 69, 56, 69, 69, 69, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 69, 98, 70, 70,
   70, 70, 70, 70, 97, 96, 92, 90, 99, 91, 62, 93, 65, 65, 65, 65, 65, 65,
   65, 65, 65, 65, 80,100, 86, 74, 88, 81, 70, 71, 71, 71, 71, 58, 71, 71,
   71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71,
   71, 70, 70, 70, 70, 71, 70, 71, 71, 71, 71, 58, 71, 71, 71, 71, 71, 71,
   71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
   70, 70, 70, 70
};

#ifndef SYNTAX_ERROR
#define SYNTAX_ERROR fprintf(stderr,"%s, line %d, column %d\n", \
  (PCB).error_message, (PCB).line, (PCB).column)
#endif

#ifndef FIRST_LINE
#define FIRST_LINE 1
#endif

#ifndef FIRST_COLUMN
#define FIRST_COLUMN 1
#endif

#ifndef PARSER_STACK_OVERFLOW
#define PARSER_STACK_OVERFLOW {fprintf(stderr, \
   "\nParser stack overflow, line %d, column %d\n",\
   (PCB).line, (PCB).column);}
#endif

#ifndef REDUCTION_TOKEN_ERROR
#define REDUCTION_TOKEN_ERROR {fprintf(stderr, \
    "\nReduction token error, line %d, column %d\n", \
    (PCB).line, (PCB).column);}
#endif


#ifndef INPUT_CODE
#define INPUT_CODE(T) (T)
#endif

typedef enum
  {ag_accept_key, ag_set_key, ag_jmp_key, ag_end_key, ag_no_match_key,
   ag_cf_accept_key, ag_cf_set_key, ag_cf_end_key} key_words;

static void ag_get_key_word(int ag_k) {
  int ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
  const  unsigned char *ag_p;
  int ag_ch;
  while (1) {
    switch (ag_key_act[ag_k]) {
    case ag_cf_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          int ag_k1 = ag_key_parm[ag_k];
          int ag_k2 = ag_key_pt[ag_k1];
          if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) goto ag_fail;
          (PCB).token_number = (evalKernel_token_type) ag_key_pt[ag_k1 + 1];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
      goto ag_fail;
    }
    case ag_end_key: {
      const  unsigned char *sp = ag_key_ends + ag_key_jmp[ag_k];
      do {
        if ((ag_ch = *sp++) == 0) {
          (PCB).token_number = (evalKernel_token_type) ag_key_parm[ag_k];
          return;
        }
      } while (CONVERT_CASE(*(PCB).la_ptr++) == ag_ch);
    }
    case ag_no_match_key:
ag_fail:
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    case ag_cf_set_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      ag_k = ag_key_jmp[ag_k];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)]) break;
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (evalKernel_token_type) ag_key_pt[ag_k1+1];
      break;
    }
    case ag_set_key:
      ag_save = (int) ((PCB).la_ptr - (PCB).pointer);
      (PCB).token_number = (evalKernel_token_type) ag_key_parm[ag_k];
    case ag_jmp_key:
      ag_k = ag_key_jmp[ag_k];
      break;
    case ag_accept_key:
      (PCB).token_number = (evalKernel_token_type) ag_key_parm[ag_k];
      return;
    case ag_cf_accept_key: {
      int ag_k1 = ag_key_parm[ag_k];
      int ag_k2 = ag_key_pt[ag_k1];
      if (ag_key_itt[ag_k2 + CONVERT_CASE(*(PCB).la_ptr)])
        (PCB).la_ptr = (PCB).pointer + ag_save;
      else (PCB).token_number = (evalKernel_token_type) ag_key_pt[ag_k1+1];
      return;
    }
    }
    ag_ch = CONVERT_CASE(*(PCB).la_ptr++);
    ag_p = &ag_key_ch[ag_k];
    while (*ag_p < ag_ch) ag_p++;
    if (*ag_p != ag_ch) {
      (PCB).la_ptr = (PCB).pointer + ag_save;
      return;
    }
    ag_k = (int) (ag_p - ag_key_ch);
  }
}


static void ag_track(void) {
  int ag_k = (int) ((PCB).la_ptr - (PCB).pointer);
  while (ag_k--) {
    switch (*(PCB).pointer++) {
    case '\n':
      (PCB).column = 1, (PCB).line++;
    case '\r':
    case '\f':
      break;
    case '\t':
      (PCB).column += (TAB_SPACING) - ((PCB).column - 1) % (TAB_SPACING);
      break;
    default:
      (PCB).column++;
    }
  }
}


static void ag_prot(void) {
  int ag_k = 128 - ++(PCB).btsx;
  if (ag_k <= (PCB).ssx) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
    return;
  }
  (PCB).bts[(PCB).btsx] = (PCB).sn;
  (PCB).bts[ag_k] = (PCB).ssx;
  (PCB).vs[ag_k] = (PCB).vs[(PCB).ssx];
  (PCB).ss[ag_k] = (PCB).ss[(PCB).ssx];
}

static void ag_undo(void) {
  if ((PCB).drt == -1) return;
  while ((PCB).btsx) {
    int ag_k = 128 - (PCB).btsx;
    (PCB).sn = (PCB).bts[(PCB).btsx--];
    (PCB).ssx = (PCB).bts[ag_k];
    (PCB).vs[(PCB).ssx] = (PCB).vs[ag_k];
    (PCB).ss[(PCB).ssx] = (PCB).ss[ag_k];
  }
  (PCB).token_number = (evalKernel_token_type) (PCB).drt;
  (PCB).ssx = (PCB).dssx;
  (PCB).sn = (PCB).dsn;
  (PCB).drt = -1;
}


static const unsigned char ag_tstt[] = {
100,99,98,97,91,90,71,69,65,62,58,56,52,47,6,0,1,72,73,
100,99,98,97,96,93,92,91,90,88,86,81,80,74,71,70,69,65,62,58,56,0,54,55,
100,99,98,97,96,93,92,91,90,88,86,81,80,74,71,70,69,65,62,58,56,51,0,49,50,
69,56,52,47,0,1,
100,99,98,97,91,90,71,65,62,58,6,0,2,3,4,5,7,8,10,16,19,21,23,26,31,32,33,
  34,37,38,40,42,57,61,75,95,
100,99,98,97,96,93,92,91,90,88,86,81,80,74,71,70,69,65,62,58,0,
56,0,
100,99,98,97,96,93,92,91,90,88,86,81,80,74,71,70,69,65,62,58,56,0,
51,0,
65,0,63,
100,99,96,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,69,65,62,58,56,52,47,
  6,0,64,
58,0,
98,97,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,96,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
100,99,96,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,69,56,52,47,6,0,1,72,
  73,
98,97,71,65,62,58,0,2,3,38,40,42,57,61,75,95,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
94,0,39,
98,97,91,90,71,65,62,58,0,2,3,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,65,62,58,0,2,3,32,33,34,37,38,40,42,57,61,75,95,
93,92,0,35,36,
91,90,0,32,33,
89,88,87,86,0,27,28,29,30,
85,84,0,24,25,
83,0,22,
100,99,97,96,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,74,71,
  69,65,58,56,52,47,6,0,1,72,73,
82,81,0,17,20,
97,79,78,77,76,74,0,11,12,13,14,15,40,
100,99,6,0,45,68,
65,0,63,
65,0,63,
91,90,65,0,59,
97,0,40,
96,0,41,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,65,62,58,0,2,3,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,65,62,58,0,2,3,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,23,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,23,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,21,23,26,31,32,33,34,37,38,40,42,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,19,21,23,26,31,32,33,34,37,38,40,42,57,61,75,
  95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,96,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,
  42,43,44,57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,
100,99,98,97,91,90,71,69,65,62,58,56,52,47,6,0,1,72,73,
100,99,98,97,91,90,71,69,65,62,58,56,52,47,6,0,1,72,73,
100,99,98,97,91,90,71,65,62,58,6,0,2,3,7,8,10,16,19,21,23,26,31,32,33,34,37,
  38,40,42,57,61,75,95,
65,0,60,
65,0,60,
100,99,96,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,69,56,52,47,6,0,1,72,
  73,
93,92,0,35,36,
93,92,0,35,36,
91,90,0,32,33,
91,90,0,32,33,
91,90,0,32,33,
91,90,0,32,33,
89,88,87,86,0,27,28,29,30,
89,88,87,86,0,27,28,29,30,
85,84,0,24,25,
83,0,22,
80,0,18,
99,0,45,
96,0,41,
65,0,
65,0,
98,97,91,90,71,69,65,62,58,56,52,47,0,1,72,73,
98,97,91,90,71,65,62,58,0,2,3,10,16,19,21,23,26,31,32,33,34,37,38,40,42,57,
  61,75,95,
98,97,91,90,71,65,62,58,0,2,3,7,10,16,19,21,23,26,31,32,33,34,37,38,40,42,
  57,61,75,95,

};


static unsigned const char ag_astt[1522] = {
  8,8,8,8,8,8,8,1,8,8,8,1,1,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,8,7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,8,7,1,1,9,9,1,1,5,3,
  5,5,1,1,1,1,2,2,1,2,5,7,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,5,3,7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,5,3,7,1,7,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,10,1,5,5,5,5,
  5,7,3,1,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,5,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,1,1,1,1,5,7,1,1,3,1,1,2,2,1,2,7,2,1,2,1,
  1,1,1,1,1,1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,
  5,5,5,1,5,5,5,1,1,1,7,1,1,3,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,5,1,1,1,1,1,
  2,2,1,2,7,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,2,2,1,2,7,1,1,1,1,2,2,1,1,1,1,
  1,1,1,1,1,5,1,1,1,1,5,1,1,1,1,1,1,5,1,1,1,1,1,1,5,1,1,1,5,1,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,10,1,10,10,1,1,1,5,7,1,1,3,1,1,5,1,1,1,1,
  1,1,1,1,4,1,1,1,1,1,1,1,1,3,7,1,1,1,4,2,1,5,2,1,1,8,7,1,1,4,1,1,7,2,5,5,5,
  5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,2,2,1,1,1,1,1,1,1,5,5,
  5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,2,2,1,1,1,1,1,1,1,5,
  5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,2,2,1,1,1,1,1,1,1,
  1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,2,1,2,7,1,1,1,1,1,
  1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,
  1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,
  1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,
  1,2,7,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,
  1,2,2,1,2,7,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,
  1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,
  7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,
  5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,
  5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,
  1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,1,1,1,1,1,1,1,
  1,1,1,1,2,1,1,1,1,1,1,1,1,1,4,1,1,2,2,1,2,7,1,1,2,2,1,1,1,1,1,1,1,1,1,2,1,
  1,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,2,2,
  1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,
  2,2,1,2,7,1,1,2,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,
  1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,2,2,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,
  5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,2,2,1,1,1,1,1,1,1,1,1,2,
  1,1,1,1,1,1,1,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,2,2,1,
  1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,5,5,5,5,5,5,5,1,5,5,5,1,1,1,5,7,1,1,3,5,5,
  5,5,5,5,5,1,5,5,5,1,1,1,5,7,1,1,3,5,5,1,1,1,1,2,2,1,2,5,7,1,1,3,3,3,1,1,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,7,1,2,7,1,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,1,1,1,1,5,7,1,1,3,1,1,4,1,1,1,1,4,1,1,1,1,4,1,1,1,1,4,1,1,1,1,4,1,1,1,1,
  4,1,1,1,1,1,1,4,1,1,1,1,1,1,1,1,4,1,1,1,1,1,1,4,1,1,1,4,1,1,7,1,1,5,1,1,7,
  2,10,4,10,4,5,5,5,5,5,1,5,5,5,1,1,1,7,1,1,3,1,1,1,1,2,2,1,2,7,1,1,2,1,1,1,
  1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,2,2,1,2,7,1,1,2,2,1,1,1,1,1,1,1,1,1,2,
  1,1,1,1,1,1,1
};


static const unsigned char ag_pstt[] = {
4,4,4,4,4,4,4,3,4,4,4,3,1,2,4,0,3,3,4,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,1,5,6,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,2,7,8,
126,126,1,2,128,126,
2,2,12,13,18,17,73,67,9,73,2,4,19,29,0,30,30,30,30,28,26,25,24,23,22,20,21,
  22,32,19,16,15,11,10,27,14,
53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,55,
56,6,
48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,50,
51,8,
31,9,66,
63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,68,32,63,63,63,63,
  63,10,65,
33,57,
127,127,127,3,127,127,127,3,1,2,12,3,3,153,
127,127,127,127,127,127,3,127,127,127,3,1,2,13,3,3,152,
127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,3,3,
  1,2,127,14,3,3,150,
12,13,73,67,9,73,15,40,34,40,16,15,11,10,27,14,
12,13,18,17,73,67,9,73,16,19,29,35,35,28,26,25,24,23,22,20,21,22,32,19,16,
  15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,17,3,3,145,
127,127,127,127,127,3,127,127,127,3,1,2,18,3,3,146,
36,35,37,
12,13,18,17,73,67,9,73,20,19,34,20,21,34,32,19,16,15,11,10,27,14,
12,13,18,17,73,67,9,73,21,19,34,20,21,33,32,19,16,15,11,10,27,14,
38,40,26,41,39,
18,17,21,43,42,
44,46,48,50,18,51,49,47,45,
52,54,16,55,53,
56,14,57,
127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
  127,127,127,127,127,74,3,74,74,3,1,2,127,27,3,3,130,
58,60,12,61,59,
13,63,65,67,69,71,38,72,70,68,66,64,62,
73,74,1,30,75,75,
31,69,70,
31,64,62,
76,77,77,33,77,
13,38,62,
78,35,39,
127,127,127,127,127,3,127,127,127,3,1,2,36,3,3,149,
12,13,18,17,73,67,9,73,37,19,34,20,21,36,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,38,3,3,148,
12,13,18,17,73,67,9,73,39,19,34,20,21,31,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,40,3,3,147,
12,13,18,17,73,67,9,73,41,19,34,20,21,30,32,19,16,15,11,10,27,14,
12,13,18,17,73,67,9,73,42,19,34,79,20,21,79,32,19,16,15,11,10,27,14,
12,13,18,17,73,67,9,73,43,19,34,80,20,21,80,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,44,3,3,144,
12,13,18,17,73,67,9,73,45,19,34,81,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,46,3,3,143,
12,13,18,17,73,67,9,73,47,19,34,82,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,48,3,3,142,
12,13,18,17,73,67,9,73,49,19,34,83,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,50,3,3,141,
12,13,18,17,73,67,9,73,51,19,34,84,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,52,3,3,140,
12,13,18,17,73,67,9,73,53,19,34,85,23,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,54,3,3,139,
12,13,18,17,73,67,9,73,55,19,34,86,23,22,20,21,22,32,19,16,15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,56,3,3,138,
12,13,18,17,73,67,9,73,57,19,34,87,24,23,22,20,21,22,32,19,16,15,11,10,27,
  14,
127,127,127,127,127,3,127,127,127,3,1,2,58,3,3,137,
12,13,18,17,73,67,9,73,59,19,34,88,25,24,23,22,20,21,22,32,19,16,15,11,10,
  27,14,
127,127,127,127,127,3,127,127,127,3,1,2,60,3,3,136,
12,13,18,17,73,67,9,73,61,19,29,89,89,28,26,25,24,23,22,20,21,22,32,19,16,
  15,11,10,27,14,
12,13,42,18,17,73,67,9,73,62,19,29,44,44,28,26,25,24,23,22,20,21,22,32,19,
  16,15,91,90,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,63,3,3,134,
12,13,18,17,73,67,9,73,64,19,29,11,11,28,26,25,24,23,22,20,21,22,32,19,16,
  15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,65,3,3,133,
12,13,18,17,73,67,9,73,66,19,29,10,10,28,26,25,24,23,22,20,21,22,32,19,16,
  15,11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,67,3,3,132,
12,13,18,17,73,67,9,73,68,19,29,9,9,28,26,25,24,23,22,20,21,22,32,19,16,15,
  11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,69,3,3,131,
12,13,18,17,73,67,9,73,70,19,29,8,8,28,26,25,24,23,22,20,21,22,32,19,16,15,
  11,10,27,14,
127,127,127,127,127,3,127,127,127,3,1,2,71,3,3,129,
12,13,18,17,73,67,9,73,72,19,29,7,7,28,26,25,24,23,22,20,21,22,32,19,16,15,
  11,10,27,14,
127,127,127,127,127,127,127,3,127,127,127,3,1,2,127,73,3,3,155,
127,127,127,127,127,127,127,3,127,127,127,3,1,2,127,74,3,3,154,
2,2,12,13,18,17,73,67,9,73,2,75,19,29,5,5,5,28,26,25,24,23,22,20,21,22,32,
  19,16,15,11,10,27,14,
71,76,92,
71,77,93,
127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,3,3,
  1,2,127,78,3,3,151,
38,40,28,41,39,
38,40,27,41,39,
18,17,25,43,42,
18,17,24,43,42,
18,17,23,43,42,
18,17,22,43,42,
44,46,48,50,20,51,49,47,45,
44,46,48,50,19,51,49,47,45,
52,54,17,55,53,
56,15,57,
94,89,95,
74,43,96,
78,91,41,
72,61,
72,60,
127,127,127,127,127,3,127,127,127,3,1,2,94,3,3,135,
12,13,18,17,73,67,9,73,95,19,34,13,28,26,25,24,23,22,20,21,22,32,19,16,15,
  11,10,27,14,
12,13,18,17,73,67,9,73,96,19,29,45,45,28,26,25,24,23,22,20,21,22,32,19,16,
  15,11,10,27,14,

};


static const unsigned short ag_sbt[] = {
     0,  19,  43,  68,  74, 110, 131, 133, 155, 157, 160, 188, 190, 204,
   221, 248, 264, 294, 310, 326, 329, 351, 373, 378, 383, 392, 397, 400,
   436, 441, 454, 460, 463, 466, 471, 474, 477, 493, 515, 531, 553, 569,
   591, 614, 637, 653, 677, 693, 717, 733, 757, 773, 797, 813, 838, 854,
   879, 895, 921, 937, 964, 980,1010,1043,1059,1089,1105,1135,1151,1181,
  1197,1227,1243,1273,1292,1311,1345,1348,1351,1378,1383,1388,1393,1398,
  1403,1408,1417,1426,1431,1434,1437,1440,1443,1445,1447,1463,1492,1522
};


static const unsigned short ag_sbe[] = {
    15,  40,  65,  72,  85, 130, 132, 154, 156, 158, 186, 189, 200, 217,
   244, 254, 272, 306, 322, 327, 337, 359, 375, 380, 387, 394, 398, 432,
   438, 447, 457, 461, 464, 469, 472, 475, 489, 501, 527, 539, 565, 577,
   599, 622, 649, 661, 689, 701, 729, 741, 769, 781, 809, 821, 850, 862,
   891, 903, 933, 945, 976, 988,1019,1055,1067,1101,1113,1147,1159,1193,
  1205,1239,1251,1288,1307,1322,1346,1349,1374,1380,1385,1390,1395,1400,
  1405,1412,1421,1428,1432,1435,1438,1441,1444,1446,1459,1471,1500,1522
};


static const unsigned char ag_fl[] = {
  2,2,0,1,1,3,1,3,3,3,3,3,1,5,1,3,1,3,1,3,3,1,3,3,3,3,1,3,3,1,3,3,1,2,2,
  1,3,1,1,3,2,4,0,1,1,3,1,1,2,0,1,3,1,2,0,1,3,1,0,1,4,4,3,0,1,2,2,1,2,1,
  2,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,1,2,2,2,2,2,2,2,2,2,2,2,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

static const unsigned char ag_ptt[] = {
    0,  4,  8,  8,  5,  5,  7,  7,  7,  7,  7,  7, 10, 10, 16, 16, 19, 19,
   21, 21, 21, 23, 23, 23, 23, 23, 26, 26, 26, 31, 31, 31, 34, 34, 34, 37,
   37, 38, 38, 38, 38, 38, 43, 43, 44, 44,  1, 49, 49, 50, 50,  1, 54, 54,
   55, 55,  1, 95, 59, 59, 95, 95, 57, 64, 64, 57, 57, 61, 61, 63, 63, 60,
   60, 75, 75,  9,  9, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
   48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 53, 53, 53, 53, 53, 53, 53, 53,
   53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 66, 66, 67, 67, 67, 72,
   72, 73, 73, 11,  3, 12, 13, 14, 15, 18, 17, 20, 22, 24, 25, 27, 28, 29,
   30, 32, 33, 35, 36, 39,  2, 41, 40, 42, 45, 68
};


static void ag_ra(void)
{
  switch(ag_rpx[(PCB).ag_ap]) {
    case 1: V(0,uint64_t) = ag_rp_1(V(0,int), V(2,uint64_t)); break;
    case 2: V(0,uint64_t) = ag_rp_2(V(0,int), V(2,uint64_t)); break;
    case 3: V(0,uint64_t) = ag_rp_3(V(0,int), V(2,uint64_t)); break;
    case 4: V(0,uint64_t) = ag_rp_4(V(0,int), V(2,uint64_t)); break;
    case 5: V(0,uint64_t) = ag_rp_5(V(0,int), V(2,uint64_t)); break;
    case 6: V(0,uint64_t) = ag_rp_6(V(0,uint64_t), V(2,uint64_t), V(4,uint64_t)); break;
    case 7: V(0,uint64_t) = ag_rp_7(V(0,uint64_t), V(2,uint64_t)); break;
    case 8: V(0,uint64_t) = ag_rp_8(V(0,uint64_t), V(2,uint64_t)); break;
    case 9: V(0,uint64_t) = ag_rp_9(V(0,uint64_t), V(2,uint64_t)); break;
    case 10: V(0,uint64_t) = ag_rp_10(V(0,uint64_t), V(2,uint64_t)); break;
    case 11: V(0,uint64_t) = ag_rp_11(V(0,uint64_t), V(2,uint64_t)); break;
    case 12: V(0,uint64_t) = ag_rp_12(V(0,uint64_t), V(2,uint64_t)); break;
    case 13: V(0,uint64_t) = ag_rp_13(V(0,uint64_t), V(2,uint64_t)); break;
    case 14: V(0,uint64_t) = ag_rp_14(V(0,uint64_t), V(2,uint64_t)); break;
    case 15: V(0,uint64_t) = ag_rp_15(V(0,uint64_t), V(2,uint64_t)); break;
    case 16: V(0,uint64_t) = ag_rp_16(V(0,uint64_t), V(2,uint64_t)); break;
    case 17: V(0,uint64_t) = ag_rp_17(V(0,uint64_t), V(2,uint64_t)); break;
    case 18: V(0,uint64_t) = ag_rp_18(V(0,uint64_t), V(2,uint64_t)); break;
    case 19: V(0,uint64_t) = ag_rp_19(V(0,uint64_t)); break;
    case 20: V(0,uint64_t) = ag_rp_20(V(1,uint64_t)); break;
    case 21: V(0,uint64_t) = ag_rp_21(V(1,uint64_t)); break;
    case 22: V(0,uint64_t) = ag_rp_22(V(0,uint64_t), V(2,uint64_t)); break;
    case 23: V(0,uint64_t) = ag_rp_23(V(0,int)); break;
    case 24: V(0,uint64_t) = ag_rp_24(V(1,uint64_t)); break;
    case 25: V(0,uint64_t) = ag_rp_25(V(1,uint64_t)); break;
    case 26: V(0,uint64_t) = ag_rp_26(V(0,int), V(2,int)); break;
    case 27: V(0,int) = ag_rp_27(); break;
    case 28: V(0,int) = ag_rp_28(V(0,uint64_t)); break;
    case 29: V(0,int) = ag_rp_29(V(0,int), V(2,uint64_t)); break;
    case 30: V(0,uint64_t) = ag_rp_30(V(0,uint64_t), V(3,int)); break;
    case 31: V(0,uint64_t) = ag_rp_31(V(0,uint64_t), V(3,int)); break;
    case 32: V(0,uint64_t) = ag_rp_32(V(0,uint64_t), V(2,uint64_t)); break;
    case 33: V(0,uint64_t) = ag_rp_33(V(1,uint64_t)); break;
    case 34: V(0,uint64_t) = ag_rp_34(V(0,int)); break;
    case 35: V(0,uint64_t) = ag_rp_35(V(0,uint64_t), V(1,int)); break;
    case 36: V(0,uint64_t) = ag_rp_36(V(0,int)); break;
    case 37: V(0,uint64_t) = ag_rp_37(V(0,int), V(1,uint64_t)); break;
    case 38: V(0,int) = ag_rp_38(V(0,int)); break;
    case 39: V(0,int) = ag_rp_39(V(0,int), V(1,int)); break;
    case 40: V(0,int) = ag_rp_40(V(0,int)); break;
    case 41: V(0,int) = ag_rp_41(V(0,int), V(1,int)); break;
  }
  (PCB).la_ptr = (PCB).pointer;
}

#define TOKEN_NAMES evalKernel_token_names
const char *const evalKernel_token_names[101] = {
  "input string",
  "white space",
  "real",
  "name",
  "input string",
  "expressions",
  "eof",
  "expression",
  "",
  "",
  "conditional expression",
  "'='",
  "\"+=\"",
  "\"-=\"",
  "\"*=\"",
  "\"/=\"",
  "logical or expression",
  "'?'",
  "':'",
  "logical and expression",
  "\"||\"",
  "equality expression",
  "\"&&\"",
  "relational expression",
  "\"==\"",
  "\"!=\"",
  "additive expression",
  "'<'",
  "\"<=\"",
  "'>'",
  "\">=\"",
  "multiplicative expression",
  "'+'",
  "'-'",
  "unary expression",
  "'*'",
  "'/'",
  "factor",
  "primary",
  "\"**\"",
  "'('",
  "')'",
  "'!'",
  "arguments",
  "argument list",
  "','",
  "",
  "\"/*\"",
  "",
  "",
  "",
  "\"*/\"",
  "\"//\"",
  "",
  "",
  "",
  "'\\n'",
  "simple real",
  "",
  "",
  "exponent",
  "integer part",
  "'.'",
  "fraction part",
  "",
  "digit",
  "letter",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "'='",
  "name",
  "\"+=\"",
  "\"-=\"",
  "\"*=\"",
  "\"/=\"",
  "':'",
  "'?'",
  "\"||\"",
  "\"&&\"",
  "\"==\"",
  "\"!=\"",
  "'<'",
  "\"<=\"",
  "'>'",
  "\">=\"",
  "'+'",
  "'-'",
  "'*'",
  "'/'",
  "\"**\"",
  "real",
  "')'",
  "'('",
  "'!'",
  "','",
  "",

};

#ifndef MISSING_FORMAT
#define MISSING_FORMAT "Missing %s"
#endif
#ifndef UNEXPECTED_FORMAT
#define UNEXPECTED_FORMAT "Unexpected %s"
#endif
#ifndef UNNAMED_TOKEN
#define UNNAMED_TOKEN "input"
#endif

/* old code uses unsafe sprintf */

#if 0
static void ag_diagnose(void) {
  int ag_snd = (PCB).sn;
  int ag_k = ag_sbt[ag_snd];

  if (*TOKEN_NAMES[ag_tstt[ag_k]] && ag_astt[ag_k + 1] == ag_action_8) {
    sprintf((PCB).ag_msg, MISSING_FORMAT, TOKEN_NAMES[ag_tstt[ag_k]]);
  }
  else if (ag_astt[ag_sbe[(PCB).sn]] == ag_action_8
          && (ag_k = ag_sbe[(PCB).sn] + 1) == ag_sbt[(PCB).sn+1] - 1
          && *TOKEN_NAMES[ag_tstt[ag_k]]) {
    sprintf((PCB).ag_msg, MISSING_FORMAT, TOKEN_NAMES[ag_tstt[ag_k]]);
  }
  else if ((PCB).token_number && *TOKEN_NAMES[(PCB).token_number]) {
    sprintf((PCB).ag_msg, UNEXPECTED_FORMAT, TOKEN_NAMES[(PCB).token_number]);
  }
  else if (isprint(INPUT_CODE((*(PCB).pointer))) && INPUT_CODE((*(PCB).pointer)) != '\\') {
    char buf[20];
    sprintf(buf, "\'%c\'", (char) INPUT_CODE((*(PCB).pointer)));
    sprintf((PCB).ag_msg, UNEXPECTED_FORMAT, buf);
  }
  else sprintf((PCB).ag_msg, UNEXPECTED_FORMAT, UNNAMED_TOKEN);
  (PCB).error_message = (PCB).ag_msg;


}
#endif

/* used instead of unsafe sprintf */

static void safe_copy(char *destination, int max_size, 
    const char *source1, const char *source2) {
  int i = 0;
  int j = 0;
  while (i < max_size - 1 && i < (int)strlen(source1)) {
    destination[i] = source1[i];
    i = i + 1;
  }
  while (i < max_size - 1 && j < (int)strlen(source2)) {
    destination[i] = source2[j];
    i = i + 1;
    j = j + 1;
  }
  destination[i] = (char) 0;
}

/* new code without sprintf */

static void ag_diagnose(void) {
  int ag_snd = (PCB).sn;
  int ag_k = ag_sbt[ag_snd];

  if (*TOKEN_NAMES[ag_tstt[ag_k]] && ag_astt[ag_k + 1] == ag_action_8) {
    safe_copy((PCB).ag_msg, 82, "Missing ", TOKEN_NAMES[ag_tstt[ag_k]]);
  }
  else if (ag_astt[ag_sbe[(PCB).sn]] == ag_action_8
          && (ag_k = ag_sbe[(PCB).sn] + 1) == ag_sbt[(PCB).sn+1] - 1
          && *TOKEN_NAMES[ag_tstt[ag_k]]) {
    safe_copy((PCB).ag_msg, 82, "Missing ", TOKEN_NAMES[ag_tstt[ag_k]]);
  }
  else if ((PCB).token_number && *TOKEN_NAMES[(PCB).token_number]) {
    safe_copy((PCB).ag_msg, 82, "Unexpected ", TOKEN_NAMES[(PCB).token_number]);
  }
  else if (isprint(INPUT_CODE((*(PCB).pointer))) && INPUT_CODE((*(PCB).pointer)) != '\\') {
    char buf[20];
    buf[0] = '\'';
    buf[1] = (char) INPUT_CODE((*(PCB).pointer));
    buf[2] = '\'';
    buf[3] = (char) 0;
    safe_copy((PCB).ag_msg, 82, "Unexpected ", buf);
  }
  else safe_copy((PCB).ag_msg, 82, "Unexpected ", UNNAMED_TOKEN);
  (PCB).error_message = (PCB).ag_msg;
}

static int ag_action_1_r_proc(void);
static int ag_action_2_r_proc(void);
static int ag_action_3_r_proc(void);
static int ag_action_4_r_proc(void);
static int ag_action_1_s_proc(void);
static int ag_action_3_s_proc(void);
static int ag_action_1_proc(void);
static int ag_action_2_proc(void);
static int ag_action_3_proc(void);
static int ag_action_4_proc(void);
static int ag_action_5_proc(void);
static int ag_action_6_proc(void);
static int ag_action_7_proc(void);
static int ag_action_8_proc(void);
static int ag_action_9_proc(void);
static int ag_action_10_proc(void);
static int ag_action_11_proc(void);
static int ag_action_8_proc(void);


static int (*const  ag_r_procs_scan[])(void) = {
  ag_action_1_r_proc,
  ag_action_2_r_proc,
  ag_action_3_r_proc,
  ag_action_4_r_proc
};

static int (*const  ag_s_procs_scan[])(void) = {
  ag_action_1_s_proc,
  ag_action_2_r_proc,
  ag_action_3_s_proc,
  ag_action_4_r_proc
};

static int (*const  ag_gt_procs_scan[])(void) = {
  ag_action_1_proc,
  ag_action_2_proc,
  ag_action_3_proc,
  ag_action_4_proc,
  ag_action_5_proc,
  ag_action_6_proc,
  ag_action_7_proc,
  ag_action_8_proc,
  ag_action_9_proc,
  ag_action_10_proc,
  ag_action_11_proc,
  ag_action_8_proc
};


static int ag_action_10_proc(void) {
  int ag_t = (PCB).token_number;
  (PCB).btsx = 0, (PCB).drt = -1;
  do {
    ag_track();
    (PCB).token_number = (evalKernel_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word(ag_k);
    }
  } while ((PCB).token_number == (evalKernel_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}

static int ag_action_11_proc(void) {
  int ag_t = (PCB).token_number;

  (PCB).btsx = 0, (PCB).drt = -1;
  do {
    (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
    (PCB).ssx--;
    ag_track();
    ag_ra();
    if ((PCB).exit_flag != AG_RUNNING_CODE) return 0;
    (PCB).ssx++;
    (PCB).token_number = (evalKernel_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
    (PCB).la_ptr++;
    if (ag_key_index[(PCB).sn]) {
      unsigned ag_k = ag_key_index[(PCB).sn];
      int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
      while (ag_key_ch[ag_k] < ag_ch) ag_k++;
      if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word(ag_k);
    }
  }
  while ((PCB).token_number == (evalKernel_token_type) ag_t);
  (PCB).la_ptr =  (PCB).pointer;
  return 1;
}

static int ag_action_3_r_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).btsx = 0, (PCB).drt = -1;
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra();
  return (PCB).exit_flag == AG_RUNNING_CODE;
}

static int ag_action_3_s_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).btsx = 0, (PCB).drt = -1;
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra();
  return (PCB).exit_flag == AG_RUNNING_CODE;;
}

static int ag_action_4_r_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  return 1;
}

static int ag_action_2_proc(void) {
  (PCB).btsx = 0, (PCB).drt = -1;
  if ((PCB).ssx >= 128) {
    (PCB).exit_flag = AG_STACK_ERROR_CODE;
    PARSER_STACK_OVERFLOW;
  }
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  ag_track();
  return 0;
}

static int ag_action_9_proc(void) {
  if ((PCB).drt == -1) {
    (PCB).drt=(PCB).token_number;
    (PCB).dssx=(PCB).ssx;
    (PCB).dsn=(PCB).sn;
  }
  ag_prot();
  (PCB).vs[(PCB).ssx] = ag_null_value;
  (PCB).ss[(PCB).ssx] = (PCB).sn;
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  (PCB).la_ptr =  (PCB).pointer;
  return (PCB).exit_flag == AG_RUNNING_CODE;
}

static int ag_action_2_r_proc(void) {
  (PCB).ssx++;
  (PCB).sn = (PCB).ag_ap;
  return 0;
}

static int ag_action_7_proc(void) {
  --(PCB).ssx;
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}

static int ag_action_1_proc(void) {
  ag_track();
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}

static int ag_action_1_r_proc(void) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}

static int ag_action_1_s_proc(void) {
  (PCB).exit_flag = AG_SUCCESS_CODE;
  return 0;
}

static int ag_action_4_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  (PCB).btsx = 0, (PCB).drt = -1;
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track();
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (unsigned char)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])() == 0) break;
  }
  return 0;
}

static int ag_action_3_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap] - 1;
  (PCB).btsx = 0, (PCB).drt = -1;
  (*(int *) &(PCB).vs[(PCB).ssx]) = *(PCB).pointer;
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else (PCB).ss[(PCB).ssx] = (PCB).sn;
  ag_track();
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra();
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (unsigned char)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_s_procs_scan[ag_astt[ag_t1]])() == 0) break;
  }
  return 0;
}

static int ag_action_8_proc(void) {
  ag_undo();
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).exit_flag = AG_SYNTAX_ERROR_CODE;
  ag_diagnose();
  SYNTAX_ERROR;
  {(PCB).la_ptr = (PCB).pointer + 1; ag_track();}
  return (PCB).exit_flag == AG_RUNNING_CODE;
}

static int ag_action_5_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  if ((PCB).drt == -1) {
    (PCB).drt=(PCB).token_number;
    (PCB).dssx=(PCB).ssx;
    (PCB).dsn=(PCB).sn;
  }
  if (ag_sd) (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  else {
    ag_prot();
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  ag_ra();
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (unsigned char)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])() == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}

static int ag_action_6_proc(void) {
  int ag_sd = ag_fl[(PCB).ag_ap];
  (PCB).reduction_token = (evalKernel_token_type) ag_ptt[(PCB).ag_ap];
  if ((PCB).drt == -1) {
    (PCB).drt=(PCB).token_number;
    (PCB).dssx=(PCB).ssx;
    (PCB).dsn=(PCB).sn;
  }
  if (ag_sd) {
    (PCB).sn = (PCB).ss[(PCB).ssx -= ag_sd];
  }
  else {
    ag_prot();
    (PCB).vs[(PCB).ssx] = ag_null_value;
    (PCB).ss[(PCB).ssx] = (PCB).sn;
  }
  (PCB).la_ptr =  (PCB).pointer;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbe[(PCB).sn] + 1;
    unsigned ag_t2 = ag_sbt[(PCB).sn+1] - 1;
    do {
      unsigned ag_tx = (ag_t1 + ag_t2)/2;
      if (ag_tstt[ag_tx] < (unsigned char)(PCB).reduction_token) ag_t1 = ag_tx + 1;
      else ag_t2 = ag_tx;
    } while (ag_t1 < ag_t2);
    (PCB).ag_ap = ag_pstt[ag_t1];
    if ((ag_r_procs_scan[ag_astt[ag_t1]])() == 0) break;
  }
  return (PCB).exit_flag == AG_RUNNING_CODE;
}


void init_evalKernel(void) {
  (PCB).la_ptr = (PCB).pointer;
  (PCB).ss[0] = (PCB).sn = (PCB).ssx = 0;
  (PCB).exit_flag = AG_RUNNING_CODE;
  (PCB).line = FIRST_LINE;
  (PCB).column = FIRST_COLUMN;
  (PCB).btsx = 0, (PCB).drt = -1;
}

void evalKernel(void) {
  init_evalKernel();
  (PCB).exit_flag = AG_RUNNING_CODE;
  while ((PCB).exit_flag == AG_RUNNING_CODE) {
    unsigned ag_t1 = ag_sbt[(PCB).sn];
    if (ag_tstt[ag_t1]) {
      unsigned ag_t2 = ag_sbe[(PCB).sn] - 1;
      (PCB).token_number = (evalKernel_token_type) AG_TCV(INPUT_CODE(*(PCB).la_ptr));
      (PCB).la_ptr++;
      if (ag_key_index[(PCB).sn]) {
        unsigned ag_k = ag_key_index[(PCB).sn];
        int ag_ch = CONVERT_CASE(INPUT_CODE(*(PCB).pointer));
        while (ag_key_ch[ag_k] < ag_ch) ag_k++;
        if (ag_key_ch[ag_k] == ag_ch) ag_get_key_word(ag_k);
      }
      do {
        unsigned ag_tx = (ag_t1 + ag_t2)/2;
        if (ag_tstt[ag_tx] > (unsigned char)(PCB).token_number)
          ag_t1 = ag_tx + 1;
        else ag_t2 = ag_tx;
      } while (ag_t1 < ag_t2);
      if (ag_tstt[ag_t1] != (unsigned char)(PCB).token_number)
        ag_t1 = ag_sbe[(PCB).sn];
    }
    (PCB).ag_ap = ag_pstt[ag_t1];
    (ag_gt_procs_scan[ag_astt[ag_t1]])();
  }
}

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

