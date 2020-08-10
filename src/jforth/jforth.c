//-----------------------------------------------------------------------------
/*

   J-Forth

   Derived from: https://github.com/zevv/zForth

 */
//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <setjmp.h>
#include <ctype.h>

#include "jforth.h"

//-----------------------------------------------------------------------------

// Flags and length encoded in words
#define JF_FLAG_IMMEDIATE (1 << 6)
#define JF_FLAG_PRIM      (1 << 5)
#define JF_FLAG_LEN(v)    (v & 0x1f)

//-----------------------------------------------------------------------------

#if JF_ENABLE_BOUNDARY_CHECKS
#define CHECK(exp, abort) if (!(exp)) jf_abort (abort);
#else
#define CHECK(exp, abort)
#endif

// setjmp env for handling aborts.
static jmp_buf jmpbuf;

// Handle abort by unwinding the C stack and sending control back into jf_eval()
static void jf_abort(jf_result result) {
	longjmp(jmpbuf, result);
}

//-----------------------------------------------------------------------------

#ifdef JF_ENABLE_RESULT_STRINGS

static const char *result_str[] = {
	"ok", // JF_OK,
	"internal error", // JF_ABORT_INTERNAL_ERROR
	"outside memory", // JF_ABORT_OUTSIDE_MEM
	"dstack underrun", // JF_ABORT_DSTACK_UNDERRUN
	"dstack overrun", // JF_ABORT_DSTACK_OVERRUN
	"rstack underrun", // JF_ABORT_RSTACK_UNDERRUN
	"rstack overrun", // JF_ABORT_RSTACK_OVERRUN
	"fstack underrun", // JF_ABORT_FSTACK_UNDERRUN
	"fstack overrun", // JF_ABORT_FSTACK_OVERRUN
	"not a word", // JF_ABORT_NOT_A_WORD
	"compile-only word", // JF_ABORT_COMPILE_ONLY_WORD
	"invalid size", // JF_ABORT_INVALID_SIZE
	"division by zero", // JF_ABORT_DIVISION_BY_ZERO
	"unknown error", // must be last
};

#define RESULT_STR_MAX (sizeof(result_str) / sizeof(const char *))

const char *jf_result_str(jf_result result) {
	if (result >= RESULT_STR_MAX) {
		result = RESULT_STR_MAX - 1;
	}
	return result_str[result];
}

#endif // JF_ENABLE_RESULT_STRINGS

//-----------------------------------------------------------------------------

typedef enum {
	JF_INPUT_INTERPRET,
	JF_INPUT_PASS_CHAR,
	JF_INPUT_PASS_WORD
} jf_input_state;

static jf_input_state input_state;

//-----------------------------------------------------------------------------
// dictionary

static uint32_t ram_dict[JF_DICT_SIZE]; // RAM dictionary (read/write)
static const uint32_t *rom_dict; // ROM dictionary (read-only)

//-----------------------------------------------------------------------------
// user variables

static uint32_t *here;     // compilation pointer in dictionary
static uint32_t *latest;   // pointer to last compiled word
static bool trace;   // trace enable flag
static bool compiling;   // compiling flag
static bool postpone;   // flag to indicate next imm word should be compiled

//-----------------------------------------------------------------------------
// data stack operations

static jf_cell dstack[JF_DSTACK_SIZE]; // data stack
static int dsp; // data stack index

static void pushd(jf_cell v) {
	CHECK(dsp < JF_DSTACK_SIZE, JF_ABORT_DSTACK_OVERRUN);
	dstack[dsp++] = v;
}

static jf_cell popd(void) {
	CHECK(dsp > 0, JF_ABORT_DSTACK_UNDERRUN);
	return dstack[--dsp];
}

static jf_cell pickd(int n) {
	CHECK(n < dsp, JF_ABORT_DSTACK_UNDERRUN);
	return dstack[dsp - n - 1];
}

//-----------------------------------------------------------------------------
// return stack operations

static jf_cell rstack[JF_RSTACK_SIZE]; // return stack
static int rsp; // return stack index

