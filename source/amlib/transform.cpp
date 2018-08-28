#include "transform.hpp"
#include <stdio.h>

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

Transform::Transform( const Vector3f &pos /*= Vector3f()*/, const Vector3f &rot /*= Vector3f()*/, const Vector3f &scale /*= Vector3f( 1.0f, 1.0f, 1.0f )*/ )
{
	m_vPosition = pos;
	m_vRotation = rot;
	m_vScale = scale;

	m_vEulerAngles[ PITCH ] = 0.0f;
	m_vEulerAngles[ ROLL ] = 0.0f;
	m_vEulerAngles[ YAW ] = 0.0f;
}

void Transform::UpdateModel()
{
	m_vEulerAngles[ PITCH ] = m_vEulerAngles[ PITCH ] - ( 360.0f * glm::floor( m_vEulerAngles[ PITCH ] / 360.0f ) );
	m_vEulerAngles[ ROLL ] = m_vEulerAngles[ ROLL ] - ( 360.0f * glm::floor( m_vEulerAngles[ ROLL ] / 360.0f ) );
	m_vEulerAngles[ YAW ] = m_vEulerAngles[ YAW ] - ( 360.0f * glm::floor( m_vEulerAngles[ YAW ] / 360.0f ) );

	m_mat4ModelMatrix = Matrix4f();

	// Scaling
	m_mat4ModelMatrix = glm::scale( m_mat4ModelMatrix, m_vScale );

	m_vBackward = glm::normalize( Vector3f( m_mat4ModelMatrix[ 0 ][ 2 ], m_mat4ModelMatrix[ 1 ][ 2 ], m_mat4ModelMatrix[ 2 ][ 2 ] ) );
	m_vUp = glm::normalize( Vector3f( m_mat4ModelMatrix[ 0 ][ 1 ], m_mat4ModelMatrix[ 1 ][ 1 ], m_mat4ModelMatrix[ 2 ][ 1 ] ) );
	m_vRight = glm::normalize( Vector3f( m_mat4ModelMatrix[ 0 ][ 0 ], m_mat4ModelMatrix[ 1 ][ 0 ], m_mat4ModelMatrix[ 2 ][ 0 ] ) );

	m_vForward = -m_vBackward;
	m_vDown = -m_vUp;
	m_vLeft = -m_vRight;

	// Y Rotation
	m_mat4ModelMatrix = glm::rotate( m_mat4ModelMatrix, glm::radians( m_vEulerAngles[ YAW ] ), m_vUp );

	// X Rotation
	m_mat4ModelMatrix = glm::rotate( m_mat4ModelMatrix, glm::radians( m_vEulerAngles[ PITCH ] ) - glm::radians( 90.0f ), m_vRight );

	// Z Rotation
	m_mat4ModelMatrix = glm::rotate( m_mat4ModelMatrix, glm::radians( m_vEulerAngles[ ROLL ] ), m_vForward );

	// Translation
	m_mat4ModelMatrix = glm::translate( m_mat4ModelMatrix, m_vPosition );
}

Matrix4f Transform::GetModelMatrix()
{
	return m_mat4ModelMatrix;
}

