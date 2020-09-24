#pragma once
#include "fmt/format.h"
#include "range/v3/all.hpp"

#include <variant>
#include <algorithm>
#include <unordered_set>

#include "YCFunc.h"
#include "packets.h"
#include "Vec2.h"
#include "yc_str.h"
#include "YCRandom.h"
#include "YCTime.h"


static vec2_t format_to_vec2_t(const Vec2f& v)
{
    return vec2_t{ v.x, v.y };
}

static Vec2f get_dir(Vec2f opos, Vec2f tpos)
{
    return (tpos - opos).normalize();
}

static Vec2f get_vel(float speed, Vec2f opos, Vec2f tpos)
{
    return get_dir(opos, tpos) * speed;
}

static Vec2f move_pos(float speed, Vec2f opos, Vec2f tpos, float t)
{
    auto vel = get_vel(speed, opos, tpos) * t;
    return opos + vel;
}

static Vec2f move_pos_dir(float speed, Vec2f opos, Vec2f dir, float t)
{
    auto v = dir * speed * t;
    return opos + v;
}


static void Test()
{
    //auto spos = yc::random_pos<Vec2f>(-100.f, 100.f);
    //auto tpos = yc::random_pos<Vec2f>(-100.f, 100.f);
    //
    //auto d = get_dir(spos, tpos);
    //auto p = yc::random_pos<Vec2f>(-1000.f, 1000.f);
    //fmt::print("x:{},y:{}\n", p.x, p.y);
    //p = yc::random_pos<Vec2f>(-1000.f, 1000.f);
    //fmt::print("x:{},y:{}\n", p.x, p.y);
    //p = yc::random_pos<Vec2f>(-1000.f, 1000.f);
    //fmt::print("x:{},y:{}\n", p.x, p.y);
    //p = yc::random_pos<Vec2f>(-1000.f, 1000.f);
    //fmt::print("x:{},y:{}\n\n", p.x, p.y);
}


struct follow_t
{
    int targat_user_id;
};

struct random_movement_t
{
    yc::time_val_t t;
    Vec2f radom_pos;
};

struct attack_t
{
    float ani_run_time;
    Vec2f dir2d;
    std::unordered_set<int> hitted;
    float as;
    bool started;
    attack_t(Vec2f d, float as_) : dir2d(d), as(as_), ani_run_time(0), started(false) {}

    template <typename F>
    void on_attack_start(F OnAttackStart)
    {
        if (!started) {
            started = true;
            OnAttackStart();
        }
    }
    template <typename F>
    void on_attack_end(F OnAttackEnd)
    {
        if (ani_run_time >= as)
        {
            OnAttackEnd();
        }
    }
    template <typename F>
    void on_hitable_time(F on_hit_event, const float& hit_col_stratT, const float& hit_col_endT, float ut)
    {
        update_time(ut);
        if (ani_run_time >= hit_col_stratT && ani_run_time <= hit_col_endT)
        {
            on_hit_event();
        }
    }
    void update_time(float t)
    {
        ani_run_time += t;
    }
};

struct skill_t
{
    Vec2f dir;
    float skill_dt;
    std::unordered_set<battler_t*> hitted;
};

struct start_t
{

};


struct battler_t
{
    champion_t c;
    int user_id;
    int clnt_id;
    Vec2f pos;
    
    std::variant<
        follow_t,
        random_movement_t,
        attack_t,
        skill_t,
        start_t
    > state;

    //###########   [stat]   ################
    float hp = 0;
    float mp = 0;

    bool dead = false;
    //######################################

    ~battler_t()
    {
        fmt::print("id : [{}] 소멸자 호출됨\n", user_id);
    }


    template<typename F1, typename F2>
    void start_skill(const std::vector<int>& player_clnt_ids, F1 send, F2 sync)
    {
        add_mp(player_clnt_ids, send, sync, -mp);
        r_skill_start r;
        r.user_id = user_id;
        yc_range::for_each(player_clnt_ids, [&](int id) {
            send(id, &r);
        });
    }


    template<typename F1, typename F2>
    void add_mp(const std::vector<int>& player_clnt_ids, F1 send, F2 sync, int mp_)
    {
        sync(clnt_id, [=] {
            mp = min(mp + mp_, c.max_mp);

            r_mp_t r;
            r.user_id = user_id;
            r.mp = mp;
            r.max_mp = c.max_mp;
            yc_range::for_each(player_clnt_ids, [&](const auto& i) {
                send(i, &r);
            });
        });
    }

