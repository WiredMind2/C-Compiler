#!/usr/bin/env python3

# In "multiple-files mode" (by default), this script runs both GCC and
# IFCC on each test-case provided and compares the results.
#
# In "single-file mode", we mimic the CLI behaviour of GCC i.e. we
# interpret the '-o', '-S', and '-c' options.
#
# Run "python3 ifcc-test.py --help" for more info.

import argparse
import glob
import os
import shutil
import sys
import subprocess


def run_command(string, logfile=None, toscreen=False):
    """ execute `string` as a shell command. Maybe write stdout+stderr to `logfile` and/or to the toscreen.
        return the exit status"""

    if args.debug:
        print("ifcc-test.py: " + string)

    process = subprocess.Popen(string, shell=True,
                               stderr=subprocess.STDOUT, stdout=subprocess.PIPE,
                               text=True, bufsize=0)
    if logfile:
        logfile = open(logfile, 'w')

    while True:
        output = process.stdout.readline()
        if len(output) == 0:  # only happens when 'process' has terminated
            break
        if logfile: logfile.write(output)
        if toscreen: sys.stdout.write(output)
    process.wait()  # collect child exit status
    assert process.returncode != None  # sanity check (I was using poll() instead of wait() previously, and did see some unsanity)
    if logfile:
        logfile.write(f'\nexit status: {process.returncode}\n')
    return process.returncode


def dumpfile(name, quiet=False):
    data = open(name, "rb").read().decode('utf-8', errors='ignore')
    if not quiet:
        print(data, end='')
    return data


######################################################################################
## ARGPARSE step: make sense of our command-line arguments

# This is where we decide between multiple-files
# mode and single-file mode

import textwrap
import shutil

width = shutil.get_terminal_size().columns - 2
twf = lambda text: textwrap.fill(text, width, initial_indent=' ' * 4, subsequent_indent=' ' * 6)

argparser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="Testing script for the ifcc compiler. operates in one of two modes:\n\n"
                + twf(
        "- Multiple-files mode (by default): Compile several programs with both GCC and IFCC, run them, and compare the results.", ) + "\n\n"
                + twf("- Single-file mode (with options -o,-c and/or -S): Compile and/or assemble and/or link a single program."),
    epilog="examples:\n\n"
           + twf("python3 ifcc-test.py testfiles") + '\n'
           + twf("python3 ifcc-test.py path/to/some/dir/*.c path/to/some/other/dir") + '\n'
           + '\n'
           + twf("python3 ifcc-test.py -o ./myprog path/to/some/source.c") + '\n'
           + twf("python3 ifcc-test.py -S -o truc.asm truc.c") + '\n'
    ,
)

argparser.add_argument('input', metavar='PATH', nargs='+', help='For each path given:'
                                                                + ' if it\'s a file, use this file;'
                                                                + ' if it\'s a directory, use all *.c files under this subtree')

argparser.add_argument('-v', '--verbose', action="count", default=0,
                       help='increase verbosity level. You can use this option multiple times.')
argparser.add_argument('-d', '--debug', action="count", default=0,
                       help='increase quantity of debugging messages (only useful to debug the test script itself)')
argparser.add_argument('-S', action="store_true", help='single-file mode: compile from C to assembly, but do not assemble')
argparser.add_argument('-c', action="store_true", help='single-file mode: compile/assemble to machine code, but do not link')
argparser.add_argument('-o', '--output', metavar='OUTPUTNAME', help='single-file mode: write output to that file')
argparser.add_argument('-a', '--arch', help='Target architecture for ifcc')

args = argparser.parse_args()

if args.debug >= 2:
    print('debug: command-line arguments ' + str(args))

orig_cwd = os.getcwd()
if "ifcc-test-output" in orig_cwd:
    print('error: cannot run ifcc-test.py from within its own output directory')
    exit(1)

pld_base_dir = os.path.normpath(os.path.dirname(__file__))
if args.debug:
    print("ifcc-test.py: " + os.path.dirname(__file__))

# cleanup stale output directory
if os.path.isdir(f'{pld_base_dir}/ifcc-test-output'):
    run_command(f'rm -rf {pld_base_dir}/ifcc-test-output')

# Ensure that the `ifcc` executable itself is up-to-date
makestatus = run_command(f'cd {pld_base_dir}/compiler; make --question ifcc')
if makestatus:  # updates are needed
    makestatus = run_command(f'cd {pld_base_dir}/compiler; make ifcc', toscreen=True)  # this time we run `make` for real
    if makestatus:  # if `make` failed, we fail too
        if os.path.exists("ifcc"):  # and we remove any out-of-date compiler (to reduce chance of confusion)
            os.unlink("ifcc")
        exit(makestatus)

