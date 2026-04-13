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
     ShadowShader = base->CreateShaderA("shadow");
# endif
     WaterShader = base->CreateShaderA("water");

     SkyTexture = base->CreateShaderA("skyTex");
     SkyDiffuse = base->CreateShaderA("skyDiffuse");
     SkyClouds = base->CreateShaderA("skyClouds");
     LensFlare = base->CreateShaderA("LensFlare");
}

RefPtr<CShaders> ShadersContainer::GetTerrain()
{
    return TerrainShader;
}

RefPtr<CShaders> ShadersContainer::GetShadow()
{
    return ShadowShader;
}

RefPtr<CShaders> ShadersContainer::GetWater()
{
    return WaterShader;
}

RefPtr<CShaders> ShadersContainer::GetSkyTexture()
{
    return SkyTexture;
}

RefPtr<CShaders> ShadersContainer::GetSkyDiffuse()
{
    return SkyDiffuse;
}

RefPtr<CShaders> ShadersContainer::GetSkyCloud()
{
    return SkyClouds;
}

RefPtr<CShaders> ShadersContainer::GetLensFlare()
{
    return LensFlare;
}
