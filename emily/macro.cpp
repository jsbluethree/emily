// macro.cpp

#include "macro.h"

// splits a line on , into a group of lines
// prefixes each line with this .append
bool macro_comma(Program& prog, int grp, int ln){
	// pull the line containing commas out of the structure
	Line comma_line{ std::move(prog.groups[grp][ln]) };
	Token tok_this{ Tok::Word, prog.intern("this"), comma_line.front().line, comma_line.front().column };
	Token tok_append{ Tok::Atom, prog.intern("append"), comma_line.front().line, comma_line.front().column };
	prog.groups[grp][ln] = { { Tok::GroupOpen, prog.groups.size(), comma_line.front().line, comma_line.front().column } };
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
bool macro_atom(Program& prog, int grp, int ln, CodePos tk){
	Token tok = *tk;
	auto next = prog.groups[grp][ln].erase(tk);
	if (next == prog.groups[grp][ln].end() || next->type != Tok::Word){
		syntax_error(tok, "expected identifier after .");
		return false;
	}
	next->type = Tok::Atom;
	return true;
}

// perform assignment transformation
bool macro_assign(Program& prog, int grp, int ln, CodePos tk){
	return false;
}

bool do_macros(Program& prog){
	return false;
}