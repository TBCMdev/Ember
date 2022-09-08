#pragma once
#include <iostream>
#include <string>
#include "Base.h"
#include "MError.h"
#include "Types.h"

#define DEBUG(x) if(MARINE__DEBUG) std::cout << "[debug] " << x << std::endl

namespace marine {
	namespace ext {
		class Node;
		class VariableNode;
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
				LHOEQ,
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
				else if (t.value == "<=") type = OPTYPE::LHOEQ;
				else if (t.value == "(") type = OPTYPE::LPR;
				else if (t.value == ")") type = OPTYPE::RPR;
				else {
					type = OPTYPE::UNKNWN;
				}
				x = t.value;
			}
			std::string str() {
				return x;
			}
			auto getType() { return type; }
			bool isValid() { return type != OPTYPE::UNKNWN; }
			lexertk::token& get() { return _t; }
			template<typename Type1, typename Type2>
			bool equateSimple(Type1 _1, Type2 _2) {
				switch (type) {
				case OPTYPE::EQUALS:
					return (_1 == _2);
				case OPTYPE::NOTEQ:
					return (_1 != _2);
				}
				return false;
			}
			template<typename Type1, typename Type2>
			bool equate(Type1 _1, Type2 _2) {
				switch (type) {
				case OPTYPE::EQUALS:
					return (_1 == _2);
				case OPTYPE::NOTEQ:
					return (_1 != _2);
				case OPTYPE::LTHAN:
					return (_1 < _2);
				case OPTYPE::GTHAN:
					return (_1 > _2);
				case OPTYPE::LHOEQ:
					return (_1 <= _2);
				case OPTYPE::GTOEQ:
					return (_1 >= _2);
				}
				return false;
			}
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
				//std::cout << "\ncreated singular node with val of: " + to.value + ", with type of:" << Base::declStr(type) << std::endl;
			}
			Node(std::shared_ptr<Node> _left, Operator _op, std::shared_ptr<Node> _right, bool __negate = false) : left(_left), right(_right), oper(new Operator(_op)), negated(__negate) {
				//std::cout << "getting root node type of:" << _left.repr() << std::endl;
				auto ltype = getRootNodeType(_left.get(), true);
				auto rtype = getRootNodeType(_right.get(), false);

				//std::cout << Base::declStr(ltype) << ", " << Base::declStr(rtype) << std::endl;
				if (ltype != rtype) throw errors::MError("performing arithmatic on two types that are not supported is not allowed");

				type = ltype;

				//std::cout << "VARIABLE final type: " << Base::declStr(type) << std::endl;
			}
			virtual ~Node() = default;
			static Base::Decl getRootNodeType(Node* rec, bool l) {
				if (rec->isSingular() || rec->isVariable()) return rec->type;
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
			virtual bool isVariable() { return false; }
			virtual bool isSingular() {
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
			virtual std::any calc() {
				//std::cout << "calculating node:" << repr() << std::endl;
				if (isSingular()) {
					switch (type) {
					case Base::Decl::INT:
						return negated ? -stoi(getToken().value) : stoi(getToken().value);
					case Base::Decl::FLOAT:
						return negated ? -stof(getToken().value) : stof(getToken().value);
					case Base::Decl::STRING:
						return String(getToken().value);
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
										return (negated ? -(-stoi(left->getToken().value) - stoi(right->getToken().value)) : -stoi(left->getToken().value) - stoi(right->getToken().value));
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
								return String(left->getToken().value) + String(right->getToken().value);
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
							return String(left->getToken().value) + std::any_cast<String>(right->calc());
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
							return String(left->getToken().value) + String(right->getToken().value);
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
								return (negated ? -(-std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) - std::any_cast<int>(right->calc()));
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
								return (negated ? -(-std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc())) : -std::any_cast<int>(left->calc()) * std::any_cast<int>(right->calc()));
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
					return String(std::any_cast<String>(left->calc()) + std::any_cast<String>(right->calc()));
				default:
					throw errors::MError("something unexpected happened.");
				}
				return NULL;
			};
		};
		class VariableNode : public Node {
		protected:
			std::shared_ptr<Variable> internal_value = nullptr;

		public:
			VariableNode(std::shared_ptr<Variable> v, bool __negate = false) : internal_value(v), Node(lexertk::token(v->str()), v->getDecl(), __negate) {
				DEBUG("CREATING VARIABLE NODE");
				DEBUG(v->str());
			}
			bool isVariable() override { return true; }
			virtual bool isSingular() override {
				return false;
				//returning false on this method tells the node calc method that it needs to call the overrided calc method to extract the value
			}
			virtual std::any calc() override{
				DEBUG("CALCING VARIABLE NODE");
				if (internal_value == nullptr) return nullptr;
				switch (internal_value->getDecl()) {
				case Base::Decl::INT:
					return internal_value->cast<int>();
				case Base::Decl::FLOAT:
					return internal_value->cast<float>();
				case Base::Decl::STRING:
					return internal_value->cast<String>();
				case Base::Decl::BOOL:
					return internal_value->cast<bool>();
				case Base::Decl::LIST:
					return internal_value->cast<ArrayList>();
				default:
					throw marine::errors::RuntimeError("this type is currently not supported.");
				}
			}
		};
		class VCNode: public Node {
		protected:
			std::shared_ptr<VContainer> internal_value = nullptr;
		public:
			//(new VContainer) needed to keep lifetime of shared ptr, if you use the & of v, v will be destroyed.
			VCNode(VContainer& v, bool __negate = false) : internal_value(new VContainer(v)), Node(lexertk::token(v.getStringified()), v.type(), __negate) {
				DEBUG("CREATING VC NODE");
			}
			bool isVariable() override { return true; }
			virtual bool isSingular() override {
				return false;
				//returning false on this method tells thet node calc method that it needs to call the overrided calc method to extract the value
			}
			virtual std::string repr() {
				return std::string("(VCNode) val:" + internal_value->getStringified());
			}
			VContainer* getValue() { return internal_value.get(); }
			virtual std::any calc() override {
				DEBUG("CALCING VC NODE");
				DEBUG(internal_value->getStringified());
				if (internal_value == nullptr) return nullptr;
				switch (internal_value->type()) {
				case Base::Decl::INT:
					return internal_value->cast<int>();
				case Base::Decl::FLOAT:
					return internal_value->cast<float>();
				case Base::Decl::STRING:
					DEBUG("CALCING STRING");
					DEBUG(internal_value->get().type().name());
					return internal_value->cast<String>();
				case Base::Decl::BOOL:
					return internal_value->cast<bool>();
				case Base::Decl::LIST:
					return internal_value->cast<ArrayList>();
				default:
					throw marine::errors::RuntimeError("this type is currently not supported.");
				}
			}
		};
		
		class BoolExpr {
		protected:
			std::shared_ptr<Node> left = nullptr;
			std::shared_ptr<Operator> oper = nullptr;
			std::shared_ptr<Node> right = nullptr;
			bool negated = false;
		public:
			Base::Decl type;
			BoolExpr(std::shared_ptr<Node> left, bool __negate = false) : left(left), negated(__negate) {
				type = Node::getRootNodeType(left.get(), true);
			}
			BoolExpr(std::shared_ptr<Node>& left, Operator _op, std::shared_ptr<Node>& right, bool __negate = false) :left(left), right(right), oper(new Operator(_op)), negated(__negate) {

				auto ltype = Node::getRootNodeType(left.get(), true);
				auto rtype = Node::getRootNodeType(right.get(), false);
				if (ltype != rtype) throw errors::MError("performing arithmatic on two types that are not supported is not allowed");
				type = ltype;

				//std::cout << "VARIABLE final type: " << Base::declStr(type) << std::endl;
			}
			void negate() {
				negated = true;
			}
			bool isNegated() { return negated; }
			Node* getLeft() { return left.get(); }
			Node* getRight() { return right.get(); }
			virtual std::string repr() {
				if (right != nullptr && left != nullptr) {
					if (oper != nullptr) {
						return std::string("(BoolExpr) left: (" + left->repr() + ") right: (" + right->repr() + ")" + ", operator: '" + oper->str() + "', negative: " + (negated ? "Y" : "N"));
					}
					return std::string("(BoolExpr) left: (" + left->repr() + ") right: (" + right->repr() + "), negative: " + (negated ? "Y" : "N"));
				}
			}
			bool evaluate() {
				DEBUG(left->isVariable());
				Base::Decl rootLeft = Node::getRootNodeType(left.get(), true);
				Base::Decl rootRight = Node::getRootNodeType(right.get(), false);

				if (right != nullptr) {
					switch (rootLeft) {
					case Base::Decl::INT:
					{
						int l = std::any_cast<int>(left->calc());
						switch (rootRight) {
						case Base::Decl::INT:
							return oper->equate<int, int>(l, std::any_cast<int>(right->calc()));
						case Base::Decl::FLOAT:
							return oper->equate<int, float>(l, std::any_cast<float>(right->calc()));
						case Base::Decl::STRING:
							break;
							//return oper->equate<int, String>(l, s);
						}
					}
					case Base::Decl::FLOAT:
					{
						float l = std::any_cast<float>(left->calc());
						switch (rootRight) {
						case Base::Decl::INT:
							return oper->equate<float, int>(l, std::any_cast<int>(right->calc()));
						case Base::Decl::FLOAT:
							return oper->equate<float, float>(l, std::any_cast<float>(right->calc()));
						case Base::Decl::STRING:
							break;
							//return oper->equate<int, String>(l, s);
						}

					}
					case Base::Decl::STRING:
					{
						String l = std::any_cast<String>(left->calc());
						DEBUG("string after call:" + l.get());
						DEBUG("left still var?");
						DEBUG(left->isVariable());
						switch (rootRight) {
						case Base::Decl::INT:
							break;
						case Base::Decl::FLOAT:
							break;
						case Base::Decl::STRING:
							return oper->equateSimple<String, String>(l, std::any_cast<String>(right->calc()));
						}

					}
					}

				}
				else {
					//evaluate single expr;
					return false;
				}
			}
		};
	};
};
