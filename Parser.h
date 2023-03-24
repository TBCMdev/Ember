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
#include "Operator.h"
#include "ObjectHandler.h"
#include "Class.h"
#include "Scope.h"
#include "Importer.h"
#include "Module.h"
#include "Register.h"

#define DEBUG(x) if (MARINE__DEBUG) std::cout << "[debug] " << x << "\n"


using namespace marine;
using namespace marine::ext;
using namespace marine::out;

namespace marine {
	class Parser {
	protected:


		std::string m_relative_running_dir;

		lexertk::generator& gen;
		lexertk::token current;
		Scope scope;
		int depth = 0;
		int index = -1;
		int old_index = -1;
		int start_range = -1;
		int old_end_range = -1;
		int end_range;

		int m_ParserLockKey = -1; // -1 specifies that parser can parse freely.

		void __reload() {
			end_range = gen.size();
			//more reloads...
		}
	protected:
		void __init_lambda__() {

			Lambda::initialize([&](Lambda* l, std::vector<VContainer>& v) -> VContainer {
				return callLambda(l, v);
			});
		}
	public:
		auto& getVariables() { return scope.getVariables(); }
		auto& getFunctions() { return scope.getFunctions(); }

		Parser(lexertk::generator& generator) : gen(generator) {
			
			end_range = generator.size();
			Importer::initialize();
			__init_lambda__();
		}
		Parser(lexertk::generator& generator, unsigned int s_range, unsigned int e_range) : gen(generator), start_range(s_range), end_range(e_range), index(start_range) {
			Importer::initialize();
			__init_lambda__();
		}
#pragma region decl
		void setRelativeRunningDirectory(std::string& x) {
			m_relative_running_dir = x;
		}
		void scan_imports() {

			// [TODO] This will only work if imports are at the VERY top of the file, sometimes this is not the case.

			while (Base::is(getNext(), "use") || Base::is(getNext(), "from")) {
				advance();
				_import();
			}
		}

		void incDepth() { depth++; }
		void decDepth() {
			depth--;
			std::vector<std::shared_ptr<ValueHolder>>::iterator iter;
			auto& vars = scope.getVariables();
			for (iter = vars.begin(); iter != vars.end();) {
				std::shared_ptr<ValueHolder>& var = *iter._Ptr;
				if (var->getDepth() > depth) {
					iter = vars.erase(iter);
				}
				else ++iter;
			}
		}
		bool module_isDecl() {
			int ind_cpy = index;
			Module* x = scope.findModule(gen[ind_cpy].value);

			if (x == nullptr || ind_cpy + 2 >= gen.size()) return false;
			bool more_than_one = Base::is(gen[ind_cpy + 3], ":"); // checks if we are accessing a sub module.
			ind_cpy ++;
			if (!more_than_one || !x->hasChildModule(gen[ind_cpy + 1].value)) return true;
			do {
				lexertk::token& y = gen[ind_cpy + 1];
				x = x->getChildModule(y.value);
				ind_cpy += 2;

			} while (Base::is(gen[ind_cpy + 1], ":"));

			if (x == nullptr && more_than_one) throw marine::errors::SyntaxError("Module does not exist.");

			return (x != nullptr);

		}
		bool isDecl() {
			//std::cout << "is decl? " << cur().value << (Base::declareParse(cur()) != Base::Decl::UNKNWN || ClassHandler::hasClass(cur().value)) << '\n';
			return Base::declareParse(cur()) != Base::Decl::UNKNWN || ClassHandler::hasClass(cur().value);
		}
		Scope* getScope() {
			return &scope;
		}
		void _import() {
			int import_type = 0;
			bool hasFrom = false;

			if (Base::is(cur(), "use")) {
				import_type = 0;
			}
			else if (Base::is(cur(), "from")) {
				import_type = 1;
				hasFrom = true;
			}
			if (import_type == 1) {
				/*std::function<bool(std::string&)> validateImport = [](std::string& x) {
					return (std::any_of(x.begin(), x.end(), [](char& x) {
						return (!isdigit(x) && !isalpha(x) && x != '_');
					}));
				};*/
				std::string source = String::trim(advance().value);

				if (!Base::is(advance(), "use")) throw marine::errors::SyntaxError("expected 'use' keyword following from import statement.");
				std::vector<std::string*> from;
				
				while (canAdvance()) {
					from.push_back(&advance().value);
					//std::string& back = *from.back();
					//if (!validateImport(back)) throw marine::errors::SyntaxError("imported names cannot contain characters other than letters numbers, or an underscore.");
					if (!Base::is(getNext(), ":")) break;
					advance();
				}
				if (!Importer::importExists(source, m_relative_running_dir)) {
					throw marine::errors::SyntaxError("Import does not exist.");
				}
				//start import.
				std::string s = Importer::readImport(source);


				lexertk::generator g = Importer::parseImport(s);
				//all works!

				Parser p(g);

				p.setRelativeRunningDirectory(m_relative_running_dir);
				p.advance();
				while (p.canAdvance()) {
					p.parse();
				}

				//concat changes
				auto& vec = scope.getVariables();
				auto& vecC = p.getVariables();
				
				//vec.insert(vec.end(), vecC.begin(), vecC.end()); ? We need to scan the vectors for what we are looking for, after implementation of modules

				auto& vecF = scope.getFunctions();
				auto& vecFC = p.getFunctions();

				//vecF.insert(vecF.end(), vecFC.begin(), vecFC.end());
			}
			else {

				//DONE

				// use <name> (:?...) (from <source (:?...)>?)
				std::vector<std::string*> _using;
				while (canAdvance()) {
					_using.push_back(&advance().value);
					//std::string& back = *from.back();
					//if (!validateImport(back)) throw marine::errors::SyntaxError("imported names cannot contain characters other than letters numbers, or an underscore.");
					if (!Base::is(getNext(), ":")) break;
					advance();
				}
				if (Base::is(getNext(), "from")) {
					std::string source = String::trim(advance(2).value);
					if (!Importer::importExists(source, m_relative_running_dir)) throw marine::errors::SyntaxError("Import does not exist.");
					//start import.
					std::string s = Importer::readImport(source);

					lexertk::generator g = Importer::parseImport(s);




					

					// Save what we want to append to our old generator:
					if (_using.size() == 1 && *_using.back() == "*") {

						int sub_index = 0;
						int end = g.size();

						lexertk::generator old = gen;
						gen = g; // set the whole gen to be the new file we just parsed.

						__reload(); // reload generator to match settings of our new generator.
						//start reparsing from index 0.
						while (sub_index < end) {
							parse();
							sub_index++;
						}
						// merge old first as that technically is our main generator.
						lexertk::generator::merge(old, gen);
					}
					else {
						// The import is a module

						// create a new parse instance.
						// the reason we want to to this is because we want to parse, and then
						// figure out what we want to keep, the Parser class does that for us.
						if (s.empty()) return;
						Parser p(g);
						while (p.canAdvance()) {
							p.parse();
						}
						// this holds our modules.
						Scope* module_holder = p.getScope();

						auto& m = module_holder->getModules();

						Module* cur = module_holder->findModule(*_using.front());
						for (int i = 1; i < _using.size(); i++) {
							if (cur == nullptr) throw marine::errors::SyntaxError("Module does not exist in external file.");
							cur = cur->getChildModule((*_using[i]));

							
						}
						// we must copy the pointer!
						// here we add the module, as even if we are using everything inside it,
						// we should still allow the programmer to access the module by its name too.
						std::shared_ptr<Module> cur_shared = std::make_shared<Module>(*cur);
						scope.newModule(cur_shared);

						// We then must add all of our variables and functions.
						scope.mergeModule(cur_shared.get());

					}




					setCaret(index);
				}
				else {
					// default use <name> 
					//if(scope.has)
					std::string source = String::trim(*_using.back());

					bool inbuilt_module = false;

					if (!Importer::importExists(source, m_relative_running_dir, &inbuilt_module)) throw marine::errors::SyntaxError("Import does not exist.");
					//start import.
					bool need = false;
					std::string s = Importer::readImport(source, inbuilt_module, &need);

					if (need) {


						marine::inb::inject_inb_std();

						return;
					}

					lexertk::generator g = Importer::parseImport(s);

					//concat the file on top.
					lexertk::generator::merge(gen, g);
					int recr_index = index + g.size();

					setCaret(0);

					__reload();
					//start reparsing from the indexes
					int end = g.size();
					while (index < end) {
						parse();
					}
					setCaret(recr_index);
				}
			}
		}


