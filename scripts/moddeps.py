#!/usr/bin/env python3

import os
import re
import argparse


class mod_dep:
    pass


def create_dep(file_name, args):
    # remove .i extension
    src = os.path.splitext(file_name)[0]
    dep = mod_dep()
    dep.src = src
    dep.is_mod = src.endswith('.cppm')
    dep.target = os.path.splitext(src)[0] + ('.pcm' if dep.is_mod else '.cpp.o')
    # dep.target = os.path.join(args.out_obj_dir, dep.target)
    dep.target = os.path.normpath(dep.target)

    with open(file_name, 'r') as f:
        lines = f.readlines()

    dep.mod_deps = []
    for line in lines:
        m = re.search(r'^export\s*module\s*(\S*);', line)
        if m is not None:
            dep.mod_name = m.group(1)
        m = re.search(r'^(export )?import\s*(\S*);', line)
        if m is not None:
            dep.mod_deps.append(m.group(2))

    return dep


def dump_copy_dep(dst, src):
    print(f'{dst}: {src}')
    print(f'\t$(Q)cp $< $@\n')


def dump_module_deps(dep, args):
    if dep.mod_deps:
        print(f'{dep.target}:', end='')
        for mod_dep in dep.mod_deps:
            print(f' {os.path.join(args.pcm_dir, mod_dep)}.pcm', end='')
        print('\n')


def dump_dep(dep, args):
    # create final pcm file
    if dep.is_mod:
        final_pcm = os.path.join(args.pcm_dir, dep.mod_name + '.pcm')
        final_pcm = os.path.normpath(final_pcm)
        dump_copy_dep(final_pcm, dep.target)

    dump_module_deps(dep, args)


def main():
    parser = argparse.ArgumentParser(description="create module order dependencies for C++ modules")
    parser.add_argument('pcm_dir', help="Directory for PCM files")
    parser.add_argument('out_obj_dir', help="Object output directory")
    parser.add_argument('files', nargs='*', help="files to look up for dependendies")

    args = parser.parse_args()

    # let's do a sanity check, we are only interested on .cpp.i and .cppm.i files
    files = [file for file in args.files if file.endswith('.cppm.i') or file.endswith('.cpp.i')]

    # per each file create an object describing the depencies
    deps = [create_dep(file, args) for file in files]

    for dep in deps:
        dump_dep(dep, args)


if __name__ == '__main__':
    main()
