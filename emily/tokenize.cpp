// tokenize.cpp
// Chris Bowers

#include "tokenize.h"

namespace emily{

	// interns a string, returning its index
	int Program::intern(std::string str){
		int i = std::find(words.begin(), words.end(), str) - words.begin();
		if (i == words.size())
			words.push_back(str);
		return i;
	}

	// interns a symbol string, returning its index
	// TODO: check for non-symbol strings?
	int Program::sym(std::string str){
		int i = std::find(symbols.begin(), symbols.end(), str) - symbols.begin();
		if (i == symbols.size())
			symbols.push_back(str);
		return i;
	}

	/**
	 *	Program tokenize(std::string)
	 *	takes a string containing the program to be tokenized
	 *	outputs a structure representing the program
	 */
	Program tokenize(std::string program){
		using namespace std;
		// set up program data structure
		Program prog{};
		prog.groups.push_back(Group{});
		prog.groups.back().push_back(Line{});
		prog.group_kinds.push_back('(');
		// initialize with keywords
		prog.words = keywords;
		// set up stack to track current group
		stack<Token> curr_group{};
		curr_group.push(Token{ Tok::Group, 0, 0, 0 });
		// track line and column number for debugging info
		int line_number = 1;
		int line_offset = 0;
		regex em_rgx{ emily_regex };
		// for each regex match, test the length of each submatch
		// nonzero submatch length indicates the type of token matched
		for (rgx_it rit{ program.begin(), program.end(), em_rgx }, rend{}; rit != rend; ++rit){
			Token tok{ -1, -1, line_number, rit->position() - line_offset };
			if ((*rit)[Tok::HexNumber].length() > 0){
				tok.type = Tok::Number;
				tok.index = prog.numbers.size();
				prog.numbers.push_back(strtol(program.c_str() + rit->position(), nullptr, 16));
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::OctNumber].length() > 0){
				tok.type = Tok::Number;
				// skip over the 0o
				tok.index = prog.numbers.size();
				prog.numbers.push_back(strtol(program.c_str() + rit->position() + 2, nullptr, 8));
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::BinNumber].length() > 0){
				tok.type = Tok::Number;
				// skip over the 0b
				tok.index = prog.numbers.size();
				prog.numbers.push_back(strtol(program.c_str() + rit->position() + 2, nullptr, 2));
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::FloatNumber].length() > 0){
				tok.type = Tok::Number;
				tok.index = prog.numbers.size();
				prog.numbers.push_back(strtod(program.c_str() + rit->position(), nullptr));
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::Word].length() > 0){
				tok.type = Tok::Word;
				tok.index = prog.intern(rit->str());
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::String].length() > 0){
				tok.type = Tok::String;
				tok.index = prog.strings.size();
				// TODO: check for escape sequences
				prog.strings.push_back((*rit)[Tok::StringContent].str());
				// check for newlines
				int nls = count(rit->begin(), rit->end(), '\n');
				if (nls > 0){
					line_number += nls;
					line_offset = rit->position() + rit->str().rfind('\n');
				}
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::Symbol].length() > 0){
				tok.type = Tok::Symbol;
				tok.index = prog.sym(rit->str());
				prog.groups[curr_group.top().index].back().push_back(tok);
			}
			else if ((*rit)[Tok::Group].length() > 0){
				// TODO: elide redundant groups here?
				tok.type = Tok::Group;
				tok.index = prog.groups.size();
				prog.group_kinds.push_back(program[rit->position()]);
				prog.groups[curr_group.top().index].back().push_back(tok);
				// add the new group to the list and push its token to the stack
				curr_group.push(tok);
				prog.groups.push_back(Group{});
				prog.groups.back().push_back(Line{});
			}
			else if ((*rit)[Tok::GroupClose].length() > 0){
				if (program[rit->position()] != closer(prog.group_kinds[curr_group.top().index])){
					syntax_error(tok, "incorrect group closer");
					return{};
				}
				if (curr_group.size() <= 1){
					syntax_error(tok, "unmatched group closer");
					return{};
				}
				curr_group.pop();
			}
			else if ((*rit)[Tok::Newline].length() > 0){
				if (program[rit->position()] == '\n'){
					++line_number;
					line_offset = rit->position() + rit->length();
				}
				// only add new line if current line is not empty
				if (!prog.groups[curr_group.top().index].back().empty())
					prog.groups[curr_group.top().index].push_back(Line{});
			}
			else if ((*rit)[Tok::LineStitch].length() > 0){
				line_number += count(rit->begin(), rit->end(), '\n');
				line_offset = rit->position() + rit->length();
			}
			else if ((*rit)[Tok::Unrecognized].length() > 0){
				syntax_error(tok, "unrecognized character");
				return{};
			}
			// for other cases, do nothing
		}
		// make sure all groups were closed
		if (curr_group.size() > 1){
			syntax_error(curr_group.top(), "unmatched group opener");
			return{};
		}
		return prog;
	}

	std::ostream& operator<<(std::ostream& os, Program prog){
		int g = 0;
		for (const auto& group : prog.groups){
			if (!group.empty())
				os << prog.group_kinds[g] << g << closer(prog.group_kinds[g]) << ":\n";
			++g;
			for (const auto& ln : group){
				for (auto tk : ln){
					switch (tk.type){
					case Tok::Number:
						os << prog.numbers[tk.index];
						break;
					case Tok::String:
						os << '"' << prog.strings[tk.index] << '"';
						break;
					case Tok::Symbol:
						os << prog.symbols[tk.index];
						break;
					case Tok::Atom:
						os << '.';
					case Tok::Word:
						os << prog.words[tk.index];
						break;
					case Tok::Group:
						os << prog.group_kinds[tk.index] << tk.index << closer(prog.group_kinds[tk.index]);
						break;
					case Tok::Closure:
						os << '^' << prog.group_kinds[prog.closures[tk.index].group_idx]
							<< prog.closures[tk.index].group_idx << closer(prog.group_kinds[prog.closures[tk.index].group_idx]);
						break;
					default:
						os << "!ERROR!";
						break;
					}
					os << ' ';
				}
				os << '\n';
			}
		}
		return os;
	}

	void syntax_error(Token& tok, const char* msg){
		std::cerr << "Syntax Error at (" << tok.line << ',' << tok.column << "): " << msg << std::endl;
		tok.type = Tok::Error;
	}

	char closer(char op){
		switch (op){
		case '(': return ')';
		case '[': return ']';
		case '{': return '}';
		default:  return '\0';
		}
	}

}