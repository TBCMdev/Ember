#pragma once
#include <iostream>
#include <string>

struct Operator
{
	enum class OPTYPE {
		//- ARITHMATIC -  
		ADD,
		SUB,
		DIV,
		MULT,
		MOD,
		ADD_S,
		SUB_S,
		DIV_S,
		MULT_S,
		MOD_S,
		//- LOGICAL - 
		ASSIGN,
		EQUALS,
		NOTEQ,
		LTHAN,
		GTHAN,
		THOEQ,
		GTOEQ,
		//- UNKNOWN? -
		UNKNWN
	};
protected:
	OPTYPE type;
	std::string x = "";
public:
	Operator(lexertk::token& t, bool th = true) {
		if (t.value == "+") type = OPTYPE::ADD;
		else if (t.value == "+") type = OPTYPE::ADD;
		else if (t.value == "-") type = OPTYPE::SUB;
		else if (t.value == "*") type = OPTYPE::MULT;
		else if (t.value == "/") type = OPTYPE::DIV;
		else if (t.value == "%") type = OPTYPE::MOD;
		else if (t.value == "+=") type = OPTYPE::ADD_S;
		else if (t.value == "-=") type = OPTYPE::SUB_S;
		else if (t.value == "*=") type = OPTYPE::MULT_S;
		else if (t.value == "/=") type = OPTYPE::DIV_S;
		else if (t.value == "%=") type = OPTYPE::MOD_S;
		else if (t.value == "=") type = OPTYPE::ASSIGN;
		else if (t.value == "==") type = OPTYPE::EQUALS;
		else if (t.value == "!=") type = OPTYPE::NOTEQ;
		else if (t.value == ">") type = OPTYPE::GTHAN;
		else if (t.value == "<") type = OPTYPE::LTHAN;
		else if (t.value == ">=") type = OPTYPE::GTOEQ;
		else if (t.value == "<=") type = OPTYPE::THOEQ;
		else {
			//if (th) throw ("invalid syntax error: expected operator, not: '" + t.value + "'.");
			type = OPTYPE::UNKNWN;
		}
		x = t.value;
	}
	std::string str() {
		return x;
	}
	bool isValid() { return type == OPTYPE::UNKNWN; }
};

class Exp {
	virtual void done() {}
};
class Node: public Exp {
	Exp* left, * right;
	Operator op;
public:
	Node(Operator _op, Exp* _left, Exp* _right) : left(_left), right(_right), op(_op) {

	}
	void done() {
		delete left;
		delete right;
		delete &op;
	}
};