		bool CheckINBLibraryDecl() {
			//if (!inb::matchINBLibraryName(cur().value)) return false;
			if (Base::is(getNext(), ".")) {
				advance(2);
				return CheckINBLibraryDecl();
			}
			if(Base::is(getNext(), "(")) return isFuncCall();
		}
		bool isVariable() {
			for (auto& x : getVariables()) {
				if (x->getName() == cur().value) {
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
			for (auto& x : getVariables()) {
				if (x->getName() == t.value) {
					////std::cout << "name:" << name << std::endl;
					return true;
				}
				else {
					//std::cout << "continuing..." << std::endl;
				}
			}
			return false;
		}
		bool isBool() {
			return Base::is(cur(), "true") || Base::is(cur(), "false");
		}
		bool isBool(lexertk::token& l) {
			return Base::is(l, "true") || Base::is(l, "false");
		}
		Variable& getVariable(lexertk::token& t) {
			for (auto& x : getVariables()) {
				if (x->getName() == t.value)  {
					return *((Variable*)x.get());
				}
			}
			// add functionality for inb variables. will help a lot with inb lib in ember.
			std::string n("null");
			Variable v(n, nullptr, {});
			return v;
		}
		bool isFuncCall() {
			if (Base::is(getNext(), "(")) {
				int br = 1;
				for (int i = index + 2; i < gen.size(); i++) {
					if (Base::is(gen[i], "(")) br++;
					if (Base::is(gen[i], ")")) br--;
					if (br == 0) break;
				}
				if (br != 0) return false;
				return true;
			}
			return false;
		}
		bool isFuncCall(int index) {
			if (index + 2 > gen.size()) return false;
			if (Base::is(gen[index + 1], "(")) {
				int br = 1;
				for (int i = index + 2; i < gen.size(); i++) {
					if (Base::is(gen[i], "(")) br++;
					if (Base::is(gen[i], ")")) br--;
					if (br == 0) break;
				}
				if (br != 0) return false;
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

					if (!isOp(getNext())) {
						br = true;
					}

				}
				else if (isBool()) {
					Node n(cur(), Base::Decl::BOOL, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}
				}
				else if (isFloat(cur())) {
					Node n(cur(), Base::Decl::FLOAT, negate_next_node);
					operationStack.push_back(std::make_shared<Node>(n));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}

				}
				else if (isVariable()) {
					Variable& v = getVariable(cur());
					if (!Base::is(getNext(), ".") && !Base::is(getNext(), "[")) {
						std::shared_ptr<Variable> shared(&v);
						operationStack.push_back(std::make_shared<VariableNode>(shared, negate_next_node));

					}
					else {
						VContainer out = parseVariableUsage();
						operationStack.push_back(std::make_shared<VCNode>(out, negate_next_node));
					}
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						br = true;
					}
				}
				else if (isString(cur())) {
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
						negate_next_node = true;
					}
					else {
						operatorStack.push_back(Operator(cur()));
					}
				}
				else if (isLogicalOp(cur())) {
					//std::cout << "is logical:" << cur().value << std::endl;
					br = true;
				}
				else {
					std::string x("unknown token: " + cur().value);
					st_spr(x, STATUS::WARN);
				}

				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					std::shared_ptr<Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();

