"""
Ark module Finder and Loader.
"""


class ArkModuleLoader:
    def __init__(self, bootstrap, source, code, path, package, name, filepath, is_publishing_build: bool):
        self._bootstrap = bootstrap
        self._source = source
        self._code = code
        self._path = path
        self._package = package
        self._name = name
        self._filepath = filepath
        self._module_executor = self._exec_module_release if is_publishing_build else self._exec_module_debug

    def create_module(self, spec):
        return None

    def exec_module(self, module):
        module.__package__ = self._package
        if self._path:
            module.__path__ = [self._path]
        if self._source:
            self._module_executor(self._bootstrap, self._source, self._code, self._filepath, module.__dict__)
        module.__name__ = self._name
        module.__file__ = self._filepath

    @staticmethod
    def _exec_module_debug(bootstrap, source, code, filepath, module_dict):
        exec(code or compile(source, filepath, 'exec'), module_dict)

    @staticmethod
    def _exec_module_release(bootstrap, source, code, filepath, module_dict):
        bootstrap._call_with_frames_removed(exec, code or source, module_dict)


PROTOCOL_ASSET_PREFIX = 'asset://'
PROTOCOL_FILE_PREFIX = 'file://'


class ArkModuleFinder:
    def __init__(self, ark, frozen_importlib, frozon_importlib_external, path: list[str], sys_module):
        self._ark = ark
        self._bootstrap = frozen_importlib
        self._util = frozon_importlib_external
        self._path = path
        self._sys_module = sys_module
        self._asset_bundle_cache = {}

    def find_spec(self, fullname, path, target=None):
        paths = [i for i in fullname.split('.') if i]
        for i in self._path:
            if i.startswith(PROTOCOL_ASSET_PREFIX):
                filepath = '/'.join([j for j in i[len(PROTOCOL_ASSET_PREFIX):].split('/') if j] + paths)
                if spec := self.find_asset_spec(fullname, filepath):
                    return spec
            elif i.startswith(PROTOCOL_FILE_PREFIX):
                path = (path or self._sys_module.path) + [i[len(PROTOCOL_FILE_PREFIX):]]
                if spec := self._util.PathFinder.find_spec(fullname, path, target):
                    return spec
            else:
                self._ark.logw('None-URL based protocol "%s" is no longer supported', i)

        return None

    def find_asset_spec(self, fullname: str, filepath: str):
        module_path, module_name = self._parse_resource_path(fullname, filepath)
        asset_bundle = self._load_asset_bundle(module_path)
        if asset_bundle:
            if module_spec := self._load_module_spec(fullname, filepath, asset_bundle, module_name, 'pyc'):
                return module_spec
            return self._load_module_spec(fullname, filepath, asset_bundle, module_name, 'py')
        return None

    def _load_module_spec(self, fullname: str, filepath: str, asset_bundle, module_name: str, module_suffix: str):
        source, code = self._load_source_from_asset_bundle(asset_bundle, f'{module_name}.{module_suffix}')
        package = fullname
        if not source:
            source, code = self._load_source_from_asset_bundle(asset_bundle, f'{module_name}/__init__.{module_suffix}')
            if source is not None:
                path = PROTOCOL_ASSET_PREFIX + filepath
                filepath = asset_bundle.get_real_path(f'{module_name}/__init__.{module_suffix}')
                return self._create_module_spec(fullname, source, code, path, package, filepath)
        else:
            package = '.'.join(fullname.split('.')[0:-1])
            filepath = asset_bundle.get_real_path(f'{module_name}.{module_suffix}')
            return self._create_module_spec(fullname, source, code,None, package, filepath)
        return None

    def _create_module_spec(self, fullname, source, code, path, package, filepath):
        loader = ArkModuleLoader(self._bootstrap, source, code, path, package, fullname, filepath, self._ark.build_type() == self._ark.Enum.BUILD_TYPE_PUBLISHED)
        spec = self._bootstrap.ModuleSpec(fullname, loader, origin=filepath, is_package=bool(path))
        spec.has_location = bool(filepath)
        return spec

    def _path_join(self, *paths):
        sep = self._ark.dir_separator()
        return sep.join(i.rstrip(sep) for i in paths)

    def _load_asset_bundle(self, asset_path):
        if asset_path in self._asset_bundle_cache:
            return self._asset_bundle_cache[asset_path]
        asset = self._ark.load_asset_bundle(asset_path)
        self._asset_bundle_cache[asset_path] = asset
        return asset

    @staticmethod
    def _parse_resource_path(fullname, filepath):
        return filepath[:-len(fullname) - 1], fullname.replace('.', '/')

    @staticmethod
    def _load_source_from_asset_bundle(asset_bundle, file_path: str):
        data = asset_bundle.get_byte_array(file_path)

        if data is None:
            return None, None

        if file_path.endswith('.pyc'):
            import marshal

            code = marshal.loads(data.to_bytes()[16:])
            return code, code

        return data.to_string(), None


def _install(sys_module, _imp_module):

    _frozen_importlib = _imp_module.init_frozen('_frozen_importlib_org')
    _ark = _imp_module.create_builtin(_frozen_importlib.ModuleSpec('ark', None))

    __name__ = _frozen_importlib.__name__
    __doc__ = _frozen_importlib.__doc__
    _frozen_importlib._install(sys_module, _imp_module)
    globals().update(_frozen_importlib.__dict__)

    _frozen_importlib_external = _imp_module.init_frozen('_frozen_importlib_external')
    sys_module.meta_path.append(ArkModuleFinder(_ark, _frozen_importlib, _frozen_importlib_external, _ark.path, sys_module))
