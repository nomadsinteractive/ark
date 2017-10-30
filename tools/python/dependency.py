#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
from os import path
import subprocess
import sys


_encoding = sys.stdout.encoding or 'utf8'

def main():
    target_dir = sys.argv[1]
    target_dirname, target_name = path.split(target_dir)
    script_dir, script_name = path.split(sys.argv[0])
    opts, args = parse_opts({'r': path.join(script_dir, 'repositories.json'), 'n': target_name})
    if any(i not in opts for i in []):
        print('Usage: %s name target_dir ' % sys.argv[0])
        sys.exit(0)
    repo = get_repository(opts['r'], opts['n'])

    if not repo or 'tag' not in repo:
        print('Repository "%s" no found or no tag specified' % opts['n'])
        sys.exit(0)

    tag = repo['tag']

    if not path.isdir(target_dir):
        if not path.isdir(target_dirname):
            os.makedirs(target_dirname)
        git_clone(target_dirname, repo['url'], target_name)

    try:
        output = subprocess.check_output(['git', 'describe', '--tags'],
                                         cwd=target_dir).decode(_encoding).strip()
    except subprocess.CalledProcessError:
        output = ''

    if output != tag:
        tags = git_tag_list(target_dir)
        if tag not in tags:
            git_pull(target_dir)
        git_checkout(target_dir, tag)


def git_clone(dirname, url, name):
    subprocess.call(['git', 'clone', url, name], cwd=dirname)


def git_checkout(target_dir, tag):
    subprocess.call(['git', 'checkout', '--detach', tag], cwd=target_dir)


def git_pull(target_dir):
    subprocess.call(['git', 'pull'], cwd=target_dir)


def git_tag_list(target_dir):
    try:
        return [i.strip() for i in subprocess.check_output(['git', 'tag'], cwd=target_dir).decode(_encoding).strip().split('\n')]
    except subprocess.CalledProcessError:
        return []


def parse_opts(default_opts):
    opts, args = getopt.getopt(sys.argv[3:], 'r:')
    opts = {i.lstrip('-'): j for i, j in opts}
    opts.update({i: default_opts[i] for i in default_opts.keys() if i not in opts})
    return opts, args


def get_repository(filename, name):
    with open(filename, 'r') as fp:
        repo = [i for i in eval(fp.read()) if 'name' in i and i['name'] == name]
    return repo[0] if repo else None


if __name__ == '__main__':
    main()