arch_option = f'--arch {args.arch}' if args.arch else ''

##########################################
## single-file mode aka "let's act just like GCC (almost)"

if args.S or args.c or args.output:
    if args.S and args.c:
        print("error: options -S and -c are not compatible")
        exit(1)
    if len(args.input) > 1:
        print("error: this mode only supports a single input file")
        exit(1)
    inputfilename = args.input[0]

    if inputfilename[-2:] != ".c":
        print("error: incorrect filename suffix (should be '.c'): " + inputfilename)
        exit(1)

    try:
        open(inputfilename, "r").close()
    except Exception as e:
        print("error: " + e.args[1] + ": " + inputfilename)
        exit(1)

    if (args.S or args.c) and not args.output:
        print("error: option '-o filename' is required in this mode")
        exit(1)

    if args.S:  # produce assembly
        if args.output[-2:] != ".asm":
            print("error: output file name must end with '.asm'")
            exit(1)
        ifccstatus = run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} {inputfilename} > {args.output}')
        if ifccstatus:  # let's show error messages on screen
            exit(run_command(f'{pld_base_dir}/compiler/ifcc {inputfilename}', toscreen=True))
        else:
            exit(0)

    elif args.c:  # produce machine code
        if args.output[-2:] != ".o":
            print("error: output file name must end with '.o'")
            exit(1)
        asmname = args.output[:-2] + ".asm"
        ifccstatus = run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} {inputfilename} > {asmname}')
        if ifccstatus:  # let's show error messages on screen
            exit(run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} {inputfilename}', toscreen=True))
        exit(run_command(f'gcc -c -o {args.output} {asmname}', toscreen=True))

    else:  # produce an executable
        if args.output[-2:] in [".o", ".c", ".asm"]:
            print("error: incorrect name for an executable: " + args.output)
            exit(1)
        asmname = args.output + ".asm"
        ifccstatus = run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} {inputfilename} > {asmname}')
        if ifccstatus:
            exit(run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} {inputfilename}', toscreen=True))
        exit(run_command(f'gcc -o {args.output} {asmname}'))

    # we should never end up here
    print("unexpected error. please report this bug.")
    exit(1)

# if we were not in single-file mode, then it means we are in
# multiple-files mode.

######################################################################################
## PREPARE step: find and copy all test-cases under ifcc-test-output

## Process each cli argument as a filename or subtree
os.chdir(orig_cwd)
inputfilenames = []
for path in args.input:
    path = os.path.normpath(path)  # collapse redundant slashes etc.
    if os.path.isfile(path):
        if path[-2:] == '.c':
            inputfilenames.append(path)
        else:
            print("error: incorrect filename suffix (should be '.c'): " + path)
            exit(1)
    elif os.path.isdir(path):
        for dirpath, dirnames, filenames in os.walk(path):
            inputfilenames += [dirpath + '/' + name for name in filenames if name[-2:] == '.c']
    else:
        print("error: cannot read input path `" + path + "'")
        sys.exit(1)

inputfilenames = sorted(inputfilenames)
## debug: after treewalk
if args.debug:
    print("debug: list of files after tree walk:", " ".join(inputfilenames))

## sanity check
if len(inputfilenames) == 0:
    print("error: found no test-case in: " + " ".join(args.input))
    sys.exit(1)

## Check that we actually can read these files. Our goal is to
## fail as early as possible when the CLI arguments are wrong.
for inputfilename in inputfilenames:
    try:
        f = open(inputfilename, "r")
        f.close()
    except Exception as e:
        print("error: " + e.args[1] + ": " + inputfilename)
        exit(1)

## We're going to copy every test-case in its own subdir of ifcc-test-output
os.mkdir(pld_base_dir + '/ifcc-test-output')

jobs = []

