#include "renderer/base/shader_data_type.h"

namespace ark {

ShaderDataType::ShaderDataType()
	: _component_type(TYPE_NONE), _number_of_components(0) {
}

ShaderDataType::ShaderDataType(ComponentType componentType, uint32_t numberOfComponents)
	: _component_type(componentType), _number_of_components(numberOfComponents) {
}

ShaderDataType::ComponentType ShaderDataType::componentType() const
{
	return _component_type;
}

uint32_t ShaderDataType::size() const
{
	return sizeOfComponent() * numberOfComponent();
}

uint32_t ShaderDataType::sizeOfComponent() const
{
	switch (_component_type) {
		case TYPE_NONE:
			return 0;
		case TYPE_BYTE:
	    case TYPE_UBYTE:
			return 1;
		case TYPE_FLOAT:
		case TYPE_INTEGER:
			return 4;
		case TYPE_USHORT:
			return 2;
		default:
			break;
	}
	FATAL("Unknow component type: %d", _component_type);
	return 0;
}

uint32_t ShaderDataType::numberOfComponent() const
{
	return _number_of_components;
}

}
