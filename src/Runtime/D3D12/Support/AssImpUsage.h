#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <D3D/SceneObject.h>

#include <Util/HRException.h>

#include <vector>


namespace C_AssImp
{
	void Import(const std::string& filePath, std::list<SceneObject>& objectList);
	void CopyNodesWithMeshes(std::list<SceneObject>& objectList, const aiScene& scene, aiNode& node, SceneObject& targetParent, glm::mat4 parentTransform = glm::mat4(1.0f));
	void CopyMeshes(const aiScene& scene, aiNode& node, SceneObject& obectToAddMeshTo);
}
