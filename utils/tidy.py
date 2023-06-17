#!/usr/bin/env python3

import glob
import os
import sys
import subprocess

def globr(re):
  return glob.glob(re, recursive=True)


def main(apply=False):
  if not os.path.exists("build/compile_commands.json"):
    print("You need to build the project first to use clang-tidy")
    sys.exit(1)

  """
  Search for all files in the current directory and all subdirectories
  with the extension .cpp or .hpp and tests them with clang-tidy.
  """
  for path in (globr("**/*.cpp") + globr("**/*.hpp")):
    if "build/" in path or "vendor/" in path or "tests/" in path or "bench/" in path:
      continue

    checks = [
      "llvm-qualified-auto",
      "modernize-use-using",
      "modernize-use-equals-default",
      "llvm-else-after-return",
      "modernize-use-trailing-return-type",
      "modernize-pass-by-value",
      # TODO: fix this one soon
      # "cppcoreguidelines-owning-memory",
    ]

    print(f"Tidying {path}")
    cmd = "clang-tidy --checks={checks} -warnings-as-errors=* -header-filter=\"{hf}\"gm {path} {apply} -p build".format(
      path=path,
      checks=",".join(checks),
      hf="include/(VM/.*|.*\\.h(pp)?(.in)?)", # only include June code, not the vendor code
      apply="--fix-errors" if apply else ""
    )
    
    try:
      subprocess.run(cmd, shell=True, check=True, stdout=sys.stdout, stderr=sys.stderr)
    except subprocess.CalledProcessError as e:
      print("\033[31;1;4mTidy failed for {path}\033[0m".format(path=path))
      # raise e


if __name__ == "__main__":
  # look for the "apply" argument
  apply = False
  if "apply" in sys.argv:
    apply = True
  try:
    main(apply)
  except subprocess.CalledProcessError as e:
    sys.exit(0) # its fine, just exit normally

