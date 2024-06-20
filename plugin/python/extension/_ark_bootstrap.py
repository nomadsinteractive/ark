"""
Ark module Finder and Loader.
"""


class ArkModuleLoader:

    def __init__(self, bootstrap, source, path, package, name, filepath, ark_is_publishing_build):
        self._bootstrap = bootstrap
        self._source = source
        self._path = path
        self._package = package
        self._name = name
        self._filepath = filepath
        self._module_executor = self._exec_module_release if ark_is_publishing_build else self._exec_module_debug

    def create_module(self, spec):
        return None

    def exec_module(self, module):
        module.__package__ = self._package
        if self._path:
            module.__path__ = [self._path]
        if self._source:
            self._module_executor(self._bootstrap, self._source, self._filepath, module.__dict__)
        module.__name__ = self._name
        module.__file__ = self._filepath

    @staticmethod
    def _exec_module_debug(bootstrap, source, filepath, module_dict):
        exec(compile(source, filepath, 'exec'), module_dict)

    @staticmethod
    def _exec_module_release(bootstrap, source, filepath, module_dict):
        bootstrap._call_with_frames_removed(exec, source, module_dict)


class ArkModuleFinder:

    PROTOCOL_ASSET = 'asset://'
    PROTOCOL_FILE = 'file://'

    def __init__(self, ark, frozen_importlib, frozon_importlib_external, ark_asset_loader_type, path: list[str], sys_module):
        self._ark = ark
        self._bootstrap = frozen_importlib
        self._util = frozon_importlib_external
        self._ark_asset_loader_type = ark_asset_loader_type
        self._path = path
        self._sys_module = sys_module
        self._asset_bundle_cache = {}

    def find_spec(self, fullname, path, target=None):
        paths = [i for i in fullname.split('.') if i]
        for i in self._path:
            if i.startswith(self.PROTOCOL_ASSET):
                filepath = '/'.join([j for j in i[len(self.PROTOCOL_ASSET):].split('/') if j] + paths)
                spec = self.find_asset_spec(fullname, filepath)
                if spec:
                    return spec
            elif i.startswith(self.PROTOCOL_FILE):
                path = (path or self._sys_module.path) + [i[len(self.PROTOCOL_FILE):]]
                return self._util.PathFinder.find_spec(fullname, path, target)
            else:
                self._ark.logw('None-URL based protocol "%s" is no longer supported', i)

        return None

    def find_asset_spec(self, fullname: str, filepath: str):
        module_path, module_name = self._parse_resource_path(fullname, filepath)
        asset = self._load_asset_bundle(module_path)
        if asset:
            source = asset.get_string(f'{module_name}.py')
            package = fullname
            if not source:
                source = asset.get_string(f'{module_name}/__init__.py')
                if source is not None:
                    path = self.PROTOCOL_ASSET + filepath
                    filepath = asset.get_real_path(f'{module_name}/__init__.py')
                    return self._create_module_spec(fullname, source, path, package, filepath)
            else:
                package = '.'.join(fullname.split('.')[0:-1])
                filepath = asset.get_real_path(module_name + '.py')
                return self._create_module_spec(fullname, source, None, package, filepath)
        return None

    def _create_module_spec(self, fullname, source, path, package, filepath):
        loader = self._ark_asset_loader_type(self._bootstrap, source, path, package, fullname, filepath, self._ark.build_type() == self._ark.Enum.BUILD_TYPE_PUBLISHED)
        spec = self._bootstrap.ModuleSpec(fullname, loader, origin=filepath, is_package=bool(path))
        spec.has_location = bool(filepath)
        return spec

    def _path_join(self, *paths):
        sep = self._ark.dir_separator()
        return sep.join(i.rstrip(sep) for i in paths)

    def _parse_resource_path(self, fullname, filepath):
        return filepath[:-len(fullname) - 1], fullname.replace('.', '/')

    def _load_asset_bundle(self, asset_path):
        if asset_path in self._asset_bundle_cache:
            return self._asset_bundle_cache[asset_path]
        asset = self._ark.load_asset_bundle(asset_path)
        self._asset_bundle_cache[asset_path] = asset
        return asset


def _install(sys_module, _imp_module):

    _frozen_importlib = _imp_module.init_frozen('_frozen_importlib_org')
    _ark = _imp_module.create_builtin(_frozen_importlib.ModuleSpec('ark', None))

    __name__ = _frozen_importlib.__name__
    __doc__ = _frozen_importlib.__doc__
    _frozen_importlib._install(sys_module, _imp_module)
    globals().update(_frozen_importlib.__dict__)

    _frozen_importlib_external = _imp_module.init_frozen('_frozen_importlib_external')
    sys_module.meta_path.append(ArkModuleFinder(_ark, _frozen_importlib, _frozen_importlib_external, ArkModuleLoader, _ark.path, sys_module))
