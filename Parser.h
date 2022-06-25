#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <any>
#include <functional>
#include "Math.h"
#include "MError.h"
#include "Base.h"
#include "helpers.h"

#define DEBUG(x) std::cout << "[debug] " << x << std::endl


using namespace marine;
using namespace marine::ext;
namespace marine {
	template <typename T>
	std::string anyToStr(T& t) {
		try {
			std::stringstream s;

			s << t;

			return s.str();
		}
		catch(...) {
			return "";
		}
	}
	class Variable {
	protected:
		std::any& value;
		std::string name;
		lexertk::token orig;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;
		bool __hasToken = true;
		std::string configsStr() {
			std::stringstream stream;
			stream << "[";

			for (int i = 0; i < configs.size(); i++) {
				stream << Base::declCStr(configs[i]);
				if (i + 1 < configs.size()) stream << ", ";
			}
			return stream.str() + "]";
		}
	public:
		Variable(std::string& _name, std::any val, lexertk::token& _orig, std::vector <Base::DeclConfig> _configs) : orig(_orig), value(val), name(_name), configs(_configs)
		{
		}
		Variable(std::string& _name, std::any val, std::vector <Base::DeclConfig> _configs) : __hasToken(false), value(val), name(_name), configs(_configs)
		{
		}
		Variable(std::string& _name, std::any val,std::string _origStr, std::vector <Base::DeclConfig> _configs) : orig(_origStr), __hasToken(false), value(val), name(_name), configs(_configs)
		{
		}
		void setDecl(Base::Decl d) { decl = d; }
		std::any& getValue() { return value; }
		std::string& getName() { return name; }
		template <typename T>
		T cast() {
			return std::any_cast<T>(value);
		}
		bool is(Base::DeclConfig d) {
			return std::count(configs.begin(), configs.end(), d) != 0;
		}
		std::any& setValue(std::any x) { value = x; return x; }
		lexertk::token& getToken() { return orig; }
		std::string str() {
			return std::string("[var] name: " + name + ", val=" + orig.value + ", decl_type: " + Base::declStr(decl) + ", configurations: " + configsStr());
		}
	};
	class Parser {
	protected:
		lexertk::generator& gen;
		lexertk::token current;
		std::vector<Variable> core_variables;
		int index = -1;
	public:
		Parser(lexertk::generator& generator) : gen(generator) {}
#pragma region decl

