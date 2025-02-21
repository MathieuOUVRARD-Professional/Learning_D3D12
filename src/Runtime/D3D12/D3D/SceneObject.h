#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Util/Vertex.h>

class SceneObject
{
	public:
		inline std::vector<Vertex>& GetVertices()
		{
			return m_vertices;
		}
		inline void SetVertices(std::vector<Vertex>& vertices)
		{
			m_vertices = vertices;
		}
		inline std::vector<uint32_t>& GetIndices()
		{
			return m_indices;
		}
		inline void SetIndices(std::vector<uint32_t>& indices)
		{
			m_indices = indices;
		}

		inline void AddChild(SceneObject* newChild)
		{
			m_childrens.push_back(newChild);
		}

		std::string m_name;
		glm::mat4 m_transform = glm::mat4(1.0f);

		SceneObject* m_parent = nullptr;
		std::vector<SceneObject*> m_childrens;
	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
};