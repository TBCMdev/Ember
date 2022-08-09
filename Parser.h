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
#include "Types.h"
#include "inb.h"



#define DEBUG(x) if (MARINE__DEBUG) std::cout << "[debug] " << x << std::endl


using namespace marine;
using namespace marine::ext;
using namespace marine::out;
namespace marine {
	template <typename T>
	std::string anyToStr(T& t) {
		try {
			std::stringstream s;

			s << t;

			return s.str();
		}
		catch (...) {
			return "";
		}
	}

	class Parser {
	protected:
		lexertk::generator& gen;
		lexertk::token current;
		std::vector<Variable> core_variables;
		std::vector<Function> core_functions;
		int depth = 0;
		int index = -1;
		int old_index = -1;
		int start_range = -1;
		int old_end_range = -1;
		int end_range;
	public:
		auto getVariables() { return core_variables; }
		Parser(lexertk::generator& generator) : gen(generator) {
			end_range = generator.size();
		}
		Parser(lexertk::generator& generator, unsigned int s_range, unsigned int e_range) : gen(generator), start_range(s_range), end_range(e_range), index(start_range) {}
#pragma region decl
		void incDepth() { depth++; }
		void decDepth() {
			depth--;
			std::vector<Variable>::iterator iter;
			for (iter = core_variables.begin(); iter != core_variables.end();) {
				if (iter->getDepth() > depth) {
					iter = core_variables.erase(iter);
				}
				else ++iter;
			}
		}
		bool isDecl() {
			return Base::declareParse(cur()) != Base::Decl::UNKNWN;
		}
		bool CheckINBLibraryDecl() {
			if (!inb::matchINBLibraryName(cur().value)) return false;
			if (Base::is(getNext(), ".")) {
				advance(2);
				return CheckINBLibraryDecl();
			}
			if(Base::is(getNext(), "(")) return isFuncCall();
		}
		bool isVariable() {
			DEBUG("is var? " + cur().value);
			for (int i = 0; i < core_variables.size(); i++) {
				auto name = core_variables[i].getName();
				//std::cout << "COMPARING:" << name << ", AND " << cur().value << std::endl;
				if (name == cur().value) {
					////std::cout << "name:" << name << std::endl;
					return true;
				}
				else {
					//std::cout << "continuing..." << std::endl;
				}
			}
			return false;
		}
		Variable& getVariable(lexertk::token& t) {
			for (auto& x : core_variables) {
				if (x.getName() == t.value) return x;
			}
			std::string n("null");
			Variable v(n, nullptr, {});
			return v;
		}
		bool isFuncCall() {
			DEBUG("checking func:" + cur().value);
			if (Base::is(getNext(), "(")) {
				int br = 1;
				for (int i = index + 2; i < gen.size(); i++) {
					if (Base::is(gen[i], "(")) br++;
					if (Base::is(gen[i], ")")) br--;
					if (br == 0) break;
				}
				if (br != 0) return false;//throw errors::SyntaxError("expected '(' after function call.");
				return true;
			}
			return false;
		}
		std::shared_ptr<Node> parseBoolExprExt(EXPRDATA* data = nullptr) {
			//current is '('
			std::vector<std::shared_ptr<Node>> operationStack;
			std::vector<Operator> operatorStack;
			bool br = false;
			bool negate_next_node = false;
			while (canAdvance()) {
				DEBUG("checking bool: " + cur().value);
				if (Base::is(cur(), "!")) {
					if (data != nullptr) data->negated = true;
				}
				else if (Base::is(cur(), "?")) {
					if (data != nullptr) data->simnull = true;
				}
				else if (isInt(cur())) {
					Node n(cur(), Base::Decl::INT, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;

					DEBUG("creating node:" + operationStack.back()->repr());
					DEBUG("checking next:" + getNext().value);
					if (!isOp(getNext())) {
						//std::cout << ("is not op5:" + getNext().value);
						br = true;
					}

				}
				else if (isFloat(cur())) {
					DEBUG("identified float:" + cur().value);
					DEBUG("checking next:" + getNext().value);
					Node n(cur(), Base::Decl::FLOAT, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op6:" + getNext().value);
						br = true;
					}

				}
				else if (isVariable()) {
					Variable& v = getVariable(cur());
					//std::cout << v.str() << "is var" << std::endl;
					VariableNode var(v, negate_next_node);
					operationStack.push_back(std::make_shared<VariableNode>(var));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op4:" + getNext().value);
						br = true;
					}
				}
				else if (isString(cur())) {
					DEBUG("identified String:" + cur().value);
					DEBUG("checking next:" + getNext().value);
					Node n(cur(), Base::Decl::STRING, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op7:" + getNext().value);
						br = true;
					}
				}
				else if (isFuncCall()) {
					marine::VContainer v = parseFuncCall<marine::VContainer>();
					Node n(lexertk::token(v.getStringified()), v.type(), negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op8:" + getNext().value);
						br = true;
					}
				}
				else if (Base::is(cur(), "(")) {
					//operationStack.push_back(brEval(negate_next_node));
					advance();
					if (!isOp(cur())) {
						br = true;
					}
					if (negate_next_node) negate_next_node = false;
				}
				else if (Base::is(cur(), ")")) {
					br = true;
				}
				else if (isOp(cur())) {
					if ((operationStack.size() < 1 || operatorStack.size() >= 1) && (Base::is(cur(), "-") || Base::is(cur(), "+"))) {
						DEBUG("found negator op:" + cur().value);
						negate_next_node = true;
					}
					else {
						DEBUG("pushing op:" + cur().value);
						operatorStack.push_back(Operator(cur()));
					}
				}
				else if (isLogicalOp(cur())) {
					//std::cout << "is logical:" << cur().value << std::endl;
					br = true;
				}
				else {
					st_spr(cur().value,STATUS::WARN);
				}

				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					std::shared_ptr<Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();


					Node full(*left, op, *right);
					operationStack.push_back(std::make_shared<Node>(full));
				}
				for (auto& x : operatorStack) DEBUG("opers on bool stack:" + x.str());
				for (auto& x : operationStack) DEBUG("nodes on bool stack:" + x->repr());
				if (isLogicalOp(getNext())) advance();
				if (br)  break;
				advance();
			}
			//std::cout << "cur after break:" << cur().value;
			//std::cout << "returning after bool:" << operationStack.back()->repr() << std::endl;
			
