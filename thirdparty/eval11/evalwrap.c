/**
 * @brief This file has been modified for use in primesieve
 * <http://primesieve.googlecode.com>.
 * @author Kim Walisch <kim.walisch@gmail.com>
 * Last updated: January 2011
 *
 * CHANGES:
 *
 * 1. Use of extern "C" for usage in C++ project
 * 2. double changed to int64_t (better precision near 1e19) type
 *    from stdint.h
 * 3. Uninitialized variables are set to INT64_MIN instead
 *    of 0
 * 4. Removed use of strdup (not ANSI) and sprintf (causes 
 *    unsafe warnings)
 * 5. Added (char*) cast for strings to silence warnings
 * 6. Unused file evaldemo.c has been deleted
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
 EVALWRAP.C  Version 1.1

 evaluateExpression: A Simple Expression Evaluator
 Copyright (c) 1996 - 1999 Parsifal Software, All
 Rights Reserved.

 The EVALWRAP.C module provides support functions for the parser function
 evalKernel(), defined by EVALKERN.SYN. It includes definitions of the
 functions called by evalKernel() as well as the definition of the
 evaluateExpression function, implemented as a wrapper function for
 evalKernel().

 This module consists of six parts:
   1. Error diagnostic procedures, including the definition of
      checkZero.
   2. Character stack procedures, including the definition of
      pushChar.
   3. Symbol table procedures, including the definition of
      locateVariable, which provides access to named variables. In this
      implementation, there are no predefined variables. If a variable
      is not found, it is added to the table and initialized to zero.
      The lookup uses a binary search.
   4. Argument stack procedures, including the definition of
      pushArg.
   5. Function call interface which provides access to
      the standard C library math functions.
      The interface consists of
      . a functionTable, each entry of which contains the name of a
        function and a wrapper function which calls the named
        function.
      . an implementation of callFunction which does a binary search of
        functionTable and then calls the appropriate wrapper function.
      Macros are used to simplify generation of the wrapper functions
      and the functionTable entries.
   6. Definition of the evaluateExpression wrapper function.

 For further information about this module, contact
   Parsifal Software
   http://www.parsifalsoft.com
   info@parsifalsoft.com
   1-800-879-2755, 1-508-358-2564
   P.O. Box 219
   Wayland, MA 01778
*/

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "evaldefs.h"
#include "evalkern.h"


/*********************************************************************

 Part 1. Error Diagnostics

*********************************************************************/

ErrorRecord errorRecord;           /* define an error record */

void diagnoseError(char *msg) {
  if (evalKernel_pcb.exit_flag == AG_RUNNING_CODE)   /* parser still running */
    evalKernel_pcb.exit_flag = AG_SEMANTIC_ERROR_CODE;      /* stop parse */
  errorRecord.message = msg;
  errorRecord.line    = evalKernel_pcb.line;
  errorRecord.column  = evalKernel_pcb.column;
}

int64_t checkZero(int64_t value) {
  if (value) return value;
  diagnoseError((char*)"Divide by Zero");
  return 1;
}


/*******************************************************************

Part 2. Accumulate variable names and function names

*******************************************************************/

static char  charStack[CHAR_STACK_LENGTH+1];
static char *charStackTop = charStack;

static void resetCharStack(void) {
  charStackTop = charStack;
}

void pushChar(int c) {              /* append char to name string */
  if (charStackTop < charStack+CHAR_STACK_LENGTH) {
    *charStackTop++ = (char) c;
    return;
  }
  /* buffer overflow, kill parse and issue diagnostic */
  diagnoseError((char*)"Character Stack Overflow");
}

static char *popString(int nChars) {                /* get string */
  *charStackTop = 0;
  return charStackTop -= nChars;
}


/**********************************************************************

Part 3. Symbol Table

**********************************************************************/

VariableDescriptor variable[N_VARIABLES];    /* Symbol table array */

int nVariables = 0;                       /* no. of entries in table */

/* Callback function to locate named variable */

int64_t *locateVariable(int nameLength) {   /* identify variable name */
  int i = 0;
  char *name = popString(nameLength);
  int first = 0;
  int last = nVariables - 1;

  while (first <= last) {                           /* binary search */
    int middle = (first+last)/2;
    int flag = strcmp(name,variable[middle].name);
    if (flag == 0) return &variable[middle].value;
    if (flag < 0) last = middle-1;
    else first = middle+1;
  }
  /* name not found, check for room in table */
  if (nVariables >= N_VARIABLES) {
    /* table is full, kill parse and issue diagnostic */
    static int64_t junk = 0;
    diagnoseError((char*)"Symbol Table Full");
    return &junk;
  }

  /* insert variable in table in sorted order */
  memmove(&variable[first+1],
          &variable[first],
          (nVariables-first)*sizeof(VariableDescriptor));
  nVariables++;
  while (i < (int)strlen(name)) {
    if (i >= PRIMESIEVE_NAMESIZE - 1)
      break;
    variable[first].name[i] = name[i];
    i = i + 1;
  }
  variable[first].name[i] = (char) 0;
/*  variable[first].name = strdup(name); */
/*  variable[first].value = 0; */
  /* INT64_MIN is used to mark uninitialized variables */
  variable[first].value = INT64_MIN;
  return &variable[first].value;
}


