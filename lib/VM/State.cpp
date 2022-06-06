#include "VM/State.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "Common.hpp"
#include "VM/Vars.hpp"
#include "VM/Vars/Base.hpp"
#include "json.hpp"

namespace june {

State::State(const std::string &selfBin, const std::string &selfBase,
             const std::vector<std::string> &args)
    : exitCalled(false), execStackCountExceeded(false), exitCode(0),
      execStackMax(kExecStackMaxDefault), execStackCount(0),
      tru(new VarBool(true, 0, 0)), fals(new VarBool(false, 0, 0)),
      nil(new VarNil(0, 0)), dylib(new Dylib()), stack(new Stack()),
      srcArgs(nullptr), _selfBin(selfBin), _selfBase(selfBase),
      srcLoadCodeFn(nullptr), srcReadCodeFn(nullptr) {
  initTypenames(*this);

  std::vector<VarBase *> srcArgsVec;
  for (auto &arg : args) {
    srcArgsVec.push_back(new VarString(arg, 0, 0));
  }
  srcArgs = new VarVec(srcArgsVec, false, 0, 0);
}

State::~State() {
  delete stack;

  for (auto &typeFn : _typeFns)
    delete typeFn.second;

  for (auto &g : _globals)
    varDref(g.second);

  for (auto &src : allSrcs)
    varDref(src.second);

  varDref(nil);
  varDref(fals);
  varDref(tru);
  varDref(srcArgs);

  for (auto &deInitFn : _modDeInitFns)
    deInitFn.second();

  delete dylib;
}

void State::pushSrc(SrcFile *src, const size_t &idx) {
  if (allSrcs.find(src->path()) == allSrcs.end()) {
    allSrcs[src->path()] = new VarSrc(src, new Vars(), src->id(), idx);
  }
  varIref(allSrcs[src->path()]);
  srcStack.push_back(allSrcs[src->path()]);
}

void State::pushSrc(const std::string &srcPath) {
  assert(allSrcs.find(srcPath) != allSrcs.end());
  varIref(allSrcs[srcPath]);
  srcStack.push_back(allSrcs[srcPath]);
}

void State::popSrc() {
  varDref(srcStack.back());
  srcStack.pop_back();
}

void State::addTypeFn(const std::uintptr_t &type, const std::string &name,
                      VarBase *fn, const bool iref) {
  if (_typeFns.find(type) == _typeFns.end()) {
    _typeFns[type] = new VarsFrame;
  }

  if (_typeFns[type]->exists(name)) {
    this->fail(this->srcStack.back()->srcId(), this->srcStack.back()->idx(),
               "function '%s' for '%s' already exists", name.c_str(),
               this->getTypeName(type).c_str());
    return;
  }

  _typeFns[type]->add(name, fn, iref);
}

VarBase *State::getTypeFn(VarBase *val, const std::string &name) {
  auto it = _typeFns.find(val->typeFnId());
  VarBase *res = nullptr;
  if (it == _typeFns.end()) {
    if (val->isAttrBased()) {
      it = _typeFns.find(val->type());
      if (it == _typeFns.end())
        return _typeFns[type_id<VarAll>()]->get(name);
      res = it->second->get(name);
      if (res)
        return res;
      return _typeFns[type_id<VarAll>()]->get(name);
    }
    return _typeFns[type_id<VarAll>()]->get(name);
  }
  res = it->second->get(name);
  if (res)
    return res;
  return _typeFns[type_id<VarAll>()]->get(name);
}

void State::setTypeName(const std::uintptr_t &type, const std::string &name) {
  _typeNames[type] = name;
}

std::string State::getTypeName(const std::uintptr_t &type) {
  if (_typeNames.find(type) != _typeNames.end())
    return _typeNames[type];
  return "TypeId{ " + std::to_string(type) + " }";
}

std::string State::getTypeName(const VarBase *val) {
  return this->getTypeName(val->type());
}

void State::globalAdd(const std::string &name, VarBase *val, const bool iref) {
  if (_globals.find(name) != _globals.end())
    return;
  if (iref)
    varIref(val);
  _globals[name] = val;
}

VarBase *State::globalGet(const std::string &name) {
  if (_globals.find(name) == _globals.end())
    return nullptr;
  return _globals[name];
}

// module loading/existance checks

bool State::juneModuleExists(std::string &mod, const std::string &ext,
                             std::string &dir) {
  bool hasExt = false;
  if (mod.size() > ext.size() && mod.substr(mod.size() - ext.size()) == ext)
    hasExt = true;
  if (mod.front() != '~' && mod.front() != '/' && mod.front() != '.') {
    bool isStandard = false;
    auto components = string::split(mod, ".");

    if (components.size() < 1)
      return false;

    if (components[0] == "Standard") {
      isStandard = true;
      components[0] = "June/Standard";
    }

    auto res = fs::exists(_selfBase + "/junelib/" + components[0]);
    auto localRes = fs::exists(fs::cwd().unwrap() + "/" + components[0]);

    // safety: if fs::exists works once, its likely to work again
    if (res.isErr())
      return false;

    if (!res.unwrap() && !localRes.unwrap())
      return false;

    auto files = fs::search(
        _selfBase + "/junelib/" + components[0],
        [&](const std::string &file) -> bool {
          auto fAbs = fs::absPath(
              _selfBase + "/junelib/" + components[0] + "/" +
              mod.substr(isStandard ? 8 : components[0].size()) + "/");

          if (!hasExt) {
            fAbs += ext;
          }

          return fAbs == fs::absPath(file);
        });
    auto localFiles = fs::search(
        fs::cwd().unwrap() + "/" + components[0],
        [&](const std::string &file) -> bool {
          auto fAbs = fs::absPath(
              dir + "/" + components[0] + "/" +
              mod.substr(isStandard ? 8 : components[0].size()) + "/");

          if (!hasExt) {
            fAbs += ext;
          }

          return fAbs == fs::absPath(file);
        });

    for (auto &file : localFiles) {
      files.push_back(file);
    }

    if (files.empty()) {
      if (fs::isDir(_selfBase + "/junelib/" + mod)) {
        dir = fs::absPath(
            // {base}/junelib/mod/sub
            // {base}/junelib/mod
            _selfBase + "/junelib/" + mod);

        mod = fs::absPath(_selfBase + "/junelib/" + mod + "/" +
                          components.back() + (hasExt ? ext : ""));
        return true;
      } else if (fs::isDir(fs::cwd().unwrap() + "/" + components[0])) {
        dir = fs::absPath(
            // {cwd}/mod/sub
            // {cwd}/mod
            fs::cwd().unwrap() + "/" + components[0]);

        mod = fs::absPath(fs::cwd().unwrap() + "/" + components[0] + "/" +
                          components.back() + (hasExt ? ext : ""));
      }

      return false;
    }

    if (!fs::exists(files[0]).unwrap()) {
      return false;
    }

    mod = files[0];
    return true;
  } else {
    if (mod.front() == '~') {
      mod.erase(mod.begin());
      std::string home = env::get("HOME");
      mod.insert(mod.begin(), home.begin(), home.end());
    } else if (mod.front() == '.') {
      // in june code, it'll look like so:
      // import ..Example
      // and resolves to:
      // {cwd}/../Example/Example.june
      assert(srcStack.size() > 0);
      mod.erase(mod.begin());
      mod = fs::dirName(srcStack.back()->src()->path());
    }

    if (fs::exists(mod + (hasExt ? "" : ext)).unwrap() && !fs::isDir(mod)) {
      mod = fs::absPath(mod + (hasExt ? "" : ext), &dir);
      return true;
    } else if (fs::isDir(mod)) {
      auto components = string::split(mod, "/");
      mod = fs::absPath(mod + "/" + components.back() + ext, &dir);
      return true;
    }
  }
  return false;
}

bool State::nativeModuleExists(std::string &mod, const std::string &ext,
                               std::string &dir) {
  bool hasExt = false;
  if (mod.size() > ext.size() && mod.substr(mod.size() - ext.size()) == ext)
    hasExt = true;
  if (mod.front() != '~' && mod.front() != '/' && mod.front() != '.') {
    bool isStandard = false;
    mod = string::replaceAll(mod, "libJune", "");
    auto components = string::split(mod, ".");

    if (components.size() < 1)
      return false;

    if (components[0] == "Standard") {
      isStandard = true;
      components[0] = "June/Standard";
    }

    auto res = fs::exists(_selfBase + "/lib/" + components[0]);
    auto localRes = fs::exists(fs::cwd().unwrap() + "/" + components[0]);

    // safety: if fs::exists works once, its likely to work again
    if (res.isErr())
      return false;

    if (!res.unwrap() && !localRes.unwrap())
      return false;

    auto files = fs::search(
        _selfBase + "/lib/" + components[0],
        [&](const std::string &file) -> bool {
          std::string fName = mod.substr(isStandard ? 8 : components[0].size());
          fName = "libJune" + fName;
          fName = string::replaceAll(fName, ".", "");

          auto fAbs =
              fs::absPath(_selfBase + "/lib/" + components[0] + "/" + fName);

          if (!hasExt) {
            fAbs += ext;
          }

          return fAbs == fs::absPath(file);
        });
    auto localFiles = fs::search(
        fs::cwd().unwrap() + "/" + components[0],
        [&](const std::string &file) -> bool {
          std::string fName = mod.substr(isStandard ? 8 : components[0].size());
          fName = "libJune" + fName;

          auto fAbs = fs::absPath(dir + "/" + components[0] + "/" + fName);

          if (!hasExt) {
            fAbs += ext;
          }

          return fAbs == fs::absPath(file);
        });

    for (auto &file : localFiles) {
      files.push_back(file);
    }

    if (files.empty()) {
      if (fs::isDir(_selfBase + "/lib/" + mod)) {
        dir = fs::absPath(
            // {base}/lib/mod/sub
            // {base}/lib/mod
            _selfBase + "/lib/" + mod);

        mod = fs::absPath(_selfBase + "/lib/" + mod + "/" + components.back() +
                          (hasExt ? ext : ""));
        return true;
      } else if (fs::isDir(fs::cwd().unwrap() + "/" + components[0])) {
        dir = fs::absPath(
            // {cwd}/mod/sub
            // {cwd}/mod
            fs::cwd().unwrap() + "/" + components[0]);

        mod = fs::absPath(fs::cwd().unwrap() + "/" + components[0] + "/" +
                          components.back() + (hasExt ? ext : ""));
      }

      return false;
    }

    if (!fs::exists(files[0]).unwrap()) {
      return false;
    }

    mod = files[0];
    return true;
  } else {
    if (mod.front() == '~') {
      mod.erase(mod.begin());
      std::string home = env::get("HOME");
      mod.insert(mod.begin(), home.begin(), home.end());
    } else if (mod.front() == '.') {
      // in june code, it'll look like so:
      // import ..Example
      // and resolves to:
      // {cwd}/../Example/Example.june
      assert(srcStack.size() > 0);
      mod.erase(mod.begin());
      mod = fs::dirName(srcStack.back()->src()->path());
    }

    if (fs::exists(mod + (hasExt ? "" : ext)).unwrap() && !fs::isDir(mod)) {
      mod = fs::absPath(mod + (hasExt ? "" : ext), &dir);
      return true;
    } else if (fs::isDir(mod)) {
      auto components = string::split(mod, "/");
      mod = fs::absPath(mod + "/" + components.back() + ext, &dir);
      return true;
    }
  }
  return false;
}

bool State::nativeModuleLoad(const std::string &modStr, const size_t &srcId,
                             const size_t &idx) {
  std::string mod = modStr.substr(modStr.find_last_of('/') + 1);
  std::string modFile = modStr;
  std::string modDir;
  modFile.insert(modFile.find_last_of('/') + 1, "libJune");

  if (!this->nativeModuleExists(modFile, nativeModuleExt(), modDir)) {
    modFile = modStr;
    modFile.insert(modFile.find_last_of('/') + 1, "lib");
    if (!this->nativeModuleExists(modFile, nativeModuleExt(), modDir)) {
      this->fail(srcId, idx, "module file '%s' not found",
                 (modStr + nativeModuleExt()).c_str());
      return false;
    }
  }

  if (dylib->exists(modFile))
    return true;

  if (!dylib->load(modFile)) {
    this->fail(srcId, idx, "failed to load module '%s'", modFile.c_str());
    return false;
  }

  ModInitFn initFn = (ModInitFn)dylib->get(modFile, "june_init");
  if (initFn == nullptr) {
    this->fail(srcId, idx, "module '%s' has no init function (june_init)",
               modStr.c_str());
    dylib->unload(modFile);
    return false;
  }

  if (!initFn(*this, srcId, idx)) {
    dylib->unload(modFile);
    this->fail(srcId, idx, "init function in module '%s' failed",
               modFile.c_str());
    return false;
  }

  ModDeInitFn deInitFn = (ModDeInitFn)dylib->get(modFile, "june_deinit");
  if (deInitFn)
    _modDeInitFns[modFile] = deInitFn;
  return true;
}

err::Errors State::juneModuleLoad(std::string &modStr, const size_t &srcId,
                                  const size_t &idx) {
  std::string modDir;
  if (!this->juneModuleExists(modStr, juneModuleExt(), modDir)) {
    this->fail(srcId, idx, "module '%s' not found",
               (modStr + juneModuleExt()).c_str());
    return err::Errors::Err(err::Error(err::ErrFileIo, "module not found"));
  }
  if (allSrcs.find(modStr) != allSrcs.end())
    return err::Errors::Ok();

  auto res = err::Errors::Ok();
  SrcFile *src = this->srcLoadCodeFn(modStr, modDir, false, res, 0, -1);
  if (res.isErr()) {
    if (src)
      delete src;
    return res;
  }

  pushSrc(src, 0);
  auto execRes = vm::exec(*this);
  popSrc();

  if (execRes.isOk()) {
    return err::Errors::Ok();
  } else {
    this->fail(srcId, idx, "module '%s' failed to load", modStr.c_str());
    return err::Errors::Err(execRes.unwrapErr());
  }
}

void State::fail(const size_t &srcId, const size_t &idx, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (fails.empty() || this->exitCalled) {
    for (auto &src : allSrcs) {
      if (src.second->src()->id() == srcId) {
        src.second->src()->fail(idx, fmt, args);
        break;
      }
    }
  } else {
    char *err = nullptr;
    vasprintf(&err, fmt, args);
    fails.push(new VarString(err, srcId, idx), false);
  }
  va_end(args);
}

void State::fail(const size_t &srcId, const size_t &idx, VarBase *val,
                 const char *fmt, const bool &iref) {
  if (iref)
    varIref(val);

  if (fails.empty() || this->exitCalled) {
    for (auto &src : allSrcs) {
      if (src.second->src()->id() == srcId) {
        std::string data;
        val->toStr(*this, data, srcId, idx);
        varDref(val);
        if (fmt)
          src.second->src()->fail(idx, "%s (%s)", fmt, data.c_str());
        else
          src.second->src()->fail(idx, data.c_str());
        break;
      }
    }
  } else {
    fails.push(val, false);
  }
}

std::vector<std::string> additionalCoreMods(State &vm, std::string base) {
  using namespace nlohmann;

  auto configRes = fs::exists(base + "/core.json");
  if (configRes.isErr() || !configRes.unwrap())
    return {};

  std::vector<std::string> mods;

  auto readFileRes = fs::readFile(base + "/core.json");
  if (readFileRes.isErr() || readFileRes.unwrap().empty())
    return {};

  auto config = json::parse(readFileRes.unwrap());

  if (!config.is_object() || config.empty())
    return {};

  if (!config.contains("coreModules"))
    return {};

  if (!config["coreModules"].is_array())
    return {};

  for (auto &mod : config["coreModules"]) {
    if (mod.is_string())
      mods.push_back(mod.get<std::string>());
    else
      vm.fail(0, 0, "core module '%s' is not a string in '%s/core.json'",
              mod.dump().c_str(), base.c_str());
  }
  return mods;
}

bool State::loadCoreModules() {
  std::vector<std::string> mods = {
      "June.Core",
      // "Utils"
  };
  for (auto &mod : additionalCoreMods(*this, _selfBase))
    mods.push_back(mod);

  for (auto &mod : mods) {
    if (!this->nativeModuleLoad(mod, 0, 0))
      return false;
  }
  return true;
}

const char *nativeModuleExt() {
#if __linux__ || __FreeBSD__ || __NetBSD__ || __OpenBSD__ || _bsdi__ ||        \
    __DragonFly__ || __sun
  return ".so";
#elif __APPLE__
  return ".dylib";
#elif __MINGW32__ || __MINGW64__ || __CYGWIN__ || __WIN32__ || __WIN64__
  return ".dll";
#else
#error "Unknown platform"
#endif
}

const char *juneModuleExt(const bool compiled) {
  if (compiled)
    return ".junec";
  return ".june";
}

} // namespace june
