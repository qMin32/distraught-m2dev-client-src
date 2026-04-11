#include "pch.h"
#include "CConstantTable.h"

CConstantTable::CConstantTable(LPDIRECT3DDEVICE9EX device, ID3DXConstantTable* table)
	:m_device(device), m_constant(table)
{
}

ID3DXConstantTable* CConstantTable::GetConstantTable()
{
	return m_constant;
}

bool CConstantTable::SetMatrix(const char* name, const D3DXMATRIX* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;

	return SUCCEEDED(m_constant->SetMatrixArray(m_device, h, value, count));
}

bool CConstantTable::SetVector(const char* name, const D3DXVECTOR4* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;

	return SUCCEEDED(m_constant->SetVectorArray(m_device, h, value, count));
}

bool CConstantTable::SetVector(const char* name, const D3DXVECTOR3* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;

	return SUCCEEDED(m_constant->SetFloatArray(m_device, h, (const float*)value, 3 * count));
}

bool CConstantTable::SetVector(const char* name, const D3DXVECTOR2* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;
	return SUCCEEDED(m_constant->SetFloatArray(m_device, h, (const float*)value, 2 * count));
}

bool CConstantTable::SetColor(const char* name, const ColorStruct* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;
	return SUCCEEDED(m_constant->SetFloatArray(m_device, h, (const float*)value, 4 * count));
}

bool CConstantTable::SetFloat(const char* name, const float* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;
	return SUCCEEDED(m_constant->SetFloatArray(m_device, h, value, 4 * count));
}

bool CConstantTable::SetInt(const char* name, const int* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;
	return SUCCEEDED(m_constant->SetIntArray(m_device, h, value, 4 * count));
}

bool CConstantTable::SetBool(const char* name, const BOOL* value, UINT count) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;
	return SUCCEEDED(m_constant->SetBoolArray(m_device, h, value, 4 * count));
}

bool CConstantTable::SetValue(const char* name, const void* data, UINT bytes) const
{
	D3DXHANDLE h = GetHandle(name);
	if (!h || !m_device)
		return false;

	return SUCCEEDED(m_constant->SetValue(m_device, h, data, bytes));
}

bool CConstantTable::SetTexture(DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture) const
{
	if (!m_device)
		return false;

	return SUCCEEDED(m_device->SetTexture(stage, pTexture));
}

D3DXHANDLE CConstantTable::GetHandle(const char* name) const
{
	if (!m_constant || !name)
		return nullptr;

	return m_constant->GetConstantByName(nullptr, name);
}
