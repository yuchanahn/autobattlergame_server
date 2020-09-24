#pragma once
#include <range/v3/all.hpp>

#include <vector>


#include "champion.h"
#include "champion_db.h"
#include "YCDB.h"
#include "packets.h"


static champion_list_t format_list(std::vector<champion_t>& list)
{
	champion_list_t r;
	int idx = 0;
	r.count = list.size();
	::ranges::for_each(list, [&](const auto& i) {
		r.c[idx++]  = db_champion_data_t{
			i.code,
			i.lv,
			i.star,
			{ 
				i.item[0],
				i.item[1],
				i.item[2],
				i.item[3]
			},
			0
		};
	});

	return r;
}

// 같은 챔피언이 3마리 존재하면 그 챔피언의 별을 올린다.
static void check_same_champions_cnt(std::vector<champion_t>& list)
{
	std::map<std::pair<int, int>, int> c_count;
	yc_range::for_each(list, [&](const champion_t& c) {
		c_count[std::make_pair(c.code, c.star)]++;
	});
	yc_range::remove_if(list, lamda(x, c_count[std::make_pair(x.code, x.star)] == 3));
	yc_range::for_each(c_count, [&](const auto& i) {
		if (i.second == 3)
		{
			champion_t c = champion_db::Get()[i.first.first];
			c.star = i.first.second + 1;
			list.push_back(c);
		}
	});
}

static void set_champions(int user_id, champion_list_t list)
{
	YCDB::set<champion_list_t>(user_id, list);
}

static std::vector<champion_t> get_champions(int user_id)
{
	auto row = YCDB::get<champion_list_t>(user_id);

	std::vector<champion_t> r;
	if (!std::get<0>(row)) return {};

	auto list = std::get<1>(row);
	for (int i = 0; i < list.count; i++) {
		auto& x = list.c[i];
		champion_t c = champion_db::Get()[x.code];
		c.star = x.star;
		c.lv = x.lv;
		for (int i = 0; i < 4; i++) {
			c.item[i] = x.item[i];
		}
		r.push_back(c);
	}
	return r;
}

