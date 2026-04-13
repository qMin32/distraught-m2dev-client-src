#include "pch.h"
#include "BaseClass.h"
#include "Eterlib/StateManager.h"

bool BaseClass::CreateRenderTarget(UINT size, D3DFORMAT format, LPDIRECT3DTEXTURE9* outTex, LPDIRECT3DSURFACE9* outSurf, LPDIRECT3DSURFACE9* outDepth)
{
    if (!outTex || !outSurf || !outDepth)
        return false;

    *outTex = nullptr;
    *outSurf = nullptr;
    *outDepth = nullptr;

    if (FAILED(m_device->CreateTexture(size, size, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, outTex, nullptr)))
        return false;

    if (FAILED((*outTex)->GetSurfaceLevel(0, outSurf)))
        return false;

    if (FAILED(m_device->CreateDepthStencilSurface(size, size, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, outDepth, nullptr)))
        return false;

    return true;
}
