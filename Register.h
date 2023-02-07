#pragma once
#include <functional>
#include <queue>
namespace marine {
	// A possibly single instanced class used for holding data during a parse iteration.
	// it holds data so we can execute it whenever we need to, for example at the end of
	// a parse iteration. We add to its 'Queue', so when another object wants to call something,
	// it will execute everything in order.
	template<typename Ret>
	struct Register {
		std::queue<std::function<Ret()>> q;

		Ret exec() {
			Ret x = q.front();
			return x;
		}
		void execAll() {
			for (const auto& x : q) {
				x();
				q.pop();
			}
		}
	};
}