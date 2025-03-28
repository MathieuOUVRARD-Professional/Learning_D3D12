#include<Support/Camera.h>

Camera::Camera(int width, int height, glm::vec3 position)
{
	this->m_width = width;
	this->m_height = height;
	this->m_position = position;
}

void Camera::UpdateWindowSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane)
{
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	view = glm::lookAt(m_position, m_position + m_orientation, m_up);
	projection = glm::perspective(glm::radians(FOVdeg), ((float)m_width / (float)m_height), nearPlane, farPlane);
	
	m_viewMatrix = view;
	m_projMatrix = projection;

	m_viewProjMatrix = projection * view;
}

// Legacy for Pyramid and cube
void Camera::SendShaderParams(ID3D12GraphicsCommandList* cmdList, int bufferSlot, glm::mat4 modelMatrix)
{
	struct Matrices
	{
		glm::mat4 viewProj = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
	};
	Matrices matrices;
	matrices.viewProj = m_viewProjMatrix;
	matrices.model = modelMatrix;
	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 32, &matrices, 0);
}

//Used for PBR 
void Camera::SendShaderParams(ID3D12GraphicsCommandList* cmdList, int bufferSlot)
{
	struct CameraData
	{
		glm::mat4 viewProj = glm::mat4(1.0f);
		glm::vec4 position = glm::vec4(0.0f);
	};
	CameraData cameraData;
	cameraData.viewProj = m_viewProjMatrix;
	cameraData.position = glm::vec4(m_position, 1.0f);
	cmdList->SetGraphicsRoot32BitConstants(bufferSlot, 20, &cameraData, 0);
}

void Camera::Inputs()
{
	if (m_controlable)
	{
		if (GetAsyncKeyState(VK_RBUTTON)) // RCLICK
		{
			if (GetAsyncKeyState(0x5A)) // Z
			{
				m_position += m_speed * m_orientation;
			}
			if (GetAsyncKeyState(0x51)) // Q
			{
				m_position += m_speed * -glm::normalize(glm::cross(m_orientation, m_up));
			}
			if (GetAsyncKeyState(0x53)) // S
			{
				m_position += m_speed * -m_orientation;
			}
			if (GetAsyncKeyState(0x44)) // D
			{
				m_position += m_speed * glm::normalize(glm::cross(m_orientation, m_up));
			}
			if (GetAsyncKeyState(VK_SPACE)) // SPACE
			{
				m_position += m_speed * m_up;
			}
			if (GetAsyncKeyState(VK_LCONTROL)) // LSHIFT
			{
				m_position += m_speed * -m_up;
			}
			if (GetAsyncKeyState(VK_LSHIFT)) // LSHIFT
			{
				m_speed = m_defaultSpeed * 2.5f;
			}
			else // !LSHIFT
			{
				m_speed = m_defaultSpeed;
			}

			BOOL ret = ShowCursor(false);
			while (ret > 0)
			{
				ret = ShowCursor(false);
			}

			if (m_firstClick)
			{
				SetCursorPos(m_width / 2, m_height / 2);
				m_firstClick = false;
			}

			POINT mousePos;
			GetCursorPos(&mousePos);

			float rotX = m_sensitivity * (float)(mousePos.y - (m_height / 2)) / m_height;
			float roty = m_sensitivity * (float)(mousePos.x - (m_width / 2)) / m_width;

			glm::vec3 newOrientation = glm::rotate(m_orientation, glm::radians(-rotX), glm::normalize(glm::cross(m_orientation, m_up)));
			if (!((glm::angle(newOrientation, m_up) <= glm::radians(5.0f)) || glm::angle(newOrientation, -m_up) <= glm::radians(5.0f)))
			{
				m_orientation = newOrientation;
			}
			m_orientation = glm::rotate(m_orientation, glm::radians(-roty), m_up);
			SetCursorPos(m_width / 2, m_height / 2);

		}
		else
		{
			BOOL ret = ShowCursor(true);
			while (ret < 0)
			{
				ret = ShowCursor(true);
			}
			m_firstClick = true;
		}
	}	
}

void Camera::ControlMode(bool hasControl)
{
	m_controlable = hasControl;
}

