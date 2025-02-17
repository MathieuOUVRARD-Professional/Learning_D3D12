#pragma once

#include <Support/Window.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

class Camera
{
	public:
		glm::vec3 m_position;
		glm::vec3 m_orientation = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 m_cameraMatrix = glm::mat4(1.0f);

		bool m_firstClick = true;

		int m_width;
		int m_height;

		float m_speed = 0.1f;
		float m_sensitivity = 50.0f;

		Camera(int width, int height, glm::vec3 position);

		void UpdateWindowSize(int width, int height);
		void Matrix(float FOVdeg, float nearPlane, float farPlane);
		void UpdateMatrix(ID3D12GraphicsCommandList* cmdList, int bufferSlot, glm::vec3 modelPosition);
		void Inputs();

	private:
		float defaultSpeed = 0.05f;
};