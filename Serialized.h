#pragma once
#include "Base.h"
#include "ValueHolder.h"
namespace marine {
	class Serialized: public ValueHolder {
	public:
		Serialized(): ValueHolder(NULL) {}
		virtual ~Serialized() = default;
		virtual std::string serialize() { return "(Serializable Object) "; };

	};
}