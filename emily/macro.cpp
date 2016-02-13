// macro.cpp

#include "macro.h"

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
	{ Sweep::L, 100, "^@", Transform::ClosureConstruct, "" },
	{ Sweep::L, 105, "=", Transform::Assignment },
	{ Sweep::L, 110, ".", Transform::Atom },
	// Pseudo-statement
	{ Sweep::L, 150, ",", Transform::Comma }
};

// splits a line on , into a group of lines
// prefixes each line with this .append
bool macro_comma(Program& prog, int grp, int ln, int tk){
	Token tok_this{ Tok::Word, prog.intern("this") };
	Token tok_append{ Tok::Atom, prog.intern("append") };
	// pull the line containing commas out of the structure
	Line comma_line = std::move(prog.groups[grp][ln]);
	prog.groups[grp][ln] = { { Tok::GroupOpen, prog.groups.size() } };
	prog.groups.push_back(Group{});
	prog.group_kinds.push_back('(');
	prog.groups.back().push_back(Line{ tok_this, tok_append });
	for (auto tok : comma_line){
		if (tok.type == Tok::Symbol && prog.symbols[tok.index] == ",")
			prog.groups.back().push_back(Line{ tok_this, tok_append });
		else
			prog.groups.back().back().push_back(tok);
	}
	return true;
}

// turns the next word into an atom
bool macro_atom(Program& prog, int grp, int ln, int tk){
	prog.groups[grp][ln].erase(prog.groups[grp][ln].begin() + tk);
	if (prog.groups[grp][ln][tk].type != Tok::Word)
		return false;
	prog.groups[grp][ln][tk].type = Tok::Atom;
	return true;
}

bool do_macros(Program& prog){
	return false;
}