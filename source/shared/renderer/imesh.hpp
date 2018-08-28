#ifndef IMESH_HPP
#define IMESH_HPP

#include "glm/glm.hpp"
#include "mathdefs.hpp"

class IMesh
{
public:
    virtual ~IMesh() = default;

	virtual void Draw() = 0;
	virtual void SetModelMatrix( Matrix4f modelMatrix ) = 0;
};

#endif // IMESH_HPP