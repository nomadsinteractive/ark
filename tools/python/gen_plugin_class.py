#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
import sys
import re
import acg
from acg import HeaderPattern

ANNOTATION_PATTERN = r'(?:\s*(?://)?\s*\[\[[^]]+\]\])*'
BUILDER_PATTERN = re.compile(r'\[\[plugin::(builder|resource-loader)(?:\("([\w\d\-_]+)"\))?\]\]\s+class\s+([\w\d_]+)\s+:\s+public\s+Builder<([^{]+)>\s+\{')
DICTIONARY_PATTERN = re.compile(r'\[\[plugin::(builder|resource-loader)::by-value(?:\("([\w\d\-_]+)"\))?\]\]\s+class\s+([\w\d_]+)\s+:\s+public\s+Builder<([^{]+)>\s+\{')
FUNCTION_PATTERN = re.compile(r'\[\[plugin::function\("([\w\d\-_]+)"\)\]\]\s*%s\s*static\s+(?:ARK_API\s+)?([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;' % ANNOTATION_PATTERN)
STYLE_PATTERN = re.compile(r'\[\[plugin::style\("([\w\d\-_]+)"\)\]\]\s+class\s+([\w\d_]+)\s+:\s+public\s+Builder<([^{]+)>\s+\{')

INDENT = '\n    '
REF_BUILDER_TEMPLATE = '''BeanFactory::Factory ${plugin_name}::createBeanFactory(const BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById)
{
    BeanFactory::Factory refBeanFactory(beanFactory.references(), documentById);
    %s
    return refBeanFactory;
}'''
RES_BUILDER_TEMPLATE = '''BeanFactory::Factory ${plugin_name}::createResourceLoader(const BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    BeanFactory::Factory resBeanFactory(beanFactory.references(), documentById);
    %s
    return resBeanFactory;
}'''
FUNC_BUILDER_TEMPLATE = '''Library ${plugin_name}::createLibrary()
{
    Library library;
    %s
    return library;
}'''

_class_members = {}


def declare_namespaces(namespaces, source):
    return '\n'.join(['namespace %s {' % i for i in namespaces]) + '\n\n' + source + '\n' +'\n'.join(['}'] * len(namespaces))


def to_member_name(name):
    return '_' + re.sub(r'[A-Z]', lambda x: '_' + x.group(0).lower(), name)


class Annotation(object):
    def __init__(self, filename, name, interface_class, implement_class, main_class, arguments):
        self._name = name
        self._filename = filename
        self._interface_class = interface_class
        self._implement_class = implement_class
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
        return '%s<%s>(%s[%s](%s)->sp<Builder<%s>> { return sp<Builder<%s>>::make<%s::%s>(%s); });' % (statement, self._interface_class, name, capture_args, builder_arguments, self._interface_class, self._interface_class, self._main_class, self._implement_class, passing_args)


class ResourceLoader(Annotation):
    def __init__(self, filename, name, interface_class, main_class, builder_name, arguments):
        super(ResourceLoader, self).__init__(filename, name, interface_class, builder_name, main_class, arguments)

    def resBuilder(self):
        return self._make_call('resBeanFactory.addBuilderFactory', 'BeanFactory& factory, const document& doc', 'createResourceLoader', RES_BUILDER_TEMPLATE)


class Builder(Annotation):
    def __init__(self, filename, name, interface_class, main_class, builder_name, arguments):
        super(Builder, self).__init__(filename, name, interface_class, builder_name, main_class, arguments)

    def refBuilder(self):
        return self._make_call('refBeanFactory.addBuilderFactory', 'BeanFactory& factory, const document& doc', 'createBeanFactory', REF_BUILDER_TEMPLATE)


class Dictionary(Annotation):
    def __init__(self, filename, name, interface_class, implement_class, main_class, arguments):
        super(Dictionary, self).__init__(filename, name, interface_class, implement_class, main_class, arguments)

    def refBuilder(self):
        return self._make_call('refBeanFactory.addDictionaryFactory', 'BeanFactory& factory, const String& value', 'createBeanFactory', REF_BUILDER_TEMPLATE)


class Function(Annotation):
    def __init__(self, filename, main_class, funcname, func, rettype, arguments):
        super(Function, self).__init__(filename, None, None, None, main_class, arguments)
        self._funcname = funcname
        self._func = func
        self._rettype = rettype
        self._argtypes = arguments

    def func(self):
        return 'library.addCallable<%s(%s)>("%s", static_cast<%s(*)(%s)>(&%s));' % (self._rettype, self._argtypes, self._funcname, self._rettype, self._argtypes, self._func)


