#include <string>
#include <list>
#include <cassert>

namespace args {

template <typename T>
struct arg {
  T data;
  char option;
};

struct parser {
  std::list<std::string_view> inputs;
  std::list<arg<bool>> flags;
  std::list<arg<int>> int_options;
  std::list<arg<std::string_view>> str_options;

  auto input () -> std::string_view&;
  auto flag (char option, bool default_value = false) -> bool&;
  auto int_option (char option, int default_value = 0) -> int&;
  auto string_option (char option, std::string_view default_value = "") -> std::string_view&;
  auto parse (int argc, char** argv) -> void;
};

auto
parser :: input () -> std::string_view& {
  this->inputs.push_back ({});

  return this->inputs.back ();
}

auto
parser :: flag (char option, bool default_value) -> bool& {
  this->flags.push_back ({
    default_value,
    option,
  });

  return this->flags.back ().data;
}

auto
parser :: int_option (char option, int default_value) -> int& {
  this->int_options.push_back ({
    default_value,
    option,
  });

  return this->int_options.back ().data;
}

auto
parser :: string_option (char option, std::string_view default_value) -> std::string_view& {
  this->str_options.push_back ({
    default_value,
    option,
  });

  return this->str_options.back ().data;
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
      // FIXME: Less ugly way to do this

      bool processed = false;

      for (auto& option: this->flags) {
        if (arg[1] == option.option) {
          option.data = !option.data;

          processed = true;
          break;
        }
      }

      if (processed)
        continue;

      for (auto& option: this->int_options) {
        if (arg[1] == option.option) {
          assert (i + 1 < argc);

          std::string_view value = argv[++i];
          option.data = std::stoi (std::string {value});

          processed = true;
          break;
        }
      }

      if (processed)
        continue;

      for (auto& option: this->str_options) {
        if (arg[1] == option.option) {
          assert (i + 1 < argc);

          std::string_view value = argv[++i];
          option.data = value;

          processed = true;
          break;
        }
      }

      // Option in none of the lists, invalid
      assert (processed);
    }
  }

  assert (input_n == this->inputs.size ());
}

}
