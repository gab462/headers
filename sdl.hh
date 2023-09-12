// TODO: cleanup for multiple scenes

#include <array>
#include <list>
#include <functional>
#include <variant>
#include <cassert>

#include <SDL.h>

namespace sdl {

enum class collision {
  none,
  up,
  down,
  left,
  right
};

struct rect {
  SDL_FRect data;

  auto collide (sdl::rect const& other) -> collision;
};

auto
rect :: collide (rect const& other) -> collision {
  SDL_FRect res;

  if (!SDL_IntersectFRect (&this->data, &other.data, &res))
    return collision::none;

  if (res.w > res.h) // vertical
    return this->data.y > other.data.y ? collision::down : collision::up;
  else // horizontal
    return this->data.x > other.data.x ? collision::left : collision::right;
}

namespace event {

enum type {
  frame,
  click,
  key_press,
  window_resize,
  total
};

struct key_pressed {
  int key;
};

struct clicked {
  int x, y;
};

struct window_resized {
  int w, h;
};

struct frame_tick {
  uint64_t dt;
};

using data = std::variant<key_pressed, clicked, window_resized, frame_tick>;

}

struct renderer {
  SDL_Window* wind;
  SDL_Renderer* rend;

  renderer ();
  ~ renderer ();

  auto size () -> SDL_Point;

  auto set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> void;
  auto clear () -> void;
  auto draw_rect (rect const& r) -> void;
  auto show () -> void;
};

renderer :: renderer () {
  assert (SDL_Init (SDL_INIT_VIDEO) >= 0);

  SDL_CreateWindowAndRenderer (800, 600,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
      &this->wind, &this->rend);

  assert (this->wind!= NULL && this->rend != NULL);
}

renderer :: ~ renderer () {
  SDL_DestroyRenderer (this->rend);
  SDL_DestroyWindow (this->wind);
  SDL_Quit ();
}

auto
renderer :: size () -> SDL_Point {
  int w, h;
  SDL_GetRendererOutputSize (this->rend, &w, &h);
  return { w, h };
}

auto
renderer :: set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> void {
  SDL_SetRenderDrawColor (this->rend, r, g, b, a);
}

auto
renderer :: clear () -> void {
  SDL_RenderClear (this->rend);
}

auto
renderer :: draw_rect (rect const& r) -> void {
  SDL_RenderFillRectF (this->rend, &r.data);
}

auto
renderer :: show () -> void {
  SDL_RenderPresent (this->rend);
}

struct app {
  renderer rend;
  const uint8_t* keyboard_state;
  std::array<std::list<std::function<void(event::data)>>, event::type::total> fns;

  auto subscribe (event::type type, std::function<void(event::data)> fn) -> void;
  auto loop (std::function<void(renderer&)> renderer_fn) -> void;
};

auto
app :: subscribe (event::type type, std::function<void(event::data)> fn) -> void {
  this->fns[type].push_back (fn);
}

auto
app :: loop (std::function<void(renderer&)> render_fn) -> void {
  uint64_t prev = 0;

  auto [w, h] = this->rend.size ();
  for (auto fn: this->fns[event::type::window_resize])
    fn (event::window_resized {w, h});

  this->keyboard_state = SDL_GetKeyboardState (NULL);

  SDL_Event e;

  for (;;) {
    uint64_t now = SDL_GetTicks64 ();
    uint64_t dt = now - prev;
    prev = now;

    while (SDL_PollEvent (&e)) {
      switch (e.type) {
      case SDL_KEYDOWN:
        for (auto fn: this->fns[event::type::key_press])
          fn (event::key_pressed {e.key.keysym.sym});
        break;
      case SDL_MOUSEBUTTONDOWN:
        for (auto fn: this->fns[event::type::click])
          fn (event::clicked {e.button.x, e.button.y});
        break;
      case SDL_WINDOWEVENT:
        if (e.window.event != SDL_WINDOWEVENT_SIZE_CHANGED)
          break;
        for (auto fn: this->fns[event::type::window_resize])
          fn (event::window_resized {e.window.data1, e.window.data2});
        break;
      case SDL_QUIT:
        return;
      default:
        break;
      }
    }

    for (auto fn: this->fns[event::type::frame])
      fn (event::frame_tick {dt});

    render_fn (this->rend);

    this->rend.show ();
  }
}

using point = SDL_FPoint;

}
