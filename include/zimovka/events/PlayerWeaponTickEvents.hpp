#ifndef ZIMOVKA_EVENTS_PLAYERWEAPONTICKEVENTS_HPP_
#define ZIMOVKA_EVENTS_PLAYERWEAPONTICKEVENTS_HPP_

namespace zimovka{
/**
 * @brief SE/エフェクトなどを呼び出すためのイベント
 * 
 * PlayerWeaponSystemがSEなどを直接呼び出さないためのイベント管理
 */
struct WeaponTickEvents{
    bool shot_fired = false;
    bool reload_started = false;
    bool reload_completed = false;
    bool spawn_failed = false; 
};

}   // namespace zimovka

#endif  // ZIMOVKA_EVENTS_PLAYERWEAPONTICKEVENTS_HPP_

