#ifndef IVIEWPORT_HPP
#define IVIEWPORT_HPP

#include "mathdefs.hpp"

#include "icamera.hpp"

class IViewPort
{
public:
	virtual bool InitPerspectiveViewPort( float flFOV, float flAspect, float flZNear, float flZFar, ICamera *pCamera ) = 0;
	virtual void Shutdown() = 0;

	virtual void UpdateViewPort() = 0;

	virtual Matrix4f GetProjection() = 0;
	virtual Matrix4f GetPerspectiveProjection() = 0;
	virtual Matrix4f GetViewMatrix() = 0;

	virtual ICamera *GetCamera() = 0;
};

#endif // IVIEWPORT_HPP
