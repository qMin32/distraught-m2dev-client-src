#pragma once
#include "Core.h"
#include <vector>

class CIndexBuffer final 
{
public:
    explicit CIndexBuffer(LPDIRECT3DDEVICE9EX device, const D3D9_BUFFER_DESC& desc);
    ~CIndexBuffer();
    IDirect3DIndexBuffer9* Get() const;
    UINT GetIndexCount() const noexcept;

    static bool RecreateAll();
    static void DestroyAll();

private:
    void CreateBuffer();
    void Destroy();
    bool RecreateBuffer();

private:
    LPDIRECT3DDEVICE9EX m_device = nullptr;
    std::vector<BYTE> m_dataStorage;
    UINT m_indexCount = 0;
    D3D9_BUFFER_DESC m_desc{};
    ComPtr<IDirect3DIndexBuffer9> m_buffer;
    static std::vector<CIndexBuffer*> s_allIndexBuffers;
};

class CVertexBuffer final
{
public:
    explicit CVertexBuffer(LPDIRECT3DDEVICE9EX device, const D3D9_BUFFER_DESC& desc);
    ~CVertexBuffer();
    IDirect3DVertexBuffer9* Get() const;
    UINT GetVertexCount() const noexcept;
    UINT GetStride() const noexcept;
    bool Update(const void* data, UINT count);
    static bool RecreateAll();
    static void DestroyAll();

private:
    void CreateBuffer();
    void Destroy();
    bool RecreateBuffer();

private:
    LPDIRECT3DDEVICE9EX m_device = nullptr;
    std::vector<BYTE> m_dataStorage;
    UINT m_vertexCount = 0;
    UINT m_stride = 0;
    D3D9_BUFFER_DESC m_desc{};
    ComPtr<IDirect3DVertexBuffer9> m_buffer;
    static std::vector<CVertexBuffer*> s_allVertexBuffers;
};
