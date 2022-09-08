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
		bool isVariable(lexertk::token& t) {
			for (int i = 0; i < core_variables.size(); i++) {
				auto name = core_variables[i].getName();
				//std::cout << "COMPARING:" << name << ", AND " << cur().value << std::endl;
				if (name == t.value) {
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
					VariableNode var(std::make_shared<Variable>(v), negate_next_node);
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

					Node full(left, op, right);
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
		Type parseExt(Base::Decl decl = Base::Decl::UNKNWN, std::any* take_raw = nullptr, Base::Decl* take_raw_decl = nullptr) {
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
				return Node(std::make_shared<Node>(*left), *op, std::make_shared<Node>(*right), negate);
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
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::INT, negate_next_node));
					if (negate_next_node) negate_next_node = false;

					DEBUG("creating node:" + operationStack.back()->repr());
					DEBUG("checking next:" + getNext().value);
					if (!isOp(getNext())) {
						//std::cout << ("is not op1:" + getNext().value);
						br = true;
					}

				}
				else if (isFloat(cur())) {
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::FLOAT, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}

				}
				else if (isString(cur())) {
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::STRING, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}
				}
				else if (Base::is(cur(), "(")) {
					operationStack.push_back(std::make_shared<Node>(brEval(negate_next_node)));
					advance();
					if (!isOp(cur())) {
						br = true;
					}
					if (negate_next_node) negate_next_node = false;
				}
				else if (isVariable()) {
					Variable& v = getVariable(cur());
					if (!Base::is(getNext(), ".")) {
						operationStack.push_back(std::make_shared<VariableNode>(std::make_shared<Variable>(v), negate_next_node));
					}
					else {
						DEBUG("parsing variable usage because of object usage from var...");
						VContainer out = parseVariableUsage();
						//if (&out == nullptr) throw marine::errors::RuntimeError("unexpected error while trying to access object.");
						operationStack.push_back(std::make_shared<VCNode>(out, negate_next_node));
					}
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}
				}
				else if (isFuncCall()) {
					marine::VContainer v = parseFuncCall<marine::VContainer>();
					//std::cout << "VARIABLE VCONTAINER TYPE 2: " << Base::declStr(v.type()) << std::endl;
					operationStack.push_back(std::make_shared<VCNode>(v, negate_next_node));
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
					std::shared_ptr<Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();


					DEBUG("creating large node");
					DEBUG("is left singular?"); DEBUG(left->isSingular()); DEBUG(left->repr());

					operationStack.push_back(std::make_shared<Node>(left, op, right));
				}
				for (auto& x : operatorStack) DEBUG("opers on stack:" + x.str());
				for (auto& x : operationStack) DEBUG("nodes on stack:" + x->repr());
				if (br)  break;
				advance();
			}
			DEBUG("breaking...");
			//should only have one node
			 
			std::shared_ptr<Node>& operation = operationStack.back();


			if (take_raw != nullptr) {
				for (auto& x : operationStack) {
					DEBUG("OPERATION POINTER CHECK:" + x->repr());
				}
				auto x = operation->calc(); 
				*take_raw_decl = Node::getRootNodeType(&*operation, true);
				*take_raw = x;
				DEBUG("node str:"); DEBUG(operation->repr());
				return finalVal;
			}
			finalVal = std::any_cast<Type>(operation->calc());
			return finalVal;
		}

		template<typename Type>
		Type parseExtParam(Base::Decl decl = Base::Decl::UNKNWN) {
			advance();
			Type finalVal{};
			bool negate_next_node = false;

			std::unique_ptr<Node> n = nullptr;


			DEBUG("checking: " + cur().value);
			if (isInt(cur())) {
				n = std::make_unique<Node>(cur(), Base::Decl::INT, negate_next_node);
			}
			else if (isFloat(cur())) {
				n = std::make_unique<Node>(cur(), Base::Decl::FLOAT, negate_next_node);
			}
			else if (isString(cur())) {
				n = std::make_unique<Node>(cur(), Base::Decl::STRING, negate_next_node);
			}
			else if (isVariable()) {
				if (!isVariableObjectUsage()) {
					Variable& v = getVariable(cur());

					n = std::make_unique<VariableNode>(std::make_shared<Variable>(v), negate_next_node);
				}
				else {
					VContainer out = parseVariableUsage();

					n = std::make_unique<VCNode>(out);
				}
			}
			else {
				//check string before this [TODO]
				DEBUG("EXPECTED PARAMETER ERROR"); DEBUG(cur().value);
				throw marine::errors::SyntaxError("expected parameter.");
			}
			finalVal = std::any_cast<Type>(n->calc());
			DEBUG(cur().value);
			//should only have one node
			return finalVal;
		}

		DynamicObject parseDynamicObjectDecl() {
			DEBUG("parsing dynamic object...");
			advance();
			if (!Base::is(cur(), "{")) throw marine::errors::SyntaxError("expected '{' after obj declaration.");

			DynamicObject _this;


			int c = 1;

			advance();


			while (canAdvance() && c > 0) {

				if (isDecl()) {
					Base::Decl type = Base::declareParse(cur());
					lexertk::token& decl_name = advance();
					if (!Base::is(advance(), ":")) throw marine::errors::SyntaxError("expected ':' afted object variable declaration.");

					VContainer* ret = nullptr;
					if (type == Base::Decl::INT) {
						int val = parseExt<int>();
						ret = new VContainer(val, depth, type);
					}
					else if (type == Base::Decl::FLOAT) {
						float val = parseExt<float>(Base::Decl::FLOAT);
						ret = new VContainer(val, depth, type);
					}
					else if (type == Base::Decl::STRING) {
						String val = parseExt<String>();

						ret = new VContainer(val, depth, type);
					}
					else if (type == Base::Decl::DYNAMIC_OBJECT) {
						DynamicObject obj = parseDynamicObjectDecl();

						ret = new VContainer(obj, depth, type);
					}
					else throw marine::errors::MError("variable type is unknown.");
					//case Base::Decl::CUSTOM:
						//parseExt<std::any>();
					auto x = ObjectHandler::getPrecomiledObject(type);
					ret->setObjSelf(&x);
					_this.add(decl_name.value, *ret);

					DEBUG("SETTING VCONTAINER OBJECT INTERNAL OBJ:");
					DEBUG(x.getName());

				}
				else if (Base::is(cur(), "{")) c++;
				else if (Base::is(cur(), "}")) c--;


				DEBUG("AT:"); DEBUG(cur().value);
				if(c > 0)advance();
			}
			return _this;
		}
		Variable parseDecl(bool push = true) {
			Base::Decl type = Base::declareParse(cur());
			//std::cout << "variable type:" << Base::declStr(type) << std::endl;
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
				else if (type == Base::Decl::DYNAMIC_OBJECT) {
					DynamicObject obj = parseDynamicObjectDecl();

					ret = new Variable(decl_name.value, obj, conf);
					ret->setDecl(Base::Decl::DYNAMIC_OBJECT);
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
			return (Base::is(cur(), "method") && Base::is(getNext(2), "("));

		}
		std::vector<Base::Decl> predictParameterInbFuncCallTypes() {
			std::vector<Base::Decl> ret;
			int brc = 0;
			int nbr_c = 0;
			for(int i = index + 1; i < gen.size(); i++){
				DEBUG("checking i:"); DEBUG(gen[i].value);
				if (Base::is(gen[i], ")")) brc--;
				else if (Base::is(gen[i], "(")) brc++;
				else if (Base::is(gen[i], "[")) nbr_c++;
				else if (Base::is(gen[i], "]")) nbr_c--;
				else if (isInt(gen[i])) ret.push_back(Base::Decl::INT);
				else if (isFloat(gen[i])) ret.push_back(Base::Decl::FLOAT);
				else if (isVariable(gen[i])) {
					DEBUG("is var: " + gen[i].value);
					Variable& v = getVariable(gen[i]);
					if (!Base::is(gen[i + 1], ".")) {
						DEBUG("adding default variable.");
						ret.push_back(v.getDecl());
					}
					else {
						DEBUG("is object accessing...");
						if (!v.isDynamicObj()) {

							bool cont = true;
							while (gen[i + 1].value == ".") {
								auto x = v.getObjSelf();
								if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
								auto y = x->getFunction(gen[i + 2].value);

								if (y != nullptr) {
									DEBUG("PUSHING: "); DEBUG(Base::declStr(y->returnType));
									ret.push_back(y->returnType);
								}
								else {
									//ex it is a variable in the object [TODO]
								}
								/*if (i + 3 < gen.size() && gen[i + 3].value != "(") break;
								int c = 1;
								while (c > 0) {
									if(gen[i])
								}*/
								// recursion so that functions and members can be accessed continuously, ex: this.something.someFunc().someValue.someFunc() [TODO]
								i += 2;
							}
						}
						else {
							//it is a dynamic object
							DynamicObject& o = v.castRef<DynamicObject>();

							auto x = gen[i + 2].value;

							// recursion so that functions and members can be accessed continuously, ex: this.something.someFunc().someValue.someFunc() [TODO]

							if (!o.has(x)) throw marine::errors::SyntaxError("object does not contain member.");

							auto* val = &o.get(x)->second;

							ret.push_back(val->type());
							i += 2;
						}
					}
				}
				//else if(brc == 1 && nbr_c == 0 && !Base::is(gen[i], ","))ret.push_back(Base::Decl::STRING);//[TODO add Base::Decl::CUSTOM]
				
				
				if (brc == 0 && nbr_c == 0) break;
			}
			return ret;
		}
		template<typename Type>
		Type parseFuncCall(bool* return_parent = nullptr) {
			lexertk::token& name = cur();

			incDepth();
			int br = 0;
			int param = 0;
			Function* f = nullptr;
			std::vector<Base::Decl> allDecls = {};
			std::vector<Variable> variables;

			for (auto& x : core_functions) {
				if (x.getName() == name.value) {
					DEBUG("FOUND FUNCTION:"); DEBUG(x.getName());
					f = &x; break;
				}
			}

			//if (f == nullptr) throw marine::errors::SyntaxError("function does not exist.");

			//DEBUG("found function: " + f->getName());
			if (f != nullptr) {
				int nbr_i = 0;
				advance();
				while (canAdvance()) {
					DEBUG("checking func param:" + cur().value);
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					else if (Base::is(cur(), ")")) br--;
					else if (Base::is(cur(), "]")) nbr_i--;
					if (Base::is(cur(), ",") && br == 1) {
						param++;
					}

					DEBUG("br? ");
					DEBUG(br);


					if (br == 0) break;
					if (f->parameters.size() == 0) { advance(); continue; }

					try {
						DEBUG(param);
						Variable& p = f->parameters[param];
						switch (p.getDecl()) {
						case Base::Decl::INT: {
							DEBUG("PARSING EXT AT"); DEBUG(cur().value);
							auto ret = parseExtParam<int>();
							p.setValue(ret);
							p.setTokenStr(ret);
							break;
						}
						case Base::Decl::FLOAT: {

							DEBUG("PARSING EXT AT"); DEBUG(cur().value);
							auto ret = parseExtParam<float>(Base::Decl::FLOAT);
							p.setValue(ret);
							p.setTokenStr(ret);
							break;
						}
						case Base::Decl::STRING: {

							DEBUG("PARSING EXT AT"); DEBUG(cur().value);
							auto ret = parseExtParam<String>();
							p.setValue(ret);
							p.setTokenStr(ret.get());
							break;
						}

						}
						allDecls.push_back(p.getDecl());
						variables.push_back(p);
						core_variables.push_back(p);
					}
					catch (std::exception& ig) {
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}

					advance();
				}

				//FUNC EXECUTION
				DEBUG(cur().value);

				advance();


				DEBUG("FUNCTION:"); DEBUG(f->str());
				int c = index - 1; // -1 so that the next parse loop will advance to the token after the '}'


				incDepth();
				if (!setCaret(f->getStart())) DEBUG("COULD NOT SET CARET.");
				VContainer v(nullptr, depth, Base::Decl::UNKNWN);
				while (index < f->getEnd()) {

					if (parse(&v)) {
						DEBUG("VALUE RETURNED:"); DEBUG(v.str());
						if(return_parent != nullptr)*return_parent = true;

						decDepth();
						setCaret(c);


						return v;
					}
				}
				decDepth();
				setCaret(c);

				return VContainer::null();
			}
			else {
				//inbuilt function

				//check for library decl ex inb.stuff.function()
				std::vector<std::any> parameters;
				//call

				int nbr_i = 0;
				auto PredictedParameterTypes = predictParameterInbFuncCallTypes();
				DEBUG(PredictedParameterTypes.size());
				for (auto& x : PredictedParameterTypes) {
					DEBUG("PREDICTED TYPE:");DEBUG(Base::declStr(x));
				}
				inb::Callable* c;
				c = &inb::getNoIncludeActionByName(name.value,PredictedParameterTypes);
				if (c->name == "NULL") {
					//std::cout << "is null";
					DEBUG("is returnable func?");
					c = &inb::getNoIncludeFunctionByName(name.value, PredictedParameterTypes);
				}
				if (c->name == "NULL") throw marine::errors::SyntaxError("Function does not exist.");
				//check parameters
				
 				advance();
				while (canAdvance()) {
					DEBUG("checking func param:" + cur().value);
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					else if (Base::is(cur(), ")")) br--;
					else if (Base::is(cur(), "]")) nbr_i--;
					if (Base::is(cur(), ",") && br == 1) {
						param++;
					}

					DEBUG("br? ");
					DEBUG(br);


					if (br == 0) break;

					try {
						DEBUG(param);
						switch (c->paramTypes[param]) {
						case Base::Decl::INT:
							DEBUG("is an int");
							parameters.push_back(parseExtParam<int>());
							allDecls.push_back(Base::Decl::INT);
							break;
						case Base::Decl::FLOAT:
							DEBUG("is a float.");
							parameters.push_back(parseExtParam<float>(Base::Decl::FLOAT));
							allDecls.push_back(Base::Decl::FLOAT);
							break;
						case Base::Decl::STRING:
							parameters.push_back(parseExtParam<String>());
							allDecls.push_back(Base::Decl::STRING);
							break;

						}
					}
					catch (std::exception& ig) {
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}
					advance();
				}

				DEBUG(parameters.size());
				DEBUG(c->name);

				VContainer returnValue;
				c->call(parameters, &returnValue);
				DEBUG("calling at:" + cur().value);

				auto x = ObjectHandler::getPrecomiledObject(returnValue.type());
				returnValue.setObjSelf(&x);


				return returnValue;
			}
	}
	void skipFuncDecl() {
		lexertk::token start;
		lexertk::token end;

		std::string name = advance().value;

		int start_index = -1 ;
		int end_index = -1;

		int cbrcount = 0;
		bool foundbr = false;
		std::vector<Variable> parameters;
		incDepth();

		DEBUG("starting at:"); DEBUG(cur().value);

		advance();

		if (Base::is(cur(), "(")) {
			DEBUG("starting parameter check...");
			while (canAdvance()) {
				advance();
				parameters.push_back(parseDecl(false));
				DEBUG("checking next:" + cur().value);
				if (Base::is(cur(), ")")) break;
				

				if (!Base::is(cur(), ",")) throw marine::errors::SyntaxError("invalid symbol after parameter decl.");

			}
		}
		DEBUG("CHECKING FUNCTION BODY AT:"); DEBUG(getNext().value);

		start_index = index + 1;
		start = getNext();


		while(canAdvance()){
			advance();
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

		DEBUG(func.str());

		for (auto& x : func.parameters) {
			DEBUG("param:");
			DEBUG(x.str());
		}
		core_functions.push_back(func);
		decDepth();
	}
	bool isComment() {
		return Base::is(cur(), "#");
	}
	bool isCommentBlock() {
		return Base::is(cur(), "###");
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
	bool parseWhileStatement() {
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
				if (parse()) return true; // true means we need to return from the parent function, if it exists.
			}
		}
		decDepth();
		setCaret(endindex);
	}
	template<typename T>
	void modifyVariable(Variable* v, Operator o, T val, lexertk::token* token = nullptr, Base::Decl d = Base::Decl::UNKNWN) {
		DEBUG("MODIFYING VARIABLE:");
		DEBUG(v->str());
		DEBUG(Base::declStr(v->getDecl()));
		switch (o.getType()) {
		case Operator::OPTYPE::ASSIGN:
			v->getValue() = val;
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
	template <typename T>
	void modifyVContainer(VContainer* v, Operator o, T val, lexertk::token* token = nullptr, Base::Decl d = Base::Decl::UNKNWN) {
		DEBUG("MODIFYING VCONTAINER:");
		DEBUG(v->getStringified());

		switch (o.getType()) {
		case Operator::OPTYPE::ASSIGN:
			v->get().reset();
			v->get() = val;
			break;
		default:
			throw marine::errors::SyntaxError("unexpected operator while trying to modify variable usage.");
			//more to come



		}
	}
	marine::VContainer parseObjectFunctionReference(ValueHolder* v) {
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

			for (const auto& x : c->paramTypes) {
				DEBUG("PARAM TYPE:"); DEBUG(Base::declStr(x));
			}
			advance();
			while (canAdvance()) {
				DEBUG("checking func param:" + cur().value);
				if (Base::is(cur(), "(")) br++;
				else if (Base::is(cur(), "[")) nbr_i++;
				if (Base::is(cur(), ")")) br--;
				else if (Base::is(cur(), "]")) nbr_i--;
				if (Base::is(cur(), ",") && br == 1) {
					DEBUG("found other parameter.");
					param++;
				}
				DEBUG("br? ");
				DEBUG(br);
				if (br == 0) break;
				if (c->paramTypes.size() == 0) { advance(); continue; }
				try {
					DEBUG(param);
					switch (c->paramTypes[param]) {
					case Base::Decl::INT:
						parameters.push_back(parseExtParam<int>());
						allDecls.push_back(Base::Decl::INT);
						break;
					case Base::Decl::FLOAT:
						parameters.push_back(parseExtParam<float>(Base::Decl::FLOAT));
						allDecls.push_back(Base::Decl::FLOAT);
						break;
					case Base::Decl::STRING:
						parameters.push_back(parseExtParam<String>());
						allDecls.push_back(Base::Decl::STRING);
						break;

					}
				}
				catch (std::exception& ig) {
					//throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
				}

				advance();

			}
			VContainer returnValue;
			DEBUG("calling..." + c->name);
			c->call(parameters,v, &returnValue);
			DEBUG("done object call");
			//do something with return value?
			auto x = ObjectHandler::getPrecomiledObject(returnValue.type());
			returnValue.setObjSelf(&x);

			DEBUG(returnValue.getStringified());
			return returnValue;
		}
		DEBUG("returning null VCONTAINER...");
		return VContainer::null();
	}
	marine::ObjectVariable* parseObjectVariableReference(ValueHolder* v) {
		if (ObjectHandler::isPrecompiledObject(v)) {
			DEBUG("IS PRECOMPILED");
		}
		auto x = v->getObjSelf();
		if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
		DEBUG(x->getName());

		if (x->hasVariable(cur().value)) {
			if (!Base::is(getNext(), ".")) return x->getVariable(cur().value);
		}
	}
	
	marine::ValueHolder& parseObjectReference(ValueHolder* v) {
		//cur should be '.'

		advance();

		if (isFuncCall()) {
			VContainer out = parseObjectFunctionReference(v);
			if (!Base::is(advance(), ".")) return out;

			return parseObjectReference(&out);
		}
		ObjectVariable* out = parseObjectVariableReference(v);
		if (!Base::is(advance(), ".")) return out->get();



		return parseObjectReference(&out->get());

	}
	bool isVariableObjectUsage() {
		return (Base::is(getNext(), "."));
	}
	VContainer parseVariableUsage() {

		Variable* v = &getVariable(cur());
		DEBUG("parsing variable usage at:"); DEBUG(cur().value);
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
			return VContainer::null();
		}
		else if (Base::is(getNext(), ".")) {
			if (!v->isDynamicObj()) {
				advance();
				DEBUG("IS DOT");
				//check if is function first
				DEBUG(getNext().value);
				VContainer x = parseObjectFunctionReference(v);

				DEBUG(Base::declStr(x.type()));
				if (Base::is(getNext(), ".")) {

					advance();
					DEBUG(getNext().value);
					return parseObjectFunctionReference(&x);
				}
				return x;
			}
			else {
				//accessing dynamic object item
				advance(2);
				std::unordered_map<std::string, VContainer>::iterator iter;
				
				DynamicObject& d = v->castRef<DynamicObject>();

				std::string& name = cur().value;
				DEBUG("accessing dynamic object item... with name of:"); DEBUG(name);


				if (!d.has(name)) throw marine::errors::RuntimeError("object does not contain value.");

				iter = d.get(name);

				if (iter->second.isDynamicObj()) {
					d = iter->second.cast<DynamicObject&>();
					advance(2);
				}
				else if (iter->second.isDynamicObj() && Base::is(getNext(), ".")) {
					// we are accessing another object that is a connected STATIC object, not a dynamic object.
					DEBUG("found object recursion...");
					advance();
					return parseObjectFunctionReference(&iter->second);
				}
				DEBUG("NEXT:"); DEBUG(getNext().value);


				if(!isOp(getNext())) return iter->second;
				else {

					auto* vc = &iter->second;
					Operator op(advance());
					//DEBUG("OPERATOR:");DEBUG((int)op.getType());
					switch (vc->getDecl()) {
					case Base::Decl::INT:
						modifyVContainer<int>(vc, op, parseExt<int>());
						break;
					case Base::Decl::FLOAT:
						modifyVContainer<float>(vc, op, parseExt<float>());
						break;
					case Base::Decl::STRING:
					{
						DEBUG("MODIFYING STRING");
						String x = parseExt<String>();
						lexertk::token t(x.get());
						modifyVContainer<String>(vc, op, x, &t);
						break;
					}
					case Base::Decl::BOOL:
						modifyVContainer<bool>(vc, op, parseExt<bool>());
						break;
					case Base::Decl::LIST:
						modifyVContainer<ArrayList>(vc, op, parseExt<ArrayList>());
						break;
					}
					return *vc;
				}
			}
			//maybe is var?
		}

		//check for usage of '.'
	}
	bool isReturnStatement() { return (Base::is(cur(), "return")); }
#pragma endregion
	bool parse(ValueHolder* v = nullptr) {
		DEBUG("checking cur:" + cur().value);
		if (isComment()) {
			while (cur().value != "#") advance();
			advance();
		}
		else if (isCommentBlock()) {
			while (cur().value != "###") advance();
			advance();
		}
		if (isDecl()) {
			DEBUG("is decl:" + cur().value);
			parseDecl();
		}
		else if (isReturnStatement()) {
			std::any x;
			Base::Decl expectedReturn;
			parseExt<std::any>(Base::Decl::UNKNWN, &x, &expectedReturn);
			if (v != nullptr) {
				v->setValue(x);
				v->setDecl(expectedReturn);
				return true;
			}

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

		return false;

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
		current = gen[index];
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


	bool setCaret(int i) {
		if (i > end_range) return false;
		DEBUG("setting caret...");
		old_index = index;
		index = i;
		DEBUG("caret set to:"); DEBUG(gen[index].value);
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