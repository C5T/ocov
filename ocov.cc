#include "current/bricks/dflags/dflags.h"
#include "current/blocks/xterm/vt100.h"
#include "current_build.h"

DEFINE_bool(version, false, "Dump version details.");
DEFINE_bool(v, false, "Dump version details.");

using namespace current::vt100;

inline void DumpVersion() {
  using namespace current::build;
  std::cout << bold << "OCOV" << reset << ", built " << green << kBuildDateTime << reset << std::endl;
  std::cout << "Branch " << magenta << kGitBranch << reset << ", commit " << blue << kGitCommit << reset << std::endl;

#ifndef NDEBUG
  std::cout << red << bold << "Warning" << reset << ": unoptimized build";
#if defined(CURRENT_POSIX) || defined(CURRENT_APPLE)
  std::cout << ", run `" << cyan << "NDEBUG=1 make clean all" << reset << '`';
#endif
  std::cout << '.' << std::endl;
#endif  // NDEBUG
}

int main(int argc, char** argv) {
  ParseDFlags(&argc, &argv);
  if (FLAGS_version || FLAGS_v) {
    DumpVersion();
    return 0;
  }
}