/*******************************************************************

Part 4. Accumulate list of function arguments

*******************************************************************/

static int64_t  argStack[ARG_STACK_LENGTH];      /* argument buffer */
static int64_t *argStackTop = argStack;

static void resetArgStack(void) {
  argStackTop = argStack;
}

void pushArg(int64_t x) {                     /* store arg in list */
  if (argStackTop < argStack + ARG_STACK_LENGTH) {
    *argStackTop++ = x;
    return;
  }
  /* too many args, kill parse and issue diagnostic */
  diagnoseError((char*)"Argument Stack Full");
}

static int64_t *popArgs(int nArgs) {                 /* fetch args */
  return argStackTop -= nArgs;
}


/**********************************************************************

 Part 5. Function Call Interface

 Define functionTable, each entry of which contains the ascii name of
 a function and a pointer to a wrapper function. The wrapper function
 checks the argument count and calls the real function.

 Then, define callFunction. Given the ascii name of a function,
 callFunction does a binary search of functionTable and on a successful
 search calls the corresponding wrapper function.

**********************************************************************/

/* define some macros to build the wrapper functions */

/*
 First, a macro to make a wrapper function for a function with one
 argument.
*/

#define WRAPPER_FUNCTION_1_ARG(FUN) \
int64_t FUN##Wrapper(int argc, double *argv) {\
  if (argc == 1) return (int64_t) FUN(argv[0]);\
  diagnoseError((char*)"Wrong Number of Arguments");\
  return 0;\
}

/*
 Now, a macro to make a wrapper function for a function with two
 arguments.
*/

#define WRAPPER_FUNCTION_2_ARGS(FUN) \
int64_t FUN##Wrapper(int argc, double *argv) {\
  if (argc==2) return (int64_t) FUN(argv[0], argv[1]);\
  diagnoseError((char*)"Wrong Number of Arguments");\
  return 0;\
}


/*
 Now define wrapper functions for the standard C library
 math functions.
*/

WRAPPER_FUNCTION_1_ARG(acos)
WRAPPER_FUNCTION_1_ARG(asin)
WRAPPER_FUNCTION_1_ARG(atan)
WRAPPER_FUNCTION_2_ARGS(atan2)
WRAPPER_FUNCTION_1_ARG(cos)
WRAPPER_FUNCTION_1_ARG(cosh)
WRAPPER_FUNCTION_1_ARG(exp)
WRAPPER_FUNCTION_1_ARG(fabs)
WRAPPER_FUNCTION_2_ARGS(fmod)
WRAPPER_FUNCTION_1_ARG(log)
WRAPPER_FUNCTION_1_ARG(log10)
WRAPPER_FUNCTION_1_ARG(sin)
WRAPPER_FUNCTION_1_ARG(sinh)
WRAPPER_FUNCTION_1_ARG(sqrt)
WRAPPER_FUNCTION_1_ARG(tan)
WRAPPER_FUNCTION_1_ARG(tanh)


/* A macro to make correct functionTable entries */
#define TABLE_ENTRY(FUN) {#FUN, FUN##Wrapper}

/* remember to fix this when you add more functions to the table */
#define N_FUNCTIONS 16

/* define the function table -- must be in sorted order! */
struct {
  const char *name;
  int64_t (*function)(int, double[]);
} functionTable[N_FUNCTIONS] = {
  TABLE_ENTRY(acos),
  TABLE_ENTRY(asin),
  TABLE_ENTRY(atan),
  TABLE_ENTRY(atan2),
  TABLE_ENTRY(cos),
  TABLE_ENTRY(cosh),
  TABLE_ENTRY(exp),
  TABLE_ENTRY(fabs),
  TABLE_ENTRY(fmod),
  TABLE_ENTRY(log),
  TABLE_ENTRY(log10),
  TABLE_ENTRY(sin),
  TABLE_ENTRY(sinh),
  TABLE_ENTRY(sqrt),
  TABLE_ENTRY(tan),
  TABLE_ENTRY(tanh),
};


/* Finally, define the callback function to perform a function call */

int64_t callFunction(int nameLength, int argCount) {
  double doubleArgValues[8];
  int i = 0;
  char *name = popString(nameLength);
  int64_t *argValues = popArgs(argCount);
  int first = 0;
  int last = N_FUNCTIONS-1;
  while (i < argCount) {
    doubleArgValues[i] = (double)argValues[i];
    i = i + 1;
  }
  while (first <= last) {                     /* binary search */
    int middle = (first+last)/2;
    int flag = strcmp(name,functionTable[middle].name);
    if (flag == 0) return functionTable[middle].function(argCount, doubleArgValues);
    if (flag < 0) last = middle-1;
    else first = middle+1;
  }
  diagnoseError((char*)"Unknown Function");
  return 0;
}

/*******************************************************************

Part 6. Wrapper function definition

*******************************************************************/

int evaluateExpression(char *expressionString) {
  resetCharStack();
  resetArgStack();
  evalKernel_pcb.pointer = (unsigned char *) expressionString;
  evalKernel();
  return evalKernel_pcb.exit_flag != AG_SUCCESS_CODE;
}

/* End of evalwrap.c */

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

