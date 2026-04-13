#pragma once
#include "Core.h"
#include <string>

class BaseClass
{
public:
	BaseClass(LPDIRECT3DDEVICE9EX device);
	~BaseClass();

public:
	void RecreateResource(); //recreate all buffers
	void DestroyResource();	 //destroy all buffers
	void SetVertexDeclaration(VertexType type);
	RefPtr<ShadersContainer> GetShaderContained();

private:
	LPDIRECT3DDEVICE9EX m_device;
	RefPtr<CVertexDecl> m_layout;
	RefPtr<ShadersContainer> m_shadersContainer;


/////////////////////////// BaseClassResource.cpp ///////////////////////////
public: 
	RefPtr<CVertexBuffer> CreateVertexBuffer(const void* data, UINT stride, UINT vertexCount, DWORD usage = D3DUSAGE_WRITEONLY);
	void SetVertexBuffer(const RefPtr<CVertexBuffer>& vb, UINT stride = 0) const;

	RefPtr<CIndexBuffer> CreateIndexBuffer(const void* data, UINT indexCount, DWORD usage = D3DUSAGE_WRITEONLY, D3DFORMAT format = D3DFMT_INDEX16);
	void SetIndexBuffer(const RefPtr<CIndexBuffer>& ib) const;

	RefPtr<CShaders> CreateShader(const std::string& vsName = "", const std::string& psName = "");
	//create shaders with common name, for example "terrain" will create "terrainVs" and "terrainPs"
	RefPtr<CShaders> CreateShaderA(const std::string& Name = "");
	void SetShader(const RefPtr<CShaders>& shader);

public:
	/////////////////////////// BaseClassHelper.cpp ///////////////////////////
	bool CreateRenderTarget(UINT size, D3DFORMAT format, LPDIRECT3DTEXTURE9* outTex, LPDIRECT3DSURFACE9* outSurf, LPDIRECT3DSURFACE9* outDepth);
};

