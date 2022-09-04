#ifndef config_hpp
#define config_hpp

/// June git revision
#define JuneGitRev "703c4b1"

/// June build date
#define JuneBuildDate "2022-09-03"

/// June version
#define JuneVersion "0.1"

/// June Memory Debugging
#define JuneMemDebug true

/// June debug check
/// The reason it's not a macro is because
/// we need to be able to override it at runtime.
static bool JuneDebug = true;

#endif

