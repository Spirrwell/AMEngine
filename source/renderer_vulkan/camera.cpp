#include "camera.hpp"
#include "input/iinput.hpp"
#include "engine/iengine.hpp"

// memoryoverride.hpp must be the last include file in a .cpp file!!!
#include "memlib/memoryoverride.hpp"

extern IInput *g_pInput;
extern IEngine *g_pEngine;

Camera::Camera( Vector3f vPosition /*= Vector3f()*/ )
{	
	SetPosition( vPosition );
}

Camera::~Camera()
{
}

void Camera::UpdateView()
{
	m_vEulerAngles[ PITCH ] = m_vEulerAngles[ PITCH ] - ( 360.0f * glm::floor( m_vEulerAngles[ PITCH ] / 360.0f ) );
	m_vEulerAngles[ ROLL ] = m_vEulerAngles[ ROLL ] - ( 360.0f * glm::floor( m_vEulerAngles[ ROLL ] / 360.0f ) );
	m_vEulerAngles[ YAW ] = m_vEulerAngles[ YAW ] - ( 360.0f * glm::floor( m_vEulerAngles[ YAW ] / 360.0f ) );

	m_mat4ViewMatrix = Matrix4f( 1.0f );

	// Y Rotation
	m_mat4ViewMatrix = glm::rotate( m_mat4ViewMatrix, glm::radians( m_vEulerAngles[ YAW ] ), Vector3f( 0.0f, 1.0f, 0.0f ) );

	// X Rotation
	m_mat4ViewMatrix = glm::rotate( m_mat4ViewMatrix, glm::radians( m_vEulerAngles[ PITCH ] ), Vector3f( m_mat4ViewMatrix[ 0 ][ 0 ] ,  m_mat4ViewMatrix[ 1 ][ 0 ],  m_mat4ViewMatrix[ 2 ][ 0 ] ) );

	// Z Rotation
	m_mat4ViewMatrix = glm::rotate( m_mat4ViewMatrix, glm::radians( m_vEulerAngles[ ROLL ] ), Vector3f( m_mat4ViewMatrix[ 0 ][ 2 ] ,  m_mat4ViewMatrix[ 1 ][ 2 ],  m_mat4ViewMatrix[ 2 ][ 2 ] ) );

	// Translation
	m_mat4ViewMatrix = glm::translate( m_mat4ViewMatrix, -m_vPosition );

	m_vForward = glm::normalize( Vector3f( m_mat4ViewMatrix[ 0 ][ 2 ], m_mat4ViewMatrix[ 1 ][ 2 ], m_mat4ViewMatrix[ 2 ][ 2 ] ) );
	m_vUp = glm::normalize( Vector3f( m_mat4ViewMatrix[ 0 ][ 1 ], m_mat4ViewMatrix[ 1 ][ 1 ], m_mat4ViewMatrix[ 2 ][ 1 ] ) );
	m_vRight = glm::normalize( Vector3f( m_mat4ViewMatrix[ 0 ][ 0 ], m_mat4ViewMatrix[ 1 ][ 0 ], m_mat4ViewMatrix[ 2 ][ 0 ] ) );

	m_vBackward = -m_vForward;
	m_vDown = -m_vUp;
	m_vLeft = -m_vRight;
}

void Camera::Update()
{
	m_vEulerAngles[ YAW ] -= glm::degrees( ( g_pInput->GetMouseDeltaX() * g_pEngine->GetDeltaTime() ) );
	m_vEulerAngles[ PITCH ] -= glm::degrees( ( g_pInput->GetMouseDeltaY() * g_pEngine->GetDeltaTime() ) );

	constexpr const float defaultspeed = 20.0f;
	constexpr const float shiftspeedmult = 3.0f;

	const float speed = g_pInput->IsButtonPressed( "Speed" ) ? defaultspeed * shiftspeedmult : defaultspeed;

	if ( g_pInput->IsButtonPressed( "Forward" ) )
		m_vPosition += ( m_vForward * speed * g_pEngine->GetDeltaTime() );
	else if ( g_pInput->IsButtonPressed( "Backward" ) )
		m_vPosition += ( m_vBackward * speed * g_pEngine->GetDeltaTime() );

	if ( g_pInput->IsButtonPressed( "Left" ) )
		m_vPosition += ( m_vLeft * speed * g_pEngine->GetDeltaTime() );
	else if ( g_pInput->IsButtonPressed( "Right" ) )
		m_vPosition += ( m_vRight * speed * g_pEngine->GetDeltaTime() );

	if ( g_pInput->IsButtonPressed( "Jump" ) )
		m_vPosition += ( Vector3f( 0, 1, 0 ) * speed * g_pEngine->GetDeltaTime() );
	else if ( g_pInput->IsButtonPressed( "Crouch" ) )
		m_vPosition += ( Vector3f( 0, -1, 0 ) * speed * g_pEngine->GetDeltaTime() );
}