for inputfilename in inputfilenames:
    if args.debug >= 2:
        print("debug: PREPARING " + inputfilename)

    if 'ifcc-test-output' in os.path.realpath(inputfilename):
        print('error: input filename is within output directory: ' + inputfilename)
        exit(1)

    ## each test-case gets copied and processed in its own subdirectory:
    ## ../somedir/subdir/file.c becomes ifcc-test-output/--somedir-subdir-file/input.c
    subdirname = inputfilename[:-2]  # remove the '.c' suffix
    if pld_base_dir in subdirname:  # hide "absolute" part of path when not meaningful
        subdirname = subdirname[len(pld_base_dir):]
    subdirname = subdirname.replace('..', '-')  # keep some punctuation to discern "bla.c" from "../bla.c"
    subdirname = subdirname.replace('./', '')  # remove meaningless part of name
    subdirname = subdirname.replace('/', '-')  # flatten path to single subdir
    if args.debug >= 2:
        print("debug: subdir=" + subdirname)

    os.mkdir(pld_base_dir + '/ifcc-test-output/' + subdirname)
    shutil.copyfile(inputfilename, pld_base_dir + '/ifcc-test-output/' + subdirname + '/input.c')
    jobs.append(subdirname)

## eliminate duplicate paths from the 'jobs' list
unique_jobs = []
for j in jobs:
    for d in unique_jobs:
        if os.path.samefile(pld_base_dir + '/ifcc-test-output/' + j, pld_base_dir + '/ifcc-test-output/' + d):
            break  # and skip the 'else' branch
    else:
        unique_jobs.append(j)
jobs = sorted(unique_jobs)

# debug: after deduplication
if args.debug:
    print("debug: list of test-cases after PREPARE step:", " ".join(jobs))


######################################################################################
## COLORS

class C:
    """ANSI color codes"""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    CYAN = '\033[36m'
    WHITE = '\033[97m'
    DIM = '\033[2m'

    @staticmethod
    def ok(s):    return C.GREEN + s + C.RESET

    @staticmethod
    def fail(s):  return C.RED + s + C.RESET

    @staticmethod
    def warn(s):  return C.YELLOW + s + C.RESET

    @staticmethod
    def info(s):  return C.CYAN + s + C.RESET

    @staticmethod
    def bold(s):  return C.BOLD + s + C.RESET

    @staticmethod
    def dim(s):   return C.DIM + s + C.RESET


def suite_of(jobname):
    """Return the test-suite name for a job (first component of the path)."""
    parts = jobname.lstrip('-').split('-')
    # jobname looks like  -testfiles-01_variables-00_return_var
    # We want to group by the directory that was originally passed (e.g. testfiles/01_variables)
    # The jobname was built by replacing '/' with '-' so the first two meaningful tokens
    # form the suite name.  We use the second token (index 1 after lstrip('-').split('-'))
    # as the suite: e.g. "testfiles-01_variables"
    # Actually let's just split on the original separator logic:
    # jobname = "-testfiles-01_variables-00_return_var"
    # strip leading '-', split on '-', take first 2 parts
    parts = jobname.lstrip('-').split('-')
    if len(parts) >= 2:
        return parts[0] + '-' + parts[1]
    return parts[0]


######################################################################################
## TEST step: actually compile/link/run each test-case with both compilers.
##
##            if both toolchains agree, this test-case is passed.
##            otherwise, this is a fail.

all_ok = True

# Group jobs by test suite
suite_results = {}  # suite_name -> list of (jobname, status_str, ok:bool)
current_suite = None


def print_suite_header(suite):
    bar = '─' * (shutil.get_terminal_size().columns - 2)
    print()
    print(C.bold(C.info(f'  ▶  Test suite: {suite}')))
    print(C.dim(bar))


def print_fail_details(label, *files):
    print(C.dim('    ┌─ ' + label))
    for f in files:
        if os.path.exists(f):
            data = open(f, errors='ignore').read().strip()
            if data:
                for line in data.splitlines():
                    print(C.dim('    │ ') + line)
    print(C.dim('    └─'))


