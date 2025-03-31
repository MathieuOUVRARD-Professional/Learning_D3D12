#include <Support/AssImpUsage.h>

void C_AssImp::Import(const std::string& filePath, ObjectList& objectList)
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
		aiProcess_SortByPType |
		aiProcess_FlipUVs
	);
	if (!scene)
	{
		D3EZ::EzException ezException = D3EZ::EzException("C_AssImp::Import", 15, "importer.ReadFile()", "scene is empty !");
	}
	else
	{
		size_t it = filePath.find_first_of('/', 0);
		std::string name = filePath.substr(0, it);

		objectList.m_name = name;

		SceneObject mainObject = SceneObject();
		mainObject.m_name = "OriginObject";

		objectList.GetList().emplace_back(mainObject);

		std::string folderPath = filePath.substr(0, filePath.find('/')) + "/"; 
		
		spdlog::info("LOADING MATERIALS\n");
		ProcessMaterials(objectList, *scene, folderPath);

		spdlog::info("LOADING MESHES\n");
		ProcessMeshesNodes(objectList, *scene, *scene->mRootNode, objectList.GetList().back());

		objectList.GetList().remove(mainObject);
	}
}

void C_AssImp::ProcessMeshesNodes(ObjectList& objectList, const aiScene& scene, aiNode& node, SceneObject& targetParent, glm::mat4 parentTransform)
{
	SceneObject* parent;
	glm::mat4 newTransform = glm::mat4(1.0f);

	//If node has meshes, create a new scene object for it
	if (node.mNumMeshes > 0) {
		SceneObject newObject = SceneObject();

		glm::mat4 nodeTransform = glm::make_mat4(node.mTransformation.Transpose()[0]);

		newObject.m_name = node.mName.C_Str();
		newObject.m_parent = &targetParent;
		newObject.m_transform.m_matrix = parentTransform * nodeTransform;

		SceneObject* tmp = &targetParent;

		objectList.GetList().emplace_back(newObject);
		targetParent.AddChild(&objectList.GetList().back());

		// copy the meshes
		C_AssImp::LoadMeshes(scene, node, objectList);

		//The new object is the parent for all child nodes
		parent = &objectList.GetList().back();
	}
	else {
		// if no meshes, skip the node, but keep its transformation
		parent = &targetParent;
		glm::mat4 nodeTransform = glm::make_mat4(node.mTransformation.Transpose()[0]);

		newTransform = nodeTransform * parentTransform;
	}

	// continue for all child nodes
	for( unsigned int child = 0; child <  node.mNumChildren; child ++)
	{
		C_AssImp::ProcessMeshesNodes(objectList, scene, *node.mChildren[child], *parent, newTransform);
	}
}

void C_AssImp::LoadMeshes(const aiScene& scene, aiNode& node, ObjectList& objectList)
{	
	// Node name
	std::string nodeName = node.mName.C_Str();
	spdlog::info("Node: " + nodeName);

	// Parent node name
	std::string parentNodeName = "";
	if (objectList.GetList().back().m_parent != 0x0000000000000000)
	{
		parentNodeName = node.mParent->mName.C_Str();
		spdlog::info("Parent node: " + parentNodeName);
	}

	// Num submeshes message
	std::string message = "Copying ";
	node.mNumMeshes > 1 ? message += fmt::format("{} submeshes", node.mNumMeshes) : message = message + "a mesh";
	spdlog::info(message);
	
	std::cout << "------------------------------ \r\n";

	for (unsigned int i = 0; i < node.mNumMeshes; i++)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		aiMesh* meshNode = scene.mMeshes[node.mMeshes[i]];
		nodeName = meshNode->mName.C_Str();

		message = node.mNumMeshes > 1 ? "Submesh: " : "Mesh: ";
		message += fmt::format("{0} | {1} faces", nodeName, meshNode->mNumFaces);
		spdlog::info(message);
		
		for (unsigned int j = 0; j < meshNode->mNumVertices; j++)
		{
			Vertex vertex{};
			vertex.x = meshNode->mVertices[j].x;
			vertex.y = meshNode->mVertices[j].y;
			vertex.z = meshNode->mVertices[j].z;

			vertex.u = meshNode->mTextureCoords[0][j].x;
			vertex.v = meshNode->mTextureCoords[0][j].y;

			vertex.nX = meshNode->mNormals[j].x;
			vertex.nY = meshNode->mNormals[j].y;
			vertex.nZ = meshNode->mNormals[j].z;

			vertex.tX = meshNode->mTangents[j].x;	
			vertex.tY = meshNode->mTangents[j].y;
			vertex.tZ = meshNode->mTangents[j].z;

			vertex.btX = meshNode->mBitangents[j].x;
			vertex.btY = meshNode->mBitangents[j].y;
			vertex.btZ = meshNode->mBitangents[j].z;			

			vertices.emplace_back(vertex);
		}

		for (unsigned int j = 0; j < meshNode->mNumFaces; j++)
		{
			aiFace face = meshNode->mFaces[j];

			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				indices.emplace_back(face.mIndices[k]);
			}
		}

		if (node.mNumMeshes > 1)
		{
			Mesh submesh;
			submesh.SetVertices(vertices);
			submesh.SetIndices(indices);
			submesh.SetMaterial(objectList.GetMaterials()[meshNode->mMaterialIndex]);

			objectList.GetList().back().m_mesh.AddSubmesh(submesh);
		}
		else
		{
			Mesh mesh;
			mesh.SetVertices(vertices);
			mesh.SetIndices(indices);
			mesh.SetMaterial(objectList.GetMaterials()[meshNode->mMaterialIndex]);
			objectList.GetList().back().SetMesh(mesh);
		}
	}	
	std::cout << std::endl;
}

