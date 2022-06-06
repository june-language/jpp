#include "Common.hpp"
#include "c/Common.h"
#include <unordered_map>

namespace june {
namespace args {

std::vector<ArgDesc> descriptors;

void ArgsAddArgument(std::string queryName, std::string shortName,
                     std::string longName, std::string description,
                     bool hasValue, bool shouldNewline) {
  ArgDesc desc;
  desc.queryName = queryName;
  desc.description = description;
  desc.longName = longName;
  desc.shortName = shortName;
  desc.hasValue = hasValue;
  desc.shouldNewline = shouldNewline;
  descriptors.push_back(desc);
}

void ArgsPrintHelp(const std::string &argv0) {
  std::cerr << "Usage: " << argv0 << " [options] [files]" << std::endl;
  std::cerr << "Options:" << std::endl;
  for (auto &descriptor : descriptors) {
    std::cerr << "  ";
    if (!descriptor.shortName.empty() && !descriptor.longName.empty()) {
      std::cerr << descriptor.shortName << ", " << descriptor.longName;
    } else if (!descriptor.shortName.empty()) {
      std::cerr << descriptor.shortName;
    } else if (!descriptor.longName.empty()) {
      std::cerr << descriptor.longName;
    }

    if (descriptor.hasValue) {
      std::cerr << " <" << descriptor.queryName << ">";
    }

    std::cerr << (descriptor.shouldNewline ? "\n  \t" : "\t")
              << descriptor.description << std::endl;
  }
}

static std::unordered_map<std::string, Arg> arguments;
static std::vector<Arg> positionalArguments;
static std::vector<std::string> codeArgs;

std::vector<std::string> ArgsGetCodeArgs() {
  return codeArgs;
}

bool ArgsAnyArgumentExists() {
  return !arguments.empty() || !positionalArguments.empty() || !codeArgs.empty();
}

void ArgsParseArguments(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg.size() > 1 && arg[0] == '-') {
      if (arg[1] == '-' && arg.size() > 2) {
        // Long argument
        for (auto &descriptor : descriptors) {
          if (descriptor.longName == arg) {
            if (descriptor.hasValue) {
              if (i + 1 < argc) {
                arguments.emplace(descriptor.queryName,
                                  Arg(descriptor.queryName, argv[i + 1]));
                i++;
              } else {
                std::cerr << "Missing value for argument: " << arg << std::endl;
                exit(1);
              }
            } else {
              arguments.emplace(descriptor.queryName,
                                Arg(descriptor.queryName, ""));
            }
            break;
          }
        }
      } else if (arg[1] == '-' && arg.size() == 2) {
        // Double dash
        // example:
        // june test.june -- code arguments here

        for (int j = ++i; j < argc; j++) {
          codeArgs.push_back(argv[j]);
          i++;
        }
        return; // by this point, we've already parsed all the arguments
      } else {
        // Short argument
        for (auto &descriptor : descriptors) {
          if (descriptor.shortName.empty()) {
            continue;
          }

          if (descriptor.shortName == arg) {
            if (descriptor.hasValue) {
              if (i + 1 < argc) {
                arguments.emplace(descriptor.queryName,
                                  Arg(descriptor.queryName, argv[i + 1]));
                i++;
              } else {
                std::cerr << "Missing value for argument " << arg << std::endl;
                exit(1);
              }
            } else {
              arguments.emplace(descriptor.queryName,
                                Arg(descriptor.queryName, ""));
            }
            break;
          }
        }
      }
    } else {
      positionalArguments.push_back(Arg("", arg));
    }
  }
}

bool ArgsArgumentExists(std::string queryName) {
  return arguments.find(queryName) != arguments.end();
}

bool ArgsPositionalExists(int position) {
  return position < positionalArguments.size();
}

Arg ArgsGetArgument(std::string queryName) { return arguments.at(queryName); }

Arg ArgsGetPositional(int position) { return positionalArguments.at(position); }

std::vector<Arg> ArgsGetPositionals() { return positionalArguments; }

void ArgsModifyArgument(std::string queryName, std::string newValue) {
  arguments.at(queryName).value = newValue;
}

} // namespace args
} // namespace june

// C API

using namespace june::args;

extern "C" void JuneArgsAddArgument(const char *queryName,
                                    const char *shortName, const char *longName,
                                    const char *description, bool hasValue,
                                    bool shouldNewline) {
  ArgsAddArgument(queryName, shortName, longName, description, hasValue,
                  shouldNewline);
}

extern "C" void JuneArgsParseArguments(int argc, char **argv) {
  ArgsParseArguments(argc, argv);
}

extern "C" bool JuneArgsArgumentExists(const char *queryName) {
  return ArgsArgumentExists(queryName);
}

extern "C" bool JuneArgsPositionalExists(int position) {
  return ArgsPositionalExists(position);
}

// typedef struct Arg *ArgHandle;

extern "C" ArgHandle JuneArgsGetArgument(const char *queryName) {
  return (ArgHandle) new june::args::Arg(ArgsGetArgument(queryName));
}

extern "C" ArgHandle JuneArgsGetPositional(int position) {
  return (ArgHandle) new june::args::Arg(ArgsGetPositional(position));
}

extern "C" void JuneArgsModifyArgument(const char *queryName,
                                       const char *newValue) {
  ArgsModifyArgument(queryName, newValue);
}

extern "C" ArgHandle *JuneArgsGetPositionals() {
  auto positionals = ArgsGetPositionals();
  ArgHandle *handles = new ArgHandle[positionals.size()];
  for (int i = 0; i < positionals.size(); i++) {
    handles[i] = (ArgHandle) new june::args::Arg(positionals[i]);
  }
  return handles;
}

extern "C" bool JuneArgsAnyArgumentExists() { return ArgsAnyArgumentExists(); }

extern "C" void JuneArgsPrintHelp(const char *argv0) { ArgsPrintHelp(argv0); }