for jobname in jobs:
    os.chdir(f'{pld_base_dir}/ifcc-test-output')

    suite = suite_of(jobname)
    if suite != current_suite:
        # Print previous suite summary if any
        if current_suite is not None and current_suite in suite_results:
            results = suite_results[current_suite]
            n_ok = sum(1 for _, _, ok in results if ok)
            n_fail = len(results) - n_ok
            summary = C.ok(f'{n_ok} passed') + '  ' + (C.fail(f'{n_fail} failed') if n_fail else C.dim('0 failed'))
            print(C.dim('─' * (shutil.get_terminal_size().columns - 2)))
            print(f'  Suite result: {summary}')
        current_suite = suite
        suite_results[suite] = []
        print_suite_header(suite)

    # Short display name = last component
    short = jobname.lstrip('-').split('-')
    display_name = '-'.join(short[2:]) if len(short) > 2 else jobname

    os.chdir(jobname)

    ## Reference compiler = GCC
    gccstatus = run_command("gcc -S -o asm-gcc.s input.c", "gcc-compile.txt")
    if gccstatus == 0:
        gccstatus = run_command("gcc -o exe-gcc asm-gcc.s", "gcc-link.txt")
    if gccstatus == 0:
        exegccstatus = run_command("./exe-gcc", "gcc-execute.txt")
        if args.verbose >= 2:
            dumpfile("gcc-execute.txt")

    ## IFCC compiler
    ifccstatus = run_command(f'{pld_base_dir}/compiler/ifcc {arch_option} input.c > asm-ifcc.s', 'ifcc-compile.txt')


    def record(ok, msg):
        global all_ok
        icon = C.ok('✔') if ok else C.fail('✘')
        label = C.ok(msg) if ok else C.fail(msg)
        print(f'    {icon}  {display_name:<40}  {label}')
        suite_results[suite].append((jobname, msg, ok))
        if not ok:
            all_ok = False


    if gccstatus != 0 and ifccstatus != 0:
        record(True, "OK (both reject invalid program)")
    elif gccstatus != 0 and ifccstatus == 0:
        record(False, "FAIL: compiler accepts an invalid program")
        if not args.verbose:
            print_fail_details('ifcc stdout', 'asm-ifcc.s')
            print_fail_details('ifcc stderr', 'ifcc-compile.txt')
    elif gccstatus == 0 and ifccstatus != 0:
        record(False, "FAIL: compiler rejects a valid program")
        if args.verbose:
            dumpfile("asm-ifcc.s")
            dumpfile("ifcc-compile.txt")
        else:
            print_fail_details('ifcc stderr', 'ifcc-compile.txt')
    else:
        ## ifcc accepted to compile valid program -> let's link it
        ldstatus = run_command("gcc -o exe-ifcc asm-ifcc.s", "ifcc-link.txt")
        if ldstatus:
            record(False, "FAIL: compiler produces incorrect assembly")
            if args.verbose:
                dumpfile("asm-ifcc.s")
                dumpfile("ifcc-link.txt")
            else:
                print_fail_details('linker output', 'ifcc-link.txt')
        else:
            ## run both executables and compare results
            run_command("./exe-ifcc", "ifcc-execute.txt")
            if open("gcc-execute.txt").read() != open("ifcc-execute.txt").read():
                record(False, "FAIL: different results at execution")
                if args.verbose:
                    print("GCC:")
                    dumpfile("gcc-execute.txt")
                    print("you:")
                    dumpfile("ifcc-execute.txt")
                else:
                    print_fail_details('expected (gcc)', 'gcc-execute.txt')
                    print_fail_details('got (ifcc)', 'ifcc-execute.txt')
            else:
                record(True, "OK")
                if args.verbose >= 2:
                    dumpfile("ifcc-execute.txt")

## Print last suite summary
if current_suite is not None and current_suite in suite_results:
    results = suite_results[current_suite]
    n_ok = sum(1 for _, _, ok in results if ok)
    n_fail = len(results) - n_ok
    summary = C.ok(f'{n_ok} passed') + '  ' + (C.fail(f'{n_fail} failed') if n_fail else C.dim('0 failed'))
    print(C.dim('─' * (shutil.get_terminal_size().columns - 2)))
    print(f'  Suite result: {summary}')

######################################################################################
## GLOBAL SUMMARY

total_ok = sum(ok for res in suite_results.values() for _, _, ok in res)
total_fail = sum(not ok for res in suite_results.values() for _, _, ok in res)
total = total_ok + total_fail

bar = '═' * (shutil.get_terminal_size().columns - 2)
print()
print(C.bold(bar))
print(C.bold('  GLOBAL SUMMARY'))
print(C.bold(bar))
for suite, results in suite_results.items():
    n_ok = sum(1 for _, _, ok in results if ok)
    n_fail = len(results) - n_ok
    icon = C.ok('✔') if n_fail == 0 else C.fail('✘')
    print(f'  {icon}  {suite:<35}  {C.ok(str(n_ok) + "✔"):>6}  {(C.fail(str(n_fail) + "✘") if n_fail else C.dim("0✘")):>6}  / {len(results)}')
print(C.bold(bar))
overall = C.ok(f'ALL {total} TESTS PASSED') if all_ok else C.fail(f'{total_fail}/{total} TESTS FAILED')
print(f'  {overall}')
print(C.bold(bar))
print()

if not all_ok:
    sys.exit(1)
