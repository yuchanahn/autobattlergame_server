#pragma once

#include <vector>
#include <unordered_set>

#include "battle.h"
#include "champion.h"

const int CHAMP_SPEED = 15.f;
const int CHAMP_ATTACK_RANGE = 20.f;

class champion_db
{
public:

	static champion_t get_champion_of(int code, int lv, int star)
	{
		auto c = Get()[code];
		c.op += c.op_g * lv;
		c.cri += c.cri_g * lv;
		
		for (int i = 0; i < star; i++) c.op *= 1.5f;
		for (int i = 0; i < star; i++) c.cri *= 1.5f;

		return c;
	}

	static std::vector<champion_t>& Get()
	{
		using namespace ::ranges;
		static std::vector<champion_t> champions = {
			champion_t {
				0,		// code
				1,		// lv
				1,		// star
				CHAMP_SPEED,		// speed
				3,		// op
				2,		// as
				.2f,	// cri
				1.f,	// random_move_rate_t
				150.f,   // follow_range
				0.2f,	// attack_col_t
				CHAMP_ATTACK_RANGE,			// attack_range
				CHAMP_ATTACK_RANGE + 0.5f,	// attack_col_range
				0.2f,	// atk_start
				0.4f,	// atk_end
				900,	// max_hp
				100,	// max_mp
				1,		// growth op
				-0.01,	// growth as
				.01f,	// growth cri
				{ -1, -1, -1, -1 },
				// 오공
				// 패시브 : 맞을때마다 물리 공격력 증가.
				//		   max 수치 = 10
				[](champion_t* c) {
					c->OnHurt = [c](int hp) {
						if (    yc_range::contains_key(c->value_, "op") 
							||  c->value_["op"] < 10) 
						{
							c->op++;
							c->value_["op"]++;
						}
					};
				},
				[](battler_t* c, Vec2f dir, std::vector<battler_t*> v) -> bool {
					if (c->mp < c->c.max_mp) return false;
 					auto p = (c->pos + dir);
					return (v | views::filter(lamda(x, x->pos.dist(p) <= CHAMP_ATTACK_RANGE))
							  | to_vector).size();
				},
				// 스킬 : [여의봉 강타!]
				//		 전방의 적들을 여의봉으로 강타한다.
				[](battler_t* c, 
				   Vec2f dir, 
				   std::vector<battler_t*> v, 
				   std::unordered_set<battler_t*>& hitted, 
				   float skill_t, 
				   std::function<void(battler_t*,int)> hit_func) 
				{
					if (skill_t >= 0.3f && skill_t <= 0.8f) {
						auto p = (c->pos + dir);
						for (auto& i : v | views::filter(lamda(x, x->pos.dist(p) <= CHAMP_ATTACK_RANGE))) {
							if (!yc_range::contains_key(hitted, i)) {
								hitted.insert(i);
								int d = 30 + (c->c).op;
								hit_func(i, 50);
							}
						}
					}
				},
				1.0f,	 // skill end time.
			},
			champion_t { 
				1,		// code
				1,		// lv
				1,		// star
				CHAMP_SPEED,		// speed
				5,		// op
				2,		// as
				.2f,	// cri
				1.f,	// random_move_rate_t
				150.f,   // follow_range
				0.2f,	// attack_col_t
				CHAMP_ATTACK_RANGE,			// attack_range
				CHAMP_ATTACK_RANGE + 0.5f,	// attack_col_range
				0.2f,	// atk_start
				0.4f,	// atk_end
				500,	// max_hp
				300,	// max_mp
				2,		// growth op
				-0.01,	// growth as
				.01f,	// growth cri
				{ -1, -1, -1, -1 },
				// 세라스
				// 패시브 : 체력이 절반 이하로 내려갔을경우
				//		   공격력이 2배가 됨.
				[](champion_t* c) {
					c->OnHurt = [c](int hp) {
						if (	yc_range::contains_key(c->value_switch, "op2")
							&&  hp <= (c->max_hp / 2)) 
						{
							c->value_switch.insert("op2");
							c->op *= 2;
						}
					};
				},
				//[](auto, auto, auto) { return false; }
			},
			champion_t { 
				2,		// code
				1,		// lv
				1,		// star
				CHAMP_SPEED,		// speed
				4,		// op
				2.f,	// as
				.2f,	// cri
				1.f,	// random_move_rate_t
				150.f,   // follow_range
				0.2f,	// attack_col_t
				CHAMP_ATTACK_RANGE,			// attack_range
				CHAMP_ATTACK_RANGE + 0.5f,	// attack_col_range
				0.2f,	// atk_start
				0.4f,	// atk_end
				800,	// max_hp
				200,	// max_mp
				3,		// growth op
				-0.01,	// growth as
				.01f,	// growth cri
				{ -1, -1, -1, -1 },
				// 신비
				// 패시브 : 평타가 광역 공격임.
				//		   다른 챔피언을 죽일때마다 공격속도 상승.
				[](champion_t* c) {
					c->attack_col_range += 10.f;
					c->attack_range += 10.f;
					c->OnKill = [c] {
						c->as = max(c->as - 0.2f, 0.1f);
					};
				},
				//[](auto, auto, auto) { return false; }
			}
		};
		return champions;
	}
};