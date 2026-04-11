#pragma once
#include "Core.h"
#include <string>

class BaseClass
{
public:
	BaseClass(LPDIRECT3DDEVICE9EX device);
	~BaseClass() {};

public:
	void RecreateResource(); //recreate all buffers
	void DestroyResource();	 //destroy all buffers


public: // BaseClassResource.cpp
	RefPtr<CVertexBuffer> CreateVertexBuffer(const void* data, UINT stride, UINT vertexCount, DWORD usage = D3DUSAGE_WRITEONLY);
	void SetVertexBuffer(const RefPtr<CVertexBuffer>& vb, UINT stride = 0) const;

	RefPtr<CIndexBuffer> CreateIndexBuffer(const void* data, UINT indexCount, DWORD usage = D3DUSAGE_WRITEONLY, D3DFORMAT format = D3DFMT_INDEX16);
	void SetIndexBuffer(const RefPtr<CIndexBuffer>& ib) const;

private:
	LPDIRECT3DDEVICE9EX m_device;
};

