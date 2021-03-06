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
#ifndef __TITLE_CONTROLS__
#define __TITLE_CONTROLS__

#include <functional>
#include <game/ui_overlay.h>
#include <min/camera.h>
#include <min/window.h>
#include <stdexcept>

namespace game
{

class title
{
  private:
    min::camera<float> *_camera;
    game::ui_overlay *_ui;
    min::window *_window;
    bool _show;

  public:
    title(min::camera<float> &camera, game::ui_overlay &ui, min::window &window)
        : _camera(&camera), _ui(&ui), _window(&window), _show(true)
    {
        // Register callbacks
        register_control_callbacks();
    }
    void register_control_callbacks()
    {
        // Enable the console and set default message
        _ui->enable_console();
        _ui->set_console_string("Click To Start");

        // Get access to the keyboard
        auto &keyboard = _window->get_keyboard();

        // Clear any keys mapped to keyboard
        keyboard.clear();

        // Register data and function callbacks
        _window->register_data((void *)this);
        _window->register_lclick_down(title::left_click_down);
        _window->register_lclick_up(nullptr);
        _window->register_rclick_down(nullptr);
        _window->register_rclick_up(nullptr);
        _window->register_update(title::on_resize);
    }
    min::camera<float> *get_camera()
    {
        return _camera;
    }
    game::ui_overlay *get_ui()
    {
        return _ui;
    }
    min::window *get_window()
    {
        return _window;
    }
    static void left_click_down(void *ptr, const uint_fast16_t x, const uint_fast16_t y)
    {
        // Cast to title pointer
        title *const t = reinterpret_cast<title *>(ptr);

        // Stop showing the title screen
        t->set_show_title(false);
    }
    void set_show_title(const bool flag)
    {
        _show = flag;
    }
    bool is_show_title() const
    {
        return _show;
    }
    static void on_resize(void *ptr, const uint_fast16_t width, const uint_fast16_t height)
    {
        // Ignore minimizing window
        if (width == 0 && height == 0)
        {
            return;
        }

        // Cast to title pointer
        title *const t = reinterpret_cast<title *>(ptr);
        min::camera<float> *const camera = t->get_camera();
        game::ui_overlay *const ui = t->get_ui();

        // Get camera frustum
        auto &f = camera->get_frustum();

        // Update the aspect ratio
        f.set_aspect_ratio(width, height);
        f.make_dirty();
        camera->make_dirty();

        // Get the screen dimensions
        const uint_fast16_t w2 = width / 2;
        const uint_fast16_t h2 = height / 2;

        // Update the screen size for ui and text
        ui->set_screen(min::vec2<float>(w2, h2), width, height);
    }
};
}

#endif
