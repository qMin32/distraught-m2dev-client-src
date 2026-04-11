#pragma once
#include "Core.h"
#include <string>

class BaseClass
{
public:
	BaseClass(LPDIRECT3DDEVICE9EX device);
	~BaseClass() {};

private:
	LPDIRECT3DDEVICE9EX m_device;
};

