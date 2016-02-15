// values.cpp

#include "values.h"

size_t std::hash<emily::Value>::operator()(const emily::Value& arg){
	if (arg.type == emily::ValType::Number)
		return std::hash<int>()((int)arg.type) ^ std::hash<double>()(arg.number);
	else
		return std::hash<int>()((int)arg.type) ^ std::hash<int>()(arg.index);
}

bool operator==(emily::Value l, emily::Value r){
	if (l.type != r.type) return false;
	if (l.type == emily::ValType::Number)
		return l.number == r.number;
	else if (l.type == emily::ValType::True || l.type == emily::ValType::Null)
		return true;
	else
		return l.index == r.index;
}