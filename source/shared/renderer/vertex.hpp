#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "mathdefs.hpp"


struct Vertex
{
	Vector3f pos;
	Vector3f color;
	Vector2f texCoord;
	Vector3f normal;
};

#endif // VERTEX_HPP