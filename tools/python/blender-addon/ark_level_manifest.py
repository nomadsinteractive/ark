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
    "description": "Export scene object's layout and location informations into generic file format(json, yaml, xml)",
    "warning": ""
}


_INDENT_BLOCK = '\t'


def to_json_field_value(name, value, indent):
    return '%s"%s": %s' % (_INDENT_BLOCK * indent, name, value)


def to_xml_attr_value(name, value):
    return '%s="%s"' % (name, value)


class ArkScene:
    def __init__(self, collections):
        self._libraries = [ArkLibraryCollection(i, j) for i, j in enumerate([i for i in collections if i.library])]
        self._library_collections = [i for i in collections if i.library]
        self._layers = [ArkLayer(self, i) for i in collections if not i.library]

    def find_library(self, collection):
        for i in self._libraries:
            if i.collection is collection:
                return i
        return None

    def to_json(self, indent):
        lines = [i.to_json(indent) for i in self._layers]
        return ',\n'.join(lines)

    def to_xml(self, indent):
        lines = ['<?xml version="1.0" encoding="utf-8"?>', '%s<scene>' % (_INDENT_BLOCK * indent)]
        lines.extend(i.to_xml(indent + 1) for i in self._libraries)
        lines.extend(i.to_xml(indent + 1) for i in self._layers)
        lines.append('%s</scene>' % (_INDENT_BLOCK * indent))
        return '\n'.join(str(i) for i in lines)


class ArkLibraryCollection:
    def __init__(self, _id, collection):
        self._id = _id
        self._name = '%s/%s' % (collection.library.name, collection.name)
        self._collection = collection

    @property
    def id(self):
        return self._id

    @property
    def name(self):
        return self._name

    @property
    def collection(self):
        return self._collection

    def to_xml(self, indent):
        return '%s<library id="%d" name="%s"/>' % (_INDENT_BLOCK * indent, self._id, self._name)


class ArkLayer:
    def __init__(self, scene: ArkScene, collection):
        self._name = collection.name
        self._render_objects = [ArkRenderObject(scene, i) for i in collection.objects]

    def to_json(self, indent):
        lines = [i.to_json(indent) for i in self._render_objects]
        return ',\n'.join(lines)

    def to_xml(self, indent):
        lines = ['%s<layer name="%s">' % (_INDENT_BLOCK * indent, self._name)]
        lines.extend(i.to_xml(indent + 1) for i in self._render_objects)
        lines.append('%s</layer>' % (_INDENT_BLOCK * indent))
        return '\n'.join(str(i) for i in lines)


class ArkRenderObject:
    def __init__(self, scene: ArkScene, obj):
        self._class = obj.type if obj.type != 'EMPTY' else None
        self._position = obj.location
        self._scale = obj.scale
        if obj.rotation_mode == 'QUATERNION':
            self._rotation = obj.rotation_quaternion
        elif obj.rotation_mode == 'AXIS_ANGLE':
            self._rotation = obj.rotation_euler
        else:
            self._rotation = obj.rotation_euler.to_quaternion()
        self._instance_of = obj.instance_collection and scene.find_library(obj.instance_collection)

    def to_json(self, indent):
        lines = [to_json_field_value('class', '"%s"' % self._class, indent + 1)] if self._class else []
        lines.extend([
            to_json_field_value('position', list(self._position), indent + 1),
            to_json_field_value('scale', list(self._scale), indent + 1),
            to_json_field_value('rotation', list(self._rotation), indent + 1)
        ])
        if self._instance_of:
            lines.append(to_json_field_value('instance-of', self._instance_of.id, indent + 1))
        return '{\n%s\n}' % ',\n'.join(lines)

    def to_xml(self, indent):
        attrs = [to_xml_attr_value('class', self._class)] if self._class else []
        attrs.extend([
            to_xml_attr_value('position', tuple(self._position)),
            to_xml_attr_value('scale', tuple(self._scale)),
            to_xml_attr_value('rotation', tuple(self._rotation))
        ])
        if self._instance_of:
            attrs.append(to_xml_attr_value('instance-of', self._instance_of.id))
        return '%s<render-object %s/>' % (_INDENT_BLOCK * indent, ' '.join(attrs))


def generate_level_manifest(context, filepath, opt_name):
    scene = ArkScene(bpy.data.collections)

    content = scene.to_xml(0) if opt_name == 'OPT_XML' else scene.to_json(0)
    with open(filepath, 'wt', encoding='utf-8') as fp:
        fp.write(content)

    return {'FINISHED'}


def on_file_selector_data_format_update(self, context):
    ArkLevelManifestExporter.filename_ext = '.' + self.data_format.split('_')[-1].lower()


class ArkLevelManifestExporter(Operator, ExportHelper):
    bl_idname = "ark_level_manifest.export"
    bl_label = "Export Ark Level Manifest"

    filename_ext = ".xml"

    filter_glob: StringProperty(default="*.json;*.yaml;*.xml", options={'HIDDEN'}, maxlen=255)

    data_format: EnumProperty(
        name="Data Format",
        description="Choose the data format",
        items=(('OPT_JSON', "JSON", "JavaScript Object Notation"),
               ('OPT_YAML', "YAML", "YAML Ain't Markup Language"),
               ('OPT_XML', "XML", "eXtensible Markup Language")),
        default='OPT_XML',
        update=on_file_selector_data_format_update
    )

    def execute(self, context):
        return generate_level_manifest(context, self.filepath, self.data_format)


def menu_func_export_button(self, context):
    self.layout.operator(ArkLevelManifestExporter.bl_idname, text="Ark Level Manifest")


def register():
    bpy.utils.register_class(ArkLevelManifestExporter)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export_button)


def unregister():
    bpy.utils.unregister_class(ArkLevelManifestExporter)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_button)


if __name__ == "__main__":
    register()
