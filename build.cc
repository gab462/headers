#include "build.hh"
#include "args.hh"

auto
main (int argc, char** argv) -> int {
  args::parser args;

  auto& type = args.string_option ('t', "pong");

  args.parse (argc, argv);

  if (type == "pong") {
    build::config bld {"examples/pong.cc"};

    bld.include ("/usr/include/SDL2");
    bld.link ("SDL2");
    bld.define ("_REENTRANT");

    bld.flags.push_back ("-o");
    bld.flags.push_back ("pong");

    bld.run ();
  }

  if (type == "self") {
    build::config bld {"build.cc"};

    bld.flags.push_back ("-o");
    bld.flags.push_back ("build");

    bld.include ("../headers");

    bld.run ();
  }

  return 0;
}
