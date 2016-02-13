// tokenize.cpp
// Chris Bowers

#include "tokenize.h"

// interns a string, returning its index
int Program::intern(std::string str){
	int i = std::find(words.begin(), words.end(), str) - words.begin();
	if (i == words.size())
		words.push_back(str);
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
	// set up stack to track current group index
	stack<int> curr_group{};
	curr_group.push(0);
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
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::OctNumber].length() > 0){
			tok.type = Tok::Number;
			// skip over the 0o
			tok.index = prog.numbers.size();
			prog.numbers.push_back(strtol(program.c_str() + rit->position() + 2, nullptr, 8));
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::BinNumber].length() > 0){
			tok.type = Tok::Number;
			// skip over the 0b
			tok.index = prog.numbers.size();
			prog.numbers.push_back(strtol(program.c_str() + rit->position() + 2, nullptr, 2));
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::FloatNumber].length() > 0){
			tok.type = Tok::Number;
			tok.index = prog.numbers.size();
			prog.numbers.push_back(strtod(program.c_str() + rit->position(), nullptr));
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::Word].length() > 0){
			tok.type = Tok::Word;
			tok.index = prog.intern(rit->str());
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::String].length() > 0){
			tok.type = Tok::String;
			tok.index = prog.strings.size();
			prog.strings.push_back((*rit)[Tok::StringContent].str());
			// check for newlines
			int nls = count(rit->begin(), rit->end(), '\n');
			if (nls > 0){
				line_number += nls;
				line_offset = rit->position() + rit->str().rfind('\n') + 1;
			}
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::Symbol].length() > 0){
			tok.type = Tok::Symbol;
			tok.index = prog.symbols.size();
			prog.symbols.push_back(rit->str());
			prog.groups[curr_group.top()].back().push_back(tok);
		}
		else if ((*rit)[Tok::GroupOpen].length() > 0){
			tok.type = Tok::GroupOpen;
			tok.index = prog.groups.size();
			prog.group_kinds.push_back(*(program.c_str() + rit->position()));
			prog.groups[curr_group.top()].back().push_back(tok);
			// add the new group to the list and push its index to the stack
			curr_group.push(prog.groups.size());
			prog.groups.push_back(Group{});
			prog.groups.back().push_back(Line{});
		}
		else if ((*rit)[Tok::GroupClose].length() > 0){
			// TODO: check that the group closer matches (impossible with the current setup?)
			// TODO: check that the group closer is matched (stack.size() > 1)
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
			if (!prog.groups[curr_group.top()].back().empty())
				prog.groups[curr_group.top()].push_back(Line{});
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
		syntax_error(prog.groups[curr_group.top()].front().front(), "unmatched group opener");
		return{};
	}
	return prog;	
}

std::ostream& operator<<(std::ostream& os, Program prog){
	int g = 0;
	for (const auto& group : prog.groups){
		os << g++ << ":\n";
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
				case Tok::GroupOpen:
					os << prog.group_kinds[tk.index] << tk.index;
					break;
				case Tok::GroupClose:
					os << '^' << prog.group_kinds[prog.closures[tk.index].group_idx]
					   << prog.closures[tk.index].group_idx;
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

void syntax_error(Token tok, const char* msg){
	std::cerr << "Syntax Error at (" << tok.line << ',' << tok.column << "): " << msg << std::endl;
}