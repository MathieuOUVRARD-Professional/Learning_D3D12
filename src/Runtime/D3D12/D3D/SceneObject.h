#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <D3D/Mesh.h>
#include <string>

class SceneObject
{
	public:
		inline void SetMesh(Mesh& mesh)
		{
			m_mesh = mesh;
		}

		inline void AddChild(SceneObject* newChild)
		{
			m_childrens.push_back(newChild);
		}

		std::string m_name;
		glm::mat4 m_transform = glm::mat4(1.0f);

		SceneObject* m_parent = nullptr;
		std::vector<SceneObject*> m_childrens;

		Mesh m_mesh;
	private:
};