class ResourceLoaderDictionaryNamed(Annotation):
    def __init__(self, filename, name, interface_class, main_class, builder_name, arguments):
        super(ResourceLoaderDictionaryNamed, self).__init__(filename, name, interface_class, main_class, builder_name, arguments)

    def resBuilder(self):
        return self._make_call('resBeanFactory.addDictionaryFactory', 'BeanFactory& factory, const String& value', 'createResourceLoader', RES_BUILDER_TEMPLATE)


class ResourceLoaderDictionaryNonamed(Annotation):
    def __init__(self, filename, interface_class, implement_class, main_class, arguments):
        super(ResourceLoaderDictionaryNonamed, self).__init__(filename, None, interface_class, implement_class, main_class, arguments)

    def resBuilder(self):
        func_arguments = parse_function_arguments('createResourceLoader', RES_BUILDER_TEMPLATE)
        capture_args, passing_args = self._build_arguments(parse_arguments('BeanFactory& factory, const String& value'), func_arguments)
        return 'resBeanFactory.addDictionaryFactory<%s>([%s](BeanFactory& factory, const String& value)->sp<Builder<%s>> { return sp<Builder<%s>>::make<%s::%s>(%s); });' % (self._interface_class, capture_args, self._interface_class, self._interface_class, self._main_class, self._implement_class, passing_args)


class Decorator(Annotation):
    def __init__(self, filename, interface_class, implement_class, main_class, style_name, arguments):
        super(Decorator, self).__init__(filename, style_name, interface_class, implement_class, main_class, arguments)

    def refBuilder(self):
        func_arguments = 'BeanFactory& factory, const sp<Builder<%s>>& delegate, const String& value' % self._interface_class
        lambda_arguments = parse_function_arguments('createBeanFactory', REF_BUILDER_TEMPLATE)
        capture_args, passing_args = self._build_arguments(parse_arguments(func_arguments), lambda_arguments)
        return 'refBeanFactory.addBuilderDecorator<%s>("%s", [%s](%s)->sp<Builder<%s>> { return sp<Builder<%s>>::make<%s::%s>(%s); });' % (self._interface_class, self._name, capture_args, func_arguments, self._interface_class, self._interface_class, self._main_class, self._implement_class, passing_args)


def find_main_class(content):
    m = re.search(r'class\s+(?:[\w\d]+\s+)*?([\w\d]+)(?:\s+final\s*)?(?:\s*:[^{]+|\s*){', content)
    return m.group(1) if m else None


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
    return ', '.join('const %s& %s' % (v, acg.camelName(k.split('_'))) for k, v in _class_members.items())


def get_plugin_arguments_assigment():
    return ', ' + ', '.join('%s(%s)' % (k, acg.camelName(k.split('_'))) for k, v in _class_members.items()) if _class_members else ''


def get_member_declare():
    if not _class_members:
        return ''
    getters = '\n    ' + '\n    '.join('const %s& get%s() const;' % (v, acg.toCamelName(k.split('_'))) for k, v in _class_members.items())

    return getters + '\n\nprivate:\n    ' + '\n    '.join('%s %s;' % (v, k) for k, v in _class_members.items()) + '\n'


