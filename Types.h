#pragma once
#include "Base.h"
#include "VContainer.h"
#include "Variable.h"
#include "Serialized.h"
#include "Iterable.h"
namespace marine {
	class ArrayList;
	template<typename VContainerList>
	static VContainerList sliceFromLeft(VContainerList& v, int from) {
		return VContainerList(v.begin() + from, v.end());
	}

	template<typename VContainerList>
	static VContainerList sliceFromRight(VContainerList& v, int from) {
		return VContainerList(v.begin(), v.end() - from);
	}

	template<typename VContainerList>
	static VContainerList sliceBetween(VContainerList& v, int from, int to) {
		return VContainerList(v.begin() + from, v.end() - to);
	}

	
	class ListOperator {
	public:
		enum class LOType {
			DEFAULT,
			LRSLICE,
			LSLICE,
			RSLICE
		};
	protected:
		bool slice = false;
		LOType type;
	public:
		std::shared_ptr<int> left, right;

		ListOperator(int* i = nullptr, bool slice = false, int* r = nullptr) : left(i), right(r) {
			// l[? ?: ?]
			type = slice ? (i == nullptr ? LOType::RSLICE : r == nullptr ? LOType::LSLICE : LOType::LRSLICE) : LOType::DEFAULT;
			std::cout << "determined slice: " << (int)type << std::endl;
		}
		bool hasSlice() { return slice; }
		LOType getType() {
			return type;
		}
	};
	class ArrayList : public Serialized, public Iterable{
		
	protected:
		std::vector < VContainer > items;
		std::vector<Base::Decl> types;
	public:
		static std::string string(ArrayList& x) {
			return x.str();
		}
		ArrayList(std::vector<VContainer> _items = {}) : items(_items), Serialized(), Iterable() {
		}
		ArrayList(std::vector<Base::Decl> v) : types(v), Serialized(), Iterable() {

		}
		void add(VContainer& existing) {
			items.push_back(existing);
		}
		template<typename T>
		void add(T v, int depth, Base::Decl x, std::vector<Base::DeclConfig> y = {}) {
			items.push_back(VContainer(v, depth, x, y));
		}
		VContainer* getItemRef(int l) {
			return &(items[l]);
		}
		VContainer get(int l) {
			return items[l];
		}
		int length() {
			return (int)items.size();
		}
		std::vector<VContainer> operateLarge(ListOperator op) {
			switch (op.getType()) {
			case ListOperator::LOType::LSLICE:
				return sliceFromLeft(items, *op.left);
			case ListOperator::LOType::RSLICE:
				return sliceFromRight(items, *op.right);
			case ListOperator::LOType::LRSLICE:
				return sliceBetween(items, *op.left, *op.right);
			}
		}
		std::vector<VContainer>& getItems() { return items; }
		std::string str() {
			std::stringstream s;
			int end = items.size() - 1;
			s << "[";
			for (int i = 0; i < items.size(); i++) {
				if (i != end) s << VContainer::VCStr(items[i]) << ", ";
				else s << VContainer::VCStr(items[i]);
			}
			s << "]";

			return s.str();
		}
		std::string serialize() override {
			return str();
		}
	};
	
	class Function {
	protected:
		std::string name;
		lexertk::token start, end;
		int start_index, end_index;
	public:
		std::unordered_map<std::string, Variable> parameters;
		Function(std::string _name, lexertk::token& _start, lexertk::token _end, int start_, int end_, std::unordered_map<std::string, Variable> _p) : name(_name), start(_start), end(_end), start_index(start_), end_index(end_), parameters(_p) {

		}
		Function() {}
		std::string& getName() { return name; }
		int getStart() { return start_index; }
		int getEnd() { return end_index; }
		std::string str() {
			std::stringstream s("(MANUAL FUNCTION) name:");
			s << name << ", start token:" << start.value << ", end token:" << end.value << ", variables from parameters:";
			return s.str();
		}
		virtual ~Function() = default;
	};
	class Lambda;


	std::function<VContainer(Lambda*, std::unordered_map<std::string, VContainer>&)> _LAMBDA_ON_CALL = nullptr;

	class Lambda : public Function {
	public:
		static VContainer callFromCPP(Lambda* l, std::unordered_map<std::string, VContainer> args) {
			if (_LAMBDA_ON_CALL != nullptr)
				return _LAMBDA_ON_CALL(l, args);

			return VContainer::null();
		}
		static void initialize(std::function<VContainer(Lambda*, std::unordered_map<std::string, VContainer>&)> l) {
			_LAMBDA_ON_CALL = l;
		}
		Lambda(lexertk::token& start, lexertk::token _end, int start_, int end_, std::unordered_map<std::string, Variable> _p) : Function("", start, _end, start_, end_, _p) {

		}
		~Lambda() = default;
		/*void call(std::vector<VContainer> values) {
			on_call(this, values);
		}*/
	};
	struct cpp_Lambda : public Function {
		std::vector<Base::Decl> param_types;
		cpp_Lambda(std::vector<Base::Decl> param_types) : Function() {

		}
		~cpp_Lambda() = default;
	};

	struct EXPRDATA {
		bool negated;
		bool simnull;
	};
};