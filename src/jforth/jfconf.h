//-----------------------------------------------------------------------------
/*

   J-Forth Configuration

 */
//-----------------------------------------------------------------------------

#ifndef JFORTH_CONFIGURATION_H
#define JFORTH_CONFIGURATION_H

//-----------------------------------------------------------------------------

/* Set to 1 to add tracing support for debugging and inspection. Requires the
 * zf_host_trace() function to be implemented. Adds about one kB to .text and
 * .rodata, dramatically reduces speed, but is very useful. Make sure to enable
 * tracing at run time when calling jf_init() or by setting the 'trace' user
 * variable to 1 */
#define JF_ENABLE_TRACE 1

/* Set to 1 to add boundary checks to stack operations. */
#define JF_ENABLE_BOUNDARY_CHECKS 1

/* Set to 1 to add result code strings */
#define JF_ENABLE_RESULT_STRINGS 1

/* Set to 1 to enable the floating point stack */
#define JF_ENABLE_FLOAT 1

//-----------------------------------------------------------------------------

/* Type to use for the basic cell, data stack and return stack. Choose a signed
 * integer type that suits your needs. */
typedef int32_t jf_cell;
#define JF_CELL_FMT "%d"

/* The type to use for pointers and adresses. */
typedef uint32_t jf_addr;
#define JF_ADDR_FMT "%08x"

// stack sizes
#define JF_RSTACK_SIZE 32
#define JF_DSTACK_SIZE 32

//-----------------------------------------------------------------------------

/* ram dictionary size */
#define JF_DICT_SIZE 1024 // uint32_t

//-----------------------------------------------------------------------------

#ifdef JF_ENABLE_FLOAT

/* The type to use for floating point values. */
typedef float jf_float;
#define JF_FLOAT_FMT "%f"

#define JF_FSTACK_SIZE 32

#endif // JF_ENABLE_FLOAT

//-----------------------------------------------------------------------------

#endif // JFORTH_CONFIGURATION_H

//-----------------------------------------------------------------------------
