#include "pch.h"
#include "CShaders.h"
#include "../PackLib/PackManager.h"
#include <fstream>
#include <d3dx9shader.h>

#include <d3dcompiler.h>
#pragma comment (lib, "d3dcompiler.lib")

// Tested only in debug mode so far.
// Not sure if reading from the pack works in release builds.
// Any brave soul to test/fix release mode? :))


static std::string BuildShaderPath(const std::string& basePath)
{
#ifdef _DEBUG
    return "shaders/debug/" + basePath + ".hlsl";
#else //read from binary
    return "shaders/release/" + basePath + ".cso";
#endif
}

#ifndef _DEBUG
static bool ReadBinaryFile(const std::string& path, std::vector<DWORD>& outByteCode)
{
    //load from pack 
    {
        TPackFile kFile;
        if (CPackManager::Instance().GetFile(path, kFile))
        {
            size_t size = kFile.size();
            if (size == 0 || (size % sizeof(DWORD)) != 0)
                return false;

            outByteCode.resize(size / sizeof(DWORD));
            memcpy(outByteCode.data(), kFile.data(), size);
        }
    }

    //load from disk
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return false;

        std::streamsize size = file.tellg();
        if (size <= 0 || (size % sizeof(DWORD)) != 0)
            return false;

        file.seekg(0, std::ios::beg);
        outByteCode.resize(static_cast<size_t>(size) / sizeof(DWORD));
        
        if (!file.read((char*)outByteCode.data(), size))
        {
            outByteCode.clear();
            return false;
        }
    }
    return true;

}
#endif
CShaders::CShaders(LPDIRECT3DDEVICE9EX device, const std::string& vsName, const std::string& psName)
    :m_device(device), m_vsName(vsName), m_psName(psName)
{
    if (!vsName.empty())
        CreateShader(vsName, "vs_3_0", m_vs, vs_table);

    if (!psName.empty())
        CreateShader(psName, "ps_3_0", m_ps, ps_table);

#ifdef _DEBUG
    StartWatcher();
#endif
}

CShaders::~CShaders()
{
#ifdef _DEBUG
    m_watcherRunnig = false;
    if (m_watcherThread.joinable())
        m_watcherThread.join();
#endif
}

CConstantTable CShaders::GetConstantVs() const
{
#ifdef _DEBUG
    if (m_needReload.exchange(false))
    {
        const_cast<CShaders*>(this)->Reload();
    }
#endif
    return CConstantTable(m_device, vs_table.Get());
}

CConstantTable CShaders::GetConstantPs() const
{
#ifdef _DEBUG
    if (m_needReload.exchange(false))
    {
        const_cast<CShaders*>(this)->Reload();
    }
#endif
    return CConstantTable(m_device, ps_table.Get());
}

bool CShaders::LoadShaderByteCode(const std::string& name, const char* profile, std::vector<DWORD>& byteCode, ComPtr<ID3DXConstantTable>& constant)
{
    const std::string path = BuildShaderPath(name);

#ifdef _DEBUG
    ID3DXBuffer* code = nullptr;
    ID3DXBuffer* error = nullptr;
    DWORD flag = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;

    HRESULT hr = D3DXCompileShaderFromFileA(path.c_str(), nullptr, nullptr, "main",
        profile, flag, &code, &error, constant.GetAddressOf());

    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((const char*)error->GetBufferPointer());
            error->Release();
        }
        if (code)
            code->Release();

        return false;
    }

    byteCode.resize(code->GetBufferSize() / sizeof(DWORD));
    memcpy(byteCode.data(), code->GetBufferPointer(), code->GetBufferSize());
    code->Release();
    if (error)
        error->Release();

    return true;
#else
    if (!ReadBinaryFile(path, byteCode))
        return false;
    return SUCCEEDED(D3DXGetShaderConstantTable(byteCode.data(), constant.GetAddressOf()))
#endif
}

void CShaders::CreateShader(const std::string& name, const char* profile, ComPtr<IDirect3DVertexShader9>& vsOut, ComPtr<ID3DXConstantTable>& constantOut)
{
    std::vector<DWORD> bytecode;
    if (!LoadShaderByteCode(name, profile, bytecode, constantOut))
        return;

    m_device->CreateVertexShader(bytecode.data(), vsOut.GetAddressOf());
}

void CShaders::CreateShader(const std::string& name, const char* profile, ComPtr<IDirect3DPixelShader9>& psOut, ComPtr<ID3DXConstantTable>& constantOut)
{
    std::vector<DWORD> bytecode;
    if (!LoadShaderByteCode(name, profile, bytecode, constantOut))
        return;

    m_device->CreatePixelShader(bytecode.data(), psOut.GetAddressOf());
}

void CShaders::StartWatcher()
{
    m_watcherRunnig = true;
    m_watcherThread = std::thread([this]()
        {
            auto getModTime = [](const std::string& path) -> FILETIME
                {
                    FILETIME ft = {};
                    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
                    if (h != INVALID_HANDLE_VALUE)
                    {
                        GetFileTime(h, nullptr, nullptr, &ft);
                        CloseHandle(h);
                    }
                    return ft;
                };

            std::string vsPath = "shaders/debug/" + m_vsName + ".hlsl";
            std::string psPath = "shaders/debug/" + m_psName + ".hlsl";

            FILETIME ftVS = getModTime(vsPath);
            FILETIME ftPS = getModTime(psPath);

            while (m_watcherRunnig)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                FILETIME newVS = getModTime(vsPath);
                FILETIME newPS = getModTime(psPath);

                if (CompareFileTime(&newVS, &ftVS) != 0 || CompareFileTime(&newPS, &ftPS) != 0)
                {
                    ftVS = newVS;
                    ftPS = newPS;
                    m_needReload = true;
                }
            }
        });
}

void CShaders::Reload()
{
    if (!m_vsName.empty())
        CreateShader(m_vsName, "vs_3_0", m_vs, vs_table);
    if (!m_psName.empty())
        CreateShader(m_psName, "ps_3_0", m_ps, ps_table);

    OutputDebugStringA((">>>> Reloaded:" + m_vsName + " / " + m_psName + "\n").c_str());
}

