#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "EterLib/ResourceManager.h"

#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "qMin32Lib/All.h"

//now we will use one texture,not 30

void CMapOutdoor::LoadWaterTexture()
{
	UnloadWaterTexture();
	m_WaterNormalMap.SetImagePointer((CGraphicImage*)
		CResourceManager::Instance().GetResourcePointer("d:/ymir Work/special/water/water_normal.dds"));
}

void CMapOutdoor::UnloadWaterTexture()
{
	m_WaterNormalMap.Destroy();
}

void CMapOutdoor::BeginWaterShader()
{
	m_dx->SetVertexDeclaration(VD_PD);
	auto shader = m_dx->GetShaderContained()->GetWater();
	m_dx->SetShader(shader);

	auto vsConstant = shader->GetConstantVs();
	vsConstant.SetMatrix("g_mView", &mat.view);
	vsConstant.SetMatrix("g_mProj", &mat.proj);

	float texScaleX = m_fWaterTexCoordBase;
	float texScaleY = -m_fWaterTexCoordBase;
	vsConstant.SetFloat("g_fTexScaleX", &texScaleX);
	vsConstant.SetFloat("g_fTexScaleY", &texScaleY);

	float fTime = ELTimer_GetMSec() / 1000.0f;
	vsConstant.SetFloat("g_fTime", &fTime);

	float fogNear = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogNearDistance() : 5000.0f;
	float fogFar = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogFarDistance() : 10000.0f;

	D3DXCOLOR fogColor = mc_pEnvironmentData
		? D3DXCOLOR(mc_pEnvironmentData->FogColor)
		: D3DXCOLOR(1, 1, 1, 1);
	ColorStruct fogCol(fogColor.r, fogColor.g, fogColor.b, fogColor.a);

	auto psConstant = shader->GetConstantPs();
	psConstant.SetFloat("g_fFogNear", &fogNear);
	psConstant.SetFloat("g_fFogFar", &fogFar);
	psConstant.SetColor("g_vFogColor", &fogCol);

	ColorStruct waterColor(0.1f, 0.3f, 0.6f, 0.65f);
	psConstant.SetColor("g_vWaterColor", &waterColor);

	STATEMANAGER.SetTexture(0, m_WaterNormalMap.GetTexturePointer()->GetD3DTexture());
}

void CMapOutdoor::RenderWater()
{
	if (m_PatchVector.empty())
		return;

	if (!IsVisiblePart(PART_WATER))
		return;

	D3DXMATRIX matTexTransformWater;
	D3DXMatrixScaling(&matTexTransformWater, m_fWaterTexCoordBase, -m_fWaterTexCoordBase, 0.0f);

	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	BeginWaterShader();

	static float s_fWaterHeightCurrent = 0;
	static float s_fWaterHeightBegin = 0;
	static float s_fWaterHeightEnd = 0;
	static DWORD s_dwLastHeightChangeTime = CTimer::Instance().GetCurrentMillisecond();
	static DWORD s_dwBlendtime = 300;

	if ((CTimer::Instance().GetCurrentMillisecond() - s_dwLastHeightChangeTime) > s_dwBlendtime)
	{
		s_dwBlendtime = random_range(1000, 3000);
		s_fWaterHeightEnd = (s_fWaterHeightEnd == 0) ? -random_range(0, 15) : 0;
		s_fWaterHeightBegin = s_fWaterHeightCurrent;
		s_dwLastHeightChangeTime = CTimer::Instance().GetCurrentMillisecond();
	}
	s_fWaterHeightCurrent = s_fWaterHeightBegin +
		(s_fWaterHeightEnd - s_fWaterHeightBegin) *
		((CTimer::Instance().GetCurrentMillisecond() - s_dwLastHeightChangeTime) / (float)s_dwBlendtime);

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	m_matWorldForCommonUse._43 = s_fWaterHeightCurrent;

	float fFogDistance = __GetFogDistance();

	for (auto i = m_PatchVector.begin(); i != m_PatchVector.end(); ++i)
		DrawWater(i->second);

	m_matWorldForCommonUse._43 = 0.0f;

	m_dx->SetShader(nullptr);
	STATEMANAGER.SetTexture(0, nullptr);

	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CMapOutdoor::DrawWater(long patchnum)
{
	assert(NULL!=m_pTerrainPatchProxyList);
	if (!m_pTerrainPatchProxyList)
		return;

	CTerrainPatchProxy& rkTerrainPatchProxy = m_pTerrainPatchProxyList[patchnum];

	if (!rkTerrainPatchProxy.isUsed())
		return;

	if (!rkTerrainPatchProxy.isWaterExists())
		return;

	auto pkVB = rkTerrainPatchProxy.GetWaterVertexBufferPointer();
	if (!pkVB)
		return;

	UINT uPriCount=rkTerrainPatchProxy.GetWaterFaceCount();
	if (!uPriCount)
		return;
	
	m_dx->SetVertexBuffer(pkVB);
	STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, uPriCount);

	ms_faceCount += uPriCount;
}
