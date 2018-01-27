#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "icamera.hpp"

class Camera : public ICamera
{
public:
	Camera( const Vector3f &vPosition = Vector3f() );
	virtual ~Camera();
	
	Matrix4f GetViewMatrix() override { return m_mat4ViewMatrix; }

	const Vector3f &GetPosition() override { return m_vPosition; };
	const Vector3f &GetEulerAngles() override { return m_vEulerAngles; }
	

	void SetPosition( const Vector3f &vPosition ) override { m_vPosition = vPosition; }

	void UpdateView() override;
	void Update() override;

private:
	Vector3f m_vPosition;
	Vector3f m_vEulerAngles;
	Matrix4f m_mat4ViewMatrix;

	Vector3f m_vForward;
	Vector3f m_vBackward;
	Vector3f m_vUp;
	Vector3f m_vDown;
	Vector3f m_vLeft;
	Vector3f m_vRight;
};

#endif // CAMERA_HPP