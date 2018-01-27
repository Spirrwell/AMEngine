#include "viewport.hpp"
#include "renderer/irenderer.hpp"
#include "factory/ifactory.hpp"

// TODO: Move this and create an extern
extern IRenderer *g_pRenderer;

ViewPort::ViewPort()
{
	g_pRenderer->AddViewPort( this );
}

ViewPort::~ViewPort()
{

}

bool ViewPort::InitPerspectiveViewPort( float flFOV, float flAspect, float flZNear, float flZFar, ICamera *pCamera )
{
	if ( pCamera == nullptr )
		return false;

	m_pCamera = pCamera;

	// TODO: Add window resize handling
	m_mat4PerspectiveMatrix = glm::perspective( flFOV, flAspect, flZNear, flZFar );

	return true;
}

void ViewPort::Shutdown()
{
    if ( m_pCamera )
    {
        delete m_pCamera;
        m_pCamera = nullptr;
    }
}

void ViewPort::UpdateViewPort()
{
	m_pCamera->Update();
	m_pCamera->UpdateView();
}

Matrix4f ViewPort::GetProjection()
{
	return m_mat4PerspectiveMatrix;
}

Matrix4f ViewPort::GetPerspectiveProjection()
{
	return m_mat4PerspectiveMatrix * m_pCamera->GetViewMatrix();
}

Matrix4f ViewPort::GetViewMatrix()
{
	return m_pCamera->GetViewMatrix();
}
