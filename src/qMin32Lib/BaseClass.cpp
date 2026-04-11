#include "pch.h"

#include "All.h"

BaseClass::BaseClass(LPDIRECT3DDEVICE9EX device) : m_device(device)
{
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
