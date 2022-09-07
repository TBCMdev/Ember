#pragma once
#include "Base.h"
#include "VContainer.h"
#include "ObjectHandler.h"
#include "Variable.h"
namespace marine {
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
	class ArrayList {
	protected:
		std::vector<VContainer> items;
		std::vector<Base::Decl> types;
		int size = -1;
	public:
		ArrayList(std::vector<VContainer> _items = {}, int _size = -1) : items(_items), size(_size){
		}
		ArrayList(std::vector<Base::Decl> v): types(v) {

		}
		void add(VContainer& item) {
			items.push_back(item);
		}
		VContainer get(int l) {
			return items[l];
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
	};
	
	class Function {
	protected:
		std::string name;
		lexertk::token start, end;
		int start_index, end_index;
	public:
		std::vector<Variable> parameters;
		Function(std::string _name, lexertk::token& _start, lexertk::token _end, int start_, int end_, std::vector<Variable> _p) : name(_name), start(_start), end(_end), start_index(start_), end_index(end_), parameters(_p) {

		}
		std::string& getName() { return name; }
		int getStart() { return start_index; }
		int getEnd() { return end_index; }
		std::string str() {
			std::stringstream s("(MANUAL FUNCTION) name:");
			s << name << ", start token:" << start.value << ", end token:" << end.value << ", variables from parameters:";
			for (auto& x : parameters) {
				s << x.str() << "\n";
			}
			return s.str();
		}
	};



	struct EXPRDATA {
		bool negated;
		bool simnull;
	};
};