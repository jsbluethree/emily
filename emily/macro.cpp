// macro.cpp

#include "macro.h"

namespace emily{

	// splits a line on , into a group of lines
	// prefixes each line with this .append
	bool macro_comma(Program& prog, Line& line){
		// pull the line containing commas out of the structure
		Line comma_line{ std::move(line) };
		Token tok_this{ Tok::Word, prog.intern("this"), comma_line.front().line, comma_line.front().column };
		Token tok_append{ Tok::Atom, prog.intern("append"), comma_line.front().line, comma_line.front().column };
		line = Line{ Token{ Tok::GroupOpen, prog.groups.size(), comma_line.front().line, comma_line.front().column } };
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
	bool macro_atom(Program& prog, Line& line, CodePos tk){
		Token tok = *tk;
		auto next = line.erase(tk);
		if (next == line.end() || next->type != Tok::Word){
			syntax_error(tok, "expected identifier after .");
			return false;
		}
		next->type = Tok::Atom;
		return true;
	}

	// perform assignment transformation
	// [target] key [^bindings] = [exp] => [target] .let key [^@bindings] ([exp])
	// target and bindings are optional
	// if first token in line is nonlocal, use .set instead of .let
	bool macro_assign(Program& prog, Line& line, CodePos tk){
		if (tk == line.begin()){
			syntax_error(*tk, "no expression left of =");
			return false;
		}
		CodePos exp = std::next(tk);
		if (exp == line.end()){
			syntax_error(*tk, "no expression right of =");
			return false;
		}
		// move the [exp] part into a new () group
		prog.groups.push_back(Group{ Line{} });
		prog.group_kinds.push_back('(');
		line.insert(exp, Token{ Tok::GroupOpen, prog.groups.size() - 1, tk->line, tk->column });
		Line& new_line = prog.groups.back().back();
		new_line.splice(new_line.begin(), line, exp, line.end());
		// find location of closure bindings if any
		CodePos clos = std::find_if(line.begin(), line.end(), [&prog](Token tok){
			return tok.type == Tok::Symbol && prog.symbols[tok.index] == "^" || prog.symbols[tok.index] == "^@";
		});
		// find location of key
		CodePos key;
		if (clos == line.end()){
			key = std::prev(tk);
		}
		else{
			key = std::prev(clos);
			clos->index = prog.sym("^@");
		}
		// insert .set or .let before key
		if (line.front().type == Tok::Word && line.front().index == prog.intern("nonlocal")){
			line.insert(key, Token{ Tok::Atom, prog.intern("set"), tk->line, tk->column });
			line.pop_front();
		}
		else{
			line.insert(key, Token{ Tok::Atom, prog.intern("let"), tk->line, tk->column });
		}
		// and remove = token
		line.erase(tk);
		return true;
	}

	// create a closure token from a list of bindings and a group token
	bool macro_closure(Program& prog, Line& line, CodePos tk, bool ret){
		Token tok_clos = *tk;
		ClosureInfo clos{};
		clos.has_return = ret;
		// consume tokens until a group is found
		while (tk != line.end() && tk->type != Tok::GroupOpen){
			switch (tk->type){
			case Tok::Symbol:
				if (prog.symbols[tk->index] == "^" || prog.symbols[tk->index] == "^@"){
					tk = line.erase(tk);
				}
				else if (prog.symbols[tk->index] == ":"){
					// construct the group
					if (!macro_apply_right(prog, line, tk)) return false;
					tk = std::prev(line.end());
				}
				else{
					syntax_error(*tk, "illegal token found in closure bindings");
					return false;
				}
				break;
			case Tok::Word:
				clos.bindings.push_back(*tk);
				tk = line.erase(tk);
				break;
			default:
				syntax_error(*tk, "illegal token found in closure bindings");
				return false;
			}
		}
		if (tk == line.end()){
			syntax_error(tok_clos, "body missing for closure");
			return false;
		}
		else if (prog.group_kinds[tk->index] == '['){
			syntax_error(*tk, "illegal construction ^[]");
			return false;
		}
		// turn the token into a closure token and push the closure info
		clos.group_idx = tk->index;
		tk->type = Tok::GroupClose;
		tk->index = prog.closures.size();
		prog.closures.push_back(clos);
		return true;
	}

	// perform ternary statement transform
	// [cond] ? [exp1] : [exp2] => tern ([cond]) ^([exp1]) ^([exp2])
	bool macro_question(Program& prog, Line& line, CodePos tk){
		// pull the line out of the structure
		Line old_line{ Token{ Tok::Word, prog.intern("tern"), tk->line, tk->column } };
		old_line.swap(line);
		// make sure there are no other ?s in the line
		CodePos que = std::find_if(tk, old_line.end(), [&prog](Token tok){
			return tok.type == Tok::Symbol && prog.symbols[tok.index] == "?";
		});
		if (que != old_line.end()){
			syntax_error(*que, "nesting like ? ? : : is not allowed");
			return false;
		}
		// find the : in the old line
		CodePos colon = std::find_if(tk, old_line.end(), [&prog](Token tok){
			return tok.type == Tok::Symbol && prog.symbols[tok.index] == ":";
		});
		if (colon == old_line.end()){
			syntax_error(*tk, "expected : after ?");
			return false;
		}
		// create new groups
		// condition group:
		line.push_back(Token{ Tok::GroupOpen, prog.groups.size(), tk->line, tk->column });
		prog.groups.push_back(Group{ Line{ old_line.begin(), tk } });
		prog.group_kinds.push_back('(');
		// statement 1:
		prog.closures.push_back(ClosureInfo{ {}, prog.groups.size(), false });
		line.push_back(Token{ Tok::GroupClose, prog.closures.size() - 1, tk->line, tk->column });
		prog.groups.push_back(Group{ Line{ std::next(tk), colon } });
		prog.group_kinds.push_back('(');
		// statement 2:
		prog.closures.push_back(ClosureInfo{ {}, prog.groups.size(), false });
		line.push_back(Token{ Tok::GroupClose, prog.closures.size() - 1, tk->line, tk->column });
		prog.groups.push_back(Group{ Line{ std::next(colon), old_line.end() } });
		prog.group_kinds.push_back('(');
		return true;
	}

	// group all tokens to the right
	bool macro_apply_right(Program& prog, Line& line, CodePos tk){
		line.insert(tk, Token{ Tok::GroupOpen, prog.groups.size(), tk->line, tk->column });
		prog.groups.push_back(Group{ Line{} });
		prog.group_kinds.push_back('(');
		Line& new_line = prog.groups.back().back();
		new_line.splice(new_line.begin(), line, std::next(tk), line.end());
		line.erase(tk);
		return true;
	}

	// perform short circuit transform
	// [exp1] OP [exp2] => op ^([exp1]) ^([exp2])
	bool macro_short_circuit(Program& prog, Line& new_line, CodePos tk, const char* str){
		// pull the line out of the structure
		Line old_line{ Token{ Tok::Word, prog.intern(str), tk->line, tk->column } };
		old_line.swap(new_line);
		// create new groups
		// left group
		prog.closures.push_back(ClosureInfo{ {}, prog.groups.size(), false });
		new_line.push_back(Token{ Tok::GroupClose, prog.closures.size() - 1, tk->line, tk->column });
		prog.groups.push_back(Group{ Line{ old_line.begin(), tk } });
		prog.group_kinds.push_back('(');
		// right group
		prog.closures.push_back(ClosureInfo{ {}, prog.groups.size(), false });
		new_line.push_back(Token{ Tok::GroupClose, prog.closures.size() - 1, tk->line, tk->column });
		prog.groups.push_back(Group{ Line{ std::next(tk), old_line.end() } });
		prog.group_kinds.push_back('(');
		return true;
	}

	// perform ifndef transform
	// works like perl // operator
	// word // [exp] => check scope .word ^([exp])
	// [exp1] token // [exp2] => check ([exp1]) token ^([exp2]) 
	bool macro_ifndef(Program& prog, Line& line, CodePos tk){
		Token tok = *tk;
		if (tk == line.begin()){
			syntax_error(tok, "nothing found left of // operator");
			return false;
		}
		// construct the closure at the end first
		prog.closures.push_back(ClosureInfo{ {}, prog.groups.size(), false });
		prog.groups.push_back(Group{ Line{} });
		prog.group_kinds.push_back('(');
		Line& new_line = prog.groups.back().back();
		new_line.splice(new_line.begin(), line, std::next(tk), line.end());
		line.erase(tk);
		line.push_back(Token{ Tok::GroupClose, prog.closures.size() - 1, tok.line, tok.column });
		CodePos word = std::prev(line.end(), 2);
		if (word == line.begin()){
			if (word->type != Tok::Word){
				syntax_error(*word, "expected variable name or field access left of // operator");
				return false;
			}
			word->type = Tok::Atom;
			line.push_front(Token{ Tok::Word, prog.intern("scope"), tok.line, tok.column });
		}
		else{
			prog.groups.push_back(Group{ Line{} });
			prog.group_kinds.push_back('(');
			Line& newer_line = prog.groups.back().back();
			newer_line.splice(newer_line.begin(), line, line.begin(), word);
			line.push_front(Token{ Tok::GroupOpen, prog.groups.size() - 1, tok.line, tok.column });
		}
		line.push_front(Token{ Tok::Word, prog.intern("check"), tok.line, tok.column });
		return true;
	}

	// creates a splitter
	// [exp1] OP [exp2] => ([exp1]) .op ([exp2])
	bool macro_splitter(Program& prog, Line& line, CodePos tk, const char* str){
		if (tk == line.begin()){
			syntax_error(*tk, "expected something left of splitter operator");
			return false;
		}
		if (std::next(tk) == line.end()){
			syntax_error(*tk, "expected something right of splitter operator");
			return false;
		}
		// create left group
		prog.groups.push_back(Group{ Line{} });
		prog.group_kinds.push_back('(');
		Line& left_line = prog.groups.back().back();
		left_line.splice(left_line.begin(), line, line.begin(), tk);
		line.push_front(Token{ Tok::GroupOpen, prog.groups.size() - 1, left_line.front().line, left_line.front().column });
		// create right group
		prog.groups.push_back(Group{ Line{} });
		prog.group_kinds.push_back('(');
		Line& right_line = prog.groups.back().back();
		right_line.splice(right_line.begin(), line, std::next(tk), line.end());
		line.push_back(Token{ Tok::GroupOpen, prog.groups.size() - 1, right_line.front().line, right_line.front().column });
		// change symbol to atom
		tk->type = Tok::Atom;
		tk->index = prog.intern(str);
		return true;
	}

	// creates a splitter, then prefixes it with not
	// [exp1] OP [exp2] => not ( ([exp1]) .op ([exp2]) )
	bool macro_splitter_inv(Program& prog, Line& line, CodePos tk, const char* str){
		if (!macro_splitter(prog, line, tk, str)) return false;
	}

	bool do_macros(Program& prog){
		return false;
	}

}