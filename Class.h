#pragma once
#include <vector>
#include "Base.h"
#include "lexertk.hpp"
#include "VContainer.h"
#include "inb.h"
namespace marine {
	struct ClassStructure {
		enum class MEMBERPROTECTION {
			PUBLIC,
			PRIVATE,
			PROTECTED,
			INTERNAL
		};
		struct ClassMember {
			MEMBERPROTECTION protection;
			bool var = true;
			ClassMember(MEMBERPROTECTION prot, bool isvar = true): protection(prot), var(isvar){
			}
		};
		struct ClassVariable : public ClassMember {
			Base::Decl decl;
			std::shared_ptr<marine::Variable> defaultedValue = nullptr;
			ClassVariable(MEMBERPROTECTION p, Base::Decl d, std::shared_ptr<marine::Variable> def = nullptr)
				: ClassMember(p), decl(d), defaultedValue(def) {
			

			}
		};
		struct ClassFunction : public ClassMember {
			bool constructor = false;
			std::shared_ptr<marine::Function> _this = nullptr;
			ClassFunction(MEMBERPROTECTION p, bool c, std::shared_ptr<marine::Function> def = nullptr)
				: ClassMember(p, false), constructor(c), _this(def) {
			
			}
		};
		struct ClassConstructor : public ClassFunction {
			struct Initializer {
				lexertk::token initialized;
				lexertk::token initializer;

				bool initialized_is_this = true;
				bool initializer_is_this = false;

			};
			typedef std::shared_ptr<std::vector<Initializer>> InitializerList;
			InitializerList initializers;
			ClassConstructor(MEMBERPROTECTION p = MEMBERPROTECTION::PUBLIC, std::vector<Initializer>* i = nullptr, std::shared_ptr<marine::Function> def = nullptr)
				: ClassFunction{p, true, def }, initializers(std::make_shared<std::vector<Initializer>>(*i)) {}
		};
		std::unordered_map<std::string, std::shared_ptr<ClassMember>> members;
		std::shared_ptr<ClassConstructor> constructor = nullptr;

	};
	class ClassInstance {
		typedef std::unordered_map<std::string, std::shared_ptr<ClassStructure::ClassMember>> MemberList;
	private:
		MemberList members;
	public:
		ClassInstance(MemberList& mem): members(mem){}
		ClassInstance(){}
		void setMembers(MemberList& mem) {
			members = mem;
		}
		MemberList* getMembers() {
			return &members;
		}
		ClassStructure::ClassFunction* getFunction(std::string& s) {
			auto f = members.find(s);

			if (f != members.end()) return (ClassStructure::ClassFunction*)f->second.get();
			return nullptr;
		}
		ClassStructure::ClassVariable* getVariable(std::string& s) {
			auto f = members.find(s);

			if (f != members.end()) return (ClassStructure::ClassVariable*)f->second.get();
			return nullptr;
		}
	};
	class Class {
		const char* name;
		ClassStructure structure;
	public:
		Class(const char* _name, ClassStructure s) : name(_name), structure(s) {}
		Class(const char* _name) : name(_name) {}

		ClassInstance instantiate() {
			return ClassInstance(structure.members);
		}

		void addNewVariable(ClassStructure::ClassVariable& v, const std::string& s) {
			structure.members.insert({ s, std::make_shared<ClassStructure::ClassVariable>(v) });
		}
		void addFunction(ClassStructure::ClassFunction& f, const std::string& s) {
			structure.members.insert({ s,std::make_shared<ClassStructure::ClassFunction>(f) });
		}
		void setConstructor(ClassStructure::ClassConstructor& c) {
			structure.constructor = std::make_shared<ClassStructure::ClassConstructor>(c);
		}
		void setStructure(ClassStructure n) {
			structure = n;
		}
		ClassStructure* getStructure() {
			return &structure;
		}
		const char* getName() {
			return name;
		}
		bool hasMember(const char* name) {
			return structure.members.find(std::string(name)) != structure.members.end();
		}
		ClassStructure::ClassVariable* getVariable(const char* name) {
			auto f = structure.members.find(std::string(name));

			if (f != structure.members.end()) return (ClassStructure::ClassVariable*)f->second.get();

			return nullptr;
		}
		ClassStructure::ClassFunction* getFunction(const char* name) {
			auto f = structure.members.find(std::string(name));

			if (f != structure.members.end()) return (ClassStructure::ClassFunction*)f->second.get();

			return nullptr;
		}

	};
	class ClassHandler;
	class ClassHandler {
		typedef std::shared_ptr<Class>& ClassRef;



		std::vector <std::shared_ptr<Class>> classes;
		static ClassHandler& instance() {
			static ClassHandler x;
			return x;
		}
	public:
		static ClassRef getClassByName(std::string& name) {
			for (auto& x : instance().classes) {
				if (x->getName() == name) return x;
			}
		}
		static bool hasClass(std::string name) {
			for (auto& x : instance().classes) {
				if (x->getName() == name) return true;
			}
			return false;
		}
		static void addClass(Class& c) {
			instance().classes.push_back(std::make_shared<Class>(c));
		}
		static void listClasses() {
			for (auto& x : instance().classes) {
				std::cout << "CLASS: " << x->getName() << "\n";
			}
		}
	};
};