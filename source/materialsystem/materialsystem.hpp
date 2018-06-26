#ifndef MATERIALSYSTEM_HPP
#define MATERIALSYSTEM_HPP

#include "imaterialsystem.hpp"
#include "renderer/irenderer.hpp"

class MaterialSystem : public IMaterialSystem
{
public:
	MaterialSystem();
	virtual ~MaterialSystem() = default;

	bool Init() override;

	IMaterial *CreateMaterial( const string &materialName ) override;
};

extern IRenderer *g_pRenderer;

#endif // MATERIALSYSTEM_HPP