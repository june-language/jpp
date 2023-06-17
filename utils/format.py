#!/usr/bin/env python3

import glob
import subprocess
import sys

def globr(regexp):
  return glob.glob(regexp, recursive=True)

def main():
  """
  Search for all files in the current directory and its subdirectories
  with the extension .cpp, .hpp, .c, or .h and format them with clang-format.
  """
  for path in (globr('**/*.[ch]') + globr('**/*.[ch]pp')):
    if "build/" in path or "vendor/" in path:
      continue
      
    print('Formatting {}'.format(path))
    subprocess.run('clang-format -i {}'.format(path), shell=True, check=True,
                   stdout=sys.stdout, stderr=sys.stderr)

if __name__ == '__main__':
  main()
