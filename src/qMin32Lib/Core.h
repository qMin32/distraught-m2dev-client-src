#pragma once
#include <d3dx9.h>
#include <memory>

#include <wrl/client.h>
using namespace Microsoft::WRL;

class BaseClass;
class CIndexBuffer;
class CVertexBuffer;
class CVertexDecl;
class CShaders;
class ShadersContainer; // create all shaders 

template <typename T> using RefPtr = std::shared_ptr<T>;
template <typename T> using UniquePtr = std::unique_ptr<T>;

struct D3D9_BUFFER_DESC //for vertex and index buffers
{
	DWORD Usage;
	D3DFORMAT Format;
	UINT Count;
	UINT Stride;
	const void* Data;

	D3D9_BUFFER_DESC() : Usage(D3DUSAGE_WRITEONLY), Format(D3DFMT_UNKNOWN), Count(0), Stride(0), Data(nullptr) {}
};

enum VertexType
{
	VD_NONE = 0,
	VD_PT,			//position + texcoord 
	VD_PD,			//position + diffuse
	VD_PN,			//position + normal
	VD_PNT,			//position + normal + texcoord0
	VD_PNT2,		//position + normal + texcoord0 + texcoord1
	VD_PDT,			//position + diffuse + texcoord0
	VD_SNOW,

	//speedree
	VD_BRANCH,
	VD_LEAF,

	VD_MAX_NUM,
};

struct ColorStruct
{
	float r, g, b, a;
	ColorStruct() : r(0), g(0), b(0), a(1) {}
	ColorStruct(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}

	ColorStruct(const D3DCOLORVALUE& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
	ColorStruct(const D3DXCOLOR& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}

	operator D3DXCOLOR() const
	{
		return D3DXCOLOR(r, g, b, a);
	}

	operator D3DXVECTOR4() const
	{
		return D3DXVECTOR4(r, g, b, a);
	}

	operator D3DCOLORVALUE() const
	{
		D3DCOLORVALUE c;
		c.r = r; c.g = g; c.b = b; c.a = a;
		return c;
	}
};

using float2 = D3DXVECTOR2;
using float3 = D3DXVECTOR3;
using float4 = D3DXVECTOR4;
using float4x4 = D3DXMATRIX;

enum LightType : int
{
	Point = 1,
	Spot = 2,
	Directional = 3,
};

struct Light 
{
	int Type = 0;

	ColorStruct Diffuse;
	ColorStruct Specular;
	ColorStruct Ambient;

	float3 Position = { 0.0f,0.0f,0.0f };
	float3 Direction = { 0.0f,0.0f,0.0f };

	float Range = 100.0f;
	float FallOff = 0.0f;
	float Attenuation0 = 0.0f;
	float Attenuation1 = 0.0f;
	float Attenuation2 = 0.0f;

	float Theta = 0.0f;
	float Phi = 0.0f;
};

