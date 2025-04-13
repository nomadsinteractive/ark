#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
import sys
import re
import acg
from acg import HeaderPattern

ANNOTATION_PATTERN = r'(?:\s*(?://)?\s*\[\[[^]]+\]\])*'
BUILDER_IMPLEMENTATION_PATTERN = r'class\s+([\w_]+)\s*(?:final)?\s*:\s*public\s+(I?Builder)<([^{]+)>\s+\{'
BUILDER_PATTERN = re.compile(r'\[\[plugin::(builder|resource-loader)(?:\("([\w\-_]+)"\))?\]\]\s+' + BUILDER_IMPLEMENTATION_PATTERN)
DICTIONARY_PATTERN = re.compile(r'\[\[plugin::(builder|resource-loader)::by-value(?:\("([\w\-_]+)"\))?\]\]\s+' + BUILDER_IMPLEMENTATION_PATTERN)

INDENT = '\n    '
REF_BUILDER_TEMPLATE = '''BeanFactory::Factory ${plugin_name}::createBeanFactory()
{
    BeanFactory::Factory refBeanFactory;
    %s
    return refBeanFactory;
}'''
RES_BUILDER_TEMPLATE = '''BeanFactory::Factory ${plugin_name}::createResourceLoader(const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    BeanFactory::Factory resBeanFactory;
    %s
    return resBeanFactory;
}'''

_class_members = {}


def declare_namespaces(namespaces, source):
    return '\n'.join(['namespace %s {' % i for i in namespaces]) + '\n\n' + source + '\n' +'\n'.join(['}'] * len(namespaces))


def to_member_name(name):
    return '_' + re.sub(r'[A-Z]', lambda x: '_' + x.group(0).lower(), name)


class Annotation:
    def __init__(self, filename, name, interface_class, implement_class, implemented_interface, main_class, arguments):
        self._name = name
        self._filename = filename
        self._interface_class = interface_class
        self._implement_class = implement_class
        self._implemented_interface = implemented_interface
        self._main_class = main_class
        self._arguments = arguments

    def include(self, include_dir):
        includes = [include_dir, self._filename] if include_dir else [self._filename]
        return '#include "%s"' % '/'.join(includes)

    def _build_arguments(self, func_arguments, lambda_arguments):
        func_args = dict(func_arguments)
        lambda_args = dict(lambda_arguments)
        args = []
        captured_args = []
        for argtype, argname in self._arguments:
            if argtype in func_args:
                args.append(func_args[argtype])
            elif argtype in lambda_args:
                captured_args.append(lambda_args[argtype])
            else:
                membername = to_member_name(argname)
                _class_members[membername] = argtype
                captured_args.append(membername)
        capture_this = any(i.startswith('_') for i in captured_args)
        capture_candidiate = ['this'] + captured_args if capture_this else captured_args
        return ', '.join(i for i in capture_candidiate if not i.startswith('_')), ', '.join(args + captured_args)

    def _make_call(self, statement, builder_arguments, tplfunc, tplcontent):
        name = '"%s", ' % self._name if self._name else ''
        func_arguments = parse_arguments(builder_arguments)
        lambda_arguments = parse_function_arguments(tplfunc, tplcontent)
        capture_args, passing_args = self._build_arguments(func_arguments, lambda_arguments)
        interface_class = self._interface_class if self._implemented_interface.startswith('I') else f'sp<{self._interface_class}>'
        result_type = f'sp<{self._implemented_interface}<{self._interface_class}>>'
        return '%s<%s>(%s[%s](%s)->%s { return %s::make<%s::%s>(%s); });' % (statement, interface_class, name, capture_args, builder_arguments, result_type, result_type, self._main_class, self._implement_class, passing_args)


class ResourceLoader(Annotation):
    def __init__(self, filename, name, interface_class, implement_class, implemented_interface, main_class, arguments):
        super().__init__(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments)

    def res_builder(self):
        return self._make_call('resBeanFactory.addBuilderFactory', 'BeanFactory& factory, const document& doc', 'createResourceLoader', RES_BUILDER_TEMPLATE)


class Builder(Annotation):
    def __init__(self, filename, name, interface_class, implement_class, implemented_interface, main_class, arguments):
        super().__init__(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments)

    def ref_builder(self):
        return self._make_call('refBeanFactory.addBuilderFactory', 'BeanFactory& factory, const document& doc', 'createBeanFactory', REF_BUILDER_TEMPLATE)


class Dictionary(Annotation):
    def __init__(self, filename, name, interface_class, implement_class, implemented_interface, main_class, arguments):
        super().__init__(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments)

    def ref_builder(self):
        return self._make_call('refBeanFactory.addDictionaryFactory', 'BeanFactory& factory, const String& value', 'createBeanFactory', REF_BUILDER_TEMPLATE)


