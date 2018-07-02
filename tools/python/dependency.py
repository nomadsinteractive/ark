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

    if not repo:
        print('Repository "%s" no found' % opts['n'])
        sys.exit(0)

    if 'tag' not in repo and 'branch' not in repo:
        print('No tag or branch specified in "%s"' % repo)
        sys.exit(0)

    if 'tag' in repo:
        checkout_specified_tag(repo, repo['tag'], target_name, target_dirname, target_dir)
    else:
        checkout_specified_branch(repo, repo['branch'], target_name, target_dirname, target_dir)


def checkout_specified_tag(repo, tag, target_name, target_dirname, target_dir):
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


def checkout_specified_branch(repo, branch, target_name, target_dirname, target_dir):
    if branch == 'master':
        return

    if not path.isdir(target_dir):
        if not path.isdir(target_dirname):
            os.makedirs(target_dirname)
        git_clone(target_dirname, repo['url'], target_name)

    try:
        output = subprocess.check_output(['git', 'status', '.'], cwd=target_dir).decode(_encoding).strip()
    except subprocess.CalledProcessError:
        output = ''
    lines = output.split('\n')
    if lines:
        current_branch = lines[0].split()[-1]
        if branch.find(current_branch) == -1:
            branches = git_branch_list(target_dir)
            if branch not in branches:
                git_pull(target_dir)
            git_checkout(target_dir, branch)


def git_clone(dirname, url, name):
    subprocess.call(['git', 'clone', url, name], cwd=dirname)


def git_checkout(target_dir, name):
    subprocess.call(['git', 'checkout', '--detach', name], cwd=target_dir)


def git_pull(target_dir):
    subprocess.call(['git', 'pull'], cwd=target_dir)


def git_tag_list(target_dir):
    try:
        return [i.strip() for i in subprocess.check_output(['git', 'tag'], cwd=target_dir).decode(_encoding).strip().split('\n')]
    except subprocess.CalledProcessError:
        return []


def git_branch_list(target_dir):
    try:
        return [i.strip() for i in subprocess.check_output(['git', 'branch', '-a'], cwd=target_dir).decode(_encoding).strip().split('\n')]
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
