#pragma once
#include "CConstantTable.h"

#include <string>
#include <vector>
#include <thread>

class CShaders
{
public:
	CShaders(LPDIRECT3DDEVICE9EX device, const std::string& vsName, const std::string& psName);
	~CShaders();

public:
	CConstantTable GetConstantVs() const;
	CConstantTable GetConstantPs() const;

	IDirect3DVertexShader9* GetVS() { return m_vs.Get(); }
	IDirect3DPixelShader9* GetPS() { return m_ps.Get(); }

private:
	bool LoadShaderByteCode(const std::string& name, const char* profile, std::vector<DWORD>& byteCode, ComPtr<ID3DXConstantTable>& constant);
	void CreateShader(const std::string& name, const char* profile, ComPtr<IDirect3DVertexShader9>& vsOut, ComPtr<ID3DXConstantTable>& constantOut);
	void CreateShader(const std::string& name, const char* profile, ComPtr<IDirect3DPixelShader9>& psOut, ComPtr<ID3DXConstantTable>& constantOut);

private:
	LPDIRECT3DDEVICE9EX m_device;

	ComPtr<IDirect3DVertexShader9> m_vs;
	ComPtr<IDirect3DPixelShader9> m_ps;

	ComPtr<ID3DXConstantTable> vs_table;
	ComPtr<ID3DXConstantTable> ps_table;

	//hot reload
private:
	void StartWatcher();
	void Reload();

	std::string m_vsName;
	std::string m_psName;
	std::thread m_watcherThread;
	std::atomic<bool> m_watcherRunnig{ false };
	mutable std::atomic<bool> m_needReload{ false };
};
