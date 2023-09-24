#include <vector>
#include <list>
#include <string>
#include <unistd.h>

namespace build {

struct cmd {
  std::vector<const char*> args;

  cmd ();
  cmd (std::vector<const char*> argv);
  template <typename ...A> cmd (A... a);

  auto execute () -> void;
};

cmd :: cmd (): args {} {}

cmd :: cmd (std::vector<const char*> argv): args {argv} {}

template <typename ...A>
cmd :: cmd (A... a): args {a..., nullptr} {}

auto
cmd :: execute () -> void {
  // FIXME: Any way other than const_cast?
  execvp (this->args[0], const_cast<char* const*> (this->args.data ()));
}

struct config { // Unibuild
  const char* compiler = "c++";
  const char* standard = "17";
  const char* entry;
  std::list<const char*> flags;
  std::list<std::string> includes;
  std::list<std::string> libraries;
  std::list<std::string> macros;

  explicit config (const char* file);

  auto include (std::string_view directory) -> void;
  auto link (std::string_view library) -> void;
  auto define (std::string_view macro) -> void;
  auto run () -> void;
};

config :: config (const char* file): entry {file} {
  this->flags.push_back ("-Wall");
  this->flags.push_back ("-Wextra");
  this->flags.push_back ("-Wshadow");
  this->flags.push_back ("-pedantic");
}

auto
config :: include (std::string_view directory) -> void {
  this->includes.push_back (std::string {"-I"}.append (directory));
}

auto
config :: link (std::string_view library) -> void {
  this->libraries.push_back (std::string {"-l"}.append (library));
}

auto
config :: define (std::string_view macro) -> void {
  this->macros.push_back (std::string {"-D"}.append (macro));
}

auto
config :: run () -> void {
  std::list<const char*> a;

  a.push_back (this->compiler);

  std::string std = std::string {"-std=c++"}.append (this->standard);

  a.push_back (std.c_str ());

  for (auto const& f: this->flags)
    a.push_back (f);

  for (auto const& d: this->includes)
    a.push_back (d.c_str ());

  a.push_back (this->entry);

  for (auto const& l: this->libraries)
    a.push_back (l.c_str ());

  for (auto const& m: this->macros)
    a.push_back (m.c_str ());

  a.push_back (nullptr);

  cmd c {std::vector<const char*> {a.begin (), a.end ()}};
  c.execute ();
}

}
