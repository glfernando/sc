#!/usr/bin/env python3

import os
import re
import argparse


class mod_dep:
    pass


def create_dep(file_name, args):
    dep = mod_dep()
    dep.src = file_name
    dep.pcm = os.path.splitext(file_name)[0] + '.pcm'
    dep.pcm = os.path.join(args.out_obj_dir, dep.pcm)

    with open(file_name, 'r') as f:
        lines = f.readlines()

    dep.mod_deps = []
    for line in lines:
        m = re.search(r'^export\s*module\s*(\S*);', line)
        if m is not None:
            dep.mod_name = m.group(1)
        m = re.search(r'import\s*(\S*);', line)
        if m is not None:
            dep.mod_deps.append(m.group(1))

    return dep


def dump_copy_dep(dst, src):
    print(f'{dst}: {src}')
    print(f'\t$(Q)cp $@ $<\n')


def dump_module_deps(dep, args):
    if dep.mod_deps:
        print(f'{dep.pcm}: |', end='')
        for mod_dep in dep.mod_deps:
            print(f' {os.path.join(args.pcm_dir, mod_dep)}.pcm', end='')
        print('\n')


def dump_dep(dep, args):
    # create final pcm file location
    final_pcm = os.path.join(args.pcm_dir, dep.mod_name + '.pcm')
    final_pcm = os.path.normpath(final_pcm)
    dep.pcm = os.path.normpath(dep.pcm)
    dump_copy_dep(final_pcm, dep.pcm)

    dump_module_deps(dep, args)


def main():
    parser = argparse.ArgumentParser(description="create module order dependencies for C++ modules")
    parser.add_argument('pcm_dir', help="Directory for PCM files")
    parser.add_argument('src_dir', help="Code source directory")
    parser.add_argument('out_obj_dir', help="Object output directory")

    args = parser.parse_args()

    # create list with all module files (*.cppm)
    mods = [os.path.join(root, file) for root, dir, files in os.walk(args.src_dir)
            for file in files if file.endswith('.cppm')]

    # per each file create an object describing the depencies
    deps = [create_dep(file, args) for file in mods]

    for dep in deps:
        dump_dep(dep, args)


if __name__ == '__main__':
    main()
