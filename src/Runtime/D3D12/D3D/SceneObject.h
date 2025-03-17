#pragma once

#include <D3D/Mesh.h>

#include <Support/Transform.h>

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
		MyTransform m_transform = MyTransform();

		SceneObject* m_parent = nullptr;
		std::vector<SceneObject*> m_childrens;

		Mesh m_mesh;

		bool operator == (const SceneObject& sceneObject) const
		{
			if (m_name == sceneObject.m_name)
			{
				return true;
			}
			return false;
		}
	private:
};