#include "pch.h"
#include "CBuffers.h"
#include <mutex>
#include <unordered_map>

std::vector<CIndexBuffer*> CIndexBuffer::s_allIndexBuffers;
std::vector<CVertexBuffer*> CVertexBuffer::s_allVertexBuffers;


CIndexBuffer::CIndexBuffer(LPDIRECT3DDEVICE9EX device, const D3D9_BUFFER_DESC& desc)
    : m_device(device), m_indexCount(desc.Count), m_desc(desc)
{
    s_allIndexBuffers.push_back(this);

    if (desc.Data && desc.Count > 0){
        UINT size = (m_desc.Format == D3DFMT_INDEX16) ? sizeof(WORD) : sizeof(DWORD);
        UINT totalSize = size * m_indexCount;
        m_dataStorage.resize(totalSize);
        memcpy(m_dataStorage.data(), desc.Data, totalSize);
    }

    CreateBuffer();
}

CIndexBuffer::~CIndexBuffer()
{
    Destroy();
}

IDirect3DIndexBuffer9* CIndexBuffer::Get() const
{
    return m_buffer.Get();
}

UINT CIndexBuffer::GetIndexCount() const noexcept
{
    return m_indexCount;
}

void CIndexBuffer::Destroy()
{
    m_buffer.Reset();
}

void CIndexBuffer::CreateBuffer()
{
    if (!m_device || m_indexCount == 0)
        return;

    Destroy();

    UINT size = (m_desc.Format == D3DFMT_INDEX16) ? sizeof(WORD) : sizeof(DWORD);

    if (FAILED(m_device->CreateIndexBuffer(size * m_indexCount, m_desc.Usage,
        m_desc.Format, D3DPOOL_DEFAULT, &m_buffer, nullptr)))
        return;

    void* pIndices = nullptr;
    if (SUCCEEDED(m_buffer->Lock(0, static_cast<UINT>(m_dataStorage.size()), &pIndices, 0)))
    {
        memcpy(pIndices, m_dataStorage.data(), m_dataStorage.size());
        m_buffer->Unlock();
    }
}

bool CIndexBuffer::RecreateBuffer()
{
    Destroy();
    CreateBuffer();
    return m_buffer != nullptr;
}

bool CIndexBuffer::RecreateAll()
{
    bool ok = true;
    for (auto* buf : s_allIndexBuffers)
    {
        ok &= buf->RecreateBuffer();
    }
    return ok;
}

void CIndexBuffer::DestroyAll()
{
    for (auto* buf : s_allIndexBuffers)
    {
        if (buf)
        {
            buf->Destroy();
        }
    }
}






CVertexBuffer::CVertexBuffer(LPDIRECT3DDEVICE9EX device, const D3D9_BUFFER_DESC& desc)
    : m_device(device), m_vertexCount(desc.Count), m_stride(desc.Stride), m_desc(desc)
{
    s_allVertexBuffers.push_back(this);

    if (desc.Data && desc.Count > 0 && desc.Stride > 0)
    {
        UINT totalSize = m_stride * m_vertexCount;
        m_dataStorage.resize(totalSize);
        memcpy(m_dataStorage.data(), desc.Data, totalSize);
    }

    CreateBuffer();
}

CVertexBuffer::~CVertexBuffer()
{
    Destroy();
}

bool CVertexBuffer::Update(const void* data, UINT count)
{
    if (!m_buffer || count > m_vertexCount)
        return false;

    DWORD lockFlags = 0;

    if (m_desc.Usage & D3DUSAGE_DYNAMIC)
        lockFlags = D3DLOCK_DISCARD;

    void* pVertices = nullptr;
    if (FAILED(m_buffer->Lock(0, m_stride * count, &pVertices, lockFlags)))
        return false;

    memcpy(pVertices, data, m_stride * count);
    m_buffer->Unlock();

    return true;
}

IDirect3DVertexBuffer9* CVertexBuffer::Get() const
{
    return m_buffer.Get();
}

UINT CVertexBuffer::GetVertexCount() const noexcept
{
    return m_vertexCount;
}

UINT CVertexBuffer::GetStride() const noexcept
{
    return m_stride;
}

void CVertexBuffer::Destroy()
{
    m_buffer.Reset();
}

void CVertexBuffer::CreateBuffer()
{
    if (!m_device || !m_vertexCount || !m_stride)
        return;

    Destroy();

    DWORD usage = m_desc.Usage | D3DUSAGE_WRITEONLY;

    HRESULT hr = m_device->CreateVertexBuffer(m_stride * m_vertexCount, usage, 0, D3DPOOL_DEFAULT, m_buffer.GetAddressOf(), nullptr);

    if (FAILED(hr))
        return;

    if (!m_dataStorage.empty())
        Update(m_dataStorage.data(), m_vertexCount);
}

bool CVertexBuffer::RecreateBuffer()
{
    Destroy();
    CreateBuffer();
    return m_buffer != nullptr;
}

bool CVertexBuffer::RecreateAll()
{
    bool ok = true;
    for (auto* buf : s_allVertexBuffers)
    {
        ok &= buf->RecreateBuffer();
    }
    return ok;
}

void CVertexBuffer::DestroyAll()
{
    for (auto* buf : s_allVertexBuffers)
    {
        if (buf)
        {
            buf->Destroy();
        }
    }
}

