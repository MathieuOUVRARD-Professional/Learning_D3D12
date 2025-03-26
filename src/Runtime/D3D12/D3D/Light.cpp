#include <D3D/Light.h>

Light Light::Directional(std::string name, glm::vec3 position, float intensity, glm::vec3 color)
{
	Light directional = Light();

	directional.m_name = name;
	directional.m_type = 0;
	directional.m_position = position;
	directional.m_direction = glm::normalize(-position);
	directional.m_intensity = intensity;
	directional.m_color = color;

	return directional;
}

Light Light::Point(std::string name, glm::vec3 position, float intensity, float radius, glm::vec3 color)
{
	Light point = Light();

	point.m_type = 1;
	point.m_position = position;
	point.m_intensity = intensity;
	point.m_radius = radius;
	point.m_color = color;

	return point;
}

Light Light::Spot(std::string name, glm::vec3 position, glm::vec3 direction, float intensity, float radius, float innerAngle, float outerAngle, glm::vec3 color)
{
	Light spot = Light();

	spot.m_name = name;
	spot.m_type = 2;
	spot.m_position = position;
	spot.m_direction = direction;
	spot.m_intensity = intensity;
	spot.m_radius = radius;
	spot.m_innerAngle = innerAngle;
	spot.m_outerAngle = outerAngle;
	spot.m_color = color;

	return spot;
}

void Light::ComputeViewProjMatrix(float ortoSize)
{

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	if (m_type == 0)
	{
		view = glm::lookAt(m_position, m_position + m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		projection = glm::ortho
		(
			-ortoSize, ortoSize, 
			-ortoSize, ortoSize, 
			-m_radius, m_radius
		);
	}
	else if (m_type == 2)
	{
		view = glm::lookAt(m_position, m_position + m_direction, glm::vec3(0.0f, -1.0f, 0.0f));
		projection = glm::perspective(m_outerAngle, 1.0f, 0.1f, m_radius);	
	}
	m_viewProjMatrix = projection * view;
}

void Light::SendShaderParams(ID3D12GraphicsCommandList* cmdList, int bufferSlot)
{
	LightData data;

	data.viewProjMatrix = m_viewProjMatrix;

	data.type = (float)m_type;

	data.position = m_position;
	data.direction = m_direction;					//Reversing direction as it's based on position

	data.intensity = m_intensity;
	data.radius = m_radius;
	data.innerAngle = glm::cos(glm::radians(m_innerAngle));
	data.outerAngle = glm::cos(glm::radians(m_outerAngle));

	data.color = m_color;
	data.shadowmapID = (float)m_shadowmapID;

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 32, &data, 0);
}

void Light::SendShaderParamsSmall(ID3D12GraphicsCommandList* cmdList, int bufferSlot)
{
	LightDataSmall data;

	data.type = (float)m_type;

	data.position = m_position;
	data.direction = m_direction;					//Reversing direction as it's based on position

	data.intensity = m_intensity;
	data.radius = m_radius;
	data.innerAngle = glm::cos(glm::radians(m_innerAngle));
	data.outerAngle = glm::cos(glm::radians(m_outerAngle));

	data.color = m_color;

	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 16, &data, 0);
}
