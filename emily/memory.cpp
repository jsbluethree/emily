// memory.cpp

#include "memory.h"

namespace emily{

	template<>
	std::string& MemoryManager::get<std::string>(Value val){
		if (val.type != ValType::String)
			throw std::exception{ "Internal Error: attempted to get string from non-string value" };
		return strings[val];
	}

	template<>
	BuiltinFun& MemoryManager::get<BuiltinFun>(Value val){
		if (val.type != ValType::BuiltinFunction)
			throw std::exception{ "Internal Error: attempted to get BuiltinFun from non-BuiltinFun value" };
		return builtinFuns[val];
	}

	template<>
	UserClosure& MemoryManager::get<UserClosure>(Value val){
		if (val.type != ValType::UserClosure)
			throw std::exception{ "Internal Error: attempted to get UserClosure from non-UserClosure value" };
		return userClosures[val];
	}

	template<>
	BuiltinClosure& MemoryManager::get<BuiltinClosure>(Value val){
		if (val.type != ValType::BuiltinClosure)
			throw std::exception{ "Internal Error: attempted to get BuiltinClosure from non-BuiltinClosure value" };
		return builtinClosures[val];
	}

	template<>
	Table& MemoryManager::get<Table>(Value val){
		if (val.type != ValType::Table)
			throw std::exception{ "Internal Error: attempted to get Table from non-Table value" };
		return tables[val];
	}

	template<>
	Continuation& MemoryManager::get<Continuation>(Value val){
		if (val.type != ValType::Continuation)
			throw std::exception{ "Internal Error: attempted to get Continuation from non-Continuation value" };
		return continuations[val];
	}

	Value MemoryManager::create(ValType v){
		switch (v){
		case ValType::String: return strings.create();
		case ValType::BuiltinFunction: return builtinFuns.create();
		case ValType::UserClosure: return userClosures.create();
		case ValType::BuiltinClosure: return builtinClosures.create();
		case ValType::Table: return tables.create();
		case ValType::Continuation: return continuations.create();
		default:
			throw std::exception{ "Internal Error: attempted to allocate object of an unmanaged type" };
		}
	}

	Value MemoryManager::ref(Value val){
		switch (val.type){
		case ValType::String: strings.ref(val); break;
		case ValType::BuiltinFunction: builtinFuns.ref(val); break;
		case ValType::UserClosure: userClosures.ref(val); break;
		case ValType::BuiltinClosure: builtinClosures.ref(val); break;
		case ValType::Continuation: continuations.ref(val); break;
		case ValType::Table: tables.ref(val); break;
		default: break;
		}
		return val;
	}

	int MemoryManager::refcount(Value val) const{
		switch (val.type){
		case ValType::String: return strings.refcount(val);
		case ValType::BuiltinFunction: return builtinFuns.refcount(val);
		case ValType::UserClosure: return userClosures.refcount(val);
		case ValType::BuiltinClosure: return builtinClosures.refcount(val);
		case ValType::Continuation: return continuations.refcount(val);
		case ValType::Table: return tables.refcount(val);
		default: return -1;
		}
	}

	void MemoryManager::deref(Value val){
		switch (val.type){
		case ValType::String: strings.deref(val); break;
		case ValType::BuiltinFunction: builtinFuns.deref(val); break;
		case ValType::UserClosure: userClosures.deref(val); break;
		case ValType::BuiltinClosure: builtinClosures.deref(val); break;
		case ValType::Continuation: continuations.deref(val); break;
		case ValType::Table: 
			if (refcount(val) == 1) free(val);
			else tables.deref(val);
		default: return;
		}
	}

	void MemoryManager::free(Value val){
		switch (val.type){
		case ValType::String: strings.free(val); break;
		case ValType::BuiltinFunction: builtinFuns.free(val); break;
		case ValType::UserClosure: userClosures.free(val); break;
		case ValType::BuiltinClosure: builtinClosures.free(val); break;
		case ValType::Continuation: continuations.free(val); break;
		case ValType::Table:
			for (auto pair : get<Table>(val)){
				deref(pair.second);
			}
			tables.free(val);
		default: return;
		}
	}
}
