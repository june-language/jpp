#include "Common.hpp"

namespace june {
namespace string {

auto endsWith(const std::string &str, const std::string &suffix) -> bool {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

auto split(const std::string &str, const std::string &delim)
    -> std::vector<std::string> {
  std::vector<std::string> result;
  auto start = 0;
  auto end = str.find(delim);
  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + delim.size();
    end = str.find(delim, start);
  }
  result.push_back(str.substr(start, end));
  return result;
}

auto replaceAll(const std::string &str, const std::string &from,
                const std::string &to) -> std::string {
  auto result = str;
  auto start = 0;
  while ((start = result.find(from, start)) != std::string::npos) {
    result.replace(start, from.length(), to);
    start += to.length();
  }
  return result;
}

auto join(const std::vector<std::string> &strs, const std::string &delim)
    -> std::string {
  std::string result;
  for (size_t i = 0; i < strs.size(); i++) {
    result += strs[i];
    if (i != strs.size() - 1) {
      result += delim;
    }
  }
  return result;
}

auto toLower(const std::string &str) -> std::string {
  auto result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

auto toUpper(const std::string &str) -> std::string {
  auto result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

auto toTitle(const std::string &str) -> std::string {
  auto result = toLower(str);
  result[0] = std::toupper(result[0]);
  return result;
}

auto duplicateAsCString(const std::string &str) -> const char * {
  auto result = new char[str.size() + 1];
  std::strcpy(result, str.c_str());
  return result;
}

} // namespace string
} // namespace june
