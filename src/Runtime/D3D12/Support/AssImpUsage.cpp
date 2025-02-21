#include <Support/AssImpUsage.h>
#include <iostream>

void C_AssImp::Import(const std::string& filePath, std::list<SceneObject>& objectList)
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
	if (!scene)
	{
		D3EZ::EzException("C_AssImp::Import", 15, "importer.ReadFile()", "scene is empty !");
	}

	SceneObject mainObject = SceneObject();
	mainObject.m_name = "Sponza";

	objectList.emplace_back(mainObject);

	CopyNodesWithMeshes(objectList, *scene, *scene->mRootNode, objectList.front());
}

void C_AssImp::CopyNodesWithMeshes(std::list<SceneObject>& objectList, const aiScene& scene, aiNode& node, SceneObject& targetParent, glm::mat4 parentTransform)
{
	SceneObject* parent;
	glm::mat4 newTransform = glm::mat4(1.0f);

	//If node has meshes, create a new scene object for it
	if (node.mNumMeshes > 0) {
		SceneObject newObject = SceneObject();

		newObject.m_name = node.mName.C_Str();
		newObject.m_parent = &targetParent;
		newObject.m_transform = parentTransform;

		SceneObject* tmp = &targetParent;

		objectList.emplace_back(newObject);
		targetParent.AddChild(&objectList.back());

		// copy the meshes
		C_AssImp::CopyMeshes(scene, node, objectList.back());

		//The new object is the parent for all child nodes
		parent = &objectList.back();
	}
	else {
		// if no meshes, skip the node, but keep its transformation
		parent = &targetParent;
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
		C_AssImp::CopyNodesWithMeshes(objectList, scene, *node.mChildren[child], *parent, newTransform);
	}
}

void C_AssImp::CopyMeshes(const aiScene& scene, aiNode& node, SceneObject& objectToAddMeshTo)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::cout << "Node: " << node.mName.C_Str() << "\r\nParent node: " << node.mParent->mName.C_Str() << "\r\nCopying " << node.mNumMeshes << " meshes\r\n";
	if (objectToAddMeshTo.m_parent != 0x0000000000000000)
	{
		std::cout << "Parent obj: " << objectToAddMeshTo.m_parent->m_name.c_str();
	}
	std::cout << "\r\n--------------- \r\n";

	for (int i = 0; i < node.mNumMeshes; i++)
	{
		aiMesh* mesh = scene.mMeshes[node.mMeshes[i]];
		std::cout << "Mesh: " << mesh->mName.C_Str() << " | " << mesh->mNumFaces << " faces\r\n";

		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex vertex{};
			vertex.x = mesh->mVertices[i].x;
			vertex.y = mesh->mVertices[i].y;
			vertex.z = mesh->mVertices[i].z;

			vertex.u = mesh->mTextureCoords[0][i].x;
			vertex.v = mesh->mTextureCoords[0][i].y;			

			vertex.nX = mesh->mNormals[i].x;
			vertex.nY = mesh->mNormals[i].y;
			vertex.nZ = mesh->mNormals[i].z;

			vertices.push_back(vertex);
		}

		for (int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace face = mesh->mFaces[j];

			for (int k = 0; k < face.mNumIndices; k++)
			{
				indices.push_back(face.mIndices[k]);
			}
		}
		objectToAddMeshTo.SetVertices(vertices);
		objectToAddMeshTo.SetIndices(indices);
	}
	std::cout << "---------------\r\n\r\n";
}