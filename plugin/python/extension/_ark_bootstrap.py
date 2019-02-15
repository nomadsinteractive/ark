"""
Ark module Finder and Loader.
"""


class ArkModuleLoader:

    def __init__(self, bootstrap, source, path, package, name, filepath):
        self._bootstrap = bootstrap
        self._source = source
        self._path = path
        self._package = package
        self._name = name
        self._filepath = filepath

    def create_module(self, spec):
        return None

    def exec_module(self, module):
        module.__package__ = self._package
        if self._path:
            module.__path__ = [self._path]
        if self._source:
            exec(compile(self._source, self._filepath, 'exec'), module.__dict__)
        module.__name__ = self._name
        module.__file__ = self._filepath


class ArkModuleFinder:

    ASSET_PROTOCOL = 'asset:///'

    def __init__(self, ark, bootstrap, util, ark_asset_loader_type, path):
        self._ark = ark
        self._bootstrap = bootstrap
        self._util = util
        self._ark_asset_loader_type = ark_asset_loader_type
        self._path = path
        self._asset_resource_cache = {}

    def find_spec(self, fullname, path, target=None):
        paths = [i for i in fullname.split('.') if i]
        for i in self._path:
            if i.startswith(self.ASSET_PROTOCOL):
                filepath = '/'.join([j for j in i[len(self.ASSET_PROTOCOL):].split('/') if j] + paths)
                spec = self.find_asset_spec(fullname, filepath)
                if spec:
                    return spec
            else:
                filepath = self._path_join(i, self._path_join(*paths))
                spec = self.find_file_spec(fullname, filepath)
                if spec:
                    return spec

        return None

    def find_asset_spec(self, fullname, filepath):
        module_path, module_name = self._parse_resource_path(fullname, filepath)
        asset = self._get_asset_resource(module_path)
        if asset:
            source = asset.get_string(module_name + '.py')
            package = fullname
            if not source:
                source = asset.get_string(module_name + '/__init__.py')
                if source is not None:
                    path = self.ASSET_PROTOCOL + filepath
                    filepath = asset.get_real_path(module_name + '/__init__.py')
                    return self._create_module_spec(fullname, source, path, package, filepath)
            else:
                package = '.'.join(fullname.split('.')[0:-1])
                filepath = asset.get_real_path(module_name + '.py')
                return self._create_module_spec(fullname, source, None, package, filepath)
        return None

    def find_file_spec(self, fullname, filepath):
        source = self._load_file(filepath + '.py')
        package = fullname
        if not source:
            if self._ark.is_directory(filepath):
                source = self._load_file(self._path_join(filepath, '__init__.py'))
                if source is not None:
                    return self._create_module_spec(fullname, source, filepath, package, filepath)
        else:
            package = '.'.join(fullname.split('.')[0:-1])
            return self._create_module_spec(fullname, source, None, package, filepath + '.py')
        return None

    def _create_module_spec(self, fullname, source, path, package, filepath):
        loader = self._ark_asset_loader_type(self._bootstrap, source, path, package, fullname, filepath)
        spec = self._bootstrap.ModuleSpec(fullname, loader, origin=filepath, is_package=bool(path))
        if filepath:
            spec.has_location = True
        return spec

    def _load_file(self, filepath):
        if self._ark.is_file(filepath):
            return self._ark.load_file(filepath, 'rt')
        return None

    def _path_join(self, *paths):
        sep = self._ark.dir_separator()
        return sep.join(i.rstrip(sep) for i in paths)

    def _parse_resource_path(self, fullname, filepath):
        return filepath[:-len(fullname) - 1], fullname.replace('.', '/')

    def _get_asset_resource(self, asset_path):
        if asset_path in self._asset_resource_cache:
            return self._asset_resource_cache[asset_path]
        asset = self._ark.get_asset_resource(asset_path)
        self._asset_resource_cache[asset_path] = asset
        return asset


def _install(sys_module, _imp_module):

    _bootstrap = _imp_module.init_frozen('_frozen_importlib_org')
    _util = _imp_module.init_frozen('_frozen_importlib_external')
    _ark = _imp_module.create_builtin(_bootstrap.ModuleSpec('ark', None))

    __name__ = _bootstrap.__name__
    __doc__ = _bootstrap.__doc__
    globals().update(_bootstrap.__dict__)
    _bootstrap._install(sys_module, _imp_module)
    globals().update(_bootstrap.__dict__)
    sys_module.meta_path.append(ArkModuleFinder(_ark, _bootstrap, _util, ArkModuleLoader, _ark.path))
