#include "pch.h"
#include "ShadersContainer.h"
#include "BaseClass.h"
#include "CShaders.h"

//#define USING_CreateShader  //just for example ,in some case we need to use same vs and different ps 
//#   ifdef USING_CreateShader
//#       define EXAMPLE_CreateShaderSameVS;
//#   endif


/// in DEBUG shaders are read from where the exe is + folders shaders/debug/.hlsl shaders
/// in Release (not tested) shaders are read from binary file (.cso) from pack + shaders/release/.cso binary file


RefPtr<CShaders> ShadersContainer::TerrainShader = nullptr;
RefPtr<CShaders> ShadersContainer::TerrainShadowShader = nullptr;
RefPtr<CShaders> ShadersContainer::WaterShader = nullptr;

ShadersContainer::ShadersContainer(BaseClass* base)
{
# ifdef USING_CreateShader
#    ifdef EXAMPLE_CreateShaderSameVS
         TerrainShader = base->CreateShader("terrainVs", "terrainPs");
         TerrainShadowShader = base->CreateShader("terrainVs", "shadowPs");
#    else
         TerrainShader = base->CreateShader("terrainVs", "terrainPs");
         TerrainShadowShader = base->CreateShader("shadowTVs", "shadowPs");
#    endif
# else
     TerrainShader = base->CreateShaderA("terrain");
     TerrainShadowShader = base->CreateShaderA("shadow");
     WaterShader = base->CreateShaderA("water");
# endif
}

RefPtr<CShaders> ShadersContainer::GetTerrain()
{
    return TerrainShader;
}

RefPtr<CShaders> ShadersContainer::GetTerrainShadow()
{
    return TerrainShadowShader;
}

RefPtr<CShaders> ShadersContainer::GetWater()
{
    return WaterShader;
}