    template<typename F1, typename F2>
    void hit(std::vector<int>& player_clnt_ids, F1 send, F2 sync, bool is_cri_b, int dmg)
    {
        sync(clnt_id, [=] {
            if (dead) return;
            if (dead && c.OnDead) c.OnDead();
            
            if(c.OnHurt) c.OnHurt(dmg);

            add_mp(player_clnt_ids, send, sync, 25);
            
            r_hit_t r;
            r.user_id = user_id;
            r.dmg = dmg;
            r.cri = is_cri_b;
            r.hp = hp - dmg;
            yc_range::for_each(player_clnt_ids, [&](const auto& i) {
                send(i, &r);
            });
            dead = (hp -= dmg) <= 0;
        });
    }

    template<typename F1, typename F2>
    void move(Vec2f vel, float dt, std::vector<int>& player_clnt_ids, F1 send, F2 sync)
    {
        sync(clnt_id, [=] {
            pos = move_pos_dir(c.speed, pos, vel, dt);
            r_champion_movement_info_t r;
            r.speed = c.speed;
            r.pos = format_to_vec2_t(pos);
            r.vel = format_to_vec2_t(vel);
            r.user_id = user_id;
            r.champ_code = c.code;
            yc_range::for_each(player_clnt_ids, [&](const auto& i) {
                send(i, &r);
            });
        });
    }

    template<typename F1, typename F2, typename F3>
    void run(float dt, std::vector<int> player_clnt_ids, F1 get_player_if, F2 sync, F3 send)
    {
        // * 상태를 바꿨으면 바로바로 return

        // 모든 플레이어를 가져온다.
        static auto all_player = [] { return true; };
        static auto id_equal = [](auto& target_id, battler_t* x) {
            return target_id == x->user_id;
        };
        static auto print_all_id = [](battler_t* x) {
            fmt::print("battlers id - {}\n", x->user_id);
            return true;
        };
        auto id_equal_self = yc::curry(id_equal)(user_id);
        static auto vec_dist_cmp = [](auto& pos, auto& val, battler_t* t) {
            return t->pos.dist(pos) < val;
        };
        auto dist_cmp_to_pos = yc::curry(vec_dist_cmp)(pos);
        static auto filter_not_self = [](auto& id_equal, auto& vec_dist_cmp, battler_t* x) {
            return !id_equal(x) && vec_dist_cmp(x);
        };
        auto not_self_and_cmp = yc::curry(filter_not_self)(id_equal_self);
        auto not_self_and_cmp_follow = not_self_and_cmp(dist_cmp_to_pos(c.follow_range));
        auto not_self_and_cmp_atk_col = not_self_and_cmp(dist_cmp_to_pos(c.attack_col_range));
        auto cmp_atk_range = dist_cmp_to_pos(c.attack_range);

        static auto is_cri = lamda(x, yc::rand(0.f, 1.f) <= x);

        std::visit(overloaded{
            [&](start_t& ) {
                fmt::print("start\n");
                sync(clnt_id, [=] { 
                    c.skill_passive(&c); 
                });
                state = random_movement_t{ yc::time_val_t(1.f), yc::random_dir2d<Vec2f>() };
            },
            [&](follow_t& f) {
                auto target = get_player_if(yc::curry(id_equal)(f.targat_user_id));

                if (!target.empty()) {
                    Vec2f dir = get_dir(pos, (target[0])->pos);

                    if (c.skill_condition && c.skill_condition(this, dir, get_player_if(lamda(_, true)))) {
                        state = skill_t{ dir, 0.f, {} };
                    } else if (cmp_atk_range(target[0])) {
                        state = attack_t(dir, c.as);
                    } else {
                        move(get_vel(c.speed, pos, (target[0])->pos), dt, player_clnt_ids, send, sync);
                    }
                } else {
                    state = random_movement_t{ yc::time_val_t(c.random_move_rate_t), yc::random_dir2d<Vec2f>() };
                }
            },
            [&](random_movement_t& r) {
                auto target = get_player_if(not_self_and_cmp_follow);
                if (!target.empty()) {
                    state = follow_t{ (target[0])->user_id };
                    return;
                }
                move(get_vel(c.speed, pos, r.radom_pos), dt, player_clnt_ids, send, sync);
                r.t.timer_end([&]() { 
                    r.radom_pos = yc::random_dir2d<Vec2f>(); 
                    r.t.reset(); 
                }, dt);
            },
            [&](attack_t& a) {
                a.on_attack_start([&]() {
                    r_attack_start r;
                    r.t = 0;
                    r.user_id = user_id;
                    yc_range::for_each(player_clnt_ids, [&](int id) {
                        send(id, &r);
                    });
                });
                a.on_hitable_time([&]() {
                    auto target = get_player_if(not_self_and_cmp_atk_col);
                    if (target.empty()) return;
                    bool is_cri_b = is_cri(c.cri);

                    int dmg = is_cri_b ? c.op * 2
                                       : c.op;
                    yc_range::for_each(target, [&](auto i) {
                        if (a.hitted.find(i->user_id) != a.hitted.end()) return;
                        a.hitted.insert(i->user_id);
                        add_mp(player_clnt_ids, send, sync, 25);
                        sync(clnt_id, [mp_ = &mp, mmp_ = c.max_mp, champ = &c, i, dmg]{
                            if (champ->OnKill && !i->dead && i->hp - dmg <= 0) {
                                    champ->OnKill();
                            }
                        });

                        i->hit(player_clnt_ids, send, sync, is_cri_b, dmg);
                    });
                }, c.atk_start, c.atk_end, dt);
                a.on_attack_end([&]() {
                    state = random_movement_t{ yc::time_val_t(c.random_move_rate_t), yc::random_dir2d<Vec2f>() };
                });
            },
            [&](skill_t& s) {
                if (s.skill_dt == 0.f) {
                    start_skill(player_clnt_ids, send, sync);
                }
                s.skill_dt += dt;
                c.skill(this, 
                        s.dir, 
                        get_player_if(lamda(x, x != this)), 
                        s.hitted, 
                        s.skill_dt, 
                        [&](battler_t* t, int dmg) {
                            t->hit(player_clnt_ids, send, sync, false, dmg);
                        }
                );
                if (s.skill_dt >= c.skill_end_t) {
                    state = random_movement_t{ yc::time_val_t(c.random_move_rate_t), yc::random_dir2d<Vec2f>() };
                }
            }
        }, state);
    }
};


