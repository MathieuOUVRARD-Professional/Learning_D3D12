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
		aiProcess_SortByPType
	);
	if (!scene)
	{
		D3EZ::EzException ezException = D3EZ::EzException("C_AssImp::Import", 15, "importer.ReadFile()", "scene is empty !");
	}
	else
	{
		SceneObject mainObject = SceneObject();
		mainObject.m_name = "Sponza";

		objectList.GetList().emplace_back(mainObject);

		std::string folderPath = filePath.substr(0, filePath.find('/')) + "/"; 
		
		std::cout << "LOADING MATERIALS" << std::endl << std::endl;
		ProcessMaterials(objectList, *scene, folderPath);

		std::cout << "LOADING MESHES" << std::endl << std::endl;
		ProcessMeshesNodes(objectList.GetList(), *scene, *scene->mRootNode, objectList.GetList().back());
	}
}

void C_AssImp::ProcessMeshesNodes(std::list<SceneObject>& objectList, const aiScene& scene, aiNode& node, SceneObject& targetParent, glm::mat4 parentTransform)
{
	SceneObject* parent;
	glm::mat4 newTransform = glm::mat4(1.0f);

	//If node has meshes, create a new scene object for it
	if (node.mNumMeshes > 0) {
		SceneObject newObject = SceneObject();

		glm::mat4 nodeTransform = glm::make_mat4(node.mTransformation.Transpose()[0]);

		newObject.m_name = node.mName.C_Str();
		newObject.m_parent = &targetParent;
		newObject.m_transform = parentTransform * nodeTransform;

		SceneObject* tmp = &targetParent;

		objectList.emplace_back(newObject);
		targetParent.AddChild(&objectList.back());

		// copy the meshes
		C_AssImp::LoadMeshes(scene, node, objectList.back());

		//The new object is the parent for all child nodes
		parent = &objectList.back();
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

void C_AssImp::LoadMeshes(const aiScene& scene, aiNode& node, SceneObject& objectToAddMeshTo)
{	
	std::cout << "Node: " << node.mName.C_Str() << "\r\nParent node: " << node.mParent->mName.C_Str() << "\r\nCopying "; 
	node.mNumMeshes > 1 ? std::cout << node.mNumMeshes << " submeshes\r\n" : std::cout << "a mesh\r\n";

	if (objectToAddMeshTo.m_parent != 0x0000000000000000)
	{
		std::cout << "Parent obj: " << objectToAddMeshTo.m_parent->m_name.c_str();
	}
	std::cout << "\r\n--------------- \r\n";

	for (unsigned int i = 0; i < node.mNumMeshes; i++)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		aiMesh* meshNode = scene.mMeshes[node.mMeshes[i]];
		node.mNumMeshes > 1 ? std::cout << "Submesh: " : std::cout << "Mesh: ";
		std::cout << meshNode->mName.C_Str() << " | " << meshNode->mNumFaces << " faces\r\n";



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
			submesh.m_materialID = meshNode->mMaterialIndex;
			objectToAddMeshTo.m_mesh.AddSubmesh(submesh);
		}
		else
		{
			Mesh mesh;
			mesh.SetVertices(vertices);
			mesh.SetIndices(indices);
			mesh.m_materialID = meshNode->mMaterialIndex;
			objectToAddMeshTo.SetMesh(mesh);
		}
	}	
	std::cout << "---------------\r\n\r\n";
}

void C_AssImp::ProcessMaterials(ObjectList& objectList, const aiScene& scene, std::string sceneDirectory)
{
	std::vector<Material> materials;
	
	for (unsigned int i = 0; i < scene.mNumMaterials; i++)
	{
		aiMaterial* materialNode = scene.mMaterials[i];
		aiString texturePath;

		std::vector<std::string> texturesPaths;
		std::vector<std::string> texturesNames;

		std::string baseColorTexturePath;
		std::string diffuseTexturePath;
		std::string normalTexturePath;
		std::string roughnessMetalnessTexturePath;

		Material material;
		aiColor3D baseColor;
		aiColor3D emissive;
		float opacity = 1.0f;

		material.m_name = materialNode->GetName().C_Str();
		std::cout << "Material name: " << material.m_name << std::endl;
		
		if (materialNode->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS )
		{
			std::cout << "BaseColor : " << baseColor.r << ", " << baseColor.g << ", " << baseColor.b << std::endl;
			material.m_baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
		}

		if (materialNode->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS && (emissive.r > 0 && emissive.g > 0 && emissive.b > 0))
		{
			std::cout << "Emissive color: " << emissive.r << ", " << emissive.g << ", " << emissive.b << std::endl;
			material.m_emissiveColor = glm::vec3(emissive.r, emissive.g, emissive.b);
		}

		if (materialNode->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity < 1.0f)
		{
			std::cout << "Material opacity: " << opacity << std::endl;
			material.m_opacity = opacity;
		}		
		
		if (materialNode->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
		{
			diffuseTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(diffuseTexturePath);

			std::string name = texturePath.C_Str();
			name = name.substr(name.find_last_of('/',name.size()) + 1, name.size());			

			texturesNames.emplace_back(name.c_str());
		}
		else if (materialNode->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == AI_SUCCESS)
		{
			baseColorTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(diffuseTexturePath);

			std::string name = texturePath.C_Str();
			name = name.substr(name.find_last_of('/', name.size()) + 1, name.size());

			texturesNames.emplace_back(name.c_str());
		}
		
		if (materialNode->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
		{
			normalTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(normalTexturePath);

			std::string name = texturePath.C_Str();
			name = name.substr(name.find_last_of('/', name.size()) + 1, name.size());

			texturesNames.emplace_back(name.c_str());
		}
		if (materialNode->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
		{
			roughnessMetalnessTexturePath = sceneDirectory + texturePath.C_Str();
			texturesPaths.emplace_back(roughnessMetalnessTexturePath);

			std::string name = texturePath.C_Str();
			name = name.substr(name.find_last_of('/', name.size()) + 1, name.size());

			texturesNames.emplace_back(name.c_str());
		}
		else
		{
			float metallicFactor = 0.0f;
			float roughnessFactor = 1.0f;

			materialNode->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
			material.m_metallicFactor = metallicFactor;
			materialNode->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
			material.m_roughnessFactor = roughnessFactor;

			std::cout << "Metallic Factor: " << metallicFactor << " Roughness Factor: " << roughnessFactor << std::endl;
		}
		std::cout << std::endl;

		if (texturesPaths.size() == 0)
		{
			texturesPaths.emplace_back("Textures/White.png");
			texturesNames.emplace_back("White");
		}

		Texture materialTextures(texturesPaths, texturesNames);
		material.SetTextures(materialTextures);

		materials.emplace_back(material);
	}
	objectList.SetMaterials(materials);
}
