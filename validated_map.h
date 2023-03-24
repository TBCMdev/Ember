#pragma once
#include <vector>


template<typename _T1, typename _T2, bool _SMART>
class validated_map {
protected:
	std::vector<_T1> keys;
	std::vector<_T2> vals;
public:
	validated_map(const std::vector<std::pair<_T1, _T2>>& c){
		for (const auto& item : c) {
			keys.push_back(item.first);
			vals.push_back(item.second);
		}
	}
	virtual _T2* operator[](const _T1& k) {
		auto f = std::find(keys.begin(), keys.end(), k);
		if (f == keys.end()) {
			if (_SMART) {
				auto s = keys.size();
				for (int i = 0; i < s; i++) {
					if (std::get<0>(keys[i]) == std::get<0>(k)) {
						return &vals[i];
					}
				}
			}
			
			return nullptr;
		}
		return &vals[std::distance(keys.begin(), f)];
	}
	void insert(const _T1& key, const _T2& val) {
		keys.push_back(key);

		vals.push_back(val);
	}
};