static void pushr(jf_cell v) {
	CHECK(rsp < JF_RSTACK_SIZE, JF_ABORT_RSTACK_OVERRUN);
	rstack[rsp++] = v;
}

static jf_cell popr(void) {
	CHECK(rsp > 0, JF_ABORT_RSTACK_UNDERRUN);
	return rstack[--rsp];
}

static jf_cell pickr(int n) {
	CHECK(n < rsp, JF_ABORT_RSTACK_UNDERRUN);
	return rstack[rsp - n - 1];
}

//-----------------------------------------------------------------------------
// float stack operations

static jf_float fstack[JF_FSTACK_SIZE]; // float stack
static int fsp; // float stack index

static void pushf(jf_float v) {
	CHECK(fsp < JF_FSTACK_SIZE, JF_ABORT_FSTACK_OVERRUN);
	fstack[fsp++] = v;
}

static jf_float popf(void) {
	CHECK(fsp > 0, JF_ABORT_FSTACK_UNDERRUN);
	return fstack[--fsp];
}

static jf_float pickf(int n) {
	CHECK(n < fsp, JF_ABORT_FSTACK_UNDERRUN);
	return fstack[fsp - n - 1];
}

//-----------------------------------------------------------------------------

// Find word in dictionary, returning address and execution token
static int find_word(const char *name, jf_addr *word, jf_addr *code) {

  (void)name;
  (void)word;
  (void)code;

#if 0

  zf_addr w = LATEST;
	size_t namelen = strlen(name);

	while (w) {
		zf_cell link, d;
		zf_addr p = w;
		size_t len;
		p += dict_get_cell(p, &d);
		p += dict_get_cell(p, &link);
		len = ZF_FLAG_LEN((int)d);
		if (len == namelen) {
			const char *name2 = (const char *)&dict[p];
			if (memcmp(name, name2, len) == 0) {
				*word = w;
				*code = p + len;
				return 1;
			}
		}
		w = link;
	}

#endif

	return 0;
}

//-----------------------------------------------------------------------------

// Inner interpreter
static void run(const char *input) {

  (void)input;

#if 0
	while (ip != 0) {
		zf_cell d;
		zf_addr i, ip_org = ip;
		zf_addr l = dict_get_cell(ip, &d);
		zf_addr code = d;

		trace("\n "ZF_ADDR_FMT " " ZF_ADDR_FMT " ", ip, code);
		for (i = 0; i<rsp; i++) trace("┊  ");

		ip += l;

		if (code <= PRIM_COUNT) {
			do_prim((zf_prim)code, input);

			/* If the prim requests input, restore IP so that the
			 * next time around we call the same prim again */

			if (input_state != ZF_INPUT_INTERPRET) {
				ip = ip_org;
				break;
			}

		} else {
			trace("%s/" ZF_ADDR_FMT " ", op_name(code), code);
			zf_pushr(ip);
			ip = code;
		}

		input = NULL;
	}
#endif
}

// Execute bytecode from given address
static void execute(jf_addr addr) {
	//ip = addr;
	rsp = 0;
	pushr(0);
	run(NULL);
}

// Handle incoming word. Compile or interpreted the word, or pass it to a
// deferred primitive if it requested a word from the input stream.
static void handle_word(const char *buf) {
	jf_addr w, c = 0;
	int found;

	// If a word was requested by an earlier operation, resume with the new word
	if (input_state == JF_INPUT_PASS_WORD) {
		input_state = JF_INPUT_INTERPRET;
		run(buf);
		return;
	}

	// Look up the word in the dictionary
	found = find_word(buf, &w, &c);
	if (found) {
		// Word found: compile or execute, depending on flags and state
		jf_cell d;
		int flags;
		dict_get_cell(w, &d);
		flags = d;
		if (compiling && (postpone || !(flags & JF_FLAG_IMMEDIATE))) {
			if (flags & JF_FLAG_PRIM) {
				dict_get_cell(c, &d);
				dict_add_op(d);
			} else {
				dict_add_op(c);
			}
			postpone = false;
		} else {
			execute(c);
		}
	} else {
		// Word not found: try to convert to a number and compile or push, depending on state
		jf_cell v = jf_host_parse_num(buf);
		if (compiling) {
			dict_add_lit(v);
		} else {
			pushd(v);
		}
	}
}

