#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <D3D/Material.h>
#include <Support/ObjectList.h>
#include <Util/HRException.h>

#include <vector>
#include <iostream>
#include <string>


namespace C_AssImp
{
	void Import(const std::string& filePath, ObjectList& objectList);
	void ProcessMeshesNodes(ObjectList& objectList, const aiScene& scene, aiNode& node, SceneObject& targetParent, glm::mat4 parentTransform = glm::mat4(1.0f));
	void LoadMeshes(const aiScene& scene, aiNode& node, ObjectList& obectToAddMeshTo);
	void ProcessMaterials(ObjectList& objectList, const aiScene& scene, std::string sceneDirectory);

	enum
	{
		DEFAULT_WHITE_TEXTURE = 0,
		DEFAULT_NORMAL_TEXTURE = 1
	};
}
