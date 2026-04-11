#pragma once
#include "Core.h"
#include <vector>

class CVertexDecl
{
public:
	CVertexDecl(LPDIRECT3DDEVICE9EX device);

	IDirect3DVertexDeclaration9* GetVertexDeclaration(VertexType type) const;

private:
	bool CreateDeclaration(VertexType type, const std::vector<D3DVERTEXELEMENT9>& elements);

	std::vector<D3DVERTEXELEMENT9> BuildDecl(VertexType type) const;

private:
	LPDIRECT3DDEVICE9EX m_device;
	ComPtr<IDirect3DVertexDeclaration9> m_decl[VD_MAX_NUM];
};
