#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <Util/HRException.h>

namespace C_AssImp
{
	bool Import(const std::string& filePath);
}
