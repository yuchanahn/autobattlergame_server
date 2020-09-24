#pragma once
#include <functional>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>

#include "Vec2.h"

struct champion_t
{
	char code;
    char lv;
    char star;
	int speed;
    int op;
    float as;
    float cri;

    float random_move_rate_t;

    float follow_range;

    float attack_col_t;
    float attack_range;
    float attack_col_range;
    float atk_start;
    float atk_end;

    float max_hp;
    float max_mp;

    float op_g;
    float as_g;
    float cri_g;

    char item[4];
    
    std::function<void(champion_t*)> skill_passive;
    std::function<bool(class battler_t*, Vec2f, std::vector<class battler_t*>)> skill_condition;
    std::function<void(class battler_t*, 
                       Vec2f, 
                       std::vector<class battler_t*>, 
                       std::unordered_set<class battler_t*>&, 
                       float, 
                       std::function<void(class battler_t*, int)>)> skill;

    float skill_end_t;

    std::function<void(int)> OnHurt;
    std::function<void()> OnDead;
    std::function<void()> OnKill;

    std::unordered_set<std::string> value_switch;
    std::unordered_map<std::string, int> value_;
};