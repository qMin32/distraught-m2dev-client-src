#pragma once
#include "Core.h"
#include <string>

class ShadersContainer
{
public:
	ShadersContainer(BaseClass* base);

	static RefPtr<CShaders> GetTerrain();
	static RefPtr<CShaders> GetTerrainShadow();
	static RefPtr<CShaders> GetWater();

private:
	static RefPtr<CShaders> TerrainShader;
	static RefPtr<CShaders> TerrainShadowShader;
	static RefPtr<CShaders> WaterShader;
};

