#include "../sdl.hh"

namespace game {

struct ball {
  sdl::rect body;
  sdl::point velocity;

  ball (): body {20, 20, 50, 50}, velocity{1, 1} {}
};

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
  sdl::app app;
  std::array<sdl::rect, wall::total> walls {};
  game::ball ball;

  for (auto& w: walls) {
    w.data.w = 10;
    w.data.h = 10;
  }

  walls[wall::player].data.w = 200;
  walls[wall::player].data.h = 20;

  app.subscribe (sdl::event::window_resize, [&] (sdl::event::data data) -> void {
    auto [w, h] = std::get<sdl::event::window_resized> (data);

    walls[wall::player].data.y = h - 100;

    walls[wall::top].data.w = w;
    walls[wall::bottom].data.w = w;
    walls[wall::bottom].data.y = h - 10;
    walls[wall::right].data.x = w - 10;
    walls[wall::left].data.h = h;
    walls[wall::right].data.h = h;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    auto [dt] = std::get<sdl::event::frame_tick> (data);
    using sdl::collision;

    for (auto& w: walls) {
      switch (ball.body.collide (w)) {
      case collision::up:
        ball.velocity.y = -1.0f;
        break;
      case collision::down:
        ball.velocity.y = 1.0f;
        break;
      case collision::left:
        ball.velocity.x = 1.0f;
        break;
      case collision::right:
        ball.velocity.x = -1.0f;
        break;
      default:
        break;
      }
    }

    ball.body.data.x += ball.velocity.x * dt / 2.0f;
    ball.body.data.y += ball.velocity.y * dt / 2.0f;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    // TODO: wrap
    auto [dt] = std::get<sdl::event::frame_tick> (data);

    if (app.keyboard_state[SDL_SCANCODE_A])
      walls[wall::player].data.x -= dt;
    if (app.keyboard_state[SDL_SCANCODE_D])
      walls[wall::player].data.x += dt;
  });

  auto render = [&] (sdl::renderer& r) {
    r.set_color (255, 255, 255, 255);
    r.clear ();
    r.set_color (255, 0, 0, 255);
    r.draw_rect (ball.body);

    for (auto& w: walls)
      r.draw_rect (w);
  };

  app.loop (render);

  return 0;
}
