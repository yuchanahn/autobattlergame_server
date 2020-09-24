#pragma once

#include <range/v3/all.hpp>

#include <iostream>
#include <vector>

#include "YCServer.h"
#include "champion.h"
#include "champion_db.h"
#include "gacha.h"
#include "packets.h"
#include "champion_list.h"
#include "item_db.h"
#include "YCFunc.h"



struct user_t
{
	std::wstring name;
	std::vector<champion_t> champion_list;
	std::vector<item_t> items;
	int pick = 0; // champion index;
	coin_t coin;
	money_t money;
	int m_user_id;
	int m_id;

	template <typename send_func>
	void add_coin(send_func send, int value)
	{
		coin.value += value;
		item_db::set_coin(m_user_id, coin);
		auto r = r_coin_t{ coin.value };
		send(m_id, &r);
	}

	template <typename send_func>
	void add_money(send_func send, int value)
	{
		money.value -= value;
		item_db::set_money(m_user_id, money);
		auto r = money_t{ money.value };
		send(m_id, &r);
	}


	template <typename send_func>
	static void set_signal(send_func send, std::unordered_map<int, user_t>* users)
	{
		ioev::Signal<gacha_t>([send, users](gacha_t* gacha_info, int id) {
			auto& user = (*users)[id];
			for (int i = 0; i < gacha_info->cnt; i++) {
				if (user.coin.value < 1) {
					break;
				}
				user.add_coin(send, -1);
				auto r = r_gacha_t{ gacha<champion_percentages>::run() };
				user.champion_list.push_back(champion_db::Get()[r.r]);
				check_same_champions_cnt(user.champion_list); // * * * -> **
				check_same_champions_cnt(user.champion_list); // ** ** ** -> ***
				set_champions(id, format_list(user.champion_list));
				send(id, &r);
			}
		});

		ioev::Signal<item_gacha_t>([send, users](item_gacha_t* gacha_info, int id) {
			auto& user = (*users)[id];
			for (int i = 0; i < gacha_info->cnt; i++) {
				if (user.coin.value < 10) {
					break;
				}
				user.add_coin(send, -10);
				item_t item;
				item.code = gacha<item_percentages>::run();
				item.type = gacha<item_type_percentages>::run();
				item.value1 = yc::rand(10, 30);
				item.equiped = false;
				user.items.push_back(item);
				auto r = r_item_gacha_t{ user.items.back() };
				send(id, &r);
			}
			item_db::set(id, item_db::format(user.items));
		});

		ioev::Signal<rq_item_list_t>([send, users](auto, int id) {
			auto& user = (*users)[id];
			auto pack = item_db::format(user.items);
			send(id, &pack);
		});

		ioev::Signal<cmd_equip_t>([send, users](cmd_equip_t* d, int id) {
			auto& user = (*users)[id];
			try
			{
				if (user.champion_list.size() <= d->champion_number) throw std::runtime_error("없는 챔피언에 장착하려고함!");
				if (user.items[d->item_num].type != d->slot_num)	 throw std::runtime_error("넣으려는 슬롯과 아이템 넘버가 맞지 않음!");
				if (user.items[d->item_num].equiped)				 throw std::runtime_error("이미 장착 되어있는 아이템임!");

				user.champion_list[d->champion_number].item[d->slot_num] = d->item_num;
				fmt::print("[{}] = {}\n", (int)d->slot_num, (int)d->item_num);
				user.items[d->item_num].equiped = true;
				item_db::set(id, item_db::format(user.items));
				set_champions(id, format_list(user.champion_list));
			}
			catch (const std::exception& e)
			{
				fmt::print("cmd_equip [error : {}]\n", e.what());
			}
		});


		ioev::Signal<rq_champion_list_t>([send, users](auto, int id) {
			auto& user = (*users)[id];
			auto pack = format_list(user.champion_list);
			send(id, &pack);
		});

		ioev::Signal<cmd_pick_t>([send, users](auto d, int id) {
			auto& user = (*users)[id];
			user.pick = d->pick;
			r_pick_t r = r_pick_t{
				user.pick
			};
			send(id, &r);
			user.add_coin(send, 1);
		});
	}
};