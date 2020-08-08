#!/usr/bin/env python3
# Convert text file to C string array.

#------------------------------------------------------------------------------

import sys
import os

#------------------------------------------------------------------------------

def print_error(*args):
  sys.stderr.write(' '.join(map(str,args)) + '\n')
  sys.stderr.flush()

def pr_usage(argv):
  print_error('Usage: %s <input_file>' % argv[0])
  sys.exit(0)

def pr_error(msg, usage=None):
  print_error(msg)
  if usage:
    pr_usage(sys.argv)
  sys.exit(1)

#------------------------------------------------------------------------------

def main():

  if len(sys.argv) != 2:
    pr_usage(sys.argv);

  filename = sys.argv[1]

  try:
    f = open(filename, "r")
  except OSError:
    pr_error("could not open %s" % filename)

  x = f.readlines()
  f.close()

  s = []
  s.append("static char *forth_code[] = {")
  for l in x:
    l = l.strip()
    if l:
      s.append("  \"%s\\n\"" % l)
  s.append("};")

  print("\n".join(s))


main()

#------------------------------------------------------------------------------
