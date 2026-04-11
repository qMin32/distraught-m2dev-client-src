#include "pch.h"
#include "CLight.h"
#include "CShaders.h"

CNewLight::CNewLight()
{
	SetType(LightType::Directional);
}

void CNewLight::SetType(LightType type)
{
	m_type = type;

	switch (type)
	{
	case LightType::Directional:
		m_light.Direction = { -1.0f, -0.3f, 0.0f };
		m_light.Attenuation0 = 0.0f;
		m_light.Attenuation1 = 0.0f;
		m_light.Attenuation2 = 0.0f;
		m_light.Theta = 0.0f;
		m_light.FallOff = 0.0f;
		break;

	case LightType::Point:
		m_light.Attenuation0 = 0.0f;
		m_light.Attenuation1 = 0.0f;
		m_light.Attenuation2 = 0.0f;
		m_light.Theta = 0.0f;
		m_light.FallOff = 0.0f;
		break;

	case LightType::Spot:
		m_light.Direction = { 0.0f, -1.0f, 0.0f };
		m_light.Attenuation0 = 1.0f;
		m_light.Attenuation1 = 0.0f;
		m_light.Attenuation2 = 0.0f;
		m_light.Theta = 0.0f;
		m_light.FallOff = 0.0f;
		break;
	}

	m_light.Position = { 0.0f,0.0f,0.0f };
	m_light.Range = 100.0f;
	m_light.Ambient = ColorStruct(0.1f, 0.1f, 0.1f, 1.0f);
	m_light.Diffuse = ColorStruct(1.0f, 1.0f, 1.0f, 1.0f);
	m_light.Specular = ColorStruct(1.0f, 1.0f, 1.0f, 1.0f);
}

void CNewLight::SetPosition(const float3& pos)
{
	m_light.Position = pos;
}

void CNewLight::SetDirection(const float3& dir)
{
	m_light.Direction = dir;
}

void CNewLight::SetDiffuse(const ColorStruct& color)
{
	m_light.Diffuse = color;
}

void CNewLight::SetAmbient(const ColorStruct& color)
{
	m_light.Ambient = color;
}

void CNewLight::SetSpecular(const ColorStruct& color)
{
	m_light.Specular = color;
}

void CNewLight::SetAttenuation(float constant, float linear, float quadratic)
{
	if (m_type != LightType::Directional)
	{
		m_light.Attenuation0 = constant;
		m_light.Attenuation1 = linear;
		m_light.Attenuation2 = quadratic;
	}
}

void CNewLight::SetSpotParam(float theta, float phi, float fallof)
{
	if (m_type == LightType::Spot)
	{
		m_light.Theta = theta;
		m_light.Phi = phi;
		m_light.FallOff = fallof;
	}
}

const Light& CNewLight::GetLight() const
{
	return m_light;
}

void CNewLight::SetToShader(const RefPtr<CShaders>& shader) const
{
	if (!shader)
		return;
	auto table = shader->GetConstantPs();
	table.SetValue("gLight", &m_light, sizeof(Light));
}
