#pragma once

#include "fmt/format.h"

#include "YCServer.h"
#include "YCPacket.h"
#include "YCSync.h"

#include "user.h"
#include "matching.h"
#include "champion_list.h"
#include "battle.h"

#define PORT 51234

class server
{
	std::unordered_map<int, user_t> users;
	YCServer* server_ptr = nullptr;
	std::vector<battle> battles;
	server()
	{
		Test();

		static matching matching_system([this](std::vector<int> v) {
			battles.push_back(battle{  });
			battles.back().start(v | ::ranges::views::transform([&](int id) { 
				auto& user = get_users()[id];

				if (user.champion_list.size() <= user.pick) user.pick = 0;

				auto& champion = user.champion_list[user.pick];
				auto c = champion_db::get_champion_of(champion.code, champion.lv, champion.star);
				if (champion.item[0] != -1) {
					c.op *= 1 + (user.items[champion.item[0]].value1 * 0.01f);
				}
				return new battler_t
				{
					c,
					id,
					id,
					yc::random_pos<Vec2f>(-2000.f, 2000.f),
					start_t {},
					c.max_hp,
					0
				};
			}) | ::ranges::to_vector);
			::ranges::for_each(v, [](int i) {
				chatting_t chat;
				yc_str::copy(fmt::format(L"server : 매칭되었습니다."), chat.text);
				get_master_server().Send(i, &chat);
			});
		});

		auto send_func = [](int id, auto&& d) { if (get_sync(id) != get_master_server().get_server_sync()) get_master_server().Send(id, d); };
		auto sync_ = [](int id, auto&& f) {
			if(auto sync = get_sync(id)) sync->Add(f);
		};
		static YCServer server_(PORT,
			[&](int id) {
				fmt::print("Client  : (id {})\n", id);
				users[id] = user_t{
					fmt::format(L"player {}", id),
					get_champions(id/*나중에 user_id로 바까야함..*/),
					item_db::format(item_db::get(id))
				};

				r_id_t r = r_id_t{
					id
				};
				send_func(id, &r);

				r_pick_t r2 = r_pick_t{
					users[id].pick
				};
				send_func(id, &r2);

				r_coin_t r3 = r_coin_t
				{
					users[id].coin.value
				};
				send_func(id, &r3);
			},
			[&](int id) {
				fmt::print("Client Disconnect : (id {})\n", id);
				sync_(id, [&, id] {
					get_master_server().get_server_sync()->Add([&, id] {
						matching_system.pop(id);
						users.erase(id);
					});
				});
			},
			[&] {
				const float fdt_t = 0.1f;
				static float dt = 0;
				if ((dt += YCTime::deltaTime) >= fdt_t)
				{
					bool t = false;
					for (auto& i : battles)
					{
						i.run(dt, send_func, sync_);
						std::visit(overloaded{
							[&](battle_exit_t& exit) {
								t = true;
							},
						    [&](auto&) {}
						}, i.state);
					}
					if (t) battles.clear();
					dt -= fdt_t;
				}
			});
		server_ptr = &server_;

		user_t::set_signal(send_func, &users);

		ioev::Signal<cmd_battle_start_t>([&](cmd_battle_start_t*, int id) {
			get_master_server().get_server_sync()->Add([&, id]() {
				matching_system.push(id);
			});
		});
	}
public:
	static server& get_master()
	{
		static server master;
		return master;
	}

	static YCServer& get_master_server()
	{
		return *(get_master().server_ptr);
	}

	static Strand* get_sync(int id)
	{
		try
		{
			return get_users().find(id) != get_users().end()
					? get_master_server().get_sync(id)
					: get_master_server().get_server_sync();
		}
		catch (const std::exception&)
		{
			fmt::print("접속 종료 클라이언트에게 메시지 보냄!\n");
			return get_master_server().get_server_sync();
		}
	}

	static std::unordered_map<int, user_t>& get_users()
	{
		return get_master().users;
	}

	static void run()
	{
		get_master_server().Srv_Start();
	}
};

