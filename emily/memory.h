// memory.h

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stack>
#include <vector>
#include "values.h"

namespace emily{

	template <typename T, ValType V>
	class MemPool{
		std::vector<T> items;
		std::vector<int> refs;
		std::stack<int> freed;

		void ref(int index);
		int refcount(int index) const;
		void deref(int index);
		void free(int index);
		T& operator[](int index);

	public:
		Value create();
		void ref(Value val);
		int refcount(Value val) const;
		void deref(Value val);
		void free(Value val);
		T& operator[](Value val);
	};

	class MemoryManager{
		MemPool<std::string, ValType::String> strings;
		MemPool<BuiltinClosure, ValType::BuiltinClosure> builtinClosures;
		MemPool<BuiltinFun, ValType::BuiltinFunction> builtinFuns;
		MemPool<UserClosure, ValType::UserClosure> userClosures;
		MemPool<Table, ValType::Table> tables;
		MemPool<Continuation, ValType::Continuation> continuations;

	public:
		Value create(ValType v);
		void ref(Value val);
		int refcount(Value val) const;
		void deref(Value val);
		void free(Value val);
		template<typename T>
		T& get(Value val);
	};

	// IMPLEMENTATION BEGINS HERE

	template<typename T, ValType V>
	Value MemPool<T, V>::create(){
		if (freed.empty()){
			items.push_back(T{});
			refs.push_back(1);
			return Value{ V, items.size() - 1 };
		}
		else{
			refs[freed.top()] = 1;
			int idx = freed.top();
			freed.pop();
			return Value{ V, idx };
		}
	}

	template<typename T, ValType V>
	void MemPool<T, V>::ref(int index){
		if (refs[index] > 0) ++refs[index];
		else throw std::exception{ "Internal Error: attempted to reference freed object" };
	}

	template<typename T, ValType V>
	void MemPool<T, V>::ref(Value val){
		if (val.type != V)
			throw std::exception{ "Internal Error: attempted to reference object of incorrect type" };
		ref(val.index);
	}

	template<typename T, ValType V>
	int MemPool<T, V>::refcount(int index) const{
		return refs[index];
	}

	template<typename T, ValType V>
	int MemPool<T, V>::refcount(Value val) const{
		if (val.type != V)
			throw std::exception{ "Internal Error: attempted to get refcount of object of incorrect type" };
		return refcount(val.index);
	}

	template<typename T, ValType V>
	void MemPool<T, V>::deref(int index){
		if (refs[index] > 1) --refs[index];
		else if (refs[index] == 1) free(index);
		else throw std::exception{ "Internal Error: attempted to dereference freed object" };
	}

	template<typename T, ValType V>
	void MemPool<T, V>::deref(Value val){
		if (val.type != V)
			throw std::exception{ "Internal Error: attempted to dereference object of incorrect type" };
		deref(val.index);
	}

	template<typename T, ValType V>
	void MemPool<T, V>::free(int index){
		if (refs[index] < 1)
			throw std::exception{ "Internal Error: attempted to free already freed object" };
		items[index] = T{};
		refs[index] = 0;
		freed.push(index);
	}

	template<typename T, ValType V>
	void MemPool<T, V>::free(Value val){
		if (val.type != V)
			throw std::exception{ "Internal Error: attempted to free object of incorrect type" };
		free(val.index);
	}

	template<typename T, ValType V>
	T& MemPool<T, V>::operator[](int index){
		if (refs[index] > 0) return items[index];
		else throw std::exception{ "Internal Error: attempted to access freed object" };
	}

	template<typename T, ValType V>
	T& MemPool<T, V>::operator[](Value val){
		if (val.type != V)
			throw std::exception{ "Internal Error: attempted to access object of incorrect type" };
		return operator[](val.index);
	}

}

#endif