			DEBUG("IS LAST VECTOR ELEMENT VARIABLE NODE?");
			DEBUG(operationStack.back()->isVariable());
			return operationStack.back();


		}
		template<typename Type>
		Type parseExt(Base::Decl decl = Base::Decl::UNKNWN) {
			advance();
			std::function<Node(bool)> brEval = [&](bool negate) -> Node {
				Node* left = nullptr, * right = nullptr;
				Operator* op = nullptr;
				bool negate_next_node_recr = false;
				while (canAdvance()) {
					//just encountered bracket
					advance();

					// -(14 - -2)
					if (Base::is(cur(), "(")) {
						return brEval(negate_next_node_recr);
					}
					else if (isInt(cur())) {
						if (left == nullptr) {
							left = new Node(cur(), Base::Decl::INT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(cur(), Base::Decl::INT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isFloat(cur())) {
						if (left == nullptr) {

							left = new Node(cur(), Base::Decl::FLOAT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(cur(), Base::Decl::FLOAT, negate_next_node_recr);

							if (negate_next_node_recr) negate_next_node_recr = false;
						}
					}
					else if (isString(cur())) {
						if (left == nullptr) {
							left = new Node(cur(), Base::Decl::STRING, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(cur(), Base::Decl::STRING, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isBool(cur())) {
						if (left == nullptr) {
							left = new Node(cur(), Base::Decl::BOOL, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(cur(), Base::Decl::BOOL, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isOp(cur())) {
						std::cout << "is op:" << cur().value << std::endl;
						if (Base::is(cur(), "-") && (left == nullptr || op != nullptr)) negate_next_node_recr = true;
						if (op == nullptr) op = new Operator(cur());
					}
					else if (isVariable()) {
						Variable v = getVariable(cur());
						if (left == nullptr) {
							left = new Node(v.getToken(), v.getDecl(), negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(v.getToken(), v.getDecl(), negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isFuncCall()) {
						marine::VContainer c = parseFuncCall<marine::VContainer>();

						if (left == nullptr) {
							left = new Node(lexertk::token(c.getStringified()), c.type(), negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(lexertk::token(c.getStringified()), c.type(), negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					if ((left != nullptr && right != nullptr && op != nullptr) || Base::is(cur(), ")")) {
						////std::cout << "exiting bracket: " << left->repr() << op->str() << right->repr() << std::endl;
						break;
					}
				}
				return Node(*left, *op, *right, negate);
			};
			std::vector<std::shared_ptr<Node>> operationStack;
			Type finalVal{};
			std::vector<Operator> operatorStack;
			bool br = false;
			bool negate_next_node = false;
			DEBUG("begin parseExt...");
			while (canAdvance()) {
				DEBUG("loop");
				DEBUG("checking: " + cur().value);
				if (isInt(cur())) {
					Node n(cur(), Base::Decl::INT, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;

					DEBUG("creating node:" + operationStack.back()->repr());
					DEBUG("checking next:" + getNext().value);
					if (!isOp(getNext())) {
						//std::cout << ("is not op1:" + getNext().value);
						br = true;
					}

				}
				else if (isFloat(cur())) {
					Node n(cur(), Base::Decl::FLOAT, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						std::cout << ("is not op2:" + getNext().value);
						br = true;
					}

				}
				else if (isString(cur())) {
					Node n(cur(), Base::Decl::STRING, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						std::cout << ("is not op3:" + getNext().value);
						br = true;
					}
				}
				else if (Base::is(cur(), "(")) {
					Node n = brEval(negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					advance();
					if (!isOp(cur())) {
						br = true;
					}
					if (negate_next_node) negate_next_node = false;
				}
				else if (isVariable()) {
					Variable v = getVariable(cur());
					VariableNode var(v, negate_next_node);

					operationStack.push_back(std::make_shared<VariableNode>(var));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op4:" + getNext().value);
						br = true;
					}
				}
				else if (isFuncCall()) {
					marine::VContainer v = parseFuncCall<marine::VContainer>();
					//std::cout << "VARIABLE VCONTAINER TYPE 2: " << Base::declStr(v.type()) << std::endl;
					Node n(lexertk::token(v.getStringified()), v.type(), negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op4:" + getNext().value);
						br = true;
					}
				}
				else if (isOp(cur())) {
					if ((operationStack.size() < 1 || operatorStack.size() >= 1) && (Base::is(cur(), "-") || Base::is(cur(), "+"))) {
						DEBUG("found negator op:" + cur().value);
						negate_next_node = true;
					}
					else {
						DEBUG("pushing op:" + cur().value);
						operatorStack.push_back(Operator(cur()));
					}
				}

				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					std::shared_ptr <Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();


					Node n(*left, op, *right);
					DEBUG("creating large node:");
					DEBUG(n.repr());
					operationStack.push_back(std::make_shared<Node>(n));
				}
				for (auto& x : operatorStack) DEBUG("opers on stack:" + x.str());
				for (auto& x : operationStack) DEBUG("nodes on stack:" + x->repr());
				if (br)  break;
				advance();
			}
			DEBUG("breaking...");
			//should only have one node
			//std::cout << "END SIZE:" << operationStack.size() << std::ObjectHandler::getPrecomiledObject(ret)endl;
			std::shared_ptr<Node> operation = operationStack.back();
			finalVal = std::any_cast<Type>(operation->calc());
			//std::cout << "end:" << cur().value << std::endl;
			return finalVal;
		}

		template<typename Type>
		Type parseExtParam(int& bra, int& nbr_i, Base::Decl decl = Base::Decl::UNKNWN) {
			advance();
			Type finalVal{};
			bool negate_next_node = false;
			DEBUG("checking: " + cur().value);
			if (isInt(cur())) {
				DEBUG("is int.");
				decl = Base::Decl::INT;
			}
			else if (isFloat(cur())) {
				DEBUG("is float.");
				decl = Base::Decl::FLOAT;
			}
			else if (isString(cur())) {
				DEBUG("found string.");
				decl = Base::Decl::STRING;
			}
			else {
				//check string before this [TODO]
				DEBUG("EXPECTED PARAMETER ERROR");
				throw marine::errors::SyntaxError("expected parameter.");
			}
			finalVal = std::any_cast<Type>(Node(cur(), decl, negate_next_node).calc());
			advance();
			DEBUG("advancing to:" + cur().value);
			if (Base::is(cur(), ")")) bra--;
			else if (Base::is(cur(), "(")) bra++;
			else if (Base::is(cur(), "[")) nbr_i++;
			else if (Base::is(cur(), "]")) nbr_i--;

			//should only have one node
			return finalVal;
		}


		Variable parseDecl(bool push = true) {
			Base::Decl type = Base::declareParse(cur());
			//std::cout << "variable type:" << Base::declStr(type) << std::endl;
			lexertk::token& start = cur();
			std::vector<Base::DeclConfig> conf{};
			if (Base::is(getNext(), ":")) {
				advance();
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
					String val = parseExt<String>();
					std::cout <<"GET:" << val.get() << std::endl;
					
					ret = new Variable(decl_name.value, val, val.get(), conf);
					ret->setDecl(Base::Decl::STRING);
				}
				else throw marine::errors::MError("variable type is unknown.");
				//case Base::Decl::CUSTOM:
					//parseExt<std::any>();
				ret->setDepth(depth);
				auto x = ObjectHandler::getPrecomiledObject(ret);
				ret->setObjSelf(&x);
				if (push) core_variables.push_back(*ret);

				DEBUG("SETTING VARIABLE INTERNAL OBJ:");
				DEBUG(x.getName());

				return Variable(*ret);
			}
			else {
				DEBUG("NO VALUE");
				//lone var with no value
				Variable ret(decl_name.value, nullptr, "NULL", conf);
				ret.setDecl(type);
				if (push)core_variables.push_back(ret);
				return ret;
			}

		}
		bool isFuncDecl() {
			if (Base::is(cur(), "method") && Base::is(getNext(2), "(")) return true;

			return false;
		}
		std::vector<Base::Decl> predictParameterInbFuncCallTypes() {
			std::vector<Base::Decl> ret;
			int brc = 0;
			int nbr_c = 0;
			for(int i = index + 1; i < gen.size(); i++){
				if (Base::is(gen[i], ")")) brc--;
				else if (Base::is(gen[i], "(")) brc++;
				else if (Base::is(gen[i], "[")) nbr_c++;
				else if (Base::is(gen[i], "]")) nbr_c--;
				else if (isInt(gen[i])) ret.push_back(Base::Decl::INT);
				else if (isFloat(gen[i])) ret.push_back(Base::Decl::FLOAT);
				else if(brc == 1 && nbr_c == 0 && !Base::is(gen[i], ","))ret.push_back(Base::Decl::STRING);//[TODO add Base::Decl::CUSTOM]
				
				
				if (brc == 0 && nbr_c == 0) break;
			}
			return ret;
		}
		template<typename Type>
		Type parseFuncCall() {
			DEBUG("PARSING FUNC CALL");
			lexertk::token& name = cur();

			incDepth();
			int br = 0;
			int param = 0;
			Function* f = nullptr;
			std::vector<Base::Decl> allDecls = {};
			std::vector<Variable> variables;

			for (auto& x : core_functions) {
				if (x.getName() == name.value) {
					f = &x; break;
				}
			}

			//if (f == nullptr) throw marine::errors::SyntaxError("function does not exist.");

			//DEBUG("found function: " + f->getName());
			if (f != nullptr) {
				int nbr_i = 0;
				while (canAdvance()) {
					advance();
					DEBUG("checking func param:" + cur().value);
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					if (Base::is(getNext(), ")")) br--;
					else if (Base::is(getNext(), "]")) nbr_i--;
					else if (br == 1 && nbr_i == 0) param++; // MOCK, WILL NOT ALWAYS WORK [TODO]
					if (Base::is(cur(), ",") && br == 1) {
						param++;
						continue;
					}
					try {
						DEBUG(f->parameters.size());
						DEBUG(param - 1);
						if (param - 1 >= f->parameters.size()) throw marine::errors::IndexError("parameter does not exist.");
						Variable& p = f->parameters[param - 1];
						switch (p.getDecl()) {

						case Base::Decl::INT:
							p.setValue(parseExtParam<int>(br, nbr_i));
							DEBUG("exited at:" + cur().value);
						case Base::Decl::FLOAT:
							p.setValue(parseExtParam<float>(br, nbr_i, Base::Decl::FLOAT));
						case Base::Decl::STRING:
							p.setValue(parseExtParam<String>(br, nbr_i));
						}
						allDecls.push_back(p.getDecl());
						variables.push_back(p);
						core_variables.push_back(p);
					}
					catch (std::exception& ig) {
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}
					DEBUG("br:");
					DEBUG(br);
					if (br == 0) break;
				}

				//FUNC EXECUTION
				if (!startDepthCaret(f->getStart(), f->getEnd())) {
					throw marine::errors::RuntimeError("exception in function execution.");
				}
			}
			else {
				//inbuilt function

				//check for library decl ex inb.stuff.function()
				DEBUG("f == nullptr!");

				std::vector<std::any> parameters;
				//call

				int nbr_i = 0;
				DEBUG("predicting at:" + cur().value);
				auto PredictedParameterTypes = predictParameterInbFuncCallTypes();
				

				inb::Callable* c;
				c = &inb::getNoIncludeActionByName(name.value,PredictedParameterTypes);
				if (c->name == "NULL") {
					//std::cout << "is null";
					DEBUG("is returnable func?");
					c = &inb::getNoIncludeFunctionByName(name.value, PredictedParameterTypes);
				}
				if (c->name == "NULL") throw marine::errors::SyntaxError("Function does not exist.");
				//check parameters
				
				while (canAdvance()) {
					advance();
					DEBUG("checking func param:" + cur().value);
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					if (Base::is(getNext(), ")")) br--;
					else if (Base::is(getNext(), "]")) nbr_i--;
					else if (br == 1 && nbr_i == 0) param++; // MOCK, WILL NOT ALWAYS WORK [TODO]
					if (Base::is(cur(), ",") && br == 1) {
						param++;
						continue;
					}
					try {
						DEBUG(param);
						for (int x = 0; x < param; x++) {
							//std::cout << "checking:" << Base::declStr(c->paramTypes[x]) << x << std::endl;
							switch (c->paramTypes[x]) {
							case Base::Decl::INT:
								DEBUG("is an int");
								parameters.push_back(parseExtParam<int>(br, nbr_i));
								allDecls.push_back(Base::Decl::INT);
								break;
							case Base::Decl::FLOAT:
								DEBUG("is a float.");
								parameters.push_back(parseExtParam<float>(br, nbr_i, Base::Decl::FLOAT));
								allDecls.push_back(Base::Decl::FLOAT);
								break;
							case Base::Decl::STRING:
								parameters.push_back(parseExtParam<String>(br, nbr_i));
								allDecls.push_back(Base::Decl::STRING);
								break;

							}
						}
					}
					catch (std::exception& ig) {
						throw ig;
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}
					DEBUG("br? ");
					DEBUG(br);
					if (br == 0) break;
				}

				DEBUG(parameters.size());
				DEBUG(c->name);

				VContainer returnValue;
				c->call(parameters, &returnValue);
				DEBUG("calling at:" + cur().value);
				return returnValue;
			}
	}
	void skipFuncDecl() {
		lexertk::token& start = cur();
		lexertk::token end;

		std::string name = advance().value;

		int start_index = index;
		int end_index = -1;

		int cbrcount = 0;
		bool foundbr = false;
		std::vector<Variable> parameters;
		incDepth();
		while (canAdvance()) {
			advance();
			if (Base::is(cur(), "(")) {
				while (canAdvance()) {
					advance();
					parameters.push_back(parseDecl(false));
					DEBUG("checking next:" + cur().value);
					if (Base::is(cur(), ")"))
						break;

					if (!Base::is(cur(), ",")) throw marine::errors::SyntaxError("invalid symbol after parameter decl.");

				}
			}
			if (Base::is(cur(), "{")) {
				foundbr = true;
				cbrcount++;
			}
			else if (Base::is(cur(), "}"))cbrcount--;

			if (cbrcount == 0 && foundbr) {
				end = cur();
				end_index = index;
				break;
			}
		}
		if (end_index == -1) throw marine::errors::MError("something unexpected happened.");
		Function func(name, start, end, start_index, end_index, parameters);
		//func.setSubParser(Parser());
		for (auto& x : func.parameters) {
			DEBUG("param:");
			DEBUG(x.str());
		}
		core_functions.push_back(func);
		decDepth();
	}
	bool isIfStatement() {
		return Base::is(cur(), "if");
	}
	bool isWhileStatement() {
		return Base::is(cur(), "while");
	}
	bool isForStatement() {
		return Base::is(cur(), "for");
	}
	BoolExpr* parseLogicalExpr() {
		int brc = 1;
		bool ifop = false;
		for (size_t i = index; i < gen.size(); i++) {
			if (Base::is(gen[i], ")")) brc--;
			if (Base::is(gen[i], "(")) brc++;

			if (isLogicalOp(gen[i]) && brc > 0) {
				ifop = true;
			}
			else if (Base::is(gen[i], ")") && brc == 0) break;
		}
		if (brc != 0) throw marine::errors::SyntaxError("expected ')' at end of logical condition.");

		BoolExpr* expr = nullptr;
		if (ifop) {
			std::shared_ptr<Node> left = parseBoolExprExt();
			
			Operator c(cur());
			advance();

			std::shared_ptr<Node> right = parseBoolExprExt();
			
			expr = new BoolExpr(left, c, right);
		}
		else {
			expr = new BoolExpr(parseBoolExprExt());
		}
		return expr;
	}
	void parseLogicalStatement() {
		advance();
		if (!Base::is(cur(), "(")) throw marine::errors::SyntaxError("expected '(' after if statement.");
		advance();

		BoolExpr* expr = parseLogicalExpr();

		DEBUG(expr->repr());
		if (!expr->evaluate()) {
			DEBUG("EVALUATION IS FALSE");
			//skip to end of }
			bool fobr = false;
			bool fcbr = false;

			int cbr = 0;
			while (canAdvance()) {
				advance();
				if (Base::is(cur(), "{")) {
					if (!fobr) fobr = true;
					cbr++;
				}
				else if (Base::is(cur(), "}")) {
					if (!fobr) throw marine::errors::SyntaxError("unexpected token: '}'");
					cbr--;
				}

				if (Base::is(cur(), "}") && cbr == 0) { break; }


			}
			if (!Base::is(cur(), "}")) throw marine::errors::SyntaxError("expected closing '}'");
			advance();
			//current should be '}'
		}
		else {
			DEBUG("condition is true:" + cur().value);
			incDepth();
			advance();
		}
	}
	void parseWhileStatement() {
		advance();
		if (!Base::is(cur(), "(")) throw marine::errors::SyntaxError("expected '(' after while statement.");
		advance();
		BoolExpr* b = parseLogicalExpr();
		advance();
		if (!Base::is(cur(), ")")) throw marine::errors::SyntaxError("expected ')' after while statement.");
		advance();
		int indexstart = index;
		int endindex = -1;
		int brc = 0;
		for (int i = index; i < gen.size(); i++) {
			if (Base::is(gen[i], "{")) brc++;
			else if(Base::is(gen[i], "}")) brc--;
			//std::cout << "cur:" << gen[i].value << ", int: " << brc << std::endl;
			if (brc == 0) { endindex = i; break; }
		}
		if (endindex == -1) throw marine::errors::SyntaxError("expected closing '}' after while loop decl.");
		DEBUG("entering while loop...");
		DEBUG(indexstart);
		DEBUG(endindex);
		incDepth();
		//std::cout << "EVAL:" << b->evaluate() << std::endl;
		while (b->evaluate()) {
			//std::cout << b->repr() << std::endl;
			setCaret(indexstart);

			if (index >= endindex) continue;

			while (canAdvance() && index < endindex) {
				parse();
			}
		}
		decDepth();
		setCaret(endindex);
	}
	template<typename T>
	void modifyVariable(Variable* v, Operator o, T val, lexertk::token* token = nullptr, Base::Decl d = Base::Decl::UNKNWN) {
		DEBUG("MODIFYING VARIABLE:");
		DEBUG(v->str());
		switch (o.getType()) {
		case Operator::OPTYPE::ASSIGN:
			v->_value.reset();
			v->_value = val;

			break;
		default:
			throw marine::errors::SyntaxError("unexpected operator while trying to modify variable usage.");
		//more to come



		}



		//assert same types of declaration



		if (token != nullptr)v->setToken(*token);
		else v->loseStringTrace();


		DEBUG("END");
		DEBUG(v->str());
	}
	marine::VContainer parseObjectFunctionReference(Variable* v) {
		DEBUG("is dot!");
		if (ObjectHandler::isPrecompiledObject(v)) {
			DEBUG("IS PRECOMPILED");
		}
		auto x = v->getObjSelf();
		if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
		DEBUG(x->getName());
		std::vector<std::any> parameters;
		std::vector<Base::Decl> allDecls;

		advance();

		DEBUG(cur().value);
		DEBUG(x->hasFunction(cur().value));
		if (x->hasFunction(cur().value)) {
			int br = 0;
			int nbr_i = 0;
			int param = 0;
			auto* c = x->getFunction(cur().value);
			while (canAdvance()) {
				advance();
				DEBUG("checking func param:" + cur().value);
				if (Base::is(cur(), "(")) br++;
				else if (Base::is(cur(), "[")) nbr_i++;
				if (Base::is(getNext(), ")")) br--;
				else if (Base::is(getNext(), "]")) nbr_i--;
				else if (br == 1 && nbr_i == 0) param++; // MOCK, WILL NOT ALWAYS WORK [TODO]
				if (Base::is(cur(), ",") && br == 1) {
					param++;
					continue;
				}
				try {
					DEBUG(param);
					for (int x = 0; x < param; x++) {
						//std::cout << "checking:" << Base::declStr(c->paramTypes[x]) << x << std::endl;
						switch (c->paramTypes[x]) {
						case Base::Decl::INT:
							DEBUG("is an int");
							parameters.push_back(parseExtParam<int>(br, nbr_i));
							allDecls.push_back(Base::Decl::INT);
							break;
						case Base::Decl::FLOAT:
							DEBUG("is a float.");
							parameters.push_back(parseExtParam<float>(br, nbr_i, Base::Decl::FLOAT));
							allDecls.push_back(Base::Decl::FLOAT);
							break;
						case Base::Decl::STRING:
							parameters.push_back(parseExtParam<String>(br, nbr_i));
							allDecls.push_back(Base::Decl::STRING);
							break;

						}
					}
				}
				catch (std::exception& ig) {
					//throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
				}
				DEBUG("br? ");
				DEBUG(br);
				if (br == 0) break;
			}
			VContainer returnValue;
			DEBUG("calling..." + c->name);
			c->call(parameters,v, &returnValue);
			DEBUG("done object call");
			//do something with return value?
			DEBUG(returnValue.getStringified());
			return returnValue;
		}
		return VContainer::null();
	}
	bool isVariableObjectUsage() {
		return (Base::is(getNext(), "."));
	}
	void parseVariableUsage() {
		Variable* v = &getVariable(cur());
		DEBUG("parsing variable usage...");
		if (v->getName() == "NULL") throw marine::errors::SyntaxError("unknown token");
		DEBUG(v->str());
		if (isOp(getNext())) {
			DEBUG(Base::declStr(v->getDecl()));
			Operator op(advance());
			//DEBUG("OPERATOR:");DEBUG((int)op.getType());
			switch(v->getDecl()){
			case Base::Decl::INT:
				modifyVariable<int>(v, op, parseExt<int>());
				break;
			case Base::Decl::FLOAT:
				modifyVariable<float>(v, op, parseExt<float>());
				break;
			case Base::Decl::STRING:
			{
				DEBUG("MODIFYING STRING");
				String x = parseExt<String>();
				lexertk::token t(x.get());
				modifyVariable<String>(v, op, x, &t);
				break;
			}
			case Base::Decl::BOOL:
				modifyVariable<bool>(v, op, parseExt<bool>());
				break;
			case Base::Decl::LIST:
				modifyVariable<ArrayList>(v, op, parseExt<ArrayList>());
				break;
			}
		}
		else if (Base::is(getNext(), ".")) {
			advance();
			DEBUG("IS DOT");
			//check if is function first
			VContainer ret = parseObjectFunctionReference(v);
			//maybe is var?
		}

		//check for usage of '.'
	}

#pragma endregion
	void parse() {
		DEBUG("checking cur:" + cur().value);
		if (isDecl()) {
			DEBUG("is decl:" + cur().value);
			parseDecl();
		}
		else if (isIfStatement()) {
			DEBUG("is if:" + cur().value);
			parseLogicalStatement();
		}
		else if (isWhileStatement()) {
			DEBUG("is while:" + cur().value);
			parseWhileStatement();
		}
		else if (isVariable()) {
			DEBUG("IS VARIABLE:" + cur().value);
			parseVariableUsage();
		}
		else if (isFuncCall()) {
			DEBUG("is func call:" + cur().value);

			parseFuncCall<std::any>();
			DEBUG("done callable.");
		}
		else if (isFuncDecl()) {
			DEBUG("is func decl:" + cur().value);

			skipFuncDecl();
		}
		DEBUG("advancing...");
		advance();

	}
	void endParse() {
		end_range = old_end_range;
		index = old_index;
		if (end_range != -1 && old_index != -1) {
			old_end_range = -1;
			old_index = -1;
			while (canAdvance()) {
				parse();
			}
		}

		for (auto& x : core_variables) {
			//std::cout << "VARIABLE:" << x.str() << std::endl;
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
	lexertk::token& getBefore(signed int x = 1) {
		if (index - x < gen.size()) return current;

		return gen[index - x];
	}


	bool startDepthCaret(int i, int end) {

		if (i > end_range) return false;

		old_index = i;
		index = i;

		old_end_range = end;
		end_range = end;

		return true;
	}
	bool setCaret(int i) {
		if (i > end_range) return false;

		old_index = index;
		index = i;
		return true;
	}
	bool canAdvance() {
		return index + 1 <= end_range;
	}
	bool canAdvance(unsigned int x) {
		return index + x <= end_range;
	}
};
}