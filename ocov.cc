#include <unistd.h>
#include <stdio.h>

#include "current/bricks/dflags/dflags.h"
#include "current/bricks/file/file.h"
#include "current/typesystem/serialization/json.h"
#include "current/blocks/xterm/vt100.h"
#include "current_build.h"

DEFINE_bool(version, false, "Dump version details.");
DEFINE_bool(v, false, "Dump version details.");

DEFINE_string(input, "", "The input file to read the coverage JSON from instead of the standard input.");
DEFINE_string(basedir, "", "The base directory to prefix the covered files paths with.");

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

inline void SynopsisAndExit(std::string const& error = "") {
  if (!error.empty()) {
    std::cerr << red << bold << "Error:    " << reset << error << std::endl;
  }
  std::cerr << bold << blue << "Synopsis" << reset << ": " << cyan << "opa test ... --coverage | ocov"
            << reset << ",  or " << cyan << "ocov --input coverage.json" << reset << '.' << std::endl;
  std::exit(-1);
}

inline std::string ReadInput() {
  if (FLAGS_input.empty()) {
    if (isatty(fileno(stdin))) {
      SynopsisAndExit("The input is a TTY, not a pipe.");
    } else {
      return std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }
  } else {
    return current::FileSystem::ReadFileAsString(FLAGS_input);
  }
  return "";
}

CURRENT_STRUCT(OCOVRow) { CURRENT_FIELD(row, uint32_t, 0u); };

CURRENT_STRUCT(OCOVBeginEnd) {
  CURRENT_FIELD(start, OCOVRow);
  CURRENT_FIELD(end, OCOVRow);
};

CURRENT_STRUCT(OCOVFile) {
  CURRENT_FIELD(covered, Optional<std::vector<OCOVBeginEnd>>);
  CURRENT_FIELD(not_covered, Optional<std::vector<OCOVBeginEnd>>);
  CURRENT_FIELD(coverage, Optional<double>);
};

CURRENT_STRUCT(OCOV) {
  CURRENT_FIELD(files, (std::map<std::string, OCOVFile>));
  CURRENT_FIELD(coverage, Optional<double>);
};

int main(int argc, char** argv) {
  ParseDFlags(&argc, &argv);
  if (FLAGS_version || FLAGS_v) {
    DumpVersion();
    return 0;
  }

  try {
    OCOV const ocov = ParseJSON<OCOV, JSONFormat::Minimalistic>(ReadInput());
    bool first = true;
    for (auto const& file : ocov.files) {
      if (!first) {
        std::cout << std::endl;
      } else {
        first = false;
      }
      std::cout << bold << "# " << blue << file.first << reset << std::endl;
      if (Exists(file.second.coverage)) {
        std::cout << bold << "# " << yellow << "file test coverage: " << Value(file.second.coverage) << '%' << reset
                  << std::endl;
      }
      try {
        std::string const filename = current::FileSystem::JoinPath(FLAGS_basedir, file.first);
        enum class Type : uint8_t { Covered = 0, NotCovered = 1 };
        struct RowMarker final {
          std::pair<uint32_t, bool> key;  // { row, false = start, true = end }.
          Type type;
          explicit RowMarker(uint32_t row = 0u, bool start = false, Type type = Type::Covered)
              : key(row, !start), type(type) {}
          bool operator<(RowMarker const& rhs) const { return key < rhs.key; }
        };
        std::vector<RowMarker> markers;
        if (Exists(file.second.covered)) {
          for (auto const& e : Value(file.second.covered)) {
            markers.emplace_back(e.start.row, true, Type::Covered);
            markers.emplace_back(e.end.row, false, Type::Covered);
          }
        }
        if (Exists(file.second.not_covered)) {
          for (auto const& e : Value(file.second.not_covered)) {
            markers.emplace_back(e.start.row, true, Type::NotCovered);
            markers.emplace_back(e.end.row, false, Type::NotCovered);
          }
        }
        std::sort(std::begin(markers), std::end(markers));
        std::pair<uint32_t, bool> key(0u, false);
        size_t index = 0u;
        bool status[2] = {false, false};
        current::FileSystem::ReadFileByLines(filename, [&key, &index, &markers, &status](std::string const& loc) {
          ++key.first;
          key.second = false;
          while (index < markers.size() && markers[index].key <= key) {
            status[static_cast<uint8_t>(markers[index].type)] = !markers[index].key.second;
            ++index;
          }
          bool need_reset = false;
          if (status[0] && !status[1]) {
            std::cout << green;
            need_reset = true;
          } else if (status[1] && !status[0]) {
            std::cout << red;
            need_reset = true;
          } else if (status[0] && status[1]) {
            std::cout << magenta;
            need_reset = true;
          }
          std::cout << loc;
          if (need_reset) {
            std::cout << reset;
          }
          std::cout << std::endl;
        });
      } catch (current::FileException const& e) {
        std::cout << bold << red << "# This file can not be read." << reset << std::endl;
      }
    }
    if (Exists(ocov.coverage)) {
      std::cout << std::endl
                << bold << "# " << yellow << "total test coverage: " << Value(ocov.coverage) << '%' << reset
                << std::endl;
    }
    return 0;
  } catch (current::Exception const& e) {
    std::cerr << red << bold << "Exception" << reset << ": " << e.Caller() << std::endl;
    std::cerr << e.OriginalDescription() << std::endl;
    return -1;
  }
}
