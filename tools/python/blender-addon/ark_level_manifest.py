import bpy
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


bl_info = {
    "name": "Ark Level Manifest",
    "author": "Nomads Interactive",
    "version": (1, 0, 0),
    "blender": (2, 81, 6),
    "location": "File > Export > Ark Level Manifest",
    "category": "Import-Export",
    "description": "Export scene object's layout and location informations into generic file(json, yaml, xml)",
    "warning": ""
}


def to_json_field_value(name, value, indent):
    return '%s"%s": %s' % ('\t' * indent, name, value)


class ArkScene:
    def __init__(self, collections):
        self._libraries = list(filter(None, [i.library for i in collections]))
        self._library_collections = [i for i in collections if i.library]
        self._layers = [ArkLayer(i.objects) for i in collections if not i.library]

    def to_json(self, indent):
        lines = [i.to_json(indent) for i in self._layers]
        return ',\n'.join(lines)


class ArkLayer:
    def __init__(self, objects):
        self._render_objects = [ArkRenderObject(i) for i in objects]

    def to_json(self, indent):
        lines = [i.to_json(indent) for i in self._render_objects]
        return ',\n'.join(lines)


class ArkRenderObject:
    def __init__(self, obj):
        self._position = obj.location
        self._scale = obj.scale
        self._rotation = obj.rotation_quaternion
        self._instance_from = obj.instance_collection

    def to_json(self, indent):
        lines = [
            to_json_field_value('position', list(self._position), indent + 1),
            to_json_field_value('scale', list(self._scale), indent + 1),
            to_json_field_value('rotation', list(self._rotation), indent + 1)
        ]
        return '{\n%s\n}' % ',\n'.join(lines)


def generate_level_manifest(context, filepath, opt_name):
    scene = ArkScene(bpy.data.collections)

    with open(filepath, 'wt', encoding='utf-8') as fp:
        fp.write(scene.to_json(0))

    return {'FINISHED'}


def on_file_selector_data_format_update(self, context):
    ArkLevelManifestExporter.filename_ext = '.' + self.data_format.split('_')[-1].lower()


class ArkLevelManifestExporter(Operator, ExportHelper):
    bl_idname = "ark_level_manifest.export"
    bl_label = "Export Ark Level Manifest"

    filename_ext = ".json"

    filter_glob: StringProperty(default="*.json;*.yaml;*.xml", options={'HIDDEN'}, maxlen=255)

    data_format: EnumProperty(
        name="Data Format",
        description="Choose the data format",
        items=(('OPT_JSON', "JSON", "JavaScript Object Notation"),
               ('OPT_YAML', "YAML", "eXtensible Markup Language"),
               ('OPT_XML', "XML", "eXtensible Markup Language")),
        default='OPT_JSON',
        update=on_file_selector_data_format_update
    )

    def execute(self, context):
        return generate_level_manifest(context, self.filepath, self.data_format)


def menu_func_export_button(self, context):
    self.layout.operator(ArkLevelManifestExporter.bl_idname, text="ArkLevelManifest")


def register():
    bpy.utils.register_class(ArkLevelManifestExporter)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export_button)


def unregister():
    bpy.utils.unregister_class(ArkLevelManifestExporter)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_button)


if __name__ == "__main__":
    register()
