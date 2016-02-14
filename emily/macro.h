// macro.h

#ifndef __MACRO_H__
#define __MACRO_H__

#include <algorithm>
#include <iterator>
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

// define built in macros
const Macro built_in_macros[] = {
	// Misc
	{ Sweep::R, 20, "`", Transform::Backtick },
	{ Sweep::R, 30, "!", Transform::MakePrefixUnary, "not" },
	// Math
	{ Sweep::R, 30, "~", Transform::MakeUnary, "negate" },
	{ Sweep::R, 40, "/", Transform::MakeSplitter, "divide" },
	{ Sweep::R, 40, "*", Transform::MakeSplitter, "times" },
	{ Sweep::R, 40, "%", Transform::MakeSplitter, "mod" },
	{ Sweep::R, 50, "-", Transform::MakeDualModeSplitter, "negate", "minus" },
	{ Sweep::R, 50, "+", Transform::MakeSplitter, "plus" },
	// Comparators
	{ Sweep::R, 60, "<", Transform::MakeSplitter, "lt" },
	{ Sweep::R, 60, "<=", Transform::MakeSplitter, "lte" },
	{ Sweep::R, 60, ">", Transform::MakeSplitter, "gt" },
	{ Sweep::R, 60, ">=", Transform::MakeSplitter, "gte" },
	{ Sweep::R, 65, "==", Transform::MakeSplitter, "eq" },
	{ Sweep::R, 65, "!=", Transform::MakeSplitterInvert, "eq" },
	// Orelse
	{ Sweep::L, 67, "//", Transform::Ifndef },
	// Boolean
	{ Sweep::R, 70, "&&", Transform::MakeShortCircuit, "and" },
	{ Sweep::R, 75, "||", Transform::MakeShortCircuit, "or" },
	{ Sweep::R, 77, "%%", Transform::MakeShortCircuit, "xor" },
	// Grouping
	{ Sweep::L, 90, ":", Transform::ApplyRight },
	{ Sweep::L, 90, "?", Transform::Question },
	// Core
	{ Sweep::L, 100, "^", Transform::ClosureConstruct },
	{ Sweep::L, 100, "^@", Transform::ClosureConstruct, "true" },
	{ Sweep::L, 105, "=", Transform::Assignment },
	{ Sweep::L, 110, ".", Transform::Atom },
	// Pseudo-statement
	{ Sweep::L, 150, ",", Transform::Comma }
};

bool macro_comma(Program& prog, int grp, int ln);
bool macro_atom(Program& prog, int grp, int ln, CodePos tk);
bool macro_assign(Program& prog, int grp, int ln, CodePos tk);
bool macro_closure(Program& prog, int grp, int ln, CodePos tk, bool ret);
bool macro_question(Program& prog, int grp, int ln, CodePos tk);
bool macro_apply_right(Program& prog, int grp, int ln, CodePos tk);
bool macro_short_circuit(Program& prog, int grp, int ln, CodePos tk, const char* str);
bool macro_ifndef(Program& prog, int grp, int ln, CodePos tk);
bool macro_splitter(Program& prog, int grp, int ln, CodePos tk, const char* str);
bool macro_splitter_inv(Program& prog, int grp, int ln, CodePos tk, const char* str);
bool macro_splitter_dual(Program& prog, int grp, int ln, CodePos tk, const char* str1, const char* str2);
bool macro_unary(Program& prog, int grp, int ln, CodePos tk, const char* str);
bool macro_unary_prefix(Program& prog, int grp, int ln, CodePos tk, const char* str);
bool macro_backtick(Program& prog, int grp, int ln, CodePos tk);

bool do_macros(Program& prog);

#endif
