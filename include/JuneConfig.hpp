#ifndef config_hpp
#define config_hpp

/// June git revision
#define JuneGitRev "7b249a8"

/// June build date
#define JuneBuildDate "2022-09-08"

/// June version
#define JuneVersion "0.1"

/// June Memory Debugging
#define JuneMemDebug true

/// June debug check
/// The reason it's not a macro is because
/// we need to be able to override it at runtime.
static bool JuneDebug = true;

#endif

