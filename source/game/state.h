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
#ifndef __GAME_STATE__
#define __GAME_STATE__

#include <game/character.h>
#include <game/file.h>
#include <game/load_state.h>
#include <game/options.h>
#include <game/player.h>
#include <game/static_instance.h>

namespace game
{
class state
{
  private:
    static constexpr unsigned _frame_average = 4;
    static constexpr float _recoil_x = 60.0;
    static constexpr float _recoil_y = -60.0;
    static constexpr unsigned _recoil_frames = 6;
    static constexpr float _run_stride = 0.05;
    load_state _state;
    bool _tracking;
    min::vec3<float> _target;
    unsigned _frame_count;
    float _x[_frame_average];
    float _y[_frame_average];
    int _recoil;
    min::camera<float> _camera;
    min::quat<float> _q;
    float _run_accum;
    float _run_accum_sin;
    min::mat4<float> _model;
    bool _dead;
    bool _pause;
    bool _respawn;
    bool _user_input;

    inline void load_camera(const options &opt)
    {
        // Set camera near and far plane, and set perspective
        auto &f = _camera.get_frustum();
        f.set_aspect_ratio(opt.width(), opt.height());
        f.set_fov(90.0);
        f.set_far(5000.0);
        _camera.set_perspective();

        // Load camera settings
        set_camera(_state.get_spawn(), _state.get_look());
    }
    inline void update_model_matrix(const float speed, const float dt)
    {
        const min::vec3<float> &f = _camera.get_forward();
        const min::vec3<float> &fup = _camera.get_frustum().get_up();
        const min::vec3<float> &fr = _camera.get_frustum().get_right();

        // Update the md5 model matrix
        const min::vec3<float> offset = _camera.get_position() + (f - fup + fr) * 0.5;

        // Accumulate frame time, reset every 180 cycles
        _run_accum += speed * dt * 3.0;
        _run_accum = std::fmod(_run_accum, 1130.97335529);
        _run_accum_sin = std::sin(_run_accum);

        // Calculate running offset
        const float stride = _run_accum_sin * _run_stride;
        const min::vec3<float> run = (fr + fup) * stride;

        // Set model matrix
        _model = min::mat4<float>(offset + run, _q);
    }
    inline min::quat<float> update_model_rotation() const
    {
        const min::vec3<float> &f = _camera.get_forward();
        const min::vec3<float> &fup = _camera.get_frustum().get_up();
        const min::vec3<float> &fr = _camera.get_frustum().get_right();

        // Calculate the forward vector
        min::vec3<float> d(f.x(), 0.0, f.z());
        d.normalize();

        // Transform the model rotation around shortest arc or Y axis
        const min::vec3<float> y(0.0, 1.0, 0.0);
        const min::vec3<float> x(-1.0, 0.0, 0.0);
        const min::quat<float> roty(x, d, y);

        // Transform the model rotation around shortest arc or RIGHT axis
        const min::quat<float> rotzx(y, fup, fr);

        // Return the transformed model rotation
        return rotzx * roty;
    }

