#include <array>
#include <list>
#include <functional>
#include <variant>
#include <cassert>

#include <SDL.h>

namespace sdl {

namespace event {

enum type {
  frame,
  click,
  key_press,
  window_resize,
  total
};

struct frame_tick {
  float dt;
};

struct clicked {
  int x, y;
};

struct key_pressed {
  int key;
};

struct window_resized {
  int w, h;
};

using data = std::variant<frame_tick, clicked, key_pressed, window_resized>;

}

struct renderer {
  SDL_Window* wind;
  SDL_Renderer* rend;

  renderer ();
  ~ renderer ();

  auto size () -> SDL_Point;

  auto set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> void;
  auto clear () -> void;
  auto draw_rect (SDL_FRect const& r) -> void;
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
  return {w, h};
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
renderer :: draw_rect (SDL_FRect const& r) -> void {
  SDL_RenderFillRectF (this->rend, &r);
}

auto
renderer :: show () -> void {
  SDL_RenderPresent (this->rend);
}

struct event_manager {
  bool running;
  const uint8_t* keyboard_state;
  std::array<std::list<std::function<void(event::data)>>, event::type::total> fns;

  event_manager ();

  auto subscribe (event::type type, std::function<void(event::data)> fn) -> void;
  auto loop (renderer& rend, std::function<void(renderer&)> renderer_fn) -> void;
};

event_manager :: event_manager (): running {true}, keyboard_state {nullptr}, fns {} {}

auto
event_manager :: subscribe (event::type type, std::function<void(event::data)> fn) -> void {
  this->fns[type].push_back (fn);
}

auto
event_manager :: loop (renderer& rend, std::function<void(renderer&)> render_fn) -> void {
  this->keyboard_state = SDL_GetKeyboardState (NULL);

  auto [w, h] = rend.size ();
  for (auto fn: this->fns[event::type::window_resize])
    fn (event::window_resized {w, h});

  uint64_t prev = 0;
  SDL_Event e;

  while (this->running) {
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
      fn (event::frame_tick {static_cast<float> (dt)});

    render_fn (rend);

    rend.show ();
  }
}

}
