#include "../sdl.hh"

namespace game {

enum class collision {
  none,
  up,
  down,
  left,
  right
};

struct body {
  SDL_FRect pos;
  SDL_FPoint vel;

  auto collide (body const& other) -> collision;
};

auto
body :: collide (body const& other) -> collision {
  SDL_FRect res;

  if (!SDL_IntersectFRect (&this->pos, &other.pos, &res))
    return collision::none;

  if (res.w > res.h) // vertical
    return this->pos.y > other.pos.y ? collision::down : collision::up;
  else // horizontal
    return this->pos.x > other.pos.x ? collision::left : collision::right;
}

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
  constexpr auto wall_size = 10.f;

  sdl::renderer renderer;
  sdl::event_manager app;
  std::array<game::body, wall::total> walls {};
  game::body ball {wall_size, wall_size, wall_size, wall_size, 1.f, 1.f};
  bool paused = false;

  for (auto& wall: walls) {
    wall.pos.w = wall_size;
    wall.pos.h = wall_size;
  }

  app.subscribe (sdl::event::window_resize, [&] (sdl::event::data data) -> void {
    auto [w, h] = std::get<sdl::event::window_resized> (data);

    walls[wall::player].pos.y = h * (5.f / 6.f);
    walls[wall::player].pos.w = w / 8.f;
    walls[wall::player].pos.h = wall_size;

    walls[wall::top].pos.w = w;
    walls[wall::bottom].pos.w = w;
    walls[wall::bottom].pos.y = h - wall_size;
    walls[wall::right].pos.x = w - wall_size;
    walls[wall::left].pos.h = h;
    walls[wall::right].pos.h = h;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    using game::collision;
    constexpr auto speed = 1.f;

    if (paused)
      return;

    auto [dt] = std::get<sdl::event::frame_tick> (data);

    for (auto& w: walls) {
      switch (ball.collide (w)) {
      case collision::up:
        ball.vel.y = -speed;
        break;
      case collision::down:
        ball.vel.y = speed;
        break;
      case collision::left:
        ball.vel.x = speed;
        break;
      case collision::right:
        ball.vel.x = -speed;
        break;
      default:
        break;
      }
    }

    ball.pos.x += ball.vel.x * dt / 1.5f;
    ball.pos.y += ball.vel.y * dt / 1.5f;
  });

  app.subscribe (sdl::event::frame, [&] (sdl::event::data data) -> void {
    if (paused)
      return;

    auto [dt] = std::get<sdl::event::frame_tick> (data);

    if (app.keyboard_state[SDL_SCANCODE_A])
      walls[wall::player].pos.x -= dt / 2.0f;
    if (app.keyboard_state[SDL_SCANCODE_D])
      walls[wall::player].pos.x += dt / 2.0f;
  });

  app.subscribe (sdl::event::key_press, [&] (sdl::event::data data) -> void {
      auto [key] = std::get<sdl::event::key_pressed> (data);

      if (key == 'p')
        paused = !paused;
  });

  auto render = [&] (sdl::renderer& r) -> void {
    r.set_color (0, 0, 0, 255);
    r.clear ();
    r.set_color (255, 255, 255, 255);
    r.draw_rect (ball.pos);

    for (auto& w: walls)
      r.draw_rect (w.pos);
  };

  app.loop (renderer, render);

  return 0;
}