  public:
    state(const options &opt)
        : _state(opt.grid(), opt.mode()), _tracking(false), _frame_count(0), _x{}, _y{},
          _recoil(0), _run_accum(0.0), _run_accum_sin(0.0),
          _dead(false), _pause(false), _respawn(false), _user_input(false)
    {
        // Load camera
        load_camera(opt);
    }
    inline min::camera<float> &get_camera()
    {
        return _camera;
    }
    inline const min::camera<float> &get_camera() const
    {
        return _camera;
    }
    inline const load_state &get_load_state() const
    {
        return _state;
    }
    inline const min::mat4<float> &get_model_matrix() const
    {
        return _model;
    }
    inline bool get_pause() const
    {
        return _pause;
    }
    inline bool get_tracking() const
    {
        return _tracking;
    }
    inline bool get_user_input() const
    {
        return _user_input;
    }
    inline bool is_dead() const
    {
        return _dead;
    }
    inline bool is_respawn() const
    {
        return _respawn;
    }
    inline void respawn()
    {
        // Reset flags
        _tracking = false;
        _recoil = 0;

        // Reload camera settings
        set_camera(_state.get_default_spawn(), _state.get_default_look());

        // Reset flags
        _run_accum = 0.0;
        _run_accum_sin = 0.0;
        _dead = false;
        _respawn = false;
    }
    inline void save_state(const static_instance &si, const player &p)
    {
        _state.save_state(si, p.get_inventory(), p.get_stats(), _camera, p.position());
    }
    inline void set_camera(const min::vec3<float> &p, const min::vec3<float> &look)
    {
        // Set camera start position and look position
        _camera.set(p + min::vec3<float>(0.0, 0.5, 0.0), look);

        // Force camera to update internals
        _camera.force_update();

        // Update rotation quaternion
        _q = update_model_rotation();
    }
    inline void set_dead(const bool flag)
    {
        _dead = flag;
    }
    inline void set_pause(const bool mode)
    {
        _pause = mode;
    }
    inline void set_recoil()
    {
        _recoil = _recoil_frames;
    }
    inline void set_respawn(const bool flag)
    {
        _respawn = flag;
    }
    inline void set_user_input(const bool mode)
    {
        _user_input = mode;
    }
    inline void set_target(const min::vec3<float> &target)
    {
        // Set the look at target to track
        _target = target;
    }
    inline void set_tracking(const bool flag)
    {
        // Enable fixed look at
        _tracking = flag;
    }
    inline bool toggle_pause()
    {
        return _pause = !_pause;
    }
    inline bool toggle_user_input()
    {
        return _user_input = !_user_input;
    }
    void update(const min::vec3<float> &p, const std::pair<uint_fast16_t, uint_fast16_t> &c, const uint_fast16_t w, const uint_fast16_t h, const float speed, const float dt)
    {
        // Calculate position to move camera to
        const min::vec3<float> move = p + min::vec3<float>(0.0, 0.5, 0.0);

        // Check if we are fixing look at target
        if (_tracking)
        {
            // Set camera start position and look position
            _camera.set(move, _target);

            // Force camera to update
            _camera.force_update();

            // Interpolate between the two rotations to avoid jerking
            _q = update_model_rotation();
        }
        else
        {
            // Set camera start position and look position
            _camera.set_position(move);

            // Increment frame count for updating and hash to current frame index
            const unsigned frame_index = (_frame_count %= _frame_average)++;

            // Get the offset from screen center
            const uint_fast16_t w2 = w / 2;
            const uint_fast16_t h2 = h / 2;
            const int_fast16_t dx = c.first - w2;
            const int_fast16_t dy = c.second - h2;
            constexpr float sensitivity = 0.25;
            _x[frame_index] = dx * sensitivity;
            _y[frame_index] = dy * sensitivity;

            // Calculate average value of x and y
            float x = 0.0;
            float y = 0.0;
            for (unsigned i = 0; i < _frame_average; i++)
            {
                x += _x[i];
                y += _y[i];
            }

            // Average value for x and y for last N frames
            x /= _frame_average;
            y /= _frame_average;

            // If we need to apply recoil
            if (_recoil > 0)
            {
                x += (_run_accum_sin * _recoil_x * dt);
                y += (_recoil_y * dt);

                // Decrement recoil flag
                _recoil--;
            }

            // If the mouse coordinates moved at all
            if (std::abs(x) > 1E-3 || std::abs(y) > 1E-3)
            {
                // Get the camera forward vector
                const min::vec3<float> &forward = _camera.get_forward();

                // Check if we have looked too far on the global y axis
                const float dy = forward.dot(min::vec3<float>::up());
                if (dy > 0.999 && y < 0.0)
                {
                    y = 0.0;
                }
                else if (dy < -0.999 && y > 0.0)
                {
                    y = 0.0;
                }

                // Adjust the camera by the offset from screen center
                _camera.move_look_at(x, y);

                // Force camera to update internals
                _camera.force_update();

                // Interpolate between the two rotations to avoid jerking
                _q = update_model_rotation();
            }
        }

        // Update the md5 model matrix
        update_model_matrix(speed, dt);
    }
};
}

#endif
