#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "iviewport.hpp"

class ViewPort : public IViewPort
{
public:
	ViewPort();
	virtual ~ViewPort();

	bool InitPerspectiveViewPort( float flFOV, float flAspect, float flZNear, float flZFar, ICamera *pCamera );
	void Shutdown();

	void UpdateViewPort();

	Matrix4f GetProjection();
	Matrix4f GetPerspectiveProjection();
	Matrix4f GetViewMatrix();

	ICamera *GetCamera() { return m_pCamera; };

private:
	ICamera *m_pCamera;

	Matrix4f m_mat4PerspectiveMatrix;
};

#endif // VIEWPORT_HPP