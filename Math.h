#pragma once
#include <iostream>
#include <string>
#include "Base.h"
#include "MError.h"
namespace marine {
	namespace ext {
		class Node;
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
			lexertk::token _t;
		public:
			
			Operator(lexertk::token t, bool th = true): _t(t) {
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
		class Node {
		protected:
			lexertk::token t;
			std::shared_ptr<Node> left = nullptr;
			std::shared_ptr<Node> right = nullptr;
			std::shared_ptr<Operator> oper = nullptr;
		public:
			Base::Decl type;
			Node(lexertk::token to, Base::Decl decl) : t(to), type(decl) {}
			Node(Node left, Operator _op, Node right) : left(new Node(left)), right(new Node(right)), oper(new Operator(_op)) {
				
				auto ltype = getRootNodeType(&left, true);
				auto rtype = getRootNodeType(&right, false);

				std::cout << "found left: " << Base::declStr(ltype) << "\nfound right:" << Base::declStr(rtype) << std::endl;

				if (ltype != rtype) throw errors::MError("performing arithmatic on two types that are not supported is not allowed");

				type = ltype;
			}
			static Base::Decl getRootNodeType(Node* rec, bool l) {
				if (rec->isSingular()) return Base::declLiteralParse(rec->getToken());
				if (l) return getRootNodeType(rec->getLeft(), l);
				else return getRootNodeType(rec->getRight(), l);
			}
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
			lexertk::token& getToken() { return t; }
			Node* getLeft() { return left.get(); }
			Node* getRight() { return right.get(); }
			bool isSingular() {
				return (left == nullptr && right == nullptr && oper == nullptr);
			}
			virtual std::string repr() {
				if (right != nullptr && left != nullptr) {
					if (oper != nullptr) {
						return std::string("left: (" + left->repr() + ") right: (" + right->repr() + ")" + ", operator: '" + oper->str() + "'");
					}
					return std::string("left: (" + left->repr() + ") right: (" + right->repr() + ")");
				}
				return std::string("val: " + t.value);
			}
			std::any calc() {
				if (left->isSingular()) {
					auto op = oper->get().value;
					if (right->isSingular()) {
						switch (left->type) {
						case Base::Decl::INT:
							if (op == "+")
								return stoi(left->getToken().value) + stoi(right->getToken().value);
							else if(op == "-")
								return stoi(left->getToken().value) - stoi(right->getToken().value);
							else if(op == "*")
								return stoi(left->getToken().value) * stoi(right->getToken().value);
							else if(op == "/")
								return stoi(left->getToken().value) / stoi(right->getToken().value);
							else if(op == "%")
								return stoi(left->getToken().value) % stoi(right->getToken().value);
						case Base::Decl::FLOAT:
							if (op == "+")
								return stof(left->getToken().value) + stof(right->getToken().value);
							else if (op == "-")
								return stof(left->getToken().value) - stof(right->getToken().value);
							else if (op == "*")
								return stof(left->getToken().value) * stof(right->getToken().value);
							else if (op == "/")
								return stof(left->getToken().value) / stof(right->getToken().value);
						case Base::Decl::STRING:
							if (op == "+")
								return left->getToken().value + right->getToken().value;
							/*else if (op == "-")
								return left->getToken().value - right->getToken().value;
							else if (op == "*")
								return stof(left->getToken().value) * stof(right->getToken().value);
							else if (op == "/")
								return stof(left->getToken().value) / stof(right->getToken().value);
							else if (op == "%")
								return stof(left->getToken().value) % stof(right->getToken().value);*/
						default:
							throw marine::errors::MError("something unexpected happened.");
						}
					}
					switch (left->type) {
					case Base::Decl::INT:
						if (op == "+")
							return stoi(left->getToken().value) + std::any_cast<int>(right->calc());
						else if (op == "-")
							return stoi(left->getToken().value) - std::any_cast<int>(right->calc());
						else if (op == "*")
							return stoi(left->getToken().value) * std::any_cast<int>(right->calc());
						else if (op == "/")
							return stoi(left->getToken().value) / std::any_cast<int>(right->calc());
						else if (op == "%")
							return stoi(left->getToken().value) % std::any_cast<int>(right->calc());
					case Base::Decl::FLOAT:
						if (op == "+")
							return stof(left->getToken().value) + std::any_cast<float>(right->calc());
						else if (op == "-")
							return stof(left->getToken().value) - std::any_cast<float>(right->calc());
						else if (op == "*")
							return stof(left->getToken().value) * std::any_cast<float>(right->calc());
						else if (op == "/")
							return stof(left->getToken().value) / std::any_cast<float>(right->calc());
					case Base::Decl::STRING:
						return left->getToken().value + std::any_cast<std::string>(right->calc());
					default:
						throw marine::errors::MError("something unexpected happened.");
					}
				}
				//LAST TODO
				switch (left->type) {
				case Base::Decl::INT:
					return std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc());
				case Base::Decl::FLOAT:
					return std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc());
				case Base::Decl::STRING:
					return std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc());
				default:
					throw errors::MError("something unexpected happened.");
				}
				return NULL;
			}
		};
	};
};
