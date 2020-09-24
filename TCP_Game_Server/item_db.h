#pragma once
#include <vector>

#include "item.h"
#include "YCDB.h"
#include "YCFunc.h"




struct item_db
{
	static void set(int user_id, db_item_list_t list)
	{
		YCDB::set<db_item_list_t>(user_id, list);
	}

	static db_item_list_t get(int user_id)
	{
		auto row = YCDB::get<db_item_list_t>(user_id);
		if (!std::get<0>(row)) return db_item_list_t{ {}, 0 };
		return std::get<1>(row);
	}

	static coin_t get_coin(int user_id)
	{
		auto row = YCDB::get<coin_t>(user_id);
		if (!std::get<0>(row)) return coin_t{ 10 };
		return std::get<1>(row);
	}
	static void set_coin(int user_id, coin_t coin)
	{
		YCDB::set<coin_t>(user_id, coin);
	}
	static money_t get_money(int user_id)
	{
		auto row = YCDB::get<money_t>(user_id);
		if (!std::get<0>(row)) return money_t{ 0 };
		return std::get<1>(row);
	}
	static void set_money(int user_id, money_t money)
	{
		YCDB::set<money_t>(user_id, money);
	}

	static std::vector<item_t> format(db_item_list_t l)
	{
		std::vector<item_t> r;
		for (int i = 0; i < l.count; i++)
		{
			r.push_back(l.item[i]);
		}
		return std::move(r);
	}

	static db_item_list_t format(std::vector<item_t>& l)
	{
		db_item_list_t r = { {}, l.size() };
		int idx = 0;
		yc_range::for_each(l, [&](auto& i) { r.item[idx++] = i; });
		return std::move(r);
	}
};