#include <Support/AssImpUsage.h>
#include <iostream>

void C_AssImp::Import(const std::string& filePath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(
		filePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);
	if (scene == nullptr)
	{
		D3EZ::EzException("C_AssImp::Import", 15, "importer.ReadFile()", "scene == nullptr");
	}

	SceneObject mainObject = SceneObject();

	CopyNodesWithMeshes(*scene->mRootNode, &mainObject);
}

void C_AssImp::CopyNodesWithMeshes(aiNode node, SceneObject* targetParent, glm::mat4 parentTransform)
{
	SceneObject* parent;
	glm::mat4 newTransform = glm::mat4(1.0f);

	//If node has meshes, create a new scene object for it
	if (node.mNumMeshes > 0) {
		SceneObject newObject = SceneObject();
		if (targetParent != nullptr)
		{
			targetParent->addChild(&newObject);
		}
		// copy the meshes
		C_AssImp::CopyMeshes(node, newObject);

		//The new object is the parent for all child nodes
		parent = &newObject;
	}
	else {
		// if no meshes, skip the node, but keep its transformation
		parent = targetParent;
		glm::mat4 nodeTransform = glm::mat4
		(
			node.mTransformation.a1, node.mTransformation.a2, node.mTransformation.a3, node.mTransformation.a4,
			node.mTransformation.b1, node.mTransformation.b2, node.mTransformation.b3, node.mTransformation.b4,
			node.mTransformation.c1, node.mTransformation.c2, node.mTransformation.c3, node.mTransformation.c4,
			node.mTransformation.d1, node.mTransformation.d2, node.mTransformation.d3, node.mTransformation.d4
		);

		newTransform = nodeTransform * parentTransform;
	}

	// continue for all child nodes
	for( unsigned int child = 0; child <  node.mNumChildren; child ++)
	{
		C_AssImp::CopyNodesWithMeshes(*node.mChildren[child], parent, newTransform);
	}
}

void C_AssImp::CopyMeshes(aiNode node, SceneObject obectToAddMeshTo)
{
	std::cout << "Copying Mesh: " << node.mName.C_Str() << "\r\nParent: " << node.mParent->mName.C_Str() << "\r\nChilds count: " << node.mNumChildren << "\r\n";
}