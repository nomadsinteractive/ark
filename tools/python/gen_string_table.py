#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
from os import path
import sys
import acg


INDENT = '    '


def declare_namespaces(namespaces, source):
    return '\n'.join(['namespace %s {' % i for i in namespaces]) + '\n' + source + '\n' +'\n'.join(['}'] * len(namespaces))


def output_tofile(content, filename, outputdir):
    if outputdir:
        filepath = path.join(outputdir, filename)
        dirpath = path.dirname(filepath)
        if not path.isdir(dirpath):
            os.makedirs(dirpath)
        with open(filepath, 'wt') as fp:
            fp.write(content)
    else:
        print(filename + ':')
        print(content)


def char_to_ord(c):
    return str(ord(c) if type(c) is str else int(c))


def text_to_chararray(text, indent, colcount):
    return ', '.join(char_to_ord(j) if i % colcount != colcount - 1 else '\n' + indent + char_to_ord(j) for i, j in enumerate(text))


if __name__ == '__main__':

    opts, args = getopt.getopt(sys.argv[1:], 'p:n:o:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in ['p', 'n']) or len(args) == 0:
        print('Usage: %s -p namespace -n name [-o output_dir] dir_paths...' % sys.argv[0])
        sys.exit(0)

    outputdir = params['o'] if 'o' in params else None
    stringtablename = params['n']
    stringtablenames = stringtablename.split('_') + ['string', 'table']
    unit_name = '_'.join(stringtablenames)
    namespaces = params['p'].replace('::', ':').split(':')

    stringtables = []
    stringitems = []

    classname = acg.toCamelName(stringtablenames)
    for i in args:
        category = path.splitext(path.basename(i))[0]
        stringtables.append('\nnamespace %s {\n' % category)
        for j in os.listdir(i):
            filepath = path.join(i, j)
            if path.isfile(filepath) and not j.startswith('.'):
                basename = path.basename(j)
                strname = basename.replace('.', '_')
                with open(path.join(i, j), 'rt') as fp:
                    text = fp.read()
                stringtables.append('static const char %s[] = {\n%s%s, 0};\n' % (strname, INDENT, text_to_chararray(text, INDENT , 16)))
                stringitems.append((category, strname, basename))
        stringtables.append('}\n')

    macro = '_'.join([i.upper() for i in namespaces] + ([i.upper() for i in stringtablenames]) + ['H_'])
    classdeclare = acg.format('''
class ${classname} : public ark::Dictionary<sp<String>> {
public:
    ${classname}();

    virtual sp<String> get(const String& name) override;

private:
    std::map<String, sp<String>> _items;

};
''', classname=classname)

    header = acg.format('''#ifndef ${macro}
#define ${macro}

#include <map>

#include "core/inf/dictionary.h"
#include "core/types/null.h"

${classdeclare}

#endif''', macro=macro, classdeclare=declare_namespaces(namespaces, classdeclare))

    classdefinition = acg.format('''
${classname}::${classname}()
{
    ${0};
}

sp<String> ${classname}::get(const String& name)
{
    auto iter = _items.find(name);
    return iter != _items.end() ? iter->second : nullptr;
}

''', ';\n    '.join(['_items["%s"] = sp<String>::make(%s::%s)' % (i[2], i[0], i[1]) for i in stringitems]), classname=classname)
    bootstrap_func = '__ark_bootstrap_%s__' % '_'.join(stringtablenames)
    source = acg.format('''#include "core/base/string_table.h"
#include "core/types/global.h"

#include "${unit_name}.h"

${body}

using namespace ark;

void ${bootstrap_func}()
{
    const Global<StringTable> string_table;
    string_table->addStringBundle("${stringtablename}", sp<${classname}>::make());
}''', unit_name=unit_name, body=declare_namespaces(namespaces, '\n'.join(stringtables) + '\n' + classdefinition),
                        bootstrap_func=bootstrap_func, stringtablename=stringtablename,
                        classname='::'.join(namespaces + ['']) + classname)

    output_tofile(header, unit_name + '.h', outputdir)
    if not outputdir:
        print('\n----------------------------------------------------------------\n')
    output_tofile(source, unit_name + '.cpp', outputdir)
