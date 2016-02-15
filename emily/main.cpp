// main.cpp

#include "tokenize.h"
#include "macro.h"
#include "keywords.h"
#include "memory.h"

int main(int argc, char** argv){
	using namespace emily;
	using namespace std;

	MemoryManager mem;

	Value s = mem.create(ValType::String);

	mem.get<string>(s) = "String! ";

	std::cout << mem.get<string>(s);

	try{
		std::cout << mem.get<Table>(s).size();
	}
	catch (exception e){
		std::cout << e.what();
	}
	
	std::cin.get();
}