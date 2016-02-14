// keywords.h

#ifndef __KEYWORDS_H__
#define __KEYWORDS_H__

#include <string>
#include <vector>

namespace emily{

	const std::vector<std::string> keywords = {
		"has", "set", "let", "parent", "!id", "current", "this", "super", "return",
		"package", "project", "directory", "internal", "nonlocal", "private", "exportLet",
		"eq", "null", "true", "print", "println", "ln", "sp", "do", "loop", "if", "while",
		"not", "and", "or", "xor", "nullfn", "tern", "append", "each", "negate", "add",
		"minus", "times", "divide", "mod", "lt", "lte", "gt", "gte", "thisTransplant",
		"thisFreeze", "thisInit", "thisUpdate", "check", "scope"
	};

}

#endif