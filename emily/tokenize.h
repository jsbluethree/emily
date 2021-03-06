// tokenize.h

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <list>
#include <regex>
#include <stack>
#include <string>
#include <vector>
#include "keywords.h"

namespace emily{

	const char emily_regex[] { R"*(((0x[0-9a-fA-F]+)|(?:0o([0-7]+))|(?:0b([01]+))|(\.\d+|\d+(?:\.\d*)?(?:[eE][+-]?\d+)?))|([a-zA-Z][a-zA-Z0-9]*)|("((?:\\"|[^"])*)")|([\[\({])|([\]\)}])|([\n;])|(#[^\n]*)|(\\version\s\d+\.\d+)|(\\(?:\s|#[^\n]*)*\n)|([^\(\)\[\]{}\\;\"\w\s]+)|([ \t\r\f\v]+)|(.))*" };

	namespace Tok{
		enum TokenType{
			Number = 1,
			HexNumber,
			OctNumber,
			BinNumber,
			FloatNumber,
			Word,
			String,
			StringContent,
			Group,
			GroupClose,
			Newline,
			Comment,
			Version,
			LineStitch,
			Symbol,
			Whitespace,
			Unrecognized,
			Atom,
			Closure,
			Error
		};
	}

	struct Token{
		int type;
		int index;
		int line;
		int column;
	};

	struct ClosureInfo{
		std::vector<Token> bindings;
		int group_idx;
		bool has_return;
	};

	typedef std::list<Token> Line;
	typedef std::vector<Line> Group;

	struct Program{
		std::vector<Group> groups;
		std::vector<double> numbers;
		std::vector<std::string> strings;
		std::vector<std::string> symbols;
		std::vector<std::string> words;
		std::vector<char> group_kinds;
		std::vector<ClosureInfo> closures;

		int intern(std::string str);
		int sym(std::string str);
	};

	typedef std::regex_iterator<std::string::iterator> rgx_it;
	typedef std::list<Token>::iterator CodePos;

	/**
	*	Program tokenize(std::string)
	*	takes a string containing the program to be tokenized
	*	outputs a structure representing the program
	*/
	Program tokenize(std::string program);

	// outputs program structure in a semi-readable form
	std::ostream& operator<<(std::ostream& os, Program prog);

	// prints an error message and changes the type of tok to Error
	void syntax_error(Token& tok, const char* msg);

	// maps group openers to appropriate closers
	// maps all other characters to null
	char closer(char op);

}

#endif
