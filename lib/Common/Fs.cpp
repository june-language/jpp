#include "Common.hpp"
#include <filesystem>
#include <fstream>

namespace june {
namespace fs {

using namespace june::err;

auto readFile(const std::string &path) -> Result<std::string, Error> {
  std::filesystem::path p(path);
  if (!std::filesystem::exists(p)) {
    return Error(ErrorKind::FileIo, "Unable to read file: file does not exist");
  }

  return std::string("");
}

auto exists(const std::string &path) -> bool {
  return std::filesystem::exists(std::filesystem::path(path));
}

auto absolutePath(const std::string &path, std::string *parent) -> std::string {
  std::filesystem::path p(path);
  auto abs = std::filesystem::absolute(p);
  if (parent != nullptr)
    *parent = abs.parent_path().string();
  return abs.string();
}

auto isAbsolute(const std::string &path) -> bool {
  return std::filesystem::path(path).is_absolute();
}

auto isDir(const std::string &path) -> bool {
  return std::filesystem::is_directory(std::filesystem::path(path));
}

auto isFile(const std::string &path) -> bool {
  return std::filesystem::is_directory(std::filesystem::path(path));
}

auto filename(const std::string &path) -> std::string {
  return std::filesystem::path(path).filename().string();
}

auto cwd() -> std::string { return std::filesystem::current_path().string(); }

auto home() -> std::string {
  return std::filesystem::absolute(june::env::get("HOME")).string();
}

auto relativePath(const std::string &path, const std::string &dir)
    -> std::string {
  return std::filesystem::relative(std::filesystem::path(path), dir).string();
}

auto search(const std::string &dir,
            const std::function<bool(const std::string &)> &matcher)
    -> std::vector<std::string> {
  std::vector<std::string> results;
  for (auto &p : std::filesystem::recursive_directory_iterator(dir)) {
    if (matcher(p.path().string())) {
      results.push_back(p.path().string());
    }
  }
  return results;
}

auto writeBytes(const std::string &path, const std::vector<u8> &bytes) -> Result<void, Error> {
  if (!exists(path)) {
    return Error(ErrorKind::FileIo, "Unable to write file: file does not exist");
  }

  if (bytes.size() == 0) {
    return Error(ErrorKind::FileIo, "Unable to write file: no bytes to write");
  }

  if (isDir(path)) {
    return Error(ErrorKind::FileIo, "Unable to write file: path is a directory");
  }

  std::ofstream file;
  file.open(path, std::ios::out | std::ios::binary);

  if (!file.is_open()) {
    return Error(ErrorKind::FileIo, "Unable to write file: unable to open file");
  }

  file.write((char *)bytes.data(), bytes.size());

  if (file.bad()) {
    return Error(ErrorKind::FileIo, "Unable to write file: unable to write to file");
  }

  file.close();
  return Result<void, Error>::Ok();
}

} // namespace fs
} // namespace june
