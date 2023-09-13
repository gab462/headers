#include "../sdl.hh"

namespace game {

enum wall {
  player,
  top,
  right,
  bottom,
  left,
  total
};

}

auto
main (int argc, char** argv) -> int {
  using game::wall;
  sdl::renderer renderer;
  sdl::event_manager app;
  std::array<sdl::body, wall::total> walls {};
  sdl::body ball {50, 50, 50, 50, 1, 1};

  for (auto& w: walls) {
    w.pos.w = 10;
    w.pos.h = 10;
  }

  walls[wall::player].pos.w = 200;
  walls[wall::player].pos.h = 20;

  app.subscribe (sdl::event::window_resize, [&] (sdl::event::data data) -> void {
    auto [w, h] = std::get<sdl::event::window_resized> (data);

    walls[wall::player].pos.y = h - 100;

    walls[wall::top].pos.w = w;
    walls[wall::bottom].pos.w = w;
    walls[wall::bottom].pos.y = h - 10;
    walls[wall::right].pos.x = w - 10;
    walls[wall::left].pos.h = h;
    walls[wall::right].pos.h = h;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    auto [dt] = std::get<sdl::event::frame_tick> (data);
    using sdl::collision;

    for (auto& w: walls) {
      switch (ball.collide (w)) {
      case collision::up:
        ball.vel.y = -1.0f;
        break;
      case collision::down:
        ball.vel.y = 1.0f;
        break;
      case collision::left:
        ball.vel.x = 1.0f;
        break;
      case collision::right:
        ball.vel.x = -1.0f;
        break;
      default:
        break;
      }
    }

    ball.pos.x += ball.vel.x * dt / 1.5f;
    ball.pos.y += ball.vel.y * dt / 1.5f;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    // TODO: wrap
    auto [dt] = std::get<sdl::event::frame_tick> (data);

    if (app.keyboard_state[SDL_SCANCODE_A])
      walls[wall::player].pos.x -= dt / 2.0f;
    if (app.keyboard_state[SDL_SCANCODE_D])
      walls[wall::player].pos.x += dt / 2.0f;
  });

  auto render = [&] (sdl::renderer& r) {
    r.set_color (255, 255, 255, 255);
    r.clear ();
    r.set_color (255, 0, 0, 255);
    r.draw_rect (ball.pos);

    for (auto& w: walls)
      r.draw_rect (w.pos);
  };

  app.loop (renderer, render);

  return 0;
}