void C_AssImp::ProcessMaterials(ObjectList& objectList, const aiScene& scene, std::string sceneDirectory)
{
	std::vector<Material> materials;
	uint32_t textureID = 0;
	
	for (unsigned int i = 0; i < scene.mNumMaterials; i++)
	{
		aiMaterial* materialNode = scene.mMaterials[i];
		aiString texturePath;

		std::vector<std::string> texturesPaths;
		std::vector<std::string> texturesNames;

		std::string baseColorTexturePath;
		std::string diffuseTexturePath;
		std::string normalTexturePath;
		std::string ormTexturePath;
		std::string emissiveTexturePath;

		Material material;
		aiColor3D baseColor;
		aiColor3D emissive;
		float opacity = 1.0f;

		if (i == 0 && !objectList.m_hasDefaultTexture)
		{
			objectList.m_hasDefaultTexture = true;

			texturesPaths.emplace_back("Textures/White.png");

			texturesNames.emplace_back("DefaultTexture");

			textureID++;
		}

		if (i == 0 && !objectList.m_hasDefaultNormalTexture)
		{

			objectList.m_hasDefaultNormalTexture = true;

			texturesPaths.emplace_back("Textures/Normal.png");

			texturesNames.emplace_back("DefaultNormalTexture");

			textureID++;
		}

		material.m_name = materialNode->GetName().C_Str();
		spdlog::info("Material name: " + material.m_name);
		
		if (materialNode->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS && (baseColor.r < 1 || baseColor.g < 1 || baseColor.b < 1))
		{
			std::string message = fmt::format("BaseColor : {0:.2f}, {1:.2f}, {2:.2f}", baseColor.r, baseColor.g, baseColor.b);
			spdlog::info(message);
			material.m_baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
		}
		if (materialNode->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS && (emissive.r > 0 || emissive.g > 0 || emissive.b > 0))
		{
			std::string message = fmt::format("Emissive color: {0:.2f}, {1:.2f}, {2:.2f}", emissive.r, emissive.g, emissive.b);
			spdlog::info(message);
			material.m_emissiveColor = glm::vec3(emissive.r, emissive.g, emissive.b);
		}
		if (materialNode->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity < 1.0f)
		{
			std::string message = fmt::format("Material opacity: {0:.3f}", opacity);
			spdlog::info(message);
			material.m_opacity = opacity;
		}		
		
		if (materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
		{
			diffuseTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(diffuseTexturePath);

			std::string name = texturePath.C_Str();
			uint32_t offset = (uint32_t)(name.find_last_of('/', name.size()) + 1);
			name = name.substr(offset, name.size() - (offset + 4));
			texturesNames.emplace_back(name.c_str());

			material.m_diffuseTextureID = textureID;
			textureID++;
		}
		else if (materialNode->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == AI_SUCCESS)
		{
			baseColorTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(baseColorTexturePath);

			std::string name = texturePath.C_Str();
			uint32_t offset = (uint32_t)(name.find_last_of('/', name.size()) + 1);
			name = name.substr(offset, name.size() - (offset + 4));
			texturesNames.emplace_back(name.c_str());

			material.m_diffuseTextureID = textureID;
			textureID++;
		}
		
		if (materialNode->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
		{
			normalTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(normalTexturePath);

			std::string name = texturePath.C_Str();
			uint32_t offset = (uint32_t)(name.find_last_of('/', name.size()) + 1);
			name = name.substr(offset, name.size() - (offset + 4));
			texturesNames.emplace_back(name.c_str());

			material.m_normalTextureID = textureID;
			textureID++;
		}
		else
		{
			material.m_normalTextureID = DEFAULT_NORMAL_TEXTURE;
		}

		if (materialNode->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
		{
			ormTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(ormTexturePath);

			std::string name = texturePath.C_Str();
			uint32_t offset = (uint32_t)(name.find_last_of('/', name.size()) + 1);
			name = name.substr(offset, name.size() - (offset + 4));
			texturesNames.emplace_back(name.c_str());

			material.m_ormTextureID = textureID;
			textureID++;
		}
		else
		{
			float metallicFactor = 0.0f;
			float roughnessFactor = 1.0f;

			materialNode->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
			material.m_metallicFactor = metallicFactor;
			materialNode->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
			material.m_roughnessFactor = roughnessFactor;			

			std::string message = fmt::format("Metallic Factor : {0} Roughness Factor: {1}", metallicFactor, roughnessFactor);
			spdlog::info(message);
		}

		if (materialNode->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS)
		{
			emissiveTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(emissiveTexturePath);

			std::string name = texturePath.C_Str();
			uint32_t offset = (uint32_t)(name.find_last_of('/', name.size()) + 1);
			name = name.substr(offset, name.size() - (offset + 4));
			material.m_emissiveTextureID = textureID;
			texturesNames.emplace_back(name.c_str());

			material.m_emissiveTextureID = textureID;
			textureID++;
		}
		else
		{
			material.m_emissiveTextureID = DEFAULT_WHITE_TEXTURE;
		}

		// No texture
		if (texturesPaths.size() == 0)		
		{
			material.m_diffuseTextureID = material.m_ormTextureID = material.m_emissiveTextureID = DEFAULT_WHITE_TEXTURE;
			material.m_normalTextureID = DEFAULT_NORMAL_TEXTURE;
		}
		else
		{
			Texture materialTextures(texturesPaths, texturesNames, true);
			material.SetTextures(materialTextures);
		}	

		std::cout << std::endl;
		materials.emplace_back(material);
	}
	objectList.SetMaterials(materials);
}
