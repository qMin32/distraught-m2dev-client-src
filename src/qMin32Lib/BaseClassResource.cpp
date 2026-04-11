#include "pch.h"

#include "All.h"
#include "../EterLib/StateManager.h"

// start VertexBuffer
RefPtr<CVertexBuffer> BaseClass::CreateVertexBuffer(const void* data, UINT stride, UINT vertexCount, DWORD usage)
{
    D3D9_BUFFER_DESC desc{};
    desc.Data = data;
    desc.Count = vertexCount;
    desc.Usage = usage;
    desc.Stride = stride;

    return std::make_shared<CVertexBuffer>(m_device, desc);
}

void BaseClass::SetVertexBuffer(const RefPtr<CVertexBuffer>& vb, UINT stride) const
{
    UINT useStride = (stride != 0) ? stride : vb->GetStride();
    STATEMANAGER.SetStreamSource(0, vb->Get(), useStride);
}
// end VertexBuffer

// start IndexBuffer
RefPtr<CIndexBuffer> BaseClass::CreateIndexBuffer(const void* data, UINT indexCount, DWORD usage, D3DFORMAT format)
{
    D3D9_BUFFER_DESC desc{};
    desc.Data = data;
    desc.Count = indexCount;
    desc.Usage = usage;
    desc.Format = format;

    return std::make_shared<CIndexBuffer>(m_device, desc);
}

void BaseClass::SetIndexBuffer(const RefPtr<CIndexBuffer>& ib) const
{
    STATEMANAGER.SetIndices(ib->Get(), 0);
}
// end IndexBuffer

// start Shader

RefPtr<CShaders> BaseClass::CreateShader(const std::string& vsName, const std::string& psName)
{
    return std::make_shared<CShaders>(m_device, vsName, psName);
}

void BaseClass::SetShader(const RefPtr<CShaders>& shader)
{
    if (!shader)
    {
        m_device->SetVertexShader(nullptr);
        m_device->SetPixelShader(nullptr);
        return;
    }

    m_device->SetVertexShader(shader->GetVS());
    m_device->SetPixelShader(shader->GetPS());
}

// end Shader