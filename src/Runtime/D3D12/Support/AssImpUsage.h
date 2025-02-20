#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <D3D/SceneObject.h>

#include <Util/HRException.h>


namespace C_AssImp
{
	void Import(const std::string& filePath);
	void CopyNodesWithMeshes(aiNode node, SceneObject* targetParent, glm::mat4 parentTransform = glm::mat4(1.0f));
	void CopyMeshes(aiNode node, SceneObject obectToAddMeshTo);
}