class ResourceLoaderDictionaryNamed(Annotation):
    def __init__(self, filename, name, interface_class, main_class, builder_name, arguments):
        super().__init__(filename, name, interface_class, main_class, 'Builder', builder_name, arguments)

    def res_builder(self):
        return self._make_call('resBeanFactory.addDictionaryFactory', 'BeanFactory& factory, const String& value', 'createResourceLoader', RES_BUILDER_TEMPLATE)


class ResourceLoaderDictionaryNonamed(Annotation):
    def __init__(self, filename, interface_class, implement_class, main_class, arguments):
        super().__init__(filename, None, interface_class, implement_class, 'Builder', main_class, arguments)

    def res_builder(self):
        func_arguments = parse_function_arguments('createResourceLoader', RES_BUILDER_TEMPLATE)
        capture_args, passing_args = self._build_arguments(parse_arguments('BeanFactory& factory, const String& value'), func_arguments)
        return 'resBeanFactory.addDictionaryFactory<sp<%s>>([%s](BeanFactory& factory, const String& value)->sp<Builder<%s>> { return sp<Builder<%s>>::make<%s::%s>(%s); });' % (self._interface_class, capture_args, self._interface_class, self._interface_class, self._main_class, self._implement_class, passing_args)


def parse_function_arguments(funcname, content):
    m = re.search(r'\b%s\(((?:[^,]*?,?)*)\)' % funcname, content)
    return parse_arguments(m.group(1)) if m else []


def parse_arguments(args):
    arglist = [i.split() for i in args.split(',')]

    def parse_type_name(s):
        if len(s) == 1:
            return s[-1].strip('&'), ''
        return s[-2].strip('&'), s[-1].strip('&')

    return [parse_type_name(i) for i in arglist if len(i)]


def get_plugin_arguments():
    return ', '.join('const %s& %s' % (v, acg.camel_name(k.split('_'))) for k, v in _class_members.items())


def get_plugin_arguments_assigment():
    return ', ' + ', '.join('%s(%s)' % (k, acg.camel_name(k.split('_'))) for k, v in _class_members.items()) if _class_members else ''


def get_member_declare():
    if not _class_members:
        return ''
    getters = '\n    ' + '\n    '.join('const %s& get%s() const;' % (v, acg.to_camel_name(k.split('_'))) for k, v in _class_members.items())

    return getters + '\n\nprivate:\n    ' + '\n    '.join('%s %s;' % (v, k) for k, v in _class_members.items()) + '\n'


def get_plugin_member_getter(plugin_name):
    if not _class_members:
        return ''

    return '\n\n' + '\n\n'.join('const %s& %s::get%s() const\n{\n    return %s;\n}' % (v, plugin_name, acg.to_camel_name(k.split('_')), k) for k, v in _class_members.items())


def output_to_file(output_file, content):
    if output_file is None:
        print(content)
    else:
        directory_name = os.path.dirname(output_file)
        if not os.path.isdir(directory_name):
            os.makedirs(directory_name)
        with open(output_file, 'wt') as fp:
            fp.write(content)


def search_for_plugins(paths):
    result = []

    def match_builder(filename, content, main_class, x):
        builder_type, name, implement_class, implemented_interface, interface_class = x
        arguments = [(i, j) for i, j in parse_function_arguments(implement_class, content) if not i.startswith('=')]
        if builder_type != 'resource-loader':
            result.append(Builder(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments))
        else:
            result.append(ResourceLoader(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments))

    def match_dictionary(filename, content, main_class, x):
        builder_type, name, implement_class, implemented_interface, interface_class = x
        arguments = parse_function_arguments(implement_class, content)
        if builder_type != 'resource-loader':
            result.append(Dictionary(filename, name, interface_class, implement_class, implemented_interface, main_class, arguments))
        elif name:
            result.append(ResourceLoaderDictionaryNamed(filename, name, interface_class, implement_class, main_class, arguments))
        else:
            result.append(ResourceLoaderDictionaryNonamed(filename, interface_class, implement_class, main_class, arguments))

    acg.match_header_patterns(paths, True,
                              HeaderPattern(BUILDER_PATTERN, match_builder),
                              HeaderPattern(DICTIONARY_PATTERN, match_dictionary))
    return result


def generate(generator, result, *args):
    return sorted(set(getattr(i, generator)(*args) for i in result if hasattr(i, generator)))


def generate_method_body(generator, result, template):
    lines = generate(generator, result)
    return template % INDENT.join(lines) if lines else 'return BeanFactory::Factory();'


def generate_plugin_includes(config, result, include_dir):
    includes = generate('include', result, include_dir)
    return '\n'.join(sorted(includes))


