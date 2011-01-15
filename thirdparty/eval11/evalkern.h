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
 
#ifndef EVALKERN_H
#define EVALKERN_H

#include <stdint.h>

typedef union {
  long alignment;
  char ag_vt_2[sizeof(int)];
  char ag_vt_4[sizeof(uint64_t)];
} evalKernel_vs_type;

typedef enum {
  evalKernel_white_space_token = 1, evalKernel_input_string_token = 4,
  evalKernel_expressions_token, evalKernel_eof_token,
  evalKernel_expression_token, evalKernel_conditional_expression_token = 10,
  evalKernel_logical_or_expression_token = 16,
  evalKernel_logical_and_expression_token = 19,
  evalKernel_equality_expression_token = 21,
  evalKernel_relational_expression_token = 23,
  evalKernel_additive_expression_token = 26,
  evalKernel_multiplicative_expression_token = 31,
  evalKernel_unary_expression_token = 34, evalKernel_factor_token = 37,
  evalKernel_primary_token, evalKernel_arguments_token = 43,
  evalKernel_argument_list_token, evalKernel_simple_real_token = 57,
  evalKernel_exponent_token = 60, evalKernel_integer_part_token,
  evalKernel_fraction_part_token = 63, evalKernel_digit_token = 65,
  evalKernel_letter_token, evalKernel_name_token = 75,
  evalKernel_real_token = 95
} evalKernel_token_type;

typedef struct {
  evalKernel_token_type token_number, reduction_token, error_frame_token;
  int input_code;
  int input_value;
  int line, column;
  int ssx, sn, error_frame_ssx;
  int drt, dssx, dsn;
  int ss[128];
  evalKernel_vs_type vs[128];
  int ag_ap;
  char *error_message;
  char read_flag;
  char exit_flag;
  int bts[128], btsx;
  unsigned char * pointer;
  unsigned char * la_ptr;
  const unsigned char *key_sp;
  int save_index, key_state;
  char ag_msg[82];
} evalKernel_pcb_type;

#ifndef PRULE_CONTEXT
#define PRULE_CONTEXT(pcb)  (&((pcb).cs[(pcb).ssx]))
#define PERROR_CONTEXT(pcb) ((pcb).cs[(pcb).error_frame_ssx])
#define PCONTEXT(pcb)       ((pcb).cs[(pcb).ssx])
#endif

#ifndef AG_RUNNING_CODE_CODE
/* PCB.exit_flag values */
#define AG_RUNNING_CODE         0
#define AG_SUCCESS_CODE         1
#define AG_SYNTAX_ERROR_CODE    2
#define AG_REDUCTION_ERROR_CODE 3
#define AG_STACK_ERROR_CODE     4
#define AG_SEMANTIC_ERROR_CODE  5
#endif

extern evalKernel_pcb_type evalKernel_pcb;
void init_evalKernel(void);
void evalKernel(void);
#endif

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

