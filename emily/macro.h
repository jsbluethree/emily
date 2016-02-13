// macro.h

#ifndef __MACRO_H__
#define __MACRO_H__

#include "keywords.h"
#include "tokenize.h"

// which direction to search lines for symbols
enum class Sweep{ L, R };

// enumerate transforms for built in macros
// except for user defined, maps to a function
enum class Transform{
	Comma,
	Atom,
	Assignment,
	ClosureConstruct,
	Question,
	ApplyRight,
	MakeShortCircuit,
	Ifndef,
	MakeSplitter,
	MakeSplitterInvert,
	MakeDualModeSplitter,
	MakeUnary,
	MakePrefixUnary,
	Backtick,
	UserDefined
};

/**	structure for macro transforms
 *	for user defined macros, use args[0] to designate function and args[1] for args to it?
 *	user defined stuff to be implemented later
 */
struct Macro{
	Sweep swp;
	float priority;
	const char* sym;
	Transform fn;
	const char* args[2];
};

extern const Macro built_in_macros[];

bool macro_comma(Program& prog, int grp, int ln, int tk);
bool macro_atom(Program& prog, int grp, int ln, int tk);
bool macro_assign(Program& prog, int grp, int ln, int tk);
bool macro_closure(Program& prog, int grp, int ln, int tk, bool ret);
bool macro_question(Program& prog, int grp, int ln, int tk);
bool macro_apply_right(Program& prog, int grp, int ln, int tk);
bool macro_short_circuit(Program& prog, int grp, int ln, int tk, const char* str);
bool macro_ifndef(Program& prog, int grp, int ln, int tk);
bool macro_splitter(Program& prog, int grp, int ln, int tk, const char* str);
bool macro_splitter_inv(Program& prog, int grp, int ln, int tk, const char* str);
bool macro_splitter_dual(Program& prog, int grp, int ln, int tk, const char* str1, const char* str2);
bool macro_unary(Program& prog, int grp, int ln, int tk, const char* str);
bool macro_unary_prefix(Program& prog, int grp, int ln, int tk, const char* str);
bool macro_backtick(Program& prog, int grp, int ln, int tk);

bool do_macros(Program& prog);

#endif
