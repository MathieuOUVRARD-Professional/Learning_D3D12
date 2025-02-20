#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SceneObject
{
	public:
		inline void addChild(SceneObject* newChild)
		{
			m_childrens.push_back(newChild);
		}

		glm::mat4 m_transform = glm::mat4(1.0f);

		SceneObject* m_parent = nullptr;
		std::vector<SceneObject*> m_childrens;
	private:
};