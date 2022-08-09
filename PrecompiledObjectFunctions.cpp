#pragma once
#include "Types.h"
#include <any>
#include <iostream>
#include <string>
#include <tuple>
using namespace marine;

#pragma region help

template<typename Result, typename... Ts>
auto runtime_get(std::size_t i, std::tuple<Ts...>& t) -> Result& {
    using Tuple = std::tuple<Ts...>;

    // A set of functions to get the element at one specific index
    auto get_at_index = []<std::size_t I>(Tuple & tuple) -> Result& {
        if constexpr (std::is_same_v<std::tuple_element_t<I, Tuple>, Result>) {
            return std::get<I>(tuple);
        }
        else {
            throw std::runtime_error("Index does not contain the right type");
        }
    };

    // Regular index_sequence trick to get a pack of indices
    return[&]<std::size_t... Is>(std::index_sequence<Is...>) -> Result& {
        // The type of a single member function pointer of the closure type, using awkward memfun syntax
        using FPtr = auto(decltype(get_at_index)::*)(Tuple&) const->Result&;
        // An array of said memfun pointers, each for one known index
        FPtr fptrs[sizeof...(Ts)]{ &decltype(get_at_index)::template operator() < Is > ... };
        // Invoke the correct pointer
        return (get_at_index.*(fptrs[i]))(t);
    }(std::index_sequence_for<Ts...>());
}


template<typename First, typename ...Types>
std::tuple<First, Types...> cast(std::vector<std::any>& v, int index = 0, std::tuple<Types...>* ret = nullptr) {
	if (ret == nullptr) ret = new std::tuple<Types...>();
	if (index < v.size()) {
        First& x = runtime_get<First>(index, *ret);
        x = std::any_cast<First>(a[index]);
		return cast<Types...>(a, index + 1, ret);
	}

	return ret;
}
#pragma endregion
	
#pragma region int
	
#pragma endregion

#pragma region string

#pragma endregion

#pragma region array
VContainer splice(std::any a[], Variable* self) {

}
#pragma endregion


int main() {
	std::vector<std::any> a = { 0, 4, "stuff" };
	auto [i0, i1, ch] = cast<int, int, const char*>(a);

	std::cout << "got:" << i0 << ", " << i1 << ", " << ch << std::endl;
}