#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "mathdefs.hpp"

class Transform
{
public:
	Transform( const Vector3f &pos = Vector3f(), const Vector3f &rot = Vector3f(), const Vector3f &scale = Vector3f( 1.0f, 1.0f, 1.0f ) );
	
	void UpdateModel();
	Matrix4f GetModelMatrix();
	
	inline const Vector3f &GetPosition() { return m_vPosition; }
	inline const Vector3f &GetRotation() { return m_vRotation; }
	inline const Vector3f &GetScale() { return m_vScale; }

	inline const float &GetPitch() { return m_vEulerAngles[ PITCH ]; }
	inline const float &GetRoll() { return m_vEulerAngles[ ROLL ]; }
	inline const float &GetYaw() { return m_vEulerAngles[ YAW ]; }

	inline void SetPitch( float flPitch ) { m_vEulerAngles[ PITCH ] = flPitch; }
	inline void SetRoll( float flRoll ) { m_vEulerAngles[ ROLL ] = flRoll; }
	inline void SetYaw( float flYaw ) { m_vEulerAngles[ YAW ] = flYaw; }
	
	inline void SetPosition( Vector3f pos ) { m_vPosition = pos; }
	inline void SetRotation( Vector3f rot ) { m_vRotation = rot; }
	inline void SetScale( Vector3f scale ) { m_vScale = scale; }

	inline const Vector3f &GetForward() { return m_vForward; }
	inline const Vector3f &GetBackward() { return m_vBackward; }
	inline const Vector3f &GetRight() { return m_vRight; }
	inline const Vector3f &GetLeft() { return m_vLeft; }
	inline const Vector3f &GetUp() { return m_vUp; }
	inline const Vector3f &GetDown() { return m_vDown; }
	
private:
	Vector3f m_vPosition;
	Vector3f m_vRotation;
	Vector3f m_vScale;

	Vector3f m_vEulerAngles;

	Vector3f m_vForward;
	Vector3f m_vBackward;
	Vector3f m_vUp;
	Vector3f m_vDown;
	Vector3f m_vLeft;
	Vector3f m_vRight;

	Matrix4f m_mat4ModelMatrix;
};

#endif // TRANSFORM_HPP