					operationStack.push_back(std::make_shared<Node>(left, op, right));
				}
				if (isLogicalOp(getNext())) advance();
				if (br)  break;
				advance();
			}
			return operationStack.back();

		}
		template<typename Type>
		Type parseExt(Base::Decl decl = Base::Decl::UNKNWN, std::any* take_raw = nullptr, Base::Decl* take_raw_decl = nullptr) {
			advance();
			// [TODO_] BREVAL NOT COMPLETE.
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
					else if (isBool()) {
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
			while (canAdvance()) {
				if (isInt(cur()) && (decl != Base::Decl::FLOAT)) {
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::INT, negate_next_node));
					if (negate_next_node) negate_next_node = false;

					if (!isOp(getNext())) {
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
				else if (isBool()) {
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::BOOL, negate_next_node));
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
					if (!Base::is(getNext(), ".") && !Base::is(getNext(), "[")) {
						operationStack.push_back(std::make_shared<VariableNode>(std::make_shared<Variable>(v), negate_next_node));
					}
					else {
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
					operationStack.push_back(std::make_shared<VCNode>(v, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(getNext())) {
						//std::cout << ("is not op4:" + getNext().value);
						br = true;
					}
				}
				else if (isOp(cur())) {
					if ((operationStack.size() < 1 || operatorStack.size() >= 1) && (Base::is(cur(), "-") || Base::is(cur(), "+"))) {
						negate_next_node = true;
					}
					else {
						operatorStack.push_back(Operator(cur()));
					}
				}
				else if (Base::is(cur(), "nothing")) {
					operationStack.push_back(std::make_shared<Node>(cur(), Base::Decl::NULLIFIED));
				}

				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					std::shared_ptr<Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();



					operationStack.push_back(std::make_shared<Node>(left, op, right));
				}
				if (br)  break;
				advance();
			}
			//should only have one node
			 
			std::shared_ptr<Node>& operation = operationStack.back();


			if (take_raw != nullptr) {
				auto x = operation->calc(); 
				*take_raw_decl = Node::getRootNodeType(operation.get(), true);
				*take_raw = x;
				return finalVal;
			}
			finalVal = std::any_cast<Type>(operation->calc());
			return finalVal;
		}
		template <typename Type>
		Type parseExt(int* i, std::any* take_raw = nullptr, Base::Decl* take_raw_decl = nullptr) {
			(*i)++;
			std::function<Node(bool)> brEval = [&](bool negate) -> Node {
				Node* left = nullptr, * right = nullptr;
				Operator* op = nullptr;
				bool negate_next_node_recr = false;
				while ((*i) + 1 < gen.size()) {
					//just encountered bracket
					(*i)++;

					// -(14 - -2)
					if (Base::is(gen[(*i)], "(")) {
						return brEval(negate_next_node_recr);
					}
					else if (isInt(gen[(*i)])) {
						if (left == nullptr) {
							left = new Node(gen[(*i)], Base::Decl::INT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(gen[(*i)], Base::Decl::INT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isFloat(gen[(*i)])) {
						if (left == nullptr) {

							left = new Node(gen[(*i)], Base::Decl::FLOAT, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(gen[(*i)], Base::Decl::FLOAT, negate_next_node_recr);

							if (negate_next_node_recr) negate_next_node_recr = false;
						}
					}
					else if (isString(gen[(*i)])) {
						if (left == nullptr) {
							left = new Node(gen[(*i)], Base::Decl::STRING, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(gen[(*i)], Base::Decl::STRING, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isBool(gen[(*i)])) {
						if (left == nullptr) {
							left = new Node(gen[(*i)], Base::Decl::BOOL, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;
						}
						else if (right == nullptr) {
							right = new Node(gen[(*i)], Base::Decl::BOOL, negate_next_node_recr);
							if (negate_next_node_recr) negate_next_node_recr = false;

						}
					}
					else if (isOp(gen[(*i)])) {
						if (Base::is(gen[(*i)], "-") && (left == nullptr || op != nullptr)) negate_next_node_recr = true;
						if (op == nullptr) op = new Operator(gen[(*i)]);
					}
					else if (isVariable()) {
						Variable v = getVariable(gen[(*i)]);
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
					if ((left != nullptr && right != nullptr && op != nullptr) || Base::is(gen[(*i)], ")")) {
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
			while ((*i) + 1 < gen.size()) {
				DEBUG("CHECKING:" + gen[(*i)].value);

				if (isInt(gen[(*i)])) {
					operationStack.push_back(std::make_shared<Node>(gen[(*i)], Base::Decl::INT, negate_next_node));
					if (negate_next_node) negate_next_node = false;

					if (!isOp(gen[(*i) + 1])) {
						//std::cout << ("is not op1:" + gen[(*i) + 1].value);
						br = true;
					}

				}
				else if (isFloat(gen[(*i)])) {
					operationStack.push_back(std::make_shared<Node>(gen[(*i)], Base::Decl::FLOAT, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(gen[(*i) + 1])) {
						br = true;
					}

				}
				else if (isBool(gen[(*i)])) {
					operationStack.push_back(std::make_shared<Node>(gen[(*i)], Base::Decl::BOOL, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(gen[(*i) + 1])) {
						br = true;
					}
				}
				else if (isString(gen[(*i)])) {
					operationStack.push_back(std::make_shared<Node>(gen[(*i)], Base::Decl::STRING, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(gen[(*i) + 1])) {
						br = true;
					}
				}
				else if (Base::is(gen[(*i)], "(")) {
					operationStack.push_back(std::make_shared<Node>(brEval(negate_next_node)));
					(*i)++;
					if (!isOp(gen[(*i)])) {
						br = true;
					}
					if (negate_next_node) negate_next_node = false;
				}
				else if (isVariable()) {
					Variable& v = getVariable(gen[(*i)]);
					if (!Base::is(gen[(*i) + 1], ".") && !Base::is(gen[(*i) + 1], "[")) {
						operationStack.push_back(std::make_shared<VariableNode>(std::make_shared<Variable>(v), negate_next_node));
					}
					else {
						VContainer out = parseVariableUsage();
						//if (&out == nullptr) throw marine::errors::RuntimeError("unexpected error while trying to access object.");
						operationStack.push_back(std::make_shared<VCNode>(out, negate_next_node));
					}
					if (negate_next_node) negate_next_node = false;
					if (!isOp(gen[(*i) + 1])) {
						br = true;
					}
				}
				else if (isFuncCall()) {
					marine::VContainer v = parseFuncCall<marine::VContainer>();
					operationStack.push_back(std::make_shared<VCNode>(v, negate_next_node));
					if (negate_next_node) negate_next_node = false;
					if (!isOp(gen[(*i) + 1])) {
						//std::cout << ("is not op4:" + gen[(*i) + 1].value);
						br = true;
					}
				}
				else if (isOp(gen[(*i)])) {
					if ((operationStack.size() < 1 || operatorStack.size() >= 1) && (Base::is(gen[(*i)], "-") || Base::is(gen[(*i)], "+"))) {
						negate_next_node = true;
					}
					else {
						operatorStack.push_back(Operator(gen[(*i)]));
					}
				}

				if (operatorStack.size() > 0 && operationStack.size() > 1) {
					std::shared_ptr<Node> right = operationStack.back();
					operationStack.pop_back();
					std::shared_ptr<Node> left = operationStack.back();
					operationStack.pop_back();
					Operator op = operatorStack.back();
					operatorStack.pop_back();



					operationStack.push_back(std::make_shared<Node>(left, op, right));
				}
				if (br)  break;
				(*i)++;
			}
			//should only have one node

			std::shared_ptr<Node>& operation = operationStack.back();


			if (take_raw != nullptr) {
				auto x = operation->calc();
				*take_raw_decl = Node::getRootNodeType(&*operation, true);
				*take_raw = x;
				return finalVal;
			}
			finalVal = std::any_cast<Type>(operation->calc());
			return finalVal;
		}
		template<typename Type>
		Type parseExtParam(Base::Decl decl = Base::Decl::UNKNWN, std::any* take = nullptr, Base::Decl* takeDecl = nullptr) {
			advance();
			//std::cout << getBefore(2).value << getBefore().value << cur().value << std::endl;
			Type finalVal{};
			bool negate_next_node = false;
			// x
			std::unique_ptr<Node> n = nullptr;
			Base::Decl determined = Base::Decl::UNKNWN;

			if (isInt(cur()) && decl != Base::Decl::FLOAT) {
				n = std::make_unique<Node>(cur(), Base::Decl::INT, negate_next_node);
				determined = Base::Decl::INT;
			}
			else if (isFloat(cur())) {
				n = std::make_unique<Node>(cur(), Base::Decl::FLOAT, negate_next_node);
				determined = Base::Decl::FLOAT;
			}
			else if (isBool()) {
				n = std::make_unique<Node>(cur(), Base::Decl::BOOL, negate_next_node);
				determined = Base::Decl::BOOL;
			}
			else if (isString(cur())) {
				n = std::make_unique<Node>(cur(), Base::Decl::STRING, negate_next_node);
				determined = Base::Decl::STRING;
			}
			else if (isVariable()) {
				if (!isVariableObjectUsage() && !Base::is(getNext(), "[")) {

					Variable& v = getVariable(cur());
					n = std::make_unique<VariableNode>(std::make_shared<Variable>(v), negate_next_node);
					determined = v.getDecl();
				}
				else {
					VContainer out = parseVariableUsage();
					n = std::make_unique<VCNode>(out);
					determined = out.getDecl();
				}
			}
			else if (isFuncCall()) {
				VContainer x = parseFuncCall<VContainer>();
				n = std::make_unique<VCNode>(x);
				determined = x.getDecl();
			}
			else {
				throw marine::errors::SyntaxError("expected parameter.");
			}
			if (take != nullptr) {
				*take = n->calc();
				*takeDecl = determined;
				return Type{};
				//[TODO] make seperate function for non type param finding.
			}
			finalVal = std::any_cast<Type>(n->calc());



			//should only have one node
			return finalVal;
		}
		Variable parseClassInstantiation(std::string& name, std::string& var_name) {
			std::shared_ptr<Class>& c = ClassHandler::getClassByName(name);
			if (Base::is(getNext(), "new")) {
				//creating a class variable by constructor

				if (advance(2).value != name) throw marine::errors::SyntaxError("variable class type is not equal to the 'new' constructor provided.");
				lexertk::token t = advance();
				int brc = t.value == "(" ? 1 : 0;
				
				if (brc > 0) {
					ClassStructure* structure = c->getStructure();
					//std::cout << "NAME: " <<c.get()->getName();
					return parseConstructorCall(structure, c.get(), name, var_name);
				}
				else {
					//no parameter constructor, validate that!
				}
				
			}
			else {
				//other ways of creating a class variable
				if (Base::is(getNext(), "nothing")) return Variable(var_name, nullptr, {});
				std::any raw;
				Base::Decl raw_decl;
				parseExt<std::any>(Base::Decl::STATIC_OBJECT, &raw, &raw_decl);

				// maybe function call, assert data types and return variable HERE.
				if (raw_decl != Base::Decl::STATIC_OBJECT) throw marine::errors::TypeError("Returned type from function does not match the assigned type.");

				Variable v(var_name, raw, {}); // the name is vital!
				v.setDecl(raw_decl);
				return v;
			}
			
		}
		ArrayList parseListDecl() {
			if (Base::is(getNext(), "[")) {
				advance();
				ArrayList _this;

				int c = 1;
				while (c > 0 && canAdvance()) {
					advance();
					if (Base::is(cur(), "[")) {
						c++;
						_this.add<ArrayList>(parseListDecl(), depth, Base::Decl::LIST);
					}
					else if (Base::is(cur(), "]")) c--;
					else if (isString(cur())) _this.add<String>(String(cur().value), depth, Base::Decl::STRING);
					else if (isInt(cur())) _this.add<int>(stoi(cur().value), depth, Base::Decl::INT);
					else if (isFloat(cur())) _this.add <float>(stof(cur().value), depth, Base::Decl::FLOAT);
					else if (isBool()) _this.add<bool>(stoi(cur().value), depth, Base::Decl::BOOL);
					else if (isVariable()) {
						VContainer v = parseVariableUsage();
						_this.add(v);
					}
					else if (isFuncCall()) {
						auto x = parseFuncCall<VContainer>();

						_this.add(x);
					}
					else if (!Base::is(cur(), ",")) {
						throw marine::errors::SyntaxError("expected ',' after creating list element.");
					}

				}
				return _this;
			}
			else {
				ArrayList l = parseExt<ArrayList>();
				return l;
			}
		}
		DynamicObject parseDynamicObjectDecl() {
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


				}
				else if (Base::is(cur(), "{")) c++;
				else if (Base::is(cur(), "}")) c--;


				if(c > 0)advance();
			}
			return _this;
		}
		Variable parseDecl(bool push = true) {
			lexertk::token& decl_raw = cur();
			Base::Decl type = Base::declareParse(cur());
			if (type == Base::Decl::UNKNWN) {
				if (ClassHandler::hasClass(cur().value.c_str())) type = Base::Decl::STATIC_OBJECT;
			}
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
			if (Base::is(getNext(), "=")) {
				advance();
				//IT IS VARIABLE
				//just handle numbers for now
				bool setDepth = false;
				Variable* ret = nullptr;
				switch (type) {
				case Base::Decl::INT: {
					int val = parseExt<int>();
					ret = new Variable(decl_name.value, val, std::to_string(val), conf);
					ret->setDecl(Base::Decl::INT);
					break;
				}
				case Base::Decl::FLOAT: {
					float val = parseExt<float>(Base::Decl::FLOAT);
					ret = new Variable(decl_name.value, val, std::to_string(val), conf);
					ret->setDecl(Base::Decl::FLOAT);
					break;
				}
				case Base::Decl::STRING: {
					String val = parseExt<String>();
					ret = new Variable(decl_name.value, val, val.get(), conf);
					ret->setDecl(Base::Decl::STRING);
					break;
				}
				case Base::Decl::DYNAMIC_OBJECT: {
					DynamicObject obj = parseDynamicObjectDecl();

					ret = new Variable(decl_name.value, obj, conf, Base::Decl::DYNAMIC_OBJECT);
					break;
				}
				case Base::Decl::LIST: {
					ArrayList list = parseListDecl();
					ret = new Variable(decl_name.value, list, conf, Base::Decl::LIST);
					break;
				}
				case Base::Decl::STATIC_OBJECT: {
					Variable v = parseClassInstantiation(decl_raw.value, decl_name.value);
					ret = new Variable(v);
					ret->setDecl(Base::Decl::STATIC_OBJECT);
					setDepth = true;
					break;
				}
				case Base::Decl::LAMBDA: {
					Lambda x = parseLambdaDecl();
					ret = new Variable(decl_name.value, x, conf, Base::Decl::LAMBDA);
					break;
				}
				default: {
					throw marine::errors::MError("variable type is unknown.");
				}
				}
				if(!setDepth) ret->setDepth(depth);
				auto x = ObjectHandler::getPrecomiledObject(ret);
				ret->setObjSelf(&x);
				if (push) scope.addVariable(*ret);

				return *ret;
			}
			else {
				//lone var with no value
				advance();
				Variable ret(decl_name.value, nullptr, conf);
				ret.setDecl(type);
				if (push)scope.addVariable(ret);
				return ret;
			}

		}
		/// <summary>
		/// returns an int that provides access to the parse function from 1 place,
		/// like a key. This is to prevent the flow of code elsewhere to continue,
		/// and we have 2 or more things trying to be parsed at once.
		/// </summary>
		void setParserLock(int key) {
			m_ParserLockKey = key;
		}
		void resetParserLock() {
			m_ParserLockKey = -1;
		}
		VContainer callLambda(Lambda* l, std::vector<VContainer>& p){
			while(m_ParserLockKey == 1) {}

			setParserLock(1);

			int c = index;
			if (!setCaret(l->getStart(), 1)) std::cout << ("COULD NOT SET CARET.\n");
			VContainer v(nullptr, depth, Base::Decl::UNKNWN);
			incDepth();


			while (index < l->getEnd()) {
				if (parse(&v, 1)) break;
			}


			setCaret(c);
			decDepth();


			resetParserLock();

			return v;
		}
		Lambda parseLambdaDecl() {
			lexertk::token start;
			lexertk::token end;

			int start_index = -1;
			int end_index = -1;

			int cbrcount = 0;
			bool foundbr = false;
			std::vector<Variable> parameters;
			incDepth();

			advance();
			if (Base::is(cur(), "(")) {
				if (!Base::is(advance(), ")")) {
					while (canAdvance()) {
						advance();
						parameters.push_back(parseDecl(false));
						if (Base::is(cur(), ")")) break;

						if (!Base::is(cur(), ",")) throw marine::errors::SyntaxError("invalid symbol after parameter decl.");

					}
				}
			}
			//we allow no parameters to be passed.
			//else throw marine::errors::SyntaxError("expected function parameter body ('(...)') after function declaration.");

			start_index = index + 1;
			start = getNext();


			while (canAdvance()) {
				advance();
				// [ERR] when importing, program thinks before the closing '}', it is end of file.
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


			decDepth();
			return Lambda(start, end, start_index, end_index, parameters);
		}
		bool isFuncDecl() {
			return (Base::is(cur(), "method") && Base::is(getNext(2), "("));

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
				else if (isString(gen[i])) ret.push_back(Base::Decl::STRING);
				else if (isBool(gen[i])) ret.push_back(Base::Decl::BOOL);
				else if (isVariable(gen[i])) {
					Variable& v = getVariable(gen[i]);
					if (!Base::is(gen[i + 1], ".") && !Base::is(gen[i + 1], "[")) {
						ret.push_back(v.getDecl());
					}
					else if (!Base::is(gen[i + 1], "[")) {
						if (!v.isDynamicObj()) {

							bool cont = true;
							while (gen[i + 1].value == ".") {
								if (v.getDecl() != Base::Decl::STATIC_OBJECT) {
									auto x = v.getObjSelf();
									if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
									auto y = x->getFunction(gen[i + 2].value);

									if (y != nullptr) {
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
								else {

									ClassInstance& instance = v.castRef<ClassInstance>();

									// [TODO_]
									if (isFuncCall(i)) {
										//parse function
										ClassStructure::ClassFunction* f = instance.getFunction(gen[i + 2].value);

										if (f != nullptr) {
											ret.push_back(Base::Decl::RUNTIME_DECIDED); // we dont know the return decl.
										}

									}
									else {
										//parse variable
										ClassStructure::ClassVariable* va = instance.getVariable(gen[i + 2].value);

										if (va != nullptr) {
											ret.push_back(Base::Decl::RUNTIME_DECIDED); // we dont know the return decl.
										}
									}
									i += 2;
								}
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
					else {
						//is list operator
						ArrayList& x = v.castRef<ArrayList>();
						i++;
						int y = parseExt<int>(&i);
						try {
							ret.push_back(x.get(y).getDecl());
						}
						catch (...) {
							throw marine::errors::IndexError("list index out of range.");
						}
					}
				}
				else if (isFuncCall(i)) {
					ret.push_back(Base::Decl::RUNTIME_DECIDED);	
				}
				
				//else if(brc == 1 && nbr_c == 0 && !Base::is(gen[i], ","))ret.push_back(Base::Decl::STRING);//[TODO add Base::Decl::CUSTOM]
				if (brc == 0 && nbr_c == 0) break;
			}
			return ret;
		}
		Variable parseConstructorCall(ClassStructure* structure, Class* parent, std::string& name, std::string& var_name) {
			std::vector<Base::Decl> allDecls;
			std::vector<Variable> variables;
			std::shared_ptr<Function>& f = structure->constructor->_this;
			ClassInstance instance = parent->instantiate();



			//CALL CONSTRUCTOR
			int br = 0;
			int param = 0;
			int nbr_i = 0;
			while (canAdvance()) {
				if (Base::is(cur(), "(")) br++;
				else if (Base::is(cur(), "[")) nbr_i++;
				else if (Base::is(cur(), ")")) br--;
				else if (Base::is(cur(), "]")) nbr_i--;
				if (Base::is(cur(), ",") && br == 1) {
					param++;
				}


				if (br == 0) break;
				if (f->parameters.size() == 0) { advance(); continue; }

				try {
					Variable& p = f->parameters[param];
					switch (p.getDecl()) {
					case Base::Decl::INT: {
						auto ret = parseExtParam<int>();
						p.setValue(ret);
						p.setTokenStr(ret);
						break;
					}
					case Base::Decl::FLOAT: {
						auto ret = parseExtParam<float>(Base::Decl::FLOAT);
						p.setValue(ret);
						p.setTokenStr(ret);
						break;
					}
					case Base::Decl::STRING: {
						String ret = parseExtParam<String>();
						p.setValue(ret);
						p.setTokenStr(ret.get());
						break;
					}
					case Base::Decl::LIST: {
						auto ret = parseListDecl();
						p.setValue(ret);
						break;
					}
					case Base::Decl::STATIC_OBJECT:
					{
						p.setValue(getVariable(cur()).getValue());
						break;
					}
					}
					allDecls.push_back(p.getDecl());
					variables.push_back(p);
					//scope.addVariable(p);
				}
				catch (std::exception& ig) {
					throw ig; 
					throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
				}

				advance();
			}
			//FUNC EXECUTION

			// vars should be set after, to allow the possibility of variadic parameters passed into the class constructor
			// we dont want to override our old variables just yet.
			scope.setLists(&instance);
			incDepth();
			//throw "TESTING";
			scope.addFunctionParameters(&variables);
			DEBUG("starting inits...");
			for (auto& _y : *structure->constructor->initializers) {
				Variable& id = getVariable(_y.initialized);
				Variable& ie = getVariable(_y.initializer);
				std::any& any = id.getValue();
				// for some reason std::make_any resolves a stack 0x0... error?!
				// std::any = operator needs value with no address apparently (&&)
				any = std::any(ie.getValue());
			}
			DEBUG("done inits.");
			advance();

			int c = index - 1; // -1 so that the next parse loop will advance to the token after the '}'


			if (!setCaret(f->getStart())) DEBUG("COULD NOT SET CARET.");
			while (index < f->getEnd()) {
				std::cout << "Cur:" << cur().value << '\n';
				if (parse()) {
					//if (return_parent != nullptr)*return_parent = true;
					break;
				}
			}
			decDepth();
			scope.popStack();
			setCaret(c);
			return Variable(var_name, instance, {}, depth);
		}
		template<typename Type>
		Type parseFuncCall(bool* return_parent = nullptr) {
			lexertk::token& name = cur();

			int br = 0;
			int param = 0;
			std::shared_ptr<Function> f;
			std::vector<Base::Decl> allDecls = {};
			std::vector<Variable> variables;

			if (isVariable(name)){
				auto& v = getVariable(name);
				if (v.getDecl() != Base::Decl::LAMBDA) throw marine::errors::SyntaxError("Attempted conversion from variable to lambda caused a problem.");
				
				f = std::make_shared<Lambda>(v.cast<Lambda>());
			}
			else {
				for (auto& x : scope.getFunctions()) {
					if (x->getName() == name.value) {
						f = x; break;
					}
				}
			}

			if (f == nullptr && ClassHandler::hasClass(name.value)) {
				f = ClassHandler::getClassByName(name.value)->getStructure()->constructor->_this;
			}

			if (f != nullptr) {
				incDepth();
				int nbr_i = 0;
				advance();
				while (canAdvance()) {
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					else if (Base::is(cur(), ")")) br--;
					else if (Base::is(cur(), "]")) nbr_i--;
					if (Base::is(cur(), ",") && br == 1) {
						param++;
					}


					if (br == 0) break;
					if (f->parameters.size() == 0) { advance(); continue; }

					try {
						Variable& p = f->parameters[param];
						switch (p.getDecl()) {
						case Base::Decl::INT: {
							auto ret = parseExtParam<int>();
							p.setValue(ret);
							p.setTokenStr(ret);
							break;
						}
						case Base::Decl::FLOAT: {
							auto ret = parseExtParam<float>(Base::Decl::FLOAT);
							p.setValue(ret);
							p.setTokenStr(ret);
							break;
						}
						case Base::Decl::STRING: {
							auto ret = parseExtParam<String>();
							p.setValue(ret);
							p.setTokenStr(ret.get());
							break;
						}
						case Base::Decl::LAMBDA: {
							//custom function MUST BE MADE. [TODO fix]
							if (!isVariable(getNext())) throw marine::errors::SyntaxError("Lambdas must be stored as variables in this current ember version. Dynamic creation of lambdas is not yet supported.");
							Variable& v = getVariable(advance());

							p.setValue(v.cast<Lambda>());

							break;
						}

						}
						allDecls.push_back(p.getDecl());
						variables.push_back(p);
						scope.addVariable(p);
					}
					catch (std::exception& ig) {
						throw ig;
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}

					advance();
				}

				//FUNC EXECUTION
				advance();
				int c = index - 1; // used to be index -1 so that the next parse loop will advance to the token after the '}'


				if (!setCaret(f->getStart())) DEBUG("COULD NOT SET CARET.");
				VContainer v(nullptr, depth, Base::Decl::UNKNWN);
				while (index < f->getEnd()) {

					if (parse(&v)) {
						if(return_parent != nullptr)*return_parent = true;

						decDepth();
						setCaret(c);


						return v;
					}
				}
				decDepth();
				setCaret(c);

				return v;
			}
			else {
				//inbuilt function
				//check for library decl ex inb.stuff.function()
				
				
				/*if (getVariable(cur()).getDecl() == Base::Decl::STATIC_OBJECT) {
					return parseVariableUsage();
				}*/

				std::vector<std::any> parameters;
				//call

				int nbr_i = 0;
				auto PredictedParameterTypes = predictParameterInbFuncCallTypes();


				inb::Callable* c = inb::getNoIncludeFunctionByName(name.value, PredictedParameterTypes);
				if (c == nullptr) throw marine::errors::SyntaxError("Function does not exist.");
				//check parameters
 				advance();
				while (canAdvance()) {
					if (Base::is(cur(), "(")) br++;
					else if (Base::is(cur(), "[")) nbr_i++;
					else if (Base::is(cur(), ")")) br--;
					else if (Base::is(cur(), "]")) nbr_i--;
					if (br == 0) break;
					if (Base::is(cur(), ",") && br == 1) {
						param++;
					}

					try {
						if (c->paramTypes.size() > param) {
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
							case Base::Decl::BOOL:
								parameters.push_back(parseExtParam<bool>());
								allDecls.push_back(Base::Decl::BOOL);
								break;
							case Base::Decl::LIST:
								parameters.push_back(parseExtParam<ArrayList>());
								allDecls.push_back(Base::Decl::LIST);
								break;
							case Base::Decl::LAMBDA:
							{
								auto& v = getVariable(advance());
								parameters.push_back(v.getValue());
								allDecls.push_back(Base::Decl::LAMBDA);
								break;
							}
							default: {
								st_spr(cur().value, marine::out::STATUS::WARN);
								break;
							}

							}
						}
						else if (c->parameter_count == -1) {
							//std::cout << "parsing parameter...";
							//unknown parameter!
							std::any raw;
							Base::Decl rawDecl;
							parseExtParam<std::any>(Base::Decl::UNKNWN, &raw, &rawDecl);

							switch (rawDecl) {
							case Base::Decl::INT:
								parameters.push_back(std::any_cast<int> (raw));
								allDecls.push_back(Base::Decl::INT);
								break;
							case Base::Decl::FLOAT:
								parameters.push_back(std::any_cast<float> (raw));
								allDecls.push_back(Base::Decl::FLOAT);
								break;
							case Base::Decl::BOOL:
								parameters.push_back(parseExtParam<bool>());
								allDecls.push_back(Base::Decl::BOOL);
								break;
							case Base::Decl::STRING:
								parameters.push_back(std::any_cast<String> (raw));
								allDecls.push_back(Base::Decl::STRING);
								break;
							case Base::Decl::LIST:
								parameters.push_back(std::any_cast<ArrayList> (raw));
								allDecls.push_back(Base::Decl::LIST);
								break;
							case Base::Decl::DYNAMIC_OBJECT:
								parameters.push_back(std::any_cast<DynamicObject> (raw));
								allDecls.push_back(Base::Decl::DYNAMIC_OBJECT);
								break;
							}
						}
					}
					catch (std::exception& ig) {
						throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
					}
					advance();
				}
				VContainer returnValue;
				c->call(parameters, &returnValue, &allDecls);

				auto x = ObjectHandler::getPrecomiledObject(returnValue.type());
				returnValue.setObjSelf(&x);

				return returnValue;
			}
	}
	Function skipFuncDecl(bool push = true) {
		lexertk::token start;
		lexertk::token end;

		std::string name = advance().value;
		int start_index = -1;
		int end_index = -1;

		int cbrcount = 0;
		bool foundbr = false;
		std::vector<Variable> parameters;
		incDepth();

		advance();

		if (Base::is(cur(), "(")) {
			if (!Base::is(getNext(), ")")) {
				while (canAdvance()) {
					advance();
					parameters.push_back(parseDecl(false));
					if (Base::is(cur(), ")")) break;

					if (!Base::is(cur(), ",")) throw marine::errors::SyntaxError("invalid symbol after parameter decl.");

				}
			}
		}
		else throw marine::errors::SyntaxError("expected function parameter body ('(...)') after function declaration.");

		start_index = index + 1;
		start = getNext();


		while(canAdvance()){
			advance();
			// [ERR] when importing, program thinks before the closing '}', it is end of file.
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


		decDepth();
		Function func(name, start, end, start_index, end_index, parameters);
		//std::cout << "adding function to: " << scope.getCurrentModule()->getName() << '\n';
		if (push) scope.addFunction(func);
		return func;
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
	BoolExpr* parseRawLogicalExpr() {
		std::shared_ptr<Node> left = parseBoolExprExt();

		Operator c(cur());

		advance();

		std::shared_ptr<Node> right = parseBoolExprExt();

		return new BoolExpr(left, c, right);

	}
	void parseLogicalStatement() {
		advance();
		if (!Base::is(cur(), "(")) throw marine::errors::SyntaxError("expected '(' after if statement.");
		advance();

		BoolExpr* expr = parseLogicalExpr();

		if (!expr->evaluate()) {
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
	void modifyVariable(Variable* v, T val, lexertk::token* token = nullptr, Base::Decl d = Base::Decl::UNKNWN) {
		v->getValue() = val;
		//assert same types of declaration

		if (token != nullptr)v->setToken(*token);
	
	}
	template <typename T>
	void modifyVContainer(VContainer* v, Operator o, T val, lexertk::token* token = nullptr, Base::Decl d = Base::Decl::UNKNWN) {
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
	marine::VContainer parseClassFunctionReference(ClassInstance* structure) {
		auto& f = structure->getFunction(cur().value)->_this;

		int br = 0;
		int param = 0;

		incDepth();
		std::vector<Variable> variables;
		std::vector<Base::Decl> allDecls;

		int nbr_i = 0;


		advance();
		while (canAdvance()) {
			if (Base::is(cur(), "(")) br++;
			else if (Base::is(cur(), "[")) nbr_i++;
			else if (Base::is(cur(), ")")) br--;
			else if (Base::is(cur(), "]")) nbr_i--;
			if (Base::is(cur(), ",") && br == 1) {
				param++;
			}


			if (br == 0) break;
			if (f->parameters.size() == 0) { advance(); continue; }

			try {
				Variable& p = f->parameters[param];
				switch (p.getDecl()) {
				case Base::Decl::INT: {
					auto ret = parseExtParam<int>();
					p.setValue(ret);
					p.setTokenStr(ret);
					break;
				}
				case Base::Decl::FLOAT: {
					auto ret = parseExtParam<float>(Base::Decl::FLOAT);
					p.setValue(ret);
					p.setTokenStr(ret);
					break;
				}
				case Base::Decl::STRING: {
					auto ret = parseExtParam<String>();
					p.setValue(ret);
					p.setTokenStr(ret.get());
					break;
				}
				case Base::Decl::LAMBDA: {

					Lambda l = getVariable(advance()).castRef<Lambda>();

					p.setValue(l);
					break;
				}
				}
				allDecls.push_back(p.getDecl());
				variables.push_back(p);
				scope.addVariable(p);
			}
			catch (std::exception& ig) {
				throw ig;
				throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
			}

			advance();
		}
		scope.setLists(structure);
		for (auto& x : variables) scope.addVariable(x);
		//FUNC EXECUTION
		advance();

		int c = index - 1; // -1 so that the next parse loop will advance to the token after the '}'


		if (!setCaret(f->getStart())) DEBUG("COULD NOT SET CARET.");
		VContainer v(nullptr, depth, Base::Decl::UNKNWN);
		while (index < f->getEnd()) {

			if (parse(&v)) {
				//if (return_parent != nullptr)*return_parent = true;

				decDepth();
				setCaret(c);


				return v;
			}
		}
		decDepth();
		setCaret(c);

		scope.popStack();
		return v;


	}
	marine::VContainer parseObjectFunctionReference(ValueHolder* v) {
		auto x = v->getObjSelf();
		if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
		std::vector<std::any> parameters;
		std::vector<Base::Decl> allDecls;


		advance();
		if (x->hasFunction(cur().value)) {
			int br = 0;
			int nbr_i = 0;
			int param = 0;
			auto* c = x->getFunction(cur().value);
			advance();
			while (canAdvance()) {
				if (Base::is(cur(), "(")) br++;
				else if (Base::is(cur(), "[")) nbr_i++;
				if (Base::is(cur(), ")")) br--;
				else if (Base::is(cur(), "]")) nbr_i--;
				if (Base::is(cur(), ",") && br == 1) {
					param++;
				}
				if (br == 0) break;
				if (c->paramTypes.size() == 0) { advance(); continue; }
				try {
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
					case Base::Decl::RUNTIME_DECIDED: {
						std::any take;
						Base::Decl takeDecl;
						parseExtParam<std::any>(Base::Decl::UNKNWN, &take, &takeDecl);
						parameters.push_back(take);
						allDecls.push_back(takeDecl);
						break;
					}
					default:
					{
						std::string m(Base::declStr(c->paramTypes[param]));
						st_spr(m, marine::out::STATUS::WARN);
						break;
					}
					}
				}
				catch (std::exception& ig) {
					throw ig;
					throw marine::errors::IndexError("function parameter either does not exist or you have exceeded the amount of parameters the function is asking for.");
				}

				advance();

			}
			VContainer returnValue;
			c->call(parameters,v, &returnValue, &allDecls);
			//do something with return value?
			auto x = ObjectHandler::getPrecomiledObject(returnValue.type());
			returnValue.setObjSelf(&x);
			return returnValue;
		}
		return VContainer::null();
	}
	marine::ClassStructure::ClassVariable* parseClassVariableReference(ClassInstance& instance) {

		auto* x = instance.getVariable(cur().value);
		return x == nullptr ? nullptr : (ClassStructure::ClassVariable*)x;
	}

	marine::ObjectVariable* parseObjectVariableReference(ValueHolder* v) {
		//if (ObjectHandler::isPrecompiledObject(v)) {
		//}
		auto x = v->getObjSelf();
		if (x->getName() == "NULL") throw marine::errors::SyntaxError("operator '.' cannot be performed on a non object like variable. this var type can only hold its value. all types will be converted to object upon release.");
		if (x->hasVariable(cur().value)) {
			if (!Base::is(getNext(), ".")) return x->getVariable(cur().value);
		}

		return nullptr;
	}
	
	marine::ValueHolder& parseObjectReference(ValueHolder* v) {
		//cur should be '.'

		advance();
		if (v->getDecl() != Base::Decl::STATIC_OBJECT) {
			if (isFuncCall()) {
				VContainer out = parseObjectFunctionReference(v);
				if (!Base::is(advance(), ".")) return out;

				return parseObjectReference(&out);
			}
			ObjectVariable* out = parseObjectVariableReference(v);
			if (!Base::is(advance(), ".")) return out->get();



			return parseObjectReference(&out->get());
		}
		else {
			// is class reference
			ClassInstance& x = v->castRef<ClassInstance>();
			
			if (isFuncCall()) {
				//class method invoking
			}
			ClassStructure::ClassVariable* mem = parseClassVariableReference(x);
			if (!Base::is(advance(), ".")) {
				Variable& value = *mem->defaultedValue.get();
				return value;
			}
			Variable& value = *mem->defaultedValue.get();
			return parseObjectReference(&value);
		}

	}
	bool isVariableObjectUsage() {
		return (Base::is(getNext(), "."));
	}
	VContainer parseSafeVariableUsage() {
		Variable* v = &getVariable(cur());
		if (v->getName() == "NULL") throw marine::errors::SyntaxError("unknown token");
		if (isOp(getNext())) {
			Operator op(advance());
			switch (v->getDecl()) {
			case Base::Decl::INT:
			{
				auto x = parseExt<int>();
				switch (op.getType()) {
				case Operator::OPTYPE::ADD_S:
					return VContainer(v->cast<int>() + x, v->getDepth(), v->getDecl());
				case Operator::OPTYPE::SUB_S:
					return VContainer(v->cast<int>() - x, v->getDepth(), v->getDecl());
				default:
					return VContainer(x, v->getDepth(), v->getDecl());
				}
			}
			case Base::Decl::FLOAT:
			{
				auto x = parseExt<float>();
				switch (op.getType()) {
				case Operator::OPTYPE::ADD_S:
					return VContainer(v->cast<float>() + x,v->getDepth(), v->getDecl());
				case Operator::OPTYPE::SUB_S:
					return VContainer(v->cast<float>() - x, v->getDepth(), v->getDecl());
				default:
					return VContainer(x, v->getDepth(), v->getDecl());
				}
			}
			case Base::Decl::STRING:
			{
				String x = parseExt<String>();
				lexertk::token t(x.get());
				return VContainer(v->cast<String>() + x, v->getDepth(), v->getDecl());
			}
			case Base::Decl::BOOL:
				return VContainer(v, v->getDepth(), v->getDecl());
			case Base::Decl::LIST:
				return VContainer(v, v->getDepth(), v->getDecl());
			}
			return VContainer::null();
		}
		else if (Base::is(getNext(), ".")) {
			if (!v->isDynamicObj()) {
				advance();
				//check if is function first
				VContainer x = parseObjectFunctionReference(v);

				if (Base::is(getNext(), ".")) {

					advance();
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

				if (!d.has(name)) throw marine::errors::RuntimeError("object does not contain value.");

				iter = d.get(name);

				if (iter->second.isDynamicObj()) {
					d = iter->second.cast<DynamicObject&>();
					advance(2);
				}
				else if (iter->second.isDynamicObj() && Base::is(getNext(), ".")) {
					// we are accessing another object that is a connected STATIC object, not a dynamic object.
					advance();
					return parseObjectFunctionReference(&iter->second);
				}

				if (!isOp(getNext())) return iter->second;
				else {

					auto* vc = &iter->second;
					Operator op(advance());
					switch (vc->getDecl()) {
					case Base::Decl::INT:
						modifyVContainer<int>(vc, op, parseExt<int>());
						break;
					case Base::Decl::FLOAT:
						modifyVContainer<float>(vc, op, parseExt<float>());
						break;
					case Base::Decl::STRING:
					{
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

	}
	VContainer parseVariableUsage() {
		Variable* v = &getVariable(cur());
		if (v->getName() == "NULL") throw marine::errors::SyntaxError("unknown token.");
		if (isOp(getNext())) {
			Operator op(advance());
			switch(v->getDecl()){
			case Base::Decl::INT: 
			{
				auto x = parseExt<int>();
				switch (op.getType()) {
				case Operator::OPTYPE::ADD_S:
					modifyVariable<int>(v, v->cast<int>() + x);
					break;
				case Operator::OPTYPE::SUB_S:
					modifyVariable<int>(v, v->cast<int>() - x);
					break;
				default:
					modifyVariable<int>(v, x);
					break;
				}
				break;
			}
			case Base::Decl::FLOAT:
			{
				auto x = parseExt<float>();
				switch (op.getType()) {
				case Operator::OPTYPE::ADD_S:
					modifyVariable<float>(v, v->cast<float>() + x);
					break;
				case Operator::OPTYPE::SUB_S:
					modifyVariable<float>(v, v->cast<float>() - x);
					break;
				default:
					modifyVariable<float>(v, x);
					break;
				}
				break;
			}
			case Base::Decl::STRING:
			{
				String x = parseExt<String>();
				lexertk::token t(x.get());
				modifyVariable<String>(v, x, &t);
				break;
			}
			case Base::Decl::BOOL:
				modifyVariable<bool>(v, parseExt<bool>());
				break;
			case Base::Decl::LIST:
				modifyVariable<ArrayList>(v, parseExt<ArrayList>());
				break;
			}
			return VContainer::null();
		}
		else if (Base::is(getNext(), ".")) {
			if (!v->isDynamicObj()) {
				advance();
				//check if is function first
				//std::cout <<("CUR:") << (cur().value);
				if (v->getDecl() != Base::Decl::STATIC_OBJECT) {
					if (isFuncCall(index + 1)) {
						VContainer x = parseObjectFunctionReference(v);
						if (Base::is(getNext(), ".")) {

							advance();
							VContainer v = parseObjectFunctionReference(&x);
							DEBUG("CUR RECR OBJECT FUNC REFERENCE: " + cur().value);
						}
						return x;
					}
					else {
						//its variable
						ObjectVariable* ov = parseObjectVariableReference(v);
						if (Base::is(getNext(), ".")) {

							advance();
							ObjectVariable* ov1 = parseObjectVariableReference(&ov->get());
							return VContainer(ov1->get().getValue(), depth, ov1->get().getDecl());
						}
						return VContainer(ov->get().getValue(), depth, ov->get().getDecl());
					}
				}
				else {
						// [TODO_]
					if (isFuncCall(index + 1)) {
						advance();
						ClassInstance& instance = v->castRef<ClassInstance>();
						VContainer v = parseClassFunctionReference(&instance);
						//check for recursions of object func

						return v;
					}
					else {
						// is variable
						advance();
						ClassInstance& instance = v->castRef<ClassInstance>();
						ClassStructure::ClassVariable* x = parseClassVariableReference(instance);
						
						if (Base::is(getNext(), ".")) {
							// [TODO_] SUPPORT FOR RECURSIVE CLASS / OBJECT NEEDS TO BE HERE.
						}
						Variable& vc = *x->defaultedValue.get();
						return VContainer(*x->defaultedValue.get(), vc.getDepth(), vc.getDecl());
					}
					
				}
			}
			else {
				//accessing dynamic object item
				advance(2);
				std::unordered_map<std::string, VContainer>::iterator iter;
				DynamicObject& d = v->castRef<DynamicObject>();

				std::string& name = cur().value;

				if (!d.has(name)) throw marine::errors::RuntimeError("object does not contain value.");

				iter = d.get(name);

				if (iter->second.isDynamicObj()) {
					d = iter->second.cast<DynamicObject&>();
					advance(2);
				}
				else if (iter->second.isDynamicObj() && Base::is(getNext(), ".")) {
					// we are accessing another object that is a connected STATIC object, not a dynamic object.
					advance();
					return parseObjectFunctionReference(&iter->second);
				}


				if(!isOp(getNext())) return iter->second;
				else {

					auto* vc = &iter->second;
					Operator op(advance());
					switch (vc->getDecl()) {
					case Base::Decl::INT:
						modifyVContainer<int>(vc, op, parseExt<int>());
						break;
					case Base::Decl::FLOAT:
						modifyVContainer<float>(vc, op, parseExt<float>());
						break;
					case Base::Decl::STRING:
					{
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
		else if (Base::is(getNext(), "[")) {
			advance();
			int x = parseExt<int>();
			advance();
			switch (v->getDecl()) {
			case Base::Decl::LIST:
				return v->castRef<ArrayList>().get(x);
			}
		}
		//check for usage of '.'
	}
	bool isReturnStatement() { return (Base::is(cur(), "return")); }
	bool parseForStatement() {

		if (!Base::is(advance(), "(")) throw marine::errors::SyntaxError("expected '(' after for declaration. ");
		lexertk::token& x = getNext(3);
		if (!Base::is(x, "in") && !Base::is(x, "of")) {
			int brc = 1;
			BoolExpr* eval = nullptr;
			int ccount = 0;
			incDepth();
			int oper_start = 0, oper_end = 0;
			while (brc > 0 && canAdvance()) {
				advance();
				if (Base::is(cur(), "(")) brc++;
				else if (Base::is(cur(), ")")) brc--;
				else if (brc == 0) break;
				else if (Base::is(cur(), ";")) ccount++;
				//parse loop statement here
				else if (isDecl() && ccount == 0) {
					parseDecl();
				}
				else if (ccount == 1) {

					eval = parseRawLogicalExpr();

				}
				else if (ccount == 2) {
					oper_start = index;

					int c = 1;
					while (canAdvance()) {

						if (Base::is(getNext(), "(")) c++;
						else if (Base::is(getNext(), ")")) c--;
						if (c == 0) break;

						advance();

					}
					oper_end = index;
				}
			}

			brc++;

			if (!Base::is(advance(), "{")) throw marine::errors::SyntaxError("expected '{' after for loop declaration.");

			int indexstart = index;

			int endindex = 0;

			advance();

			while (brc > 0 && canAdvance()) {
				if (Base::is(cur(), "{")) brc++;
				else if (Base::is(cur(), "}")) brc--;
				//parse contained code here
				if (brc < 1) {
					endindex = index;
				}
				advance();
			}

			if (eval != nullptr) {
				int c = 0;
				while (eval->evaluate()) {

					//if (index >= endindex) continue;
					c++;

					setCaret(indexstart);
					while (canAdvance() && index < endindex) {
						if (parse()) return true; // true means we need to return from the parent function, if it exists.
					}

					if (!eval->evaluate()) break;


					setCaret(oper_start);

					VContainer val = parseSafeVariableUsage();

					if (!eval->evaluateExt(&val)) break;

					setCaret(oper_start);

					parseVariableUsage();

				}
				decDepth();
				setCaret(endindex);
			}
			return false;
		}
		else {
			Base::Decl conv = Base::declareParse(advance());
			lexertk::token& t = advance();
			Variable iter(t.value, nullptr, cur(), {});
			iter.setDecl(conv);
			std::any res;
			Base::Decl resDecl;
			advance();
			parseExt<std::any>(Base::Decl::UNKNWN, &res, &resDecl);
			if (Base::is(x, "in")) {
				switch (resDecl) {
				case Base::Decl::LIST: {
					ArrayList x = std::any_cast<ArrayList>(res);

					if (!Base::is(advance(), ")")) throw marine::errors::SyntaxError("expected ')' after opening bracket.");
					if (!Base::is(advance(), "{"))throw marine::errors::SyntaxError("expected '{' after opening bracket.");
					int _iter = 0;
					int brc = 1;
					int indexstart = index;
					int endindex = 0;

					advance();

					while (brc > 0 && canAdvance()) {
						if (Base::is(cur(), "{")) brc++;
						else if (Base::is(cur(), "}")) brc--;
						//parse contained code here
						if (brc < 1) {
							endindex = index;
						}
						advance();
					}
					incDepth();
					iter.setDepth(depth);
					scope.addVariable(iter);
					Variable* insertedIter = &getVariable(t);
					while (_iter < x.length()) {
						VContainer z = x.get(_iter);
						//std::cout << "SETTING DECL FROM:" << Base::declStr(insertedIter->getDecl()) << ", TO: " << Base::declStr(z.getDecl());
						if (conv != z.getDecl() && conv != Base::Decl::RUNTIME_DECIDED) throw marine::errors::TypeError("for loop variable type did not equal the type of an element in the iterable. specify type 'any' to iterate through multi type arrays.");
						insertedIter->setValue(z.get());
						insertedIter->setDecl(z.getDecl());
						//std::cout << "new decl:" << Base::declStr(insertedIter->getDecl());
						//if (index >= endindex) continue;
						setCaret(indexstart);
						while (canAdvance() && index < endindex) {
							if (parse()) return true; // true means we need to return from the parent function, if it exists.
						}
						_iter++;
					}
					decDepth();
					setCaret(endindex);
					return false;
				}
				case Base::Decl::STRING:
					break;
				}

				return false;
			}
			else {
				//object iteration
			}
		}
	}
	ClassStructure::ClassConstructor parseClassConstructor() {
		if (!Base::is(advance(), "(")) throw marine::errors::SyntaxError("expected '(' after constructor declaration.");
		lexertk::token start;
		lexertk::token end;

		int start_index = -1;
		int end_index = -1;

		int cbrcount = 0;
		bool foundbr = false;

		std::vector<Variable> parameters;
		while (canAdvance()) {
			advance();
			parameters.push_back(parseDecl(false));

			if (Base::is(cur(), ")")) break;

			if (!Base::is(cur(), ",")) throw marine::errors::SyntaxError("invalid symbol after parameter decl.");

		}
		std::vector<ClassStructure::ClassConstructor::Initializer> initializers;
		DEBUG("next:" + getNext().value);
		if (Base::is(getNext(), "=>")) {
			advance();
			do {
				lexertk::token& initialized = advance();
				for (auto& z : parameters) {
					if (z.getName() == cur().value) throw marine::errors::SyntaxError("cannot initialize parameter variable in constructor. try changing the name of your class members, so that they are not the same as the constructor parameters.");
				}
				if (!Base::is(advance(), "(")) throw marine::errors::SyntaxError("expected '(' after class member smart initialization. usage: func([TYPE] t) => member(t) {}");
				lexertk::token& initalizer = advance();
				initializers.push_back({initialized, initalizer}); // TODO: add support for the 'this' keyword.
				if (!Base::is(advance(), ")")) throw marine::errors::SyntaxError("missing ')' after class member smart initialization. usage: func([TYPE] t) => member(t) {}");
				
				advance();

			} while (Base::is(cur(), ","));
			DEBUG("end:" + cur().value);
		}
		
		
		start_index = index + 1;
		start = getNext();
		
		while (canAdvance()) {
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
			advance();
		}
		if (end_index == -1) throw marine::errors::MError("something unexpected happened.");


		return ClassStructure::ClassConstructor(ClassStructure::MEMBERPROTECTION::PUBLIC,
			&initializers, std::make_shared<Function>(Function("constructor", start, end, start_index, end_index, parameters)));
	}
	void parseClassObject() {
		lexertk::token& name = advance();

		if (ClassHandler::hasClass(name.value)) throw marine::errors::SyntaxError("class name already exists.");
		if (!Base::is(advance(), "{")) throw marine::errors::SyntaxError("expected '{' after class declaration");

		signed int brc = 1;
		int curProtType = 0;

		Class _obj(name.value.c_str());
		DEBUG("creating class");
		DEBUG(_obj.getName());


		while (brc > 0 && canAdvance()) {
			DEBUG("checking: " + cur().value);
			advance();
			if (Base::is(cur(), "{")) brc++;
			else if (Base::is(cur(), "}")) brc--;
			if (Base::is(cur(), "[")) {
				lexertk::token& protection = advance();
				if (!Base::is(advance(), "]")) throw marine::errors::SyntaxError("expected ']' after class protection specifier.");
				int curProtType = (protection.value == "public" ? 0 :
					protection.value == "private" ? 1 :
					protection.value == "protected" ? 2 :
					protection.value == "internal" ? 3 : -1);
				if (curProtType == -1) throw marine::errors::SyntaxError("exepected 'public', 'private', 'protected' or 'internal' in a class protection specifier.");
			}
			else if (Base::is(cur(), "constructor")) {
				DEBUG("PARSING CONSTRUCTOR AT:" + cur().value);
				ClassStructure::ClassConstructor construct = parseClassConstructor();
				_obj.setConstructor(construct);
			}
			else if (isDecl()) {
				Variable x = parseDecl(false);
				ClassStructure::ClassVariable var(x.getName(), (ClassStructure::MEMBERPROTECTION)curProtType, x.getDecl(), std::make_shared<Variable>(x));
				_obj.addNewVariable(var);
			}
			else if (isFuncDecl()) {
				Function func = skipFuncDecl(false);
				ClassStructure::ClassFunction function(func.getName(), (ClassStructure::MEMBERPROTECTION)curProtType, false, std::make_shared<Function>(func));
				_obj.addFunction(function);
			}
		}
		DEBUG(cur().value);
		if (!Base::is(cur(), "}")) throw marine::errors::SyntaxError("expected ending '}' after class declaration.");
		ClassHandler::addClass(_obj);
		DEBUG("end:" + cur().value);
	}
	void parseModule() {
		lexertk::token& name = advance();

		//check string for invalid symbols
		Module* current = scope.getCurrentModule().get();
		std::shared_ptr<Module> current_copy = nullptr;
		if (current == nullptr) {
			//first module

			std::shared_ptr<Module> ptr = std::make_shared<Module>(name.value);
			scope.newModule(ptr);
			scope.setCurrentModule(ptr);
		}
		else {
			//std::cout << "in module " << current->getName() << ", making child:" << name.value << "\n";
			// we are in a module currently
			std::shared_ptr<Module> ptr = std::make_shared<Module>(name.value);
			current->addChildModule(ptr);

			// we copy the instance, dont worry! im pretty sure the code still only keeps 1 pointer at once.
			current_copy = std::make_shared<Module>(*current);
			scope.setCurrentModule(ptr);

		}
		if (!Base::is(advance(), "{")) throw marine::errors::SyntaxError("expected opening '{' brace after module declaration.");
		int brc = 1;
		while (brc > 0 && canAdvance()) {
			parse();
			if (Base::is(cur(), "}")) brc--;
			else if (Base::is(cur(), "{")) brc++;
		}
		if (brc != 0) throw marine::errors::SyntaxError("could not find closing module brace.");
		
		scope.setCurrentModule(current_copy);

		//std::cout << "current module:" << scope.getCurrentModule()->getName() << "\n";
	}
	ValueHolder parseModuleUsage() {
		std::vector<std::string*> traverser;

		traverser.push_back(&cur().value);
		while (canAdvance() && Base::is(getNext(), ":")) {
			traverser.push_back(&advance(2).value);
		}
		Module* current = scope.getCurrentModule().get();

		if (current != nullptr) {
			current = current->getChildModule(*traverser[0]);
		}


		// TEMP -- we find the module by looking in the parent dir of modules (ie: the global scope)
		if (current == nullptr) current = scope.getModules()[*traverser[0]].get();

		if (current == nullptr) throw marine::errors::SyntaxError("Module not found.");
		if (traverser.size() > 1) {
			traverser.erase(traverser.begin());
			for (std::string*& x : traverser) {
				Module* to = current->getChildModule(*x);
				if (to == nullptr) break;
				current = to;
			}

		}
		// ADVANCE CALL WAS HERE, removed to allow sub modules to function.
		return scope.enterModuleTemporarily<ValueHolder>(current, [&]() -> ValueHolder {

			if (isVariable()) {
				if (!isVariableObjectUsage() && !Base::is(getNext(), "[")) {
					return getVariable(cur());
				}
				else {
					return parseVariableUsage();
				}
			}
			else if (isFuncCall()) {
				return parseFuncCall<VContainer>();
			}

			});


	}
	bool isLambdaDecl() {
		// lambda (...?)? {...}
		return (Base::is(cur(), "lambda"));
	}
#pragma endregion
	bool parse(ValueHolder* v = nullptr, const int lockKey = -1) {
		if (lockKey != m_ParserLockKey && m_ParserLockKey != -1) return false;
		if (isDecl()) {
			parseDecl();
		}
		else if (module_isDecl()) {
			//we are doing something inside a module.
			parseModuleUsage();
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
		else if (Base::is(cur(), "use") || Base::is(cur(), "from")) {
			_import();
		}
		else if (Base::is(cur(), "class")) {
			parseClassObject();
		}
		else if (Base::is(cur(), "module")) {
			parseModule();
		}
		else if (isForStatement()) {
			parseForStatement();
		}
		else if (isIfStatement()) {
			parseLogicalStatement();
		}
		else if (isWhileStatement()) {
			parseWhileStatement();
		}
		else if (isFuncCall()) {
			parseFuncCall<std::any>();
		}
		else if (isVariable()) {
			parseVariableUsage();
		}
		else if (isFuncDecl()) {
			skipFuncDecl();
		}
		//st_spr(cur().value, marine::out::STATUS::WARN);
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

		for (auto& x : getVariables()) {
			//std::cout << "VARIABLE:" << x.str() << std::endl;
		}
	}
	lexertk::token& cur() {
		return gen[index];
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
		if (index - x >= gen.size()) return current;

		return gen[index - x];
	}


	bool setCaret(int i, int lock = -1) {

		if (lock != m_ParserLockKey && m_ParserLockKey != -1) return false;

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