// Handle one character. Split into words to pass to handle_word(), or pass the
// char to a deferred prim if it requested a character from the input stream.
static void handle_char(char c) {
	static char buf[32];
	static size_t len = 0;

	if (input_state == JF_INPUT_PASS_CHAR) {

		input_state = JF_INPUT_INTERPRET;
		run(&c);

	} else if (c != '\0' && !isspace(c)) {

		if (len < sizeof(buf) - 1) {
			buf[len++] = c;
			buf[len] = '\0';
		}

	} else {

		if (len > 0) {
			len = 0;
			handle_word(buf);
		}
	}
}

// Eval forth string
jf_result jf_eval(const char *buf) {
	jf_result r = (jf_result)setjmp(jmpbuf);

	if (r == JF_OK) {
		const char *tmp = buf;
		for (;;) {
			handle_char(*tmp);
			if (*tmp == '\0') {
				return JF_OK;
			}
			tmp++;
		}
	} else {
		compiling = false;
		rsp = 0;
		dsp = 0;
		fsp = 0;
		return r;
	}
}

//-----------------------------------------------------------------------------
// primitive operations

// @ (32-bit fetch) (addr -- n)
static void p_load32(void) {
  uint32_t *addr = (uint32_t *)popd();
  pushd(*addr);
}

// ! (32-bit store) (n, addr -- )
static void p_store32(void) {
  uint32_t n = popd();
  uint32_t *addr = (uint32_t *)popd();
  *addr = n;
}

static void p_pushr(void) {
}

static void p_popr(void) {
}

//-----------------------------------------------------------------------------


#if JF_ENABLE_BOOTSTRAP

// the primitive and usernames are stored in a character array with 0 as the
// name delimiter and 00 as the end of list marker.

#define _(s) s "\0"

static const char prim_names[] =
	_("exit")    _("lit")        _("<0")    _(":")     _("_;")        _("+")
	_("-")       _("*")          _("/")     _("%")     _("drop")      _("dup")
	_("pickr")   _("_immediate") _("@@")    _("!!")    _("swap")      _("rot")
	_("jmp")     _("jmp0")       _("'")     _("_(")    _(">r")        _("r>")
	_("=")       _("sys")        _("pick")  _(",,")    _("key")       _("lits")
	_("##")      _("&");

static const char uservar_names[] =
	_("h")   _("latest") _("trace")  _("compiling")  _("_postpone");


static void add_prim(const char *name) {
	bool imm = false;

	if (name[0] == '_') {
		name++;
		imm = true;
	}

	create(name, JF_FLAG_PRIM);
	dict_add_op(op);
	dict_add_op(PRIM_EXIT);
	if (imm) {
    make_immediate();
  }
}

static void add_uservar(const char *name, zf_addr addr) {

}

// Add primitives and user variables to dictionary.
void jf_bootstrap(void) {
	const char *p;

	jf_addr i = 0;

  for (p = prim_names; *p; p += strlen(p) + 1) {
		add_prim(p);
	}

	i = 0;
	for (p = uservar_names; *p; p += strlen(p) + 1) {
		add_uservar(p, i++);
	}
}

#else
void jf_bootstrap(void) {
}
#endif // JF_ENABLE_BOOTSTRAP

//-----------------------------------------------------------------------------

// Initialisation
void jf_init(const uint32_t *dict,  bool enable) {
	here = &ram_dict[0];
	latest = &ram_dict[0];
	trace = enable;
	rom_dict = dict;

	if (rom_dict != NULL) {
		// latest is the last word defined in the rom dictionary
		latest = (uint32_t *)&rom_dict[rom_dict[0]];
	}

	dsp = 0;
	rsp = 0;
	fsp = 0;

	compiling = false;
}

//-----------------------------------------------------------------------------
