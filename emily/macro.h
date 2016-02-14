// macro.h

#ifndef __MACRO_H__
#define __MACRO_H__

#include <algorithm>
#include <iterator>
#include "tokenize.h"

namespace emily{

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
	const std::vector<Macro> built_in_macros = {
		// Misc
		Macro{ Sweep::R, 20, "`", Transform::Backtick },
		Macro{ Sweep::R, 30, "!", Transform::MakePrefixUnary, { "not" } },
		// Math
		Macro{ Sweep::R, 30, "~", Transform::MakeUnary, { "negate" } },
		Macro{ Sweep::R, 40, "/", Transform::MakeSplitter, { "divide" } },
		Macro{ Sweep::R, 40, "*", Transform::MakeSplitter, { "times" } },
		Macro{ Sweep::R, 40, "%", Transform::MakeSplitter, { "mod" } },
		Macro{ Sweep::R, 50, "-", Transform::MakeDualModeSplitter, { "negate", "minus" } },
		Macro{ Sweep::R, 50, "+", Transform::MakeSplitter, { "plus" } },
		// Comparators
		Macro{ Sweep::R, 60, "<", Transform::MakeSplitter, { "lt" } },
		Macro{ Sweep::R, 60, "<=", Transform::MakeSplitter, { "lte" } },
		Macro{ Sweep::R, 60, ">", Transform::MakeSplitter, { "gt" } },
		Macro{ Sweep::R, 60, ">=", Transform::MakeSplitter, { "gte" } },
		Macro{ Sweep::R, 65, "==", Transform::MakeSplitter, { "eq" } },
		Macro{ Sweep::R, 65, "!=", Transform::MakeSplitterInvert, { "eq" } },
		// Orelse
		Macro{ Sweep::L, 67, "//", Transform::Ifndef },
		// Boolean
		Macro{ Sweep::R, 70, "&&", Transform::MakeShortCircuit, { "and" } },
		Macro{ Sweep::R, 75, "||", Transform::MakeShortCircuit, { "or" } },
		Macro{ Sweep::R, 77, "%%", Transform::MakeShortCircuit, { "xor" } },
		// Grouping
		Macro{ Sweep::L, 90, ":", Transform::ApplyRight },
		Macro{ Sweep::L, 90, "?", Transform::Question },
		// Core
		Macro{ Sweep::L, 100, "^", Transform::ClosureConstruct },
		Macro{ Sweep::L, 100, "^@", Transform::ClosureConstruct, { "true" } },
		Macro{ Sweep::L, 105, "=", Transform::Assignment },
		Macro{ Sweep::L, 110, ".", Transform::Atom },
		// Pseudo-statement
		Macro{ Sweep::L, 150, ",", Transform::Comma }
	};

	bool macro_comma(Program& prog, Line& line);
	bool macro_atom(Program& prog, Line& line, CodePos tk);
	bool macro_assign(Program& prog, Line& line, CodePos tk);
	bool macro_closure(Program& prog, Line& line, CodePos tk, bool ret);
	bool macro_question(Program& prog, Line& line, CodePos tk);
	bool macro_apply_right(Program& prog, Line& line, CodePos tk);
	bool macro_short_circuit(Program& prog, Line& line, CodePos tk, const char* str);
	bool macro_ifndef(Program& prog, Line& line, CodePos tk);
	bool macro_splitter(Program& prog, Line& line, CodePos tk, const char* str);
	bool macro_splitter_inv(Program& prog, Line& line, CodePos tk, const char* str);
	bool macro_splitter_dual(Program& prog, Line& line, CodePos tk, const char* str_unary, const char* str_binary);
	bool macro_unary(Program& prog, Line& line, CodePos tk, const char* str);
	bool macro_unary_prefix(Program& prog, Line& line, CodePos tk, const char* str);
	bool macro_backtick(Program& prog, Line& line, CodePos tk);

	bool do_macros(Program& prog);

}

#endif
