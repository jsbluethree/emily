// tokenize.h

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <stack>
#include <string>
#include <vector>

const char emily_regex[] { R"*(((0x[0-9a-fA-F]+)|(?:0o([0-7]+))|(?:0b([01]+))|(\.\d+|\d+(?:\.\d*)?(?:[eE][+-]?\d+)?))|([a-zA-Z][a-zA-Z0-9]*)|("((?:\\"|[^"])*)")|([\[\({])|([\]\)}])|([\n;])|(#[^\n]*)|(\\version\s\d+\.\d+)|(\\(?:\s|#[^\n]*)*\n)|([^\(\)\[\]{}\\;\"\w\s]+)|([ \t\r\f\v]+)|(.))*" };

namespace Tok{
	enum EmilyToken{
		Number = 1,
		HexNumber,
		OctNumber,
		BinNumber,
		FloatNumber,
		Word,
		String,
		StringContent,
		GroupOpen,
		GroupClose,
		Newline,
		Comment,
		Version,
		LineStitch,
		Symbol,
		Whitespace,
		Unrecognized
	};
}

struct Token{
	int type;
	int index;
	int line;
	int column;
};

struct ClosureInfo{
	std::vector<std::string> bindings;
	int group_idx;
	char group_kind;
	bool has_return;
};

typedef std::vector<Token> Line;
typedef std::vector<Line> Group;

struct Program{
	std::vector<Group> groups;
	std::vector<double> numbers;
	std::vector<std::string> strings;
	std::vector<std::string> symbols;
	std::vector<std::string> words;
	std::vector<char> group_kinds;
	std::vector<ClosureInfo> closures;
};

typedef std::regex_iterator<std::string::iterator> rgx_it;

Program tokenize(std::string program);

std::ostream& operator<<(std::ostream& os, Program prog);

#endif