def generate_method_def(generator, result, template, **args):
    lines = generate(generator, result)
    return '\n\n' + acg.format(template, **args) % INDENT.join(lines) if lines else ''


def load_config_from_file(file):
    with open(file, 'rt') as fp:
        return eval(fp.read())


def parse_app_arguments(params, **kwargs):
    name = params['n'].strip() if 'n' in params else ''
    config_file = params['c'] if 'c' in params else None
    namespace = [i for i in params['s'].strip().split(':') if i] if 's' in params else []
    args = load_config_from_file(config_file) if config_file else {'name': name, 'namespace': namespace}
    for k, v in kwargs.items():
        if k not in args:
            args[k] = v
    return args


def generate_bootstrap_func(func_name, namespace, plugin_name):
    classname = '::'.join(namespace + [plugin_name]) if namespace else plugin_name
    return '''
void __ark_bootstrap_%s__()
{
    const ark::Global<ark::PluginManager> pluginManager;
    pluginManager->addPlugin(ark::sp<%s>::make());
}
''' % (func_name, classname)


def main():
    opts, args = getopt.getopt(sys.argv[1:], 'c:n:o:s:b:t:i:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in []) or len(args) == 0:
        print('Usage: %s [-c config_file] [-n name] [-t type] [-o output_file] [-s namespace] [-b bootstrapfunc] [-i include_dir] src_paths...' % sys.argv[0])
        sys.exit(0)

    output_file = params['o'] if 'o' in params else None
    bootstrap_func = params['b'].strip() if 'b' in params else None
    include_dir = params.get('i', None)
    config = parse_app_arguments(params, type=params['t'].strip() if 't' in params else 'core')

    result = search_for_plugins(args)
    file_path = 'stdout'
    if output_file:
        file_dir, file_path = os.path.split(output_file)
    plugin_name = config['plugin_name'] if 'plugin_name' in config else acg.to_camel_name(file_path.split('_'))
    ark_namespace = 'ark::' if 'ark' not in config['namespace'] else ''
    ns = ark_namespace

    ref_builder = generate_method_def('ref_builder', result, REF_BUILDER_TEMPLATE, plugin_name=plugin_name)
    ref_builder_declare = f'{ns}BeanFactory::Factory createBeanFactory() override;' if ref_builder else ''

    res_builder = generate_method_def('res_builder', result, RES_BUILDER_TEMPLATE, plugin_name=plugin_name)
    res_builder_declare = f'{ns}BeanFactory::Factory createResourceLoader(const {ns}sp<{ns}ResourceLoaderContext>& resourceLoaderContext) override;' if res_builder else ''

    classdeclare = acg.format('''class ${plugin_name} final : public ${ns}Plugin {
public:
    ${plugin_name}(${plugin_arguments});

    ${ref_builder_declare}
    ${res_builder_declare}
${member_declare}
};
''', plugin_name=plugin_name, ns=ark_namespace, member_declare=get_member_declare(), plugin_arguments=get_plugin_arguments(), ref_builder_declare=ref_builder_declare, res_builder_declare=res_builder_declare)
    content = acg.format('''#pragma once

#include "core/base/plugin.h"
#include "core/base/resource_loader.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

${classdeclare}

''', header_macro=file_path.upper(), classdeclare=declare_namespaces(config['namespace'], classdeclare))
    acg.write_to_file(output_file + '.h' if output_file else None, content)

    classdefine = acg.format('''${plugin_name}::${plugin_name}(${plugin_arguments})
    : Plugin("${name}", Plugin::PLUGIN_TYPE_${type})${plugin_arguments_assigment} {
}
${ref_builder}${res_builder}${plugin_member_getter}''', name=config['name'], type=config['type'].upper(), plugin_name=plugin_name, plugin_arguments=get_plugin_arguments(), plugin_arguments_assigment=get_plugin_arguments_assigment(), ref_builder=ref_builder, res_builder=res_builder, plugin_member_getter=get_plugin_member_getter(plugin_name))
    using_namespace = 'using namespace ark;' if 'ark' not in config['namespace'] else ''
    content = acg.format('''#include "${file_path}.h"

#include "core/base/bean_factory.h"
#include "core/base/plugin_manager.h"
#include "core/types/global.h"
${plugin_includes}
${using_namespace}
${classdefine}
${bootstrap_func}''', file_path=file_path, plugin_includes=generate_plugin_includes(config, result, include_dir), using_namespace=using_namespace, classdefine=declare_namespaces(config['namespace'], classdefine), bootstrap_func=generate_bootstrap_func(bootstrap_func, config['namespace'], plugin_name) if bootstrap_func else '')
    acg.write_to_file(output_file + '.cpp' if output_file else None, content)


if __name__ == '__main__':
    main()
