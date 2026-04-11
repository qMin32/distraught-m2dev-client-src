#include "pch.h"
#include "CVertexDecl.h"
#include "SpeedTreeLib/SpeedTreeConfig.h"

CVertexDecl::CVertexDecl(LPDIRECT3DDEVICE9EX device) : m_device(device)
{
    for (int i = 0; i < VD_MAX_NUM; ++i)
        m_decl[i].Reset();

    if (!m_device)
        return;

    for (int i = 0; i < VD_MAX_NUM; ++i)
    {
        auto type = static_cast<VertexType>(i);
        auto decl = BuildDecl(type);

        if (!decl.empty())
        {
            CreateDeclaration(type, decl);
        }
    }
}

IDirect3DVertexDeclaration9* CVertexDecl::GetVertexDeclaration(VertexType type) const
{
    return m_decl[type].Get();
}

bool CVertexDecl::CreateDeclaration(VertexType type, const std::vector<D3DVERTEXELEMENT9>& elements)
{
    if (!m_device)
        return false;

    if (type < 0 || type >= VD_MAX_NUM)
        return false;

    if (elements.empty())
        return false;

    if (m_decl[type])
    {
        m_decl[type].Reset();
    }

    HRESULT hr = m_device->CreateVertexDeclaration(elements.data(), m_decl[type].GetAddressOf());
    return SUCCEEDED(hr);
}

std::vector<D3DVERTEXELEMENT9> CVertexDecl::BuildDecl(VertexType type) const
{
    switch (type)
    {
    case VD_PNT:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
            {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            D3DDECL_END()
        };

    case VD_PNT2:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
            {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
            D3DDECL_END()
        };

    case VD_PN:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
            D3DDECL_END()
        };

    case VD_PDT:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_D3DCOLOR,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
            {0, 16, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            D3DDECL_END()
        };

    case VD_SNOW:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT4,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0},
            {0, 16, D3DDECLTYPE_D3DCOLOR,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0},
            {0, 20, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0},
            D3DDECL_END()
        };

    case VD_PT:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            D3DDECL_END()
        };

    case VD_PD:
        return {
            {0, 0,  D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_D3DCOLOR,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
            D3DDECL_END()
        };

    case VD_BRANCH:
        return {
            { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
            { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
            D3DDECL_END()
        };

    case VD_LEAF:
        return {
            { 0,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0 },
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            { 0, 12, D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,			0 },
            { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		0 },
    #if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT	
            { 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		2 },
    #endif
#else
            { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 },
            { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		0 },
    #if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT	
            { 0, 24, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,		2 },
    #endif
#endif
            D3DDECL_END()
        };

    default:
        return {};
    }
}
