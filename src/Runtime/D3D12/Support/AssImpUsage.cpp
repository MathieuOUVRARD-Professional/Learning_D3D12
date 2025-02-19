#include <Support/AssImpUsage.h>

bool C_AssImp::Import(const std::string& filePath)
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
		return false;
	}
}