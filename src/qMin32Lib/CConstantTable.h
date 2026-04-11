#pragma once
#include "Core.h"

class CConstantTable
{
public:
	CConstantTable() = default;
	CConstantTable(LPDIRECT3DDEVICE9EX device, ID3DXConstantTable* table);

	ID3DXConstantTable* GetConstantTable();

	//matrix
	bool SetMatrix(const char* name, const D3DXMATRIX* value, UINT count = 1) const;

	//vector
	bool SetVector(const char* name, const D3DXVECTOR4* value, UINT count = 1) const;
	bool SetVector(const char* name, const D3DXVECTOR3* value, UINT count = 1) const;
	bool SetVector(const char* name, const D3DXVECTOR2* value, UINT count = 1) const;

	//color
	bool SetColor(const char* name, const ColorStruct* value, UINT count = 1) const;

	//float
	bool SetFloat(const char* name, const float* value, UINT count = 1) const;

	//int
	bool SetInt(const char* name, const int* value, UINT count = 1) const;

	//bool
	bool SetBool(const char* name, const BOOL* value, UINT count = 1) const;

	//texture 
	bool SetTexture(DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture) const;
	
	//value of a struct (name, color, sizeof(ColorStruct);
	bool SetValue(const char* name, const void* data, UINT bytes) const;

	//from SetValue easy way -> SetStruct(name,color);
	template<typename T> bool SetStruct(const char* name, const T& value) const
	{
		return SetValue(name, &value, (UINT)sizeof(T));
	}

private:
	D3DXHANDLE GetHandle(const char* name) const;

private:
	LPDIRECT3DDEVICE9EX m_device;
	ID3DXConstantTable* m_constant;
};

