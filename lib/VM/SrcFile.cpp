#include "VM/SrcFile.hpp"
#include "Common.hpp"
#include "VM/OpCodes.hpp"
#include "c/OpCodes.h"
#include "c/SrcFile.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>

static size_t srcId() {
  static size_t sid = 0;
  return sid++;
}

namespace june {

SrcFile::SrcFile(const std::string &dir, const std::string &path,
                 const bool isMain)
    : _id(srcId()), _dir(dir), _path(path), _isMain(isMain) {}

using namespace err;

Errors SrcFile::loadFile() {
  FILE *fp;
  bool isBytecode = true;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(_path.c_str(), "rb");
  if (fp == NULL) {
    return Errors::Err(err::Error(ErrKind::ErrFileIo, "Failed to open file"));
  }

  char magic[4];
  if (fread(magic, 1, 4, fp) != 4) {
    perror("fread");
    return Errors::Err(err::Error(ErrKind::ErrFileIo, "Failed to read file"));
  }

  if (magic[0] != 'J' || magic[1] != 'U' || magic[2] != 'N' ||
      magic[3] != 'E') {
    isBytecode = false;
  }

  _isBytecode = isBytecode;
  fseek(fp, 0, SEEK_SET);

  if (!isBytecode) {
    size_t prefixIdx = _data.size();
    std::string code;
    std::vector<SrcColRange> cols;
    size_t begin, end;
    while ((read = getline(&line, &len, fp)) != -1) {
      begin = code.size();
      code += line;
      end = code.size();
      cols.push_back({prefixIdx + begin, prefixIdx + end});
    }

    fclose(fp);
    if (line)
      free(line);

    if (code.empty()) {
      return Errors::Err(
          err::Error(ErrKind::ErrFileIo, "Encountered empty file"));
    }

    addData(code);
    addCols(cols);
  } else {
    // reset file pointer
    fseek(fp, 0, SEEK_SET);

    // read all data into a buffer
    fs::u8 *buffer = NULL;
    size_t bufferSize = 0;

    // read 4kb at a time
    while ((read = fread(&buffer, 4 * 1024, 1, fp)) != 0) {
      bufferSize += read;
    }

    fclose(fp);

    if (bufferSize == 0) {
      return Errors::Err(
          err::Error(ErrKind::ErrFileIo, "Encountered empty file"));
    }

    // create bytecode
    auto decompressResult = fs::readBytecode(buffer);
    if (decompressResult.isErr()) {
      return Errors::Err(
          err::Error(ErrKind::ErrFileIo, "Failed to decompress file"));
    }

    auto bytecode = decompressResult.unwrap();
    addBytecode(bytecode.bytecode);
    addCols(bytecode.srcRanges);
  }

  return Errors::Ok();
}

void SrcFile::addData(const std::string &data) { _data += data; }

void SrcFile::addCols(const std::vector<SrcColRange> &cols) { _cols = cols; }

void SrcFile::addBytecode(const std::vector<june::Op> &bytecode) {
  _bytecode.getMut().resize(bytecode.size());
  for (size_t i = 0; i < bytecode.size(); i++) {
    _bytecode.getMut()[i] = bytecode[i];
  }
}

void SrcFile::fail(const size_t &idx, const char *msg, ...) const {
  va_list vargs;
  va_start(vargs, msg);
  fail(idx, msg, vargs);
  va_end(vargs);
}

void SrcFile::fail(const size_t &idx, const char *msg, va_list vargs) const {
  size_t line, colBegin, colEnd, col;
  bool found = false;
  for (size_t i = 0; i < _cols.size(); i++) {
    if (idx >= _cols[i].begin && idx < _cols[i].end) {
      line = i;
      colBegin = _cols[i].begin;
      colEnd = _cols[i].end;
      col = idx - colBegin;
      found = true;
      break;
    }
  }

  if (!found) {
    std::cerr << "Could not find line and column for index " << idx
              << std::endl;
    std::vfprintf(stderr, msg, vargs);
    std::cerr << std::endl;
    std::cerr << "in file " << _path << " with index " << idx << std::endl;
    return;
  }

  std::cerr << june::fs::relativePath(_path, _dir) << ":" << line + 1 << ":"
            << col + 1 << ": ";
  vfprintf(stderr, msg, vargs);
  std::cerr << std::endl;

  if (_isBytecode)
    return; // source code is not available for bytecode, so we
            // can't print it

  std::string errLine = _data.substr(colBegin, colEnd - colBegin);
  if (errLine.back() == '\n')
    errLine.pop_back();
  std::cerr << errLine << std::endl;

  std::string spcs;
  for (size_t i = 0; i < col; i++) {
    if (errLine[i] == '\t') {
      spcs += "\t";
    } else {
      spcs += " ";
    }
  }

  std::cerr << spcs << "^" << std::endl;
}
} // namespace june

// C API

june::SrcFile *srcFileFromC(SrcFileHandle handle) {
  return reinterpret_cast<june::SrcFile *>(handle);
}

extern "C" SrcFileHandle JuneSrcFileNew(const char *dir, const char *path,
                                        bool isMain) {
  return new june::SrcFile(dir, path, isMain);
}

extern "C" void JuneSrcFileFree(SrcFileHandle handle) {
  delete srcFileFromC(handle);
}

extern "C" size_t JuneSrcFileGetId(SrcFileHandle handle) {
  return srcFileFromC(handle)->id();
}

extern "C" const char *JuneSrcFileGetPath(SrcFileHandle handle) {
  return june::string::duplicateAsCString(srcFileFromC(handle)->path());
}

extern "C" const char *JuneSrcFileGetDir(SrcFileHandle handle) {
  return june::string::duplicateAsCString(srcFileFromC(handle)->dir());
}

extern "C" bool JuneSrcFileIsMain(SrcFileHandle handle) {
  return srcFileFromC(handle)->isMain();
}

extern "C" bool JuneSrcFileIsBytecode(SrcFileHandle handle) {
  return srcFileFromC(handle)->isBytecode();
}

extern "C" BytecodeHandle JuneSrcFileGetBytecode(SrcFileHandle handle) {
  return new june::Bytecode(srcFileFromC(handle)->bytecode());
}

extern "C" void JuneSrcFileFail(SrcFileHandle handle, const size_t idx,
                                const char *msg, ...) {
  va_list vargs;
  va_start(vargs, msg);
  srcFileFromC(handle)->fail(idx, msg, vargs);
  va_end(vargs);
}
