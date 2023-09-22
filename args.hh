#include <string>
#include <list>
#include <variant>
#include <type_traits>
#include <cassert>

namespace args {

struct arg {
  char c;
  std::variant<bool, int, std::string_view> value;
};

struct parser {
  std::list<std::string_view> inputs;
  std::list<arg> options;

  auto input () -> std::string_view&;
  auto flag (char c, bool default_value = false) -> bool&;
  auto number_option (char c, int default_value = 0) -> int&;
  auto string_option (char c, std::string_view default_value = "") -> std::string_view&;
  auto parse (int argc, char** argv) -> void;
};

auto
parser :: input () -> std::string_view& {
  this->inputs.push_back ({});

  return this->inputs.back ();
}

auto
parser :: flag (char c, bool default_value) -> bool& {
  this->options.push_back ({c, default_value});

  return std::get<bool> (this->options.back ().value);
}

auto
parser :: number_option (char c, int default_value) -> int& {
  this->options.push_back ({c, default_value});

  return std::get<int> (this->options.back ().value);
}

auto
parser :: string_option (char c, std::string_view default_value) -> std::string_view& {
  this->options.push_back ({c, default_value});

  return std::get<std::string_view> (this->options.back ().value);
}

auto
parser :: parse (int argc, char** argv) -> void {
  size_t input_n = 0;

  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];

    if (arg[0] != '-') {
      assert (input_n < this->inputs.size ());

      this->inputs.assign (input_n++, arg);
    } else {
      for (auto& opt: this->options) {
        if (arg[1] != opt.c)
          continue;

        std::visit ([&] (auto& val) -> void {
          using T = std::decay_t<decltype (val)>;

          if constexpr (std::is_same_v<T, bool>) {
            val = !val;
          } else {
            assert (i + 1 < argc);

            if constexpr (std::is_same_v<T, int>)
              val = std::stoi (argv[++i]);
            else // (std::is_same_v<T, std::string_view)
              val = argv[++i];
          }
        }, opt.value);

        break;
      }
    }
  }

  assert (input_n == this->inputs.size ());
}

}
