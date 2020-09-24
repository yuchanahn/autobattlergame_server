#pragma once
#include <vector>
#include <Functional>

#include "user.h"

#define MATCHMAX 3


class matching
{
	std::vector<int> pool;
	std::function<void(std::vector<int>)> battle_start;
public:

	matching(std::function<void(std::vector<int>)> f) : battle_start(f)
	{ }

	void push(int user_id)
	{
		pool.push_back(user_id);
		if (pool.size() == MATCHMAX) {
			battle_start(pool);
			pool.clear();
		}
	}

	void pop(int user_id)
	{
		yc_range::remove_if(pool, lamda(x, x == user_id));
	}
};