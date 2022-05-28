#pragma once
#include <iostream>
#include <string>
#include "Parser.h"
class Node {
protected:
	lexertk::token& t;
public:
	Node(lexertk::token& to): t(to){}
	lexertk::token& getToken() { return t; }
	static int precedence(Node& o) {
		if (marine::Base::is(o.getToken(), "(") ||
		marine::Base::is(o.getToken(), ")")) return 1;
		if (marine::Base::is(o.getToken(), "*") || marine::Base::is(o.getToken(), "/") || marine::Base::is(o.getToken(), "%")) return 2;
		if (marine::Base::is(o.getToken(), "+") || marine::Base::is(o.getToken(), "-"))  return 3;
		return -1;
	}
	static int precedence(lexertk::token& o) {
		if (o.value == "(" ||
			o.value == ")") return 1;
		if (o.value == "*" || o.value == "/" || o.value == "%") return 2;
		if (o.value == "+" || o.value == "-")  return 3;
		return -1;
	}
};
struct Operator: public Node
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
		

		//-BRACKETS-
		LPR,
		RPR,
		//- UNKNOWN? -
		UNKNWN
	};
protected:
	OPTYPE type;
	std::string x = "";
public:
	Operator(lexertk::token& t, bool th = true): Node(t) {
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
		else if (t.value == "(") type = OPTYPE::LPR;
		else if(t.value == ")") type = OPTYPE::RPR;
		else {
			//if (th) throw ("invalid syntax error: expected operator, not: '" + t.value + "'.");
			type = OPTYPE::UNKNWN;
		}
		x = t.value;
	}
	std::string str() {
		return x;
	}
	bool isValid() { return type != OPTYPE::UNKNWN; }
	
};
class BinOpNode: public Node {
	Node* left = nullptr, * right = nullptr;
	Operator* op = nullptr;
public:
	BinOpNode(Node& n): left(&n), Node(n.getToken()) {
		
	}
	BinOpNode(Node& left, Operator& _op, Node& right) : left(&left), right(&right), op(&_op) {

	}
	bool hasLeft() { return left != nullptr; }
	bool hasOperator() { return op != nullptr; }
	bool hasRight() { return right != nullptr; }
};
class EtcParser {
	static void parse(marine::Parser & p) {
		std::vector<Node> exprStack;
		std::vector<Operator> opStack;
		while (p.canAdvance()) {
			if (marine::isInt(p.cur()) || marine::isFloat(p.cur())) {
				exprStack.push_back(Node(p.cur()));
			}
			else if (p.cur().value == "(") {
				opStack.push_back(Operator(p.cur()));
			}
			else if (marine::isOp(p.cur())) {
				while (Node::precedence(opStack.back()) != -1 && Node::precedence(opStack.back()) >= Node::precedence(p.cur())) {
					Operator& n = opStack.back();
					opStack.pop_back();

					Node& e1 = exprStack.back();
					exprStack.pop_back();
					Node& e2 = exprStack.back();
					exprStack.pop_back();
					exprStack.push_back(BinOpNode(e1, n, e2));
				}
				opStack.pop_back();
			}
			else throw ("unexpected unexplainable error occured.")
		}
	}
};
