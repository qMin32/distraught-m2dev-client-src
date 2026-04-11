#pragma once
#include "Core.h"

class CNewLight
{
public:
	CNewLight();
	virtual ~CNewLight() = default;

	void SetType(LightType type);
	void SetPosition(const float3& pos);
	void SetDirection(const float3& dir);
	void SetDiffuse(const ColorStruct& color);
	void SetAmbient(const ColorStruct& color);
	void SetSpecular(const ColorStruct& color);

	void SetAttenuation(float constant, float linear, float quadratic);
	void SetSpotParam(float theta, float phi, float fallof);

	const Light& GetLight() const;

	void SetToShader(const RefPtr<CShaders>& shader) const;

private:
	LightType m_type = LightType::Directional;
	Light m_light = {};
};