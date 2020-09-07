from typing import Union

import bpy
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator
from mathutils import Quaternion, Vector


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


class XmlWriter:
    def __init__(self, indent=0):
        self._body_lines = []
        self._element_lines = []
        self._element_name = None
        self._indent = indent

    def begin_element(self, name):
        self._element_name = name

    def write_property(self, name: str, value: Union[int, float, tuple, Quaternion, Vector]):
        if type(value) in (list, Quaternion, Vector):
            value = tuple(value)
        self._element_lines.append(to_xml_attr_value(name, value))

    def end_element(self):
        self._body_lines.append('%s<%s %s/>' % (_INDENT_BLOCK * self._indent, self._element_name, ' '.join(self._element_lines)))
        self._element_lines.clear()

    def to_str(self) -> str:
        return '\n'.join(self._body_lines)


class ArkScene:
    def __init__(self, layer_collection):
        self._collections = bpy.data.collections
        instance_collections = set(filter(None, (i.instance_collection for i in bpy.data.objects)))
        self._libraries = [ArkInstanceCollection(i, j) for i, j in enumerate(instance_collections)]
        collections_not_excluded = [i.collection for i in layer_collection.children if not i.exclude]
        self._layers = [ArkLayer(self, i) for i in collections_not_excluded if not i.library]

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


class ArkInstanceCollection:
    def __init__(self, _id, collection):
        self._id = _id
        self._name = '%s/%s' % (collection.library.name, collection.name) if collection.library else collection.name
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
        xml_writer = XmlWriter(indent + 1)
        for i in self._render_objects:
            i.write(xml_writer)
        lines.append(xml_writer.to_str())
        lines.append('%s</layer>' % (_INDENT_BLOCK * indent))
        return '\n'.join(str(i) for i in lines)


class ArkRenderObject:
    def __init__(self, scene: ArkScene, obj):
        self._object = obj
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

    def write(self, writer):
        writer.begin_element('render-object')

        if self._class:
            writer.write_property('name', self._object.name)
            writer.write_property('class', self._class)
            if self._object.type == 'CAMERA':
                writer.write_property('fov', self._object.data.angle)
                writer.write_property('clip-near', self._object.data.clip_start)
                writer.write_property('clip-far', self._object.data.clip_end)

        writer.write_property('position', self._position)
        writer.write_property('scale', self._scale)
        writer.write_property('rotation', self._rotation)
        if self._instance_of:
            writer.write_property('instance-of', self._instance_of.id)

        writer.end_element()
        return writer.to_str()


def generate_level_manifest(context, filepath, opt_name):

    view_layers = sum([i.view_layers.values() for i in bpy.data.scenes], [])
    if view_layers:
        scene = ArkScene(view_layers[0].layer_collection)

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
