#include "pch.h"
#include "ShadersContainer.h"
#include "BaseClass.h"
#include "CShaders.h"

RefPtr<CShaders> ShadersContainer::TerrainShader = nullptr;
RefPtr<CShaders> ShadersContainer::TerrainShadowShader = nullptr;

ShadersContainer::ShadersContainer(BaseClass* base)
{
    TerrainShader = base->CreateShader("terrainVs", "terrainPs");
    TerrainShadowShader = base->CreateShader("shadowTerrainVs", "shadowTerrainPs");
}

RefPtr<CShaders> ShadersContainer::GetTerrain()
{
    return TerrainShader;
}

RefPtr<CShaders> ShadersContainer::GetTerrainShadow()
{
    return TerrainShadowShader;
}