		bool isDecl() {
			return Base::declareParse(cur()) != Base::Decl::UNKNWN;
		}
		bool isFuncCall() {
			//just do print for now
			if (cur().value == "print") return true;

			if (Base::is(getNext(), "(")) {
				int br = 1;
				for (int i = index; i < gen.size(); i++) {
					lexertk::token& t = gen[i];
					if (Base::is(t, "(")) br++;
					if (Base::is(t, ")")) br--;
					if (br == 0) break;
				}
				if (br != 0) return false;//throw errors::SyntaxError("expected '(' after function call.");
				return true;
			}
			return false;
		}
		template<typename Type>
		Type parseExt(Base::Decl decl = Base::Decl::UNKNWN) {
			advance();
			std::function<Node()> brEval = [&]() -> Node {
				Node* left = nullptr, * right = nullptr;
				Operator* op = nullptr;
				while (canAdvance()) {
					//just encountered bracket
					advance();


					if (Base::is(cur(), "(")) {
						return brEval();
					}
					else if (isFloat(cur())) {
						if (left == nullptr) left = new Node(cur(), Base::Decl::FLOAT);
						else if (right == nullptr) right = new Node(cur(), Base::Decl::FLOAT);
					}
					else if (isInt(cur())) {
						if (left == nullptr) left = new Node(cur(), Base::Decl::FLOAT);
						else if (right == nullptr) right = new Node(cur(), Base::Decl::FLOAT);
					}
					else if (isOp(cur())) {

						if (op == nullptr) op = new Operator(cur());
					}
					if ((left != nullptr && right != nullptr && op != nullptr) || Base::is(cur(), ")")) {
						advance();
						//std::cout << "exiting bracket: " << left->repr() << op->str() << right->repr() << std::endl;
						break;
					}
				}
				return Node(*left, *op, *right);
			};
			std::vector<Node> operationStack;
			Type finalVal{};
			std::vector<Operator> operatorStack;
			bool br = false;
			while (canAdvance()) {
				DEBUG("checking: " + cur().value);
				if (isInt(cur())) {
					operationStack.push_back(Node(cur(), Base::Decl::INT));
					DEBUG("creating node:" + operationStack.back().repr());
					DEBUG("checking next:" + getNext().value);
					if (!isOp(getNext())) {
						std::cout << ("is not op:" + getNext().value);
						br = true;
					}
					std::cout << ("is op:" + getNext().value);

				}
				else if (isFloat(cur())) {
					DEBUG("checking next:" + getNext().value);
					operationStack.push_back(Node(cur(), Base::Decl::FLOAT));
					if (!isOp(getNext())) {
						std::cout << ("is not op:" + getNext().value);
						br = true;
					}
					std::cout << ("is op:" + getNext().value);

				}
				else if (Base::is(cur(), "(")) { operationStack.push_back(brEval()); }
				else if (isOp(cur())) {
					DEBUG("pushing op:" + cur().value);
					operatorStack.push_back(Operator(cur()));
				}
				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					Node right = operationStack.back();
					operationStack.pop_back();
					Node left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();
					Node n(left, op, right);
					//DEBUG("creating node:" + n.repr());
					std::cout << n.repr() << std::endl;
					operationStack.push_back(Node(left, op, right));
				}
				for (auto& x : operatorStack) DEBUG("opers on stack:" + x.str());
				for (auto& x : operationStack) DEBUG("nodes on stack:" + x.repr());
				if (br) { DEBUG("BREAKING FROM LOOP"); break; }
				advance();
				DEBUG(canAdvance());
			}
			//should only have one node
			std::cout << "END SIZE:" << operationStack.size() << std::endl;
			Node& operation = operationStack.back();
			for (auto& a : operationStack) {
				std::cout << "NODE: " << a.repr() << std::endl;
			}
			finalVal = std::any_cast<Type>(operation.calc());
			std::cout << "found:" << finalVal;
			return finalVal;
		}
		void parseDecl() {
			Base::Decl type = Base::declareParse(cur());
			std::cout << "variable type:" << Base::declStr(type) << std::endl;
			lexertk::token& start = cur();
			std::vector<Base::DeclConfig> conf{};
			if (Base::is(advance(), ":")) {
				conf.push_back(Base::declCParse(advance()));
				while (Base::is(getNext(), ",")) {
					conf.push_back(Base::declCParse(advance(2)));
					if (!Base::is(getNext(), ",")) break;
				}
				if (conf.size() == 0) throw errors::SyntaxError("no config state was supplied after ':'");
			}
			lexertk::token& decl_name = advance();
			if (Base::is(advance(), "=")) {
				//IT IS VARIABLE
				//just handle numbers for now
				Variable* ret = nullptr;
				if (type == Base::Decl::INT) {
					int val = parseExt<int>();
					ret = new Variable(decl_name.value, val, anyToStr<int>(val), conf);
					ret->setDecl(Base::Decl::INT);
				}
				else if (type == Base::Decl::FLOAT) {
					float val = parseExt<float>(Base::Decl::FLOAT);
					ret = new Variable(decl_name.value, val, anyToStr<float>(val), conf);
					ret->setDecl(Base::Decl::FLOAT);
				}
				else if (type == Base::Decl::STRING) {
					std::string val = parseExt<std::string>();
					ret = new Variable(decl_name.value, val, val, conf);
					ret->setDecl(Base::Decl::STRING);
				}
					//case Base::Decl::CUSTOM:
						//parseExt<std::any>();
				if (ret == nullptr) return;
				core_variables.push_back(*ret);
				return;
			}
			else if (Base::is(cur(), "(")) {
				//IT IS FUNCTION
			}

		}
		void parseFuncCall() {
			//MOCK CODE
			if (cur().value == "print") {
				if (advance().value == "(") {
					Variable* v = nullptr;
					advance();
					for (int i = 0; i < core_variables.size(); i++) {
						if (core_variables[i].getName() == cur().value) {
							v = &core_variables[i];
							break;
						}
					}
					advance();
					if (v == nullptr) return;
					std::cout << v->getToken().value << std::endl;
				}
			}
		}
		bool isVariableUsage() {
			return false;
		}
		void parseVariableUsage() {
		}
#pragma endregion
		void parse() {
			advance();
			while (canAdvance()) {
				if (isDecl()) {
					std::cout << "isDecl:" << cur().value << std::endl;
					parseDecl();
				}
				else if (isFuncCall()) {
					parseFuncCall();
				}
				else if (isVariableUsage()) {
					parseVariableUsage();
				}

				advance();
			}
			for (auto& x : core_variables) {
				std::cout << x.str() << std::endl;
			}
		}
		lexertk::token& cur() {
			return current;
		}
		lexertk::token& advance() {
			if (!canAdvance()) return current;
			index++;
			current = gen[index];
			return gen[index];

		}
		lexertk::token& advance(unsigned int x) {
			if (!canAdvance(x)) return current;
			index += x;
			current = gen[index];
			return gen[index];

		}
		lexertk::token& getNext() {
			if (!canAdvance()) return current;

			return gen[index + 1];
		}
		lexertk::token& getNext(unsigned int x) {
			if (!canAdvance(x)) return current;

			return gen[index + x];
		}
		bool canAdvance() {
			return index + 1 <= gen.size();
		}
		bool canAdvance(unsigned int x) {
			return index + x <= gen.size();
		}
	};
}