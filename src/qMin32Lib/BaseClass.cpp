#include "pch.h"

#include "All.h"

BaseClass::BaseClass(LPDIRECT3DDEVICE9EX device) : m_device(device)
{
    m_layout = std::make_shared<CVertexDecl>(device);

	// initialize shaders container after vertex declaration
	// we create all the shaders with BaseClass so we use  this  
    // to get BaseClass pointer in ShadersContainer constructor
	m_shadersContainer = std::make_shared<ShadersContainer>(this);
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
