/* Copyright [2013-2018] [Aaron Springstroh, Minimal Graphics Library]

This file is part of the Beyond Dying Skies.

Beyond Dying Skies is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beyond Dying Skies is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beyond Dying Skies.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __STATS__
#define __STATS__

#include <array>
#include <cmath>
#include <cstdint>
#include <game/callback.h>
#include <string>

namespace game
{

enum class stat_alert
{
    none,
    level,
    thruster
};

class stats
{
  private:
    static constexpr float _health_consume = 0.5;
    static constexpr float _energy_consume = 1.0;
    static constexpr float _oxygen_consume = 0.001;
    static constexpr float _health_regen = 2.5;
    static constexpr float _energy_regen = 5.0;
    static constexpr size_t _max_attr = 10;
    static constexpr size_t _max_attr_str = _max_attr - 2;
    static constexpr size_t _max_stats = 7;
    static constexpr float _per_second = 1.0 / _physics_frames;

    // Costs
    static constexpr float _beam_cost = 5.0;
    static constexpr float _charge_cost = 10.0;
    static constexpr float _grapple_cost = 0.1;
    static constexpr float _grenade_cost = 10.0;
    static constexpr float _jet_cost = 0.1;
    static constexpr float _missile_cost = 10.0;
    static constexpr float _portal_cost = 0.5;
    static constexpr float _scatter_cost = 20.0;

    float _energy;
    bool _low_energy;
    float _max_exp;
    float _exp;
    float _health;
    bool _low_health;
    float _max_oxygen;
    float _oxygen;
    bool _low_oxygen;
    float _hit;
    bool _dead;
    bool _dirty;
    stat_alert _alert;
    std::array<float, _max_attr> _attr;
    std::array<uint16_t, _max_stats> _stat;
    uint16_t _stat_points;
    float _sqrt_level;

    inline float calc_damage_mult() const
    {
        return 1.0 + (std::log10(power() * 4.0) * (_sqrt_level - 1) * 0.75);
    }
    inline float calc_thrust_consume() const
    {
        return 1.08 / std::log10(speed() * _sqrt_level + 1.0);
    }
    inline float calc_damage_reduc() const
    {
        return std::log10(vital()) * (_sqrt_level * 0.05);
    }
    inline float calc_cooldown_reduc() const
    {
        return std::log10(cooldown()) * (_sqrt_level * 0.075);
    }
    inline float calc_health_regen() const
    {
        return (_health_regen + std::log10(regen() * 2.0)) * (_sqrt_level * _per_second);
    }
    inline float calc_energy_regen() const
    {
        return (_energy_regen + std::log10(regen() * 3.0)) * (_sqrt_level * _per_second);
    }
    inline float calc_health_consume() const
    {
        return (_health_consume / _sqrt_level) * _per_second;
    }
    inline float calc_energy_consume() const
    {
        return (_energy_consume / _sqrt_level) * _per_second;
    }
    inline float calc_max_health() const
    {
        return std::log10(vital()) * (_sqrt_level * 100.0);
    }
    inline float calc_max_energy() const
    {
        return std::log10(cooldown()) * (_sqrt_level * 33.333);
    }
    inline float calc_max_exp() const
    {
        return std::floor(600.0 * std::exp(_sqrt_level * 1.2));
    }
    inline bool can_consume_energy(const float energy) const
    {
        // Try to consume energy
        if (_energy >= energy)
        {
            return true;
        }

        // Not enough energy
        return false;
    }
    inline float get_damage_mult() const
    {
        return _attr[0];
    }
    inline float get_thrust_cost_frac() const
    {
        return _attr[1];
    }
    inline float get_damage_reduc() const
    {
        return _attr[2];
    }
    inline float get_cooldown_reduc() const
    {
        return _attr[3];
    }
    inline float get_health_regen() const
    {
        return _attr[4];
    }
    inline float get_energy_regen() const
    {
        return _attr[5];
    }
    inline float get_health_consume() const
    {
        return _attr[8];
    }
    inline float get_energy_consume() const
    {
        return _attr[9];
    }
    inline float get_beam_cost() const
    {
        return get_damage_mult() * _beam_cost;
    }
    inline float get_charge_cost() const
    {
        return get_damage_mult() * _charge_cost;
    }
    inline float get_grapple_cost() const
    {
        return get_max_energy() * _grapple_cost;
    }
    inline float get_grenade_cost() const
    {
        return get_damage_mult() * _grenade_cost;
    }
    inline float get_jet_cost() const
    {
        // Cost per frame
        return _jet_cost;
    }
    inline float get_missile_cost() const
    {
        return get_damage_mult() * _missile_cost;
    }
    inline float get_portal_cost() const
    {
        return get_max_energy() * _portal_cost;
    }
    inline float get_scatter_cost() const
    {
        return get_damage_mult() * _scatter_cost;
    }
    inline float get_thrust_cost() const
    {
        return get_max_energy() * get_thrust_cost_frac();
    }
    inline void set_energy(const float energy)
    {
        // Check above warning threshold
        const bool above = _energy >= 25.0;

        // Set energy
        _energy = energy;

        // Check for low energy
        if (above && is_low_energy())
        {
            _low_energy = true;
        }
    }
    inline void set_health(const float health)
    {
        // Check above warning threshold
        const bool above = _health >= 25.0;

        // Set health
        _health = health;

        // Check for low health
        if (_health <= 0.0)
        {
            _dead = true;
        }
        else if (above && is_low_health())
        {
            _low_health = true;
        }
    }
    inline void set_oxygen(const float oxygen)
    {
        // Check above warning threshold
        const bool above = _oxygen >= 25.0;

        // Set oxygen
        _oxygen = oxygen;

        // Check for low oxygen
        if (_oxygen <= 0.0)
        {
            _dead = true;
        }
        else if (above && is_low_oxygen())
        {
            _low_oxygen = true;
        }
    }
    void update_cache()
    {
        // Cache the sqrt of level
        _sqrt_level = std::sqrt(level());

        // Update attributes
        _attr[0] = calc_damage_mult();
        _attr[1] = calc_thrust_consume();
        _attr[2] = calc_damage_reduc();
        _attr[3] = calc_cooldown_reduc();
        _attr[4] = calc_health_regen();
        _attr[5] = calc_energy_regen();
        _attr[6] = calc_max_health();
        _attr[7] = calc_max_energy();
        _attr[8] = calc_health_consume();
        _attr[9] = calc_energy_consume();

        // Update max experience
        _max_exp = calc_max_exp();
    }

  public:
    static std::array<std::string, _max_attr_str> _attr_str;
    static std::array<std::string, _max_stats> _stat_str;
    stats()
        : _energy(10.0), _low_energy(false),
          _max_exp(100.0), _exp(0.0),
          _health(70.0), _low_health(false),
          _max_oxygen(100.0), _oxygen(_max_oxygen), _low_oxygen(false),
          _hit(0.0), _dead(false), _dirty(false), _alert(stat_alert::none),
          _attr{}, _stat{4, 3, 5, 2, 0, 3, 1}, _stat_points(0), _sqrt_level(1.0)
    {
        // Update the stat cache
        update_cache();
    }

    inline void add_energy(const float energy)
    {
        _energy += energy;
    }
    inline void add_exp(const float exp)
    {
        _exp += exp;

        // If experience overflow
        if (_exp >= _max_exp)
        {
            _exp -= _max_exp;

            // Level up
            level_up();
        }
    }
    inline void add_health(const float health)
    {
        _health += health;
    }
    inline void add_oxygen(const float oxy)
    {
        _oxygen += oxy;

        // If oxygen overflow
        if (_oxygen > _max_oxygen)
        {
            _oxygen = _max_oxygen;
        }
    }
    inline static constexpr size_t attr_str_size()
    {
        return _max_attr_str;
    }
    inline static const std::string &attr_str(const size_t index)
    {
        return _attr_str[index];
    }
    inline float attr_value(const size_t index)
    {
        // Formatting attr for display
        switch (index)
        {
        case 0:
            return _attr[index];
        case 1:
            return _attr[index] * 100.0;
        case 2:
            return _attr[index] * 100.0;
        case 3:
            return _attr[index] * 100.0;
        case 4:
            return _attr[index] * _physics_frames;
        case 5:
            return _attr[index] * _physics_frames;
        default:
            return _attr[index];
        }
    }
    inline void clean()
    {
        _dirty = false;
    }
    inline void clear_alert()
    {
        _alert = stat_alert::none;
    }
    inline void clear_hit()
    {
        _hit = 0.0;
    }
    inline void clear_low_energy_flag()
    {
        _low_energy = false;
    }
    inline void clear_low_health_flag()
    {
        _low_health = false;
    }
    inline void clear_low_oxygen_flag()
    {
        _low_oxygen = false;
    }
    inline bool can_consume_beam() const
    {
        return can_consume_energy(get_beam_cost());
    }
    inline bool can_consume_charge() const
    {
        return can_consume_energy(get_charge_cost());
    }
    inline bool can_consume_grapple() const
    {
        return can_consume_energy(get_grapple_cost());
    }
    inline bool can_consume_grenade() const
    {
        return can_consume_energy(get_grenade_cost());
    }
    inline bool can_consume_jet() const
    {
        return can_consume_energy(get_jet_cost());
    }
    inline bool can_consume_missile() const
    {
        return can_consume_energy(get_missile_cost());
    }
    inline bool can_consume_portal() const
    {
        return can_consume_energy(get_portal_cost());
    }
    inline bool can_consume_scatter() const
    {
        return can_consume_energy(get_scatter_cost());
    }
    inline bool can_consume_thrust() const
    {
        return can_consume_energy(get_thrust_cost());
    }
    inline void consume_beam()
    {
        consume_energy(get_beam_cost());
    }
    inline void consume_charge()
    {
        consume_energy(get_charge_cost());
    }
    inline void consume_grapple()
    {
        consume_energy(get_grapple_cost());
    }
    inline void consume_grenade()
    {
        consume_energy(get_grenade_cost());
    }
    inline void consume_jet()
    {
        consume_energy(get_jet_cost());
    }
    inline void consume_missile()
    {
        consume_energy(get_missile_cost());
    }
    inline void consume_portal()
    {
        consume_energy(get_portal_cost());
    }
    inline void consume_scatter()
    {
        consume_energy(get_scatter_cost());
    }
    inline void consume_thrust()
    {
        consume_energy(get_thrust_cost());
    }
    inline void consume_energy(const float energy)
    {
        set_energy(_energy - energy);
    }
    inline void consume_health(const float hit)
    {
        // Calculate damage
        _hit += hit;

        set_health(_health - hit);
    }
    inline void consume_oxygen()
    {
        set_oxygen(_oxygen - _oxygen_consume);
    }
    inline void damage(const float in)
    {
        // Calculate damage reduction
        const float reduc = get_damage_reduc();

        // Apply damage reduction
        consume_health((1.0 - reduc) * in);
    }
    inline float do_damage(const float in) const
    {
        return get_damage_mult() * in;
    }
    inline stat_alert get_alert() const
    {
        return _alert;
    }
    inline float get_cooldown_mult() const
    {
        return 1.0 - get_cooldown_reduc();
    }
    inline float get_drone_health() const
    {
        return 100.0 * _sqrt_level;
    }
    inline float get_energy() const
    {
        return _energy;
    }
    inline float get_energy_fraction() const
    {
        return _energy / get_max_energy();
    }
    inline float get_exp() const
    {
        return _exp;
    }
    inline float get_drop_exp() const
    {
        return 25.0;
    }
    inline float get_mob_exp() const
    {
        return _max_exp / level();
    }
    inline float get_experience_fraction() const
    {
        return _exp / _max_exp;
    }
    inline float get_health() const
    {
        return _health;
    }
    inline float get_max_energy() const
    {
        return _attr[7];
    }
    inline float get_max_health() const
    {
        return _attr[6];
    }
    inline float get_health_fraction() const
    {
        return _health / get_max_health();
    }
    inline float get_hit() const
    {
        return _hit;
    }
    inline float get_oxygen() const
    {
        return _oxygen;
    }
    inline float get_oxygen_fraction() const
    {
        return _oxygen / _max_oxygen;
    }
    inline uint16_t get_stat_points() const
    {
        return _stat_points;
    }
    inline void fill(const std::array<uint16_t, _max_stats> &stat, const float energy, const float exp, const float health, const float oxygen, const uint16_t stats)
    {
        // Copy stats into stat array
        for (size_t i = 0; i < _max_stats; i++)
        {
            _stat[i] = stat[i];
        }

        // Update the stat cache
        update_cache();

        // Set the energy
        set_energy(energy);

        // Set the exp
        add_exp(exp);

        // Set the health
        set_health(health);

        // Set the oxygen
        set_oxygen(oxygen);

        // Set the stat points
        _stat_points = stats;
    }
    inline bool has_stat_points() const
    {
        return _stat_points > 0;
    }
    inline bool is_dead() const
    {
        return _dead;
    }
    inline bool is_dirty() const
    {
        return _dirty;
    }
    inline bool is_hit() const
    {
        return (_hit > 0.0);
    }
    inline bool is_level_up() const
    {
        return _dirty;
    }
    inline bool is_low_energy() const
    {
        return _energy < get_max_energy() * 0.25;
    }
    inline bool is_low_energy_flag() const
    {
        return _low_energy;
    }
    inline bool is_low_health() const
    {
        return _health < get_max_health() * 0.25;
    }
    inline bool is_low_health_flag() const
    {
        return _low_health;
    }
    inline bool is_low_oxygen() const
    {
        return _oxygen < 25.0;
    }
    inline bool is_low_oxygen_flag() const
    {
        return _low_oxygen;
    }
    inline void regen_energy()
    {
        // Absorb this amount of energy if not full
        const float max_energy = get_max_energy();
        if (_energy < max_energy)
        {
            // Calculate energy change
            const float energy = get_energy_regen();
            _energy += energy;

            // Cap energy at max_energy
            if (_energy > max_energy)
            {
                _energy = max_energy;
            }
        }
        // Consume energy if over full
        else if (_energy > max_energy)
        {
            // Calculate energy change
            const float energy = get_energy_consume();
            _energy -= energy;

            // Cap energy at max_energy
            if (_energy < max_energy)
            {
                _energy = max_energy;
            }
        }
    }
    inline void regen_health()
    {
        // Absorb this amount of health if not full
        const float max_health = get_max_health();
        if (_health < max_health)
        {
            // Calculate health change
            const float health = get_health_regen();
            _health += health;

            // Cap health at max_health
            if (_health > max_health)
            {
                _health = max_health;
            }
        }
        // Consume health if over full
        else if (_health > max_health)
        {
            // Calculate health change
            const float health = get_health_consume();
            _health -= health;

            // Cap health at max_health
            if (_health < max_health)
            {
                _health = max_health;
            }
        }
    }
    inline void respawn()
    {
        // Reset energy
        _energy = get_max_energy();
        _low_energy = false;

        // Reset experience
        _exp = 0.0;

        // Reset health
        _health = get_max_health();
        _low_health = false;

        // Reset health
        _oxygen = _max_oxygen;
        _low_oxygen = false;
        _hit = 0.0;
        _dead = false;
    }
    inline void set_point(const size_t index)
    {
        if (_stat_points > 0)
        {
            // Are thrusters offline?
            const bool thrust_above = get_thrust_cost_frac() > 1.0;

            // Subtract stat point
            _stat_points--;

            // Allocate stat point
            _stat[index]++;

            // Update the stat cache
            update_cache();

            // Set thrust alert if enabled
            if (thrust_above && get_thrust_cost_frac() <= 1.0)
            {
                _alert = stat_alert::thruster;
            }

            // Set dirty flag
            _dirty = true;
        }
    }
    inline static constexpr size_t stat_str_size()
    {
        return _max_stats;
    }
    inline static const std::string &stat_str(const size_t index)
    {
        return _stat_str[index];
    }
    inline uint16_t stat_value(const size_t index) const
    {
        return _stat[index];
    }
    inline uint16_t power() const
    {
        return _stat[0];
    }
    inline uint16_t speed() const
    {
        return _stat[1];
    }
    inline uint16_t vital() const
    {
        return _stat[2];
    }
    inline uint16_t cooldown() const
    {
        return _stat[3];
    }
    inline uint16_t range() const
    {
        return _stat[4];
    }
    inline uint16_t regen() const
    {
        return _stat[5];
    }
    inline uint16_t level() const
    {
        return _stat[6];
    }
    inline void level_up()
    {
        // Update stats
        _stat[6]++;

        // Add stat points
        _stat_points += 5;

        // Update the stat cache
        update_cache();

        // Set level alert
        _alert = stat_alert::level;

        // Set dirty flag
        _dirty = true;
    }
};

// Initialize public static string stats
std::array<std::string, stats::attr_str_size()> stats::_attr_str = {"Damage Multiplier", "Thrust Cost (%)", "Damage Reduction (%)", "Cooldown Reduction (%)", "Health Regen (/s)", "Energy Regen (/s)", "Max Health", "Max Energy"};
std::array<std::string, stats::stat_str_size()> stats::_stat_str = {"Power", "Dynamism", "Tenacity", "Tranquility", "Vision", "Zeal", "Level"};
}

#endif