struct in_battle_t
{
    std::vector<battler_t*> players;
};
struct battle_end_t
{
    std::optional<battler_t*> winner;
};
struct battle_exit_t { int flag; };

class battle
{

public:
    std::variant<
        in_battle_t,
        battle_end_t,
        battle_exit_t
    > state;

    void start(std::vector<battler_t*> player_list)
    {
        state = in_battle_t{
            player_list
        };
    }

    template <typename send_func, typename sync_func>
    void run(float dt, send_func send, sync_func sync)
    { 
        std::visit(overloaded{
            [&](in_battle_t& in_battle) {
                using namespace ::ranges;
                yc_range::for_each(in_battle.players, [&](battler_t* i) {
                    i->run(dt,
                           in_battle.players | views::transform(lamda(x, x->clnt_id))
                                             | to_vector,
                           [&](auto&& condition) {
                               return in_battle.players | views::filter(condition)
                                                        | to_vector;
                           },
                           sync,
                           send);
                });
                auto dead_battlers = in_battle.players | views::filter(lamda(x, x->dead))
                                                       | to_vector;
                yc_range::remove_if(in_battle.players, [&](battler_t* i) {
                    bool condition = false;
                    yc_range::for_each(dead_battlers, [&](battler_t* x) { if (i == x) condition = true; });
                    return condition;
                });
                yc_range::for_each(dead_battlers, [&](battler_t* i) {
                    if(i->dead) sync(i->clnt_id, [i] { delete i; });
                });
                if (in_battle.players.size() <= 1) {
                    state = battle_end_t{ in_battle.players.size() ? std::make_optional(in_battle.players.back()) : std::nullopt };
                }
            },
            [&](battle_end_t& end) {
                if (end.winner) {
                    chatting_t t;
                    yc_str::copy(L"server: 승리!", t.text);
                    send(end.winner.value()->clnt_id, &t);
                    sync(end.winner.value()->clnt_id, [i = end.winner.value()] { delete i; });
                }
                state = battle_exit_t { 1 };
            },
            [&](auto&) {}
        }, state);
    }
};