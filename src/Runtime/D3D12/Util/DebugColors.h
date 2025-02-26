#pragma once

#include <glm/glm.hpp>

#include <vector>

struct DebugColors
{
	glm::vec4 green			= glm::vec4(0.0000f, 1.0000f, 0.1160f, 1.0000f);
	glm::vec4 blue			= glm::vec4(0.0000f, 0.0000f, 0.7790f, 1.0000f);
	glm::vec4 red			= glm::vec4(1.0000f, 0.1040f, 0.0000f, 1.0000f);
	glm::vec4 teal			= glm::vec4(0.1662f, 0.8562f, 0.8409f, 1.0000f);
	glm::vec4 greyGreen		= glm::vec4(0.4021f, 0.5124f, 0.4660f, 1.0000f);
	glm::vec4 gold			= glm::vec4(1.0000f, 0.8197f, 0.0000f, 1.0000f);
	glm::vec4 pink			= glm::vec4(0.9780f, 0.1426f, 0.8498f, 1.0000f);
	glm::vec4 turquoise		= glm::vec4(0.0462f, 0.7732f, 0.8538f, 1.0000f);
	glm::vec4 aqua			= glm::vec4(0.0000f, 1.0000f, 1.0000f, 1.0000f);
	glm::vec4 softGreen		= glm::vec4(0.7023f, 0.7988f, 0.0921f, 1.0000f);
	glm::vec4 darkYellow	= glm::vec4(0.8110f, 0.7976f, 0.0000f, 1.0000f);
	glm::vec4 softPink		= glm::vec4(0.8794f, 0.0813f, 0.7059f, 1.0000f);
	glm::vec4 lightpurple	= glm::vec4(0.8234f, 0.0000f, 1.0000f, 1.0000f);
	glm::vec4 lightPink		= glm::vec4(1.0000f, 0.7974f, 1.0000f, 1.0000f);
	glm::vec4 brown			= glm::vec4(0.4000f, 0.2784f, 0.1529f, 1.0000f);
	glm::vec4 purple		= glm::vec4(0.4800f, 0.2200f, 0.9100f, 1.0000f);
	glm::vec4 orange		= glm::vec4(0.9200f, 0.4200f, 0.1000f, 1.0000f);
	glm::vec4 lightGreen	= glm::vec4(0.5873f, 0.9334f, 0.4961f, 1.0000f); 
	glm::vec4 lavender		= glm::vec4(0.6928f, 0.5777f, 0.6734f, 1.0000f);
	glm::vec4 darkPurple	= glm::vec4(0.6656f, 0.0000f, 0.7792f, 1.0000f);
	glm::vec4 lightOrange	= glm::vec4(1.0000f, 0.6616f, 0.1754f, 1.0000f);
	/*glm::vec4(0.0000f, 0.7426f, 0.9548f, 1.0000f);
	glm::vec4(0.5836f, 0.8561f, 0.3198f, 1.0000f);
	glm::vec4(0.9504f, 0.8106f, 0.3455f, 1.0000f);
	glm::vec4(0.5814f, 0.9438f, 0.0527f, 1.0000f);
	glm::vec4(0.1657f, 0.5382f, 0.7472f, 1.0000f);
	glm::vec4(0.9538f, 0.7946f, 0.0000f, 1.0000f);
	glm::vec4(0.6544f, 0.0285f, 0.7349f, 1.0000f);
	glm::vec4(0.6048f, 1.0000f, 0.5767f, 1.0000f);
	glm::vec4(1.0000f, 0.2333f, 0.4163f, 1.0000f);
	glm::vec4(0.5537f, 0.4280f, 0.7090f, 1.0000f);
	glm::vec4(0.0840f, 0.7636f, 0.7990f, 1.0000f)*/
};

inline std::vector<glm::vec4> DebugColorsVector =
{
	glm::vec4(0.0000f, 1.0000f, 0.1160f, 1.0000f),
	glm::vec4(0.0000f, 0.0000f, 0.7790f, 1.0000f),
	glm::vec4(1.0000f, 0.1040f, 0.0000f, 1.0000f),
	glm::vec4(0.1662f, 0.8562f, 0.8409f, 1.0000f),
	glm::vec4(0.4021f, 0.5124f, 0.4660f, 1.0000f),
	glm::vec4(1.0000f, 0.8197f, 0.0000f, 1.0000f),
	glm::vec4(0.9780f, 0.1426f, 0.8498f, 1.0000f),
	glm::vec4(0.0462f, 0.7732f, 0.8538f, 1.0000f),
	glm::vec4(0.0000f, 1.0000f, 1.0000f, 1.0000f),
	glm::vec4(0.7023f, 0.7988f, 0.0921f, 1.0000f),
	glm::vec4(0.8110f, 0.7976f, 0.0000f, 1.0000f),
	glm::vec4(0.8794f, 0.0813f, 0.7059f, 1.0000f),
	glm::vec4(0.8234f, 0.0000f, 1.0000f, 1.0000f),
	glm::vec4(1.0000f, 0.7974f, 1.0000f, 1.0000f),
	glm::vec4(0.4000f, 0.2784f, 0.1529f, 1.0000f),
	glm::vec4(0.4800f, 0.2200f, 0.9100f, 1.0000f),
	glm::vec4(0.9200f, 0.4200f, 0.1000f, 1.0000f),
	glm::vec4(0.5873f, 0.9334f, 0.4961f, 1.0000f),
	glm::vec4(0.6928f, 0.5777f, 0.6734f, 1.0000f),
	glm::vec4(0.6656f, 0.0000f, 0.7792f, 1.0000f),
	glm::vec4(1.0000f, 0.6616f, 0.1754f, 1.0000f),
	//No names
	glm::vec4(0.0000f, 0.7426f, 0.9548f, 1.0000f),
	glm::vec4(0.5836f, 0.8561f, 0.3198f, 1.0000f),
	glm::vec4(0.9504f, 0.8106f, 0.3455f, 1.0000f),
	glm::vec4(0.5814f, 0.9438f, 0.0527f, 1.0000f),
	glm::vec4(0.1657f, 0.5382f, 0.7472f, 1.0000f),
	glm::vec4(0.9538f, 0.7946f, 0.0000f, 1.0000f),
	glm::vec4(0.6544f, 0.0285f, 0.7349f, 1.0000f),
	glm::vec4(0.6048f, 1.0000f, 0.5767f, 1.0000f),
	glm::vec4(1.0000f, 0.2333f, 0.4163f, 1.0000f),
	glm::vec4(0.5537f, 0.4280f, 0.7090f, 1.0000f),
	glm::vec4(0.0840f, 0.7636f, 0.7990f, 1.0000f)
};