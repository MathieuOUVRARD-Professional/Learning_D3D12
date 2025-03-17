#pragma once
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class MyTransform
{
	public:

		glm::mat4 m_matrix = glm::mat4(1.0f);
		glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::mat4 m_rotationMat = glm::mat4(1.0f);
		glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

		MyTransform(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f), glm::mat4 rotationMat = glm::mat4(1.0f), glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));
		
	private:
};