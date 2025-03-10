#pragma once

struct VertexWithoutUVs
{
	float x, y, z;
};

struct VertexWithUV
{
	float x, y, z;
	float u, v;
	float nX, nY, nZ;
};

struct Vertex
{
	float x, y, z;
	float u, v;
	float nX, nY, nZ;
	float tX, tY, tZ;
	float btX, btY, btZ;
};
