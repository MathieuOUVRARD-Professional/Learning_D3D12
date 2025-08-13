#pragma once

#include <D3D/Mesh.h>

#include <Support/Transform.h>

#include <string>

// SceneObject posses a non copiable member with m_mesh being a unique_ptr<Mesh>.
// As unique_ptr are uniques they're not copyable and though move semantics must be use.

// SO SCENEOBJECTS CANNOT BE COPIED BUT ONLY MOVED AND PASS BY REFERENCE

// Using unique_ptr through the whole life of the resource make so that it's never copied.
// But it's ownership can be passed so it never gets deleted as long as it's referenced 

class SceneObject
{
	public:

		// Delete copy constructor & assignment
		SceneObject(const SceneObject&) = delete;
		SceneObject& operator=(const SceneObject&) = delete;

		// Move constructor & assignement
		SceneObject(SceneObject&& other) noexcept = default;
		SceneObject& operator=(SceneObject&& other) noexcept = default;

		SceneObject();

		inline void SetMesh(std::unique_ptr<Mesh> & mesh)
		{
			m_mesh = std::move(mesh);
		}

		inline void AddChild(SceneObject* newChild)
		{
			m_childrens.emplace_back(std::move(newChild));
		}

		std::string m_name;
		MyTransform m_transform = MyTransform();

		SceneObject* m_parent = nullptr;
		std::vector<SceneObject*> m_childrens;

		std::unique_ptr<Mesh> m_mesh = nullptr;

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