#pragma once

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class InputParser {
 public:
  struct Option {
    std::string defaultValue;
    bool isMandatory;
    bool isSet;
  };

  InputParser(int &argc,
              char **argv,
              const std::unordered_map<std::string, Option> &options,
              const std::vector<std::string> &positionalArgNames)
      : options(options),
        positionalArgNames(positionalArgNames),
        executableName(argv[0]) {
    this->options.insert({"-h", {"", false, false}});
    parseArguments(argc, argv);
    checkMandatoryOptions();
    checkPositionalArguments();
  }

  template <class T>
  T getCmdOption(const std::string &option) const {
    auto itr = tokens.find(option);
    if (itr != tokens.end()) {
      return convert<T>(itr->second);
    }
    auto opt = options.find(option);
    if (opt != options.end()) {
      return convert<T>(opt->second.defaultValue);
    }
    for (size_t i = 0; i < positionalArgNames.size(); ++i) {
      if (positionalArgNames[i] == option) {
        return convert<T>(positionalArgs[i]);
      }
    }
    printUsage();
    throw std::invalid_argument("Option not found: " + option);
  }

 private:
  std::unordered_map<std::string, Option> options;
  std::unordered_map<std::string, std::string> tokens;
  std::vector<std::string> positionalArgs;
  std::vector<std::string> positionalArgNames;
  std::string executableName;

  template <class T>
  T convert(const std::string &str) const {
    T result;
    std::istringstream iss(str);
    iss >> result;
    return result;
  }

  void parseArguments(int &argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
      if (std::strncmp(argv[i], "-", 1) == 0) {
        std::string key(argv[i]);
        if (i + 1 < argc && std::strncmp(argv[i + 1], "-", 1) != 0) {
          std::string value(argv[i + 1]);
          if (this->options.find(key) != this->options.end()) {
            tokens[key] = value;
            this->options[key].isSet = true;
          } else {
            printUsage();
            throw std::invalid_argument("Invalid option: " + key);
          }
          ++i;  // Skip the next argument
        } else if (key == "-h") {
          printUsage();
          std::exit(0);
        } else {
          printUsage();
          throw std::invalid_argument("Missing value for option: " + key);
        }
      } else {
        positionalArgs.push_back(argv[i]);
      }
    }
  }

  void checkMandatoryOptions() {
    for (const auto &opt : options) {
      if (opt.second.isMandatory && tokens.find(opt.first) == tokens.end()) {
        printUsage();
        throw std::invalid_argument("Missing mandatory option: " + opt.first);
      }
    }
  }

  void checkPositionalArguments() {
    if (positionalArgs.size() != positionalArgNames.size()) {
      printUsage();
      throw std::invalid_argument(
          "Incorrect number of positional arguments. Expected " +
          std::to_string(positionalArgNames.size()) + " but got " +
          std::to_string(positionalArgs.size()));
    }
  }

  void printUsage() const {
    std::cerr << "Usage: " << executableName;
    for (const auto &arg : positionalArgNames) {
      std::cerr << " <" << arg << ">";
    }
    for (const auto &opt : options) {
      std::cerr << " [" << opt.first << " " << opt.second.defaultValue << "]";
    }
    std::cerr << std::endl;
  }
};
