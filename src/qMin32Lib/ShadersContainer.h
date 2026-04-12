#pragma once
#include "Core.h"
#include <string>

class ShadersContainer
{
public:
	ShadersContainer(BaseClass* base);

	static RefPtr<CShaders> GetTerrain();
	static RefPtr<CShaders> GetTerrainShadow();

private:
	static RefPtr<CShaders> TerrainShader;
	static RefPtr<CShaders> TerrainShadowShader;

};

