#include <vector>
#include <list>
#include <string>
#include <unistd.h>

namespace build {

struct cmd {
  const char* program;
  std::vector<const char*> args;

  cmd ();
  template <typename ...A> cmd (const char* c, A... a);

  auto execute () -> void;
};

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

cmd :: cmd (): program {nullptr}, args {} {}

template <typename ...A>
cmd :: cmd (const char* c, A... a): program {c}, args {c, a..., nullptr} {}

auto
cmd :: execute () -> void {
  // FIXME: Any way other than const_cast?
  execvp (program, const_cast<char* const*>(args.data ()));
}

config :: config (const char* file): entry {file} {
  this->flags.push_back ("-Wall");
  this->flags.push_back ("-Wextra");
  this->flags.push_back ("-Wshadow");
  this->flags.push_back ("-pedantic");
}

auto
config :: include (std::string_view directory) -> void {
  includes.push_back (std::string {"-I"}.append (directory));
}

auto
config :: link (std::string_view library) -> void {
  libraries.push_back (std::string {"-l"}.append (library));
}

auto
config :: define (std::string_view macro) -> void {
  macros.push_back (std::string {"-D"}.append (macro));
}

auto
config :: run () -> void {
  std::vector<const char*> v;

  v.resize (4 // compiler, standard, entry and nullptr
            + this->flags.size ()
            + this->includes.size ()
            + this->libraries.size ()
            + this->macros.size ());

  size_t i = 0;

  v[i++] = this->compiler;

  std::string std = std::string {"-std=c++"}.append (this->standard);

  v[i++] = std.c_str ();

  for (auto const& f: this->flags)
    v[i++] = f;

  for (auto const& d: this->includes)
    v[i++] = d.c_str ();

  v[i++] = this->entry;

  for (auto const& l: this->libraries)
    v[i++] = l.c_str ();

  for (auto const& m: this->macros)
    v[i++] = m.c_str ();

  v[i] = nullptr;

  cmd c;
  c.program = this->compiler;
  c.args.resize (v.size ());
  c.args = v;

  c.execute ();
}

}
