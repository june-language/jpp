#include "Common.hpp"
#include "c/Common.h"

#include <algorithm>

bool june::string::endsWith(const std::string &str, const std::string &suffix) {
  if (str.length() < suffix.length()) {
    return false;
  }
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) ==
         0;
}

std::vector<std::string> june::string::split(const std::string &str,
                                             const std::string &delim) {
  std::vector<std::string> result;
  size_t pos = 0;
  size_t lastPos = 0;
  while ((pos = str.find(delim, lastPos)) != std::string::npos) {
    result.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = pos + delim.length();
  }
  result.push_back(str.substr(lastPos));
  return result;
}

std::string june::string::replaceAll(const std::string &str, const std::string &from,
                       const std::string &to) {
  std::string result = str;
  size_t start_pos = 0;
  while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
    result.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return result;
}

std::string june::string::join(const std::vector<std::string> &strs,
                               const std::string &delim) {
  std::string result;
  for (size_t i = 0; i < strs.size(); i++) {
    result += strs[i];
    if (i < strs.size() - 1) {
      result += delim;
    }
  }
  return result;
}

std::string june::string::toLower(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

std::string june::string::toUpper(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  return result;
}

const char *june::string::duplicateAsCString(const std::string &str) {
  char *result = new char[str.length() + 1];
  strcpy(result, str.c_str());
  return result;
}

// C API

extern "C" bool endsWith(const char *str, const char *suffix) {
  return june::string::endsWith(str, suffix);
}

extern "C" const char **split(const char *str, const char *delim) {
  std::vector<std::string> result = june::string::split(str, delim);
  const char **resultPtr = new const char *[result.size()];
  for (size_t i = 0; i < result.size(); i++) {
    resultPtr[i] = result[i].c_str();
  }
  return resultPtr;
}

extern "C" const char *join(const char **strs, const char *delim) {
  std::vector<std::string> result;
  for (size_t i = 0; strs[i] != nullptr; i++) {
    result.push_back(strs[i]);
  }

  auto s = june::string::join(result, delim);
  return june::string::duplicateAsCString(s);
}

extern "C" const char *toLower(const char *str) {
  return june::string::duplicateAsCString(june::string::toLower(str));
}

extern "C" const char *toUpper(const char *str) {
  return june::string::duplicateAsCString(june::string::toUpper(str));
}
