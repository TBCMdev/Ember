#pragma once
#include <iostream>
#include <string>
namespace marine {
	namespace ext {
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


				//-BRACKETS-
				LPR,
				RPR,
				//- UNKNOWN? -
				UNKNWN
			};
		protected:
			OPTYPE type;
			std::string x = "";
			lexertk::token& _t;
		public:
			Operator(lexertk::token& t, bool th = true): _t(t) {
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
				else if (t.value == ")") type = OPTYPE::RPR;
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
			lexertk::token& get() { return _t; }
		};
		class Node;
		class Node {
		protected:
			lexertk::token t;
			Node* left = nullptr;
			Node* right = nullptr;
			Operator* oper = nullptr;
		public:
			Node(lexertk::token to) : t(to) {}
			Node(Node left, Operator _op, Node right) : left(&left), right(&right), oper(&_op) {

			}
			lexertk::token& getToken() { return t; }
			static int precedence(Node& o) {
				if (o.getToken().value == "(" ||
					o.getToken().value == ")") return 1;
				if (o.getToken().value == "*" || o.getToken().value == "/" || o.getToken().value == "%") return 2;
				if (o.getToken().value == "+" || o.getToken().value == "-")  return 3;
				return -1;
			}
			static int precedence(Operator& o) {
				if (o.get().value == "(" ||
					o.get().value == ")") return 1;
				if (o.get().value == "*" || o.get().value == "/" || o.get().value == "%") return 2;
				if (o.get().value == "+" || o.get().value == "-")  return 3;
				return -1;
			}
			static int precedence(lexertk::token& o) {
				if (o.value == "(" ||
					o.value == ")") return 1;
				if (o.value == "*" || o.value == "/" || o.value == "%") return 2;
				if (o.value == "+" || o.value == "-")  return 3;
				return -1;
			}
			std::string repr() {
				if (right != nullptr && left != nullptr) {
					if (oper != nullptr) {
						return std::string("left: " + left->getToken().value + "right:" + right->getToken().value + ", operator: " + oper->str());
					}
					return std::string("left: " + left->getToken().value + "right:" + right->getToken().value);
				}
				return std::string("val: " + t.value);
			}
		};
		
	};
};
