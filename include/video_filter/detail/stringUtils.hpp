#pragma once

#include <string>

inline std::string getExtension(const std::string& filename) {
  size_t dotPos = filename.find_last_of(".");
  if (dotPos == std::string::npos) {
    return "";
  }
  return filename.substr(dotPos + 1);
}
