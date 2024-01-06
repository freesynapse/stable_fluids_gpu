#pragma once

#include <glm/glm.hpp>
#include <synapse/SynapseCore/Renderer/Buffers/Framebuffer.hpp>
using namespace Syn;

//
#define DENSITY_FIELD       0
#define PRESSURE_FIELD      1
#define CURL_FIELD          2

#define FIELD_COUNT         3


//
class FieldFBO : public FramebufferBase
{
public:
		
	FieldFBO(const ColorFormat& _format=ColorFormat::RGBA32F, 
			 const glm::ivec2& _size=glm::ivec2(0),
			 const std::string& _name="");
	// ~FieldFBO() { printf("deleting field '%s'\n", m_name.c_str()); }
	~FieldFBO() = default;

	virtual void resize(const glm::ivec2& _size) override;

	// Reads the texure into _buffer.
	void readFieldData(void *_buffer);
	uint32_t fieldSizeFloats() { return (m_size.x * m_size.y * getPixelFmtChannels(m_format)); }
	uint32_t fieldSize() { return (m_size.x * m_size.y); }

	// Extract min and max components from field
	std::pair<glm::vec4, glm::vec4> range(int _channels=2);

};


// aliases for comprehension
static inline Ref<FieldFBO> ScalarField(const glm::ivec2 &_dim, const std::string &_name) 
{ 
	std::string name = strcmp(_name.c_str(), "") == 0 ? "scalar_field" : _name;
	return std::make_shared<FieldFBO>(ColorFormat::RGBA32F, _dim, name);
}

//
static inline Ref<FieldFBO> VectorField(const glm::ivec2 &_dim, const std::string &_name)
{ 
	std::string name = strcmp(_name.c_str(), "") == 0 ? "vector_field" : _name;
	return std::make_shared<FieldFBO>(ColorFormat::RGBA32F, _dim, name);
}

