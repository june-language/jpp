#ifndef vm_dylib_h
#define vm_dylib_h

typedef void *DylibManagerHandle;

DylibManagerHandle JuneDylibManagerNew();
void JuneDylibManagerDelete(DylibManagerHandle handle);

void *JuneDylibManagerLoad(DylibManagerHandle handle, const char *file);
void JuneDylibManagerUnload(DylibManagerHandle handle, void *dylib);
void *JuneDylibManagerGetSymbol(DylibManagerHandle handle, void *dylib,
                                const char *symbol);
bool JuneDylibManagerHasLoaded(DylibManagerHandle handle, const char *file);

#endif
