#!/usr/bin/env python3
"""
Very simple python script to compile and launch several programs.
It prints system and compiler info before that.
"""

from subprocess import check_call, check_output
import argparse
import os


class Config:
    def __init__(self, args):
        self.gcc = self.tool('gcc', args.gcc)
        self.gpp = self.tool('g++', args.gcc)
        self.clang = self.tool('clang', args.clang)
        self.clangpp = self.tool('clang++', args.clang)

    @staticmethod
    def tool(base, suffix):
        if suffix:
            return f'{base}-{suffix}'
        else:
            return base

c = None


def mkdirp(path):
    try:
        os.mkdir(path)
    except FileExistsError:
        pass


def call(*args):
    print(">>>", " ".join(*args))
    return check_call(*args)


def print_info():
    """Gather system and compilators information"""
    print("========== INFO ========\n")
    call(['uname', '-srvmo'])
    print("\n")
    call(['lsb_release', '-a'])
    print("\n")
    call(['lscpu'])
    print("\n")

    for compiler in (c.gcc, c.clang):
        call([compiler, '--version'])
    print("\n\n")


def cc_run(cc, cc_args, main_soruce):
    exe = 'out/a.exe'
    mkdirp(os.path.dirname(exe))
    call([cc, *cc_args, main_soruce, '-o', exe])
    call([exe])


def main(argv):
    parser = argparse.ArgumentParser(description="Benchmark launcher")
    parser.add_argument('--gcc', help='suffix for gnu compiler executables', default='')
    parser.add_argument('--clang', help='suffix for clang executables', default='')

    global c
    c = Config(parser.parse_args())

    print_info()

    CC_ARGS = ['-O3', '-march=native', '-std=gnu++17']
    
    print("======== BENCH FIXES ==========\n")
    for cc in (c.gpp, c.clangpp):
      cc_run(cc, CC_ARGS, 'src/main.cpp')

    print("======== BENCH DATA ==========\n")
    for cc in (c.gpp, c.clangpp):
      cc_run(cc, CC_ARGS, 'src/data.cpp')



if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    import sys
    main(sys.argv)
