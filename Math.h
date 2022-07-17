#pragma once
#include <iostream>
#include <string>
#include "Base.h"
#include "MError.h"
#include "Types.h"

#define DEBUG(x) std::cout << "[debug] " << x << std::endl

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

			Operator(lexertk::token t, bool th = true) : _t(t) {
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
			bool negated = false;
		public:
			Base::Decl type;
			Node(lexertk::token to, Base::Decl decl, bool __negate = false) : t(to), type(decl), negated(__negate) {
				std::cout << "\ncreated singular node with val of: " + to.value + ", with type of:" << Base::declStr(type) << std::endl;
			}
			Node(Node left, Operator _op, Node right, bool __negate = false) : left(new Node(left)), right(new Node(right)), oper(new Operator(_op)), negated(__negate) {

				auto ltype = getRootNodeType(&left, true);
				auto rtype = getRootNodeType(&right, false);


				if (ltype != rtype) throw errors::MError("performing arithmatic on two types that are not supported is not allowed");

				type = ltype;

				std::cout << "VARIABLE final type: " << Base::declStr(type) << std::endl;
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
			void negate() {
				negated = true;
			}
			bool isNegated() { return negated; }
			lexertk::token& getToken() { return t; }
			Node* getLeft() { return left.get(); }
			Node* getRight() { return right.get(); }
			bool isSingular() {
				return (left == nullptr && right == nullptr && oper == nullptr);
			}
			virtual std::string repr() {
				if (right != nullptr && left != nullptr) {
					if (oper != nullptr) {
						return std::string("left: (" + left->repr() + ") right: (" + right->repr() + ")" + ", operator: '" + oper->str() + "', negative: " + (negated ? "Y" : "N"));
					}
					return std::string("left: (" + left->repr() + ") right: (" + right->repr() + "), negative: " + (negated ? "Y" : "N"));
				}
				return std::string("val: " + (negated ? "-" + t.value : t.value) + ", type: '" + Base::declStr(type) + "'");
			}
			std::any calc() {
				if (isSingular()) {
					DEBUG("singular:" + repr());
					switch (type) {
					case Base::Decl::INT:
						return negated ? -stoi(getToken().value) : stoi(getToken().value);
					case Base::Decl::FLOAT:
						return negated ? -stof(getToken().value) : stof(getToken().value);
					case Base::Decl::STRING:
						return marine::String(getToken().value);
					case Base::Decl::BOOL:
						return (getToken().value == "true" ? true : getToken().value == "false" ? false : throw marine::errors::SyntaxError("could not convert variable to type of 'bool'"));
					default:
						throw marine::errors::MError("something unexpected happened.");
					}
				}
				auto op = oper->get().value;
				if (left->isSingular()) {
					DEBUG("left is singular:" + left->repr());
					if (right->isSingular()) {
						switch (left->type) {
						case Base::Decl::INT:
							if (op == "+")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) + stoi(right->getToken().value)) : -stoi(left->getToken().value) + stoi(right->getToken().value));
									else
										return (negated ? -(-stoi(left->getToken().value) - stoi(right->getToken().value)): -stoi(left->getToken().value) - stoi(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) - stoi(right->getToken().value)) : stoi(left->getToken().value) - stoi(right->getToken().value));
								else return (negated ? -(stoi(left->getToken().value) + stoi(right->getToken().value)) : stoi(left->getToken().value) + stoi(right->getToken().value));

							else if (op == "-")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) - stoi(right->getToken().value)) : -stoi(left->getToken().value) - stoi(right->getToken().value));
									else
										return (negated ? -(stoi(left->getToken().value) + stoi(right->getToken().value)) : stoi(left->getToken().value) + stoi(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) + stoi(right->getToken().value)) : stoi(left->getToken().value) + stoi(right->getToken().value));
								else return (negated ? -(stoi(left->getToken().value) - stoi(right->getToken().value)) : stoi(left->getToken().value) - stoi(right->getToken().value));
							else if (op == "*") {
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) * stoi(right->getToken().value)) : -stoi(left->getToken().value) * stoi(right->getToken().value));
									else
										return (negated ? -(-stoi(left->getToken().value) * -stoi(right->getToken().value)) : -stoi(left->getToken().value) * -stoi(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) * -stoi(right->getToken().value)) : stoi(left->getToken().value) * -stoi(right->getToken().value));
								else return (negated ? -(stoi(left->getToken().value) * stoi(right->getToken().value)) : stoi(left->getToken().value) * stoi(right->getToken().value));
							}
							else if (op == "/")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) / stoi(right->getToken().value)) : -stoi(left->getToken().value) / stoi(right->getToken().value));
									else
										return (negated ? -(-stoi(left->getToken().value) / -stoi(right->getToken().value)) : -stoi(left->getToken().value) / -stoi(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) / -stoi(right->getToken().value)) : stoi(left->getToken().value) / -stoi(right->getToken().value));
								else return (negated ? -(stoi(left->getToken().value) / stoi(right->getToken().value)) : stoi(left->getToken().value) / stoi(right->getToken().value));
							else if (op == "%")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) % stoi(right->getToken().value)) : -stoi(left->getToken().value) % stoi(right->getToken().value));
									else
										return (negated ? -(-stoi(left->getToken().value) % -stoi(right->getToken().value)) : -stoi(left->getToken().value) % -stoi(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) % -stoi(right->getToken().value)) : stoi(left->getToken().value) % -stoi(right->getToken().value));
								else return (negated ? -(stoi(left->getToken().value) % stoi(right->getToken().value)) : stoi(left->getToken().value) % stoi(right->getToken().value));
						case Base::Decl::FLOAT:
							if (op == "+")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) + stof(right->getToken().value)) : -stof(left->getToken().value) + stof(right->getToken().value));
									else
										return (negated ? -(-stof(left->getToken().value) + -stof(right->getToken().value)) : -stof(left->getToken().value) + -stof(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) - stof(right->getToken().value)) : stof(left->getToken().value) - stof(right->getToken().value));
								else return (negated ? -(stof(left->getToken().value) + stof(right->getToken().value)) : stof(left->getToken().value) + stof(right->getToken().value));
							else if (op == "-")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) - stof(right->getToken().value)) : -stof(left->getToken().value) - stof(right->getToken().value));
									else
										return (negated ? -(stof(left->getToken().value) + stof(right->getToken().value)) : stof(left->getToken().value) + stof(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) - -stof(right->getToken().value)) : stof(left->getToken().value) - -stof(right->getToken().value));
								else return (negated ? -(stof(left->getToken().value) - stof(right->getToken().value)) : stof(left->getToken().value) - stof(right->getToken().value));
							else if (op == "*") {
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) * stof(right->getToken().value)) : -stof(left->getToken().value) * stof(right->getToken().value));
									else
										return (negated ? -(-stof(left->getToken().value) * -stof(right->getToken().value)) : -stof(left->getToken().value) * -stof(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) * -stof(right->getToken().value)) : stof(left->getToken().value) * -stof(right->getToken().value));
								else return (negated ? -(stof(left->getToken().value) * stof(right->getToken().value)) : stof(left->getToken().value) * stof(right->getToken().value));
							}
							else if (op == "/")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) / stof(right->getToken().value)) : -stof(left->getToken().value) / stof(right->getToken().value));
									else
										return (negated ? -(-stof(left->getToken().value) / -stof(right->getToken().value)) : -stof(left->getToken().value) / -stof(right->getToken().value));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) / -stof(right->getToken().value)) : stof(left->getToken().value) / -stof(right->getToken().value));
								else return (negated ? -(stof(left->getToken().value) / stof(right->getToken().value)) : stof(left->getToken().value) / stof(right->getToken().value));
						case Base::Decl::STRING:
							if (op == "+")
								return marine::String(left->getToken().value + right->getToken().value);
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
					else {
						switch (left->type) {
						case Base::Decl::INT:
							if (op == "+")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) + std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) + std::any_cast<int>(right->calc()));
									else
										return (negated ? -(-stoi(left->getToken().value) - std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) - std::any_cast<int>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) + std::any_cast<int>(right->calc())) : stoi(left->getToken().value) + std::any_cast<int>(right->calc()));
								else return (negated ? -(stoi(left->getToken().value) + std::any_cast<int>(right->calc())) : stoi(left->getToken().value) + std::any_cast<int>(right->calc()));
							else if (op == "-")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) - std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) - std::any_cast<int>(right->calc()));
									else
										return (negated ? -(stoi(left->getToken().value) + std::any_cast<int>(right->calc())) : stoi(left->getToken().value) + std::any_cast<int>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) + std::any_cast<int>(right->calc())) : stoi(left->getToken().value) + std::any_cast<int>(right->calc()));
								else return stoi(left->getToken().value) - std::any_cast<int>(right->calc());
							else if (op == "*")
								if (left->isNegated())
									if (right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) * -std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) * -std::any_cast<int>(right->calc()));
									else
										return (negated ? -(-stoi(left->getToken().value) * std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) * std::any_cast<int>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) * -std::any_cast<int>(right->calc())) : stoi(left->getToken().value) * -std::any_cast<int>(right->calc()));
								else return stoi(left->getToken().value) * std::any_cast<int>(right->calc());
							else if (op == "/")
								if (left->isNegated())
									if (right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) / -std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) / -std::any_cast<int>(right->calc()));
									else
										return (negated ? -(-stoi(left->getToken().value) / std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) / std::any_cast<int>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) / -std::any_cast<int>(right->calc())) : stoi(left->getToken().value) / -std::any_cast<int>(right->calc()));
								else return (negated ? -(stoi(left->getToken().value) / std::any_cast<int>(right->calc())) : stoi(left->getToken().value) / std::any_cast<int>(right->calc()));
							else if (op == "%")
								if (left->isNegated())
									if (right->isNegated())
										return (negated ? -(-stoi(left->getToken().value) % -std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) % -std::any_cast<int>(right->calc()));
									else
										return (negated ? -(-stoi(left->getToken().value) % std::any_cast<int>(right->calc())) : -stoi(left->getToken().value) % std::any_cast<int>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stoi(left->getToken().value) % -std::any_cast<int>(right->calc())) : stoi(left->getToken().value) % -std::any_cast<int>(right->calc()));
								else return (negated ? -(stoi(left->getToken().value) % std::any_cast<int>(right->calc())) : stoi(left->getToken().value) % std::any_cast<int>(right->calc()));
						case Base::Decl::FLOAT:
							if (op == "+")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) + std::any_cast<float>(right->calc())) : -stof(left->getToken().value) + std::any_cast<float>(right->calc()));
									else
										return (negated ? -(-stof(left->getToken().value) - std::any_cast<float>(right->calc())) : -stof(left->getToken().value) - std::any_cast<float>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) + std::any_cast<float>(right->calc())) : stof(left->getToken().value) + std::any_cast<float>(right->calc()));
								else return (negated ? -(stof(left->getToken().value) + std::any_cast<float>(right->calc())) : stof(left->getToken().value) + std::any_cast<float>(right->calc()));
							else if (op == "-")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) - std::any_cast<float>(right->calc())) : -stof(left->getToken().value) - std::any_cast<float>(right->calc()));
									else
										return (negated ? -(-stof(left->getToken().value) + std::any_cast<float>(right->calc())) : -stof(left->getToken().value) + std::any_cast<float>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) + std::any_cast<float>(right->calc())) : stof(left->getToken().value) + std::any_cast<float>(right->calc()));
								else return (negated ? -(stof(left->getToken().value) - std::any_cast<float>(right->calc())) : stof(left->getToken().value) - std::any_cast<float>(right->calc()));
							else if (op == "*")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) * std::any_cast<float>(right->calc())) : -stof(left->getToken().value) * std::any_cast<float>(right->calc()));
									else
										return (negated ? -(stof(left->getToken().value) * std::any_cast<float>(right->calc())) : stof(left->getToken().value) * std::any_cast<float>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) * -std::any_cast<float>(right->calc())) : stof(left->getToken().value) * -std::any_cast<float>(right->calc()));
								else return (negated ? -(stof(left->getToken().value) * std::any_cast<float>(right->calc())) : stof(left->getToken().value) * std::any_cast<float>(right->calc()));
							else if (op == "/")
								if (left->isNegated())
									if (!right->isNegated())
										return (negated ? -(-stof(left->getToken().value) / std::any_cast<float>(right->calc())) : -stof(left->getToken().value) / std::any_cast<float>(right->calc()));
									else
										return (negated ? -(-stof(left->getToken().value) / std::any_cast<float>(right->calc())) : -stof(left->getToken().value) / std::any_cast<float>(right->calc()));
								else if (right->isNegated())
									return (negated ? -(stof(left->getToken().value) / -std::any_cast<float>(right->calc())) : stof(left->getToken().value) / -std::any_cast<float>(right->calc()));
								else return (negated ? -(stof(left->getToken().value) / std::any_cast<float>(right->calc())) : stof(left->getToken().value) / std::any_cast<float>(right->calc()));
						case Base::Decl::STRING:
							return marine::String(left->getToken().value + std::any_cast<String>(right->calc()));
						default:
							throw marine::errors::MError("something unexpected happened.");
						}
					}
				}
				else if (right->isSingular()) {
				switch (left->type) {
				case Base::Decl::INT:
					DEBUG("is int");
					if (op == "+")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) + stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) + stoi(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) - stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) - stoi(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) - stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) - stoi(right->getToken().value));
						else return (negated ? -(std::any_cast<int>(left->calc()) + stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) + stoi(right->getToken().value));

					else if (op == "-")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) - stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) - stoi(right->getToken().value));
							else
								return (negated ? -(std::any_cast<int>(left->calc()) + stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) + stoi(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) + stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) + stoi(right->getToken().value));
						else return (negated ? -(std::any_cast<int>(left->calc()) - stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) - stoi(right->getToken().value));
					else if (op == "*") {
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) * stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) * stoi(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) * -stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) * -stoi(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) * -stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) * -stoi(right->getToken().value));
						else return (negated ? -(std::any_cast<int>(left->calc()) * stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) * stoi(right->getToken().value));
					}
					else if (op == "/")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) / stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) / stoi(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) / -stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) / -stoi(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) / -stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) / -stoi(right->getToken().value));
						else return (negated ? -(std::any_cast<int>(left->calc()) / stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) / stoi(right->getToken().value));
					else if (op == "%")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) % stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) % stoi(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) % -stoi(right->getToken().value)) : -std::any_cast<int>(left->calc()) % -stoi(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) % -stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) % -stoi(right->getToken().value));
						else return (negated ? -(std::any_cast<int>(left->calc()) % stoi(right->getToken().value)) : std::any_cast<int>(left->calc()) % stoi(right->getToken().value));
				case Base::Decl::FLOAT:
					if (op == "+")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) + stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) + stof(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<float>(left->calc()) + -stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) + -stof(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) - stof(right->getToken().value)) : std::any_cast<float>(left->calc()) - stof(right->getToken().value));
						else return (negated ? -(std::any_cast<float>(left->calc()) + stof(right->getToken().value)) : std::any_cast<float>(left->calc()) + stof(right->getToken().value));
					else if (op == "-")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) - stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) - stof(right->getToken().value));
							else
								return (negated ? -(std::any_cast<float>(left->calc()) + stof(right->getToken().value)) : std::any_cast<float>(left->calc()) + stof(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) - -stof(right->getToken().value)) : std::any_cast<float>(left->calc()) - -stof(right->getToken().value));
						else return (negated ? -(std::any_cast<float>(left->calc()) - stof(right->getToken().value)) : std::any_cast<float>(left->calc()) - stof(right->getToken().value));
					else if (op == "*") {
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) * stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) * stof(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<float>(left->calc()) * -stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) * -stof(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) * -stof(right->getToken().value)) : std::any_cast<float>(left->calc()) * -stof(right->getToken().value));
						else return (negated ? -(std::any_cast<float>(left->calc()) * stof(right->getToken().value)) : std::any_cast<float>(left->calc()) * stof(right->getToken().value));
					}
					else if (op == "/")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) / stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) / stof(right->getToken().value));
							else
								return (negated ? -(-std::any_cast<float>(left->calc()) / -stof(right->getToken().value)) : -std::any_cast<float>(left->calc()) / -stof(right->getToken().value));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) / -stof(right->getToken().value)) : std::any_cast<float>(left->calc()) / -stof(right->getToken().value));
						else return (negated ? -(std::any_cast<float>(left->calc()) / stof(right->getToken().value)) : std::any_cast<float>(left->calc()) / stof(right->getToken().value));
				case Base::Decl::STRING:
					if (op == "+")
						return left->getToken().value + right->getToken().value;
				default:
					throw marine::errors::MError("something unexpected happened.");
				}
				}
				//LAST TODO
				switch (left->type) {
				case Base::Decl::INT:
					if (op == "+")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc()));
							else
								return ( negated ? -(-std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc()));
						else return (negated ? -(std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc()));
					else if (op == "-")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc()));
							else
								return (negated ? -(std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) + std::any_cast<int>(right->calc()));
						else return (negated ? -(std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc()));
					else if (op == "*")
						if (left->isNegated())
							if (right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) * -std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) * -std::any_cast<int>(right->calc()));
							else
								return (negated ?  -(-std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) * -std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) * -std::any_cast<int>(right->calc()));
						else return (negated ? -(std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc()));
					else if (op == "/")
						if (left->isNegated())
							if (right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) / -std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) / -std::any_cast<int>(right->calc()));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) / std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) / std::any_cast<int>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) / -std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) / -std::any_cast<int>(right->calc()));
						else return (negated ? -(std::any_cast<int>(left->calc()) / std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) / std::any_cast<int>(right->calc()));
					else if (op == "%")
						if (left->isNegated())
							if (right->isNegated())
								return (negated ? -(-std::any_cast<int>(left->calc()) % -std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) % -std::any_cast<int>(right->calc()));
							else
								return (negated ? -(-std::any_cast<int>(left->calc()) % std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) % std::any_cast<int>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<int>(left->calc()) % -std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) % -std::any_cast<int>(right->calc()));
						else return (negated ? -(std::any_cast<int>(left->calc()) % std::any_cast<int>(right->calc())) : std::any_cast<int>(left->calc()) % std::any_cast<int>(right->calc()));
				case Base::Decl::FLOAT:
					if (op == "+")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
							else
								return (negated ? -(-std::any_cast<float>(left->calc()) - std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
						else return (negated ? -(std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
					else if (op == "-")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) - std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) - std::any_cast<float>(right->calc()));
							else
								return (negated ? -(-std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) + std::any_cast<float>(right->calc()));
						else return (negated ? -(std::any_cast<float>(left->calc()) - std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) - std::any_cast<float>(right->calc()));
					else if (op == "*")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc()));
							else
								return (negated ? -(std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) * -std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) * -std::any_cast<float>(right->calc()));
						else return (negated ? -(std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) * std::any_cast<float>(right->calc()));
					else if (op == "/")
						if (left->isNegated())
							if (!right->isNegated())
								return (negated ? -(-std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc())) : -std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc()));
							else
								return (negated ? -(std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc()));
						else if (right->isNegated())
							return (negated ? -(std::any_cast<float>(left->calc()) / -std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) / -std::any_cast<float>(right->calc()));
						else return (negated ? -(std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc())) : std::any_cast<float>(left->calc()) / std::any_cast<float>(right->calc()));
				case Base::Decl::STRING:
					return marine::String(std::any_cast<String>(left->calc()) + std::any_cast<String>(right->calc()));
				default:
					throw errors::MError("something unexpected happened.");
				}
				return NULL;
			}
		};
	};
};
