#pragma once
#include <d3dx9.h>
#include <memory>

#include <wrl/client.h>
using namespace Microsoft::WRL;

class BaseClass;
class CIndexBuffer;
class CVertexBuffer;

template <typename T> using RefPtr = std::shared_ptr<T>;
template <typename T> using UniquePtr = std::unique_ptr<T>;

struct D3D9_BUFFER_DESC //for vertex and index buffers
{
	DWORD Usage;
	D3DFORMAT Format;
	UINT Count;
	UINT Stride;
	const void* Data;

	D3D9_BUFFER_DESC() : Usage(D3DUSAGE_WRITEONLY), Format(D3DFMT_UNKNOWN), Count(0), Stride(0), Data(nullptr) {}
};