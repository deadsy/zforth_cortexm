//-----------------------------------------------------------------------------
/*

   J-Forth

 */
//-----------------------------------------------------------------------------

#ifndef JFORTH_H
#define JFORTH_H

//-----------------------------------------------------------------------------

#include "jfconf.h"

//-----------------------------------------------------------------------------

typedef enum {
	JF_OK,
	JF_ABORT_INTERNAL_ERROR,
	JF_ABORT_OUTSIDE_MEM,
	JF_ABORT_DSTACK_UNDERRUN,
	JF_ABORT_DSTACK_OVERRUN,
	JF_ABORT_RSTACK_UNDERRUN,
	JF_ABORT_RSTACK_OVERRUN,
	JF_ABORT_FSTACK_UNDERRUN,
	JF_ABORT_FSTACK_OVERRUN,
	JF_ABORT_NOT_A_WORD,
	JF_ABORT_COMPILE_ONLY_WORD,
	JF_ABORT_INVALID_SIZE,
	JF_ABORT_DIVISION_BY_ZERO
} jf_result;

#ifdef JF_ENABLE_RESULT_STRINGS
const char *zf_result_str(jf_result rc);
#endif

//-----------------------------------------------------------------------------

jf_result jf_eval(const char *buf);


//-----------------------------------------------------------------------------
// Host provides these functions

//zf_input_state zf_host_sys(zf_syscall_id id, const char *last_word);
//void zf_host_trace(const char *fmt, va_list va);
jf_cell jf_host_parse_num(const char *buf);

//-----------------------------------------------------------------------------


#endif // JFORTH_H

//-----------------------------------------------------------------------------
