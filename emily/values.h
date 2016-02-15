// values.h

#ifndef __VALUES_H__
#define __VALUES_H__

#include <functional>
#include <unordered_map>
#include <vector>
#include "tokenize.h"

namespace emily{

	enum class ValType{
		// singleton types: no need to store a value
		Null,
		True,
		// simple type: value stored in place
		Number,
		// value interned in program structure
		Atom,
		// complex values: memory needs to be managed
		String,
		BuiltinFunction,
		UserClosure,
		BuiltinClosure,
		Table,
		Continuation
	};

	enum class ClosureThis{
		Blank,
		Never,
		Current,
		Frozen
	};

	struct Value{
		ValType type;
		union{
			double number;
			int index;
		};
	};

	struct UserClosure{
		std::vector<Value> bound;
		ClosureInfo info;
		Value thisBindings[2];
		Value envScope;
		ClosureThis thisKind;
	};

	typedef std::function<Value(std::vector<Value>)> BuiltinFun;

	struct BuiltinClosure{
		BuiltinFun fn;
		std::vector<Value> bound;
		Value thisBindings[2];
		int argc;
		ClosureThis thisKind;
	};

	enum class RegState{
		LineStart,
		FirstValue,
		PairValue
	};

	struct StackFrame{
		Value reg[2];
		Value scope;
		RegState state;
		int group;
		int line;
		CodePos pos;
	};

	typedef std::vector<StackFrame> ExecStack;

	struct Continuation{
		ExecStack stack;
		Token position;
	};

	typedef std::unordered_map<Value, Value> Table;

	// operator== for values
	// for bools and numbers does value equality
	// for others, index equality
	bool operator==(Value l, Value r);

}

template<>
struct std::hash<emily::Value>{
	typedef size_t result_type;
	typedef emily::Value argument_type;
	size_t operator()(const emily::Value& arg);
};

#endif
