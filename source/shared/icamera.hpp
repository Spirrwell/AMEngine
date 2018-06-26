#ifndef ICAMERA_HPP
#define ICAMERA_HPP

#include "mathdefs.hpp"
#include "amlib/transform.hpp"

class ICamera
{
public:
	virtual ~ICamera() = default;
	virtual Matrix4f GetViewMatrix() = 0;

	virtual const Vector3f &GetPosition() = 0;
	virtual const Vector3f &GetEulerAngles() = 0;

	virtual void SetPosition( const Vector3f &vPosition ) = 0;

	virtual void UpdateView() = 0;
	virtual void Update() = 0;
};

#endif // ICAMERA_HPP
