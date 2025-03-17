#include<Support/Transform.h>

MyTransform::MyTransform(glm::vec3 position, glm::vec3 rotationEuler, glm::mat4 rotationMat, glm::vec3 scale)
{
	m_position = position;
	m_rotationEuler = rotationEuler;
	m_rotationMat = rotationMat;
	m_scale = scale;

	m_matrix = glm::translate(glm::mat4(1.0f), m_position);
	m_matrix *= m_rotationMat;
	m_matrix = glm::scale(m_matrix, m_scale);
}