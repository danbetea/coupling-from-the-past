"""
A thin wrapper around fast coupling from the past, implemented in C in the file squareice.c (see said file for authors)

-h for help

Example usage:
    :: generate a 15 by 15 uniform random alternating sign matrix ::
    python random_asm.py 15

(C) Dan Betea 2017
"""

import subprocess, os, sys, getopt

def rand_asm(n):
    cmd_exec = ["./sqi", str(n+1),"-asm"]
    try:
        result = subprocess.Popen(cmd_exec, stdout=subprocess.PIPE, stderr=open('/dev/null','w'))
    except OSError:
        sys.stderr.write("Running program for the first time. Compiling C code...\n")
        # note: using call, as it ends for the completion of the compilation
        cmd_compile = ['gcc', 'squareice.c', 'r250.c', '-O', '-o', 'sqi']
        result_compile = subprocess.call(cmd_compile, stderr=open('/dev/null','w'))
        sys.stderr.write("End of compilation.\n")
        result = subprocess.Popen(cmd_exec, stdout=subprocess.PIPE, stderr=open('/dev/null','w'))

    return [[int(x) for x in o.split()] for o in result.stdout.read().split('\n')[:-1]]

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "h", ["help"])
        except getopt.error, msg:
            raise Usage(msg)
        # process options
        for o, a in opts:
            if o in ("-h", "--help"):
                print __doc__
                sys.exit(0)
        # process arguments
        n = int(args[0])
        print rand_asm(n)
    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, "for help use --help"
        return 2


if __name__ == "__main__":
    sys.exit(main())
