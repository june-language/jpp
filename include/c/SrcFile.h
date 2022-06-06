#ifndef vm_srcfile_h
#define vm_srcfile_h

#include "OpCodes.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *SrcFileHandle;

SrcFileHandle JuneSrcFileNew(const char *dir, const char *path, bool isMain);
void JuneSrcFileFree(SrcFileHandle handle);

void JuneSrcFileLoadFile(SrcFileHandle handle, bool *didError,
                         const char *errorStr);

size_t JuneSrcFileGetId(SrcFileHandle handle);
const char *JuneSrcFileGetPath(SrcFileHandle handle);
const char *JuneSrcFileGetDir(SrcFileHandle handle);

bool JuneSrcFileIsMain(SrcFileHandle handle);
bool JuneSrcFileIsBytecode(SrcFileHandle handle);
BytecodeHandle JuneSrcFileGetBytecode(SrcFileHandle handle);

void JuneSrcFileFail(SrcFileHandle handle, const size_t idx, const char *msg,
                     ...);

#ifdef __cplusplus
}
#endif

#endif
