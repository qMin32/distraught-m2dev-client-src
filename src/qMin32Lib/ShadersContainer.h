#pragma once
#include "Core.h"
#include <string>

class ShadersContainer
{
public:
	ShadersContainer(BaseClass* base);

	RefPtr<CShaders> GetTerrain();
	RefPtr<CShaders> GetShadow();
	RefPtr<CShaders> GetWater();

	//skybox 
	RefPtr<CShaders> GetSkyTexture();
	RefPtr<CShaders> GetSkyDiffuse();
	RefPtr<CShaders> GetSkyCloud();
	RefPtr<CShaders> GetLensFlare();

	//mesh
	RefPtr<CShaders> GetMeshPnt();
	RefPtr<CShaders> GetMeshPnt2();

private:
	RefPtr<CShaders> TerrainShader;
	RefPtr<CShaders> ShadowShader;
	RefPtr<CShaders> WaterShader;

	RefPtr<CShaders> SkyTexture;
	RefPtr<CShaders> SkyDiffuse;
	RefPtr<CShaders> SkyClouds;
	RefPtr<CShaders> LensFlare;

	RefPtr<CShaders> MeshPnt;
	RefPtr<CShaders> MeshPnt2;
};

