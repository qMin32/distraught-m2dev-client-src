#include "pch.h"

#include "All.h"

BaseClass::BaseClass(LPDIRECT3DDEVICE9EX device) : m_device(device)
{
    m_layout = std::make_shared<CVertexDecl>(device);
}

void BaseClass::RecreateResource()
{
    CIndexBuffer::RecreateAll();
    CVertexBuffer::RecreateAll(); 
}

void BaseClass::DestroyResource()
{
    CIndexBuffer::DestroyAll();
    CVertexBuffer::DestroyAll();
}

void BaseClass::SetVertexDeclaration(VertexType type)
{
    m_device->SetVertexDeclaration(m_layout->GetVertexDeclaration(type));
}
