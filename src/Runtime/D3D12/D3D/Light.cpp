#include <D3D/Light.h>

Light Light::Directional(glm::vec3 direction, float intensity, glm::vec3 color)
{
	Light directional = Light();

	directional.m_type = 0.0f;
	directional.m_direction = direction;
	directional.m_intensity = intensity;
	directional.m_color = color;

	return directional;
}

Light Light::Point(glm::vec3 position, float intensity, float radius, glm::vec3 color)
{
	Light point = Light();

	point.m_type = 1.0f;
	point.m_position = position;
	point.m_intensity = intensity;
	point.m_radius = radius;
	point.m_color = color;

	return point;
}

Light Light::Spot(glm::vec3 position, glm::vec3 direction, float intensity, float radius, float innerAngle, float outerAngle, glm::vec3 color)
{
	Light spot = Light();

	spot.m_type = 2.0f;
	spot.m_position = position;
	spot.m_direction = direction;
	spot.m_intensity = intensity;
	spot.m_radius = radius;
	spot.m_innerAngle = innerAngle;
	spot.m_outerAngle = outerAngle;
	spot.m_color = color;

	return spot;
}

void Light::SendShaderParams(ID3D12GraphicsCommandList* cmdList, int bufferSlot)
{
	LightData data;

	data.type = m_type;

	data.position = m_position;
	data.direction = m_direction;

	data.intensity = m_intensity;
	data.radius = m_radius;
	data.innerAngle = m_innerAngle;
	data.outerAngle = m_outerAngle;

	data.color = m_color;

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 16, &m_color, 0);
}