def get_plugin_member_getter(plugin_name):
    if not _class_members:
        return ''

    return '\n\n' + '\n\n'.join('const %s& %s::get%s() const\n{\n    return %s;\n}' % (v, plugin_name, acg.toCamelName(k.split('_')), k) for k, v in _class_members.items())


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
        builder_type = x[0]
        name = x[1]
        builder_name = x[2]
        interface_class = x[3]
        arguments = [(i, j) for i, j in parse_function_arguments(builder_name, content) if not i.startswith('=')]
        if builder_type != 'resource-loader':
            result.append(Builder(filename, name, interface_class, main_class, builder_name, arguments))
        else:
            result.append(ResourceLoader(filename, name, interface_class, main_class, builder_name, arguments))

    def match_dictionary(filename, content, main_class, x):
        builder_type = x[0]
        name = x[1]
        implement_class = x[2]
        interface_class = x[3]
        arguments = parse_function_arguments(implement_class, content)
        if builder_type != 'resource-loader':
            result.append(Dictionary(filename, name, interface_class, implement_class, main_class, arguments))
        elif name:
            result.append(ResourceLoaderDictionaryNamed(filename, name, interface_class, implement_class, main_class, arguments))
        else:
            result.append(ResourceLoaderDictionaryNonamed(filename, interface_class, implement_class, main_class, arguments))

    def match_function(filename, content, main_class, x):
        funcname = x[0]
        s = x[1].split()
        func = main_class + '::' + s[-1]
        rettype = ' '.join(s[:-1])

        def remove_arg_name(arg):
            s = arg.split(' ')
            return ' '.join(i for i in s[:-1] if i) if len(s) > 1 else arg

        arguments = ', '.join(remove_arg_name(i) for i in x[2].split(','))
        result.append(Function(filename, main_class, funcname, func, rettype, arguments))

    def match_style(filename, content, main_class, x):
        style_name = x[0]
        implement_class = x[1]
        interface_class = x[2]
        arguments = parse_function_arguments(implement_class, content)
        result.append(Decorator(filename, interface_class, implement_class, main_class, style_name, arguments))

    acg.match_header_patterns(paths, True,
                              HeaderPattern(BUILDER_PATTERN, match_builder),
                              HeaderPattern(DICTIONARY_PATTERN, match_dictionary),
                              HeaderPattern(FUNCTION_PATTERN, match_function),
                              HeaderPattern(STYLE_PATTERN, match_style))
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
    plugin_name = config['plugin_name'] if 'plugin_name' in config else acg.toCamelName(file_path.split('_'))
    ark_namespace = 'ark::' if 'ark' not in config['namespace'] else ''

    ref_builder = generate_method_def('refBuilder', result, REF_BUILDER_TEMPLATE, plugin_name=plugin_name)
    ref_builder_declare = acg.format('virtual ${ns}BeanFactory::Factory createBeanFactory(const ${ns}BeanFactory& beanFactory, const ${ns}sp<${ns}Dictionary<${ns}document>>& documentById) override;', ns=ark_namespace) if ref_builder else ''

    res_builder = generate_method_def('resBuilder', result, RES_BUILDER_TEMPLATE, plugin_name=plugin_name)
    res_builder_declare = acg.format('virtual ${ns}BeanFactory::Factory createResourceLoader(const ${ns}BeanFactory& beanFactory, const ${ns}sp<${ns}Dictionary<${ns}document>>& documentById, const ${ns}sp<${ns}ResourceLoaderContext>& resourceLoaderContext) override;', ns=ark_namespace) if res_builder else ''

    func_builder = generate_method_def('func', result, FUNC_BUILDER_TEMPLATE, plugin_name=plugin_name)
    function_declare = acg.format('virtual ${ns}Library createLibrary() override;', ns=ark_namespace) if func_builder else ''

    classdeclare = acg.format('''class ${plugin_name} : public ${ns}Plugin {
public:
    ${plugin_name}(${plugin_arguments});

    ${refBuilderDeclare}
    ${resBuilderDeclare}
    ${functionDeclare}
${member_declare}
};
''', plugin_name=plugin_name, ns=ark_namespace, member_declare=get_member_declare(), plugin_arguments=get_plugin_arguments(), refBuilderDeclare=ref_builder_declare, resBuilderDeclare=res_builder_declare, functionDeclare=function_declare)
    content = acg.format('''#ifndef ${header_macro}_PLUGIN_H_
#define ${header_macro}_PLUGIN_H_

#include "core/base/plugin.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/resource_loader.h"

${classdeclare}

#endif
''', header_macro=file_path.upper(), classdeclare=declare_namespaces(config['namespace'], classdeclare))
    acg.write_to_file(output_file + '.h' if output_file else None, content)

    classdefine = acg.format('''${plugin_name}::${plugin_name}(${plugin_arguments})
    : Plugin("${name}", Plugin::PLUGIN_TYPE_${type})${plugin_arguments_assigment} {
}
${refBuilder}${resBuilder}${funcBuilder}${plugin_member_getter}''', name=config['name'], type=config['type'].upper(), plugin_name=plugin_name, plugin_arguments=get_plugin_arguments(), plugin_arguments_assigment=get_plugin_arguments_assigment(), refBuilder=ref_builder, resBuilder=res_builder, funcBuilder=func_builder, plugin_member_getter=get_plugin_member_getter(plugin_name))
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
