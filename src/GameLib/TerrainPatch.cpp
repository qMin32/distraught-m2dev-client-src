// TerrainPatch.cpp: implementation of the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TerrainPatch.h"

void CTerrainPatch::Clear()
{
	ClearID();
	SetUse(false);

	m_bWaterExist = false;
	m_bNeedUpdate = true;

	m_dwWaterPriCount = 0;
	m_byType = PATCH_TYPE_PLAIN;
	
	m_fMinX = m_fMaxX = m_fMinY = m_fMaxY = m_fMinZ = m_fMaxZ = 0.0f;

	m_dwVersion=0;
}

void CTerrainPatch::BuildWaterVertexBuffer(SWaterVertex* akSrcVertex, UINT uWaterVertexCount)
{
	m_WaterVertexBuffer = m_dx->CreateVertexBuffer(akSrcVertex, sizeof(SWaterVertex), uWaterVertexCount, D3DUSAGE_DYNAMIC);

	m_dwWaterPriCount = uWaterVertexCount / 3;
}
		
void CTerrainPatch::BuildTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex* akSrcVertex)
{
	m_kVB = m_dx->CreateVertexBuffer(akSrcVertex, sizeof(HardwareTransformPatch_SSourceVertex), TERRAIN_VERTEX_COUNT, D3DUSAGE_DYNAMIC);
}

UINT CTerrainPatch::GetWaterFaceCount()
{
	return m_dwWaterPriCount;
}

CTerrainPatchProxy::CTerrainPatchProxy()
{
	Clear();
}

CTerrainPatchProxy::~CTerrainPatchProxy()
{
	Clear();
}

void CTerrainPatchProxy::SetCenterPosition(const D3DXVECTOR3& c_rv3Center)
{
	m_v3Center=c_rv3Center;
}

bool CTerrainPatchProxy::IsIn(const D3DXVECTOR3& c_rv3Target, float fRadius)
{
	float dx=m_v3Center.x-c_rv3Target.x;
	float dy=m_v3Center.y-c_rv3Target.y;
	float fDist=dx*dx+dy*dy;
	float fCheck=fRadius*fRadius;

	if (fDist<fCheck)
		return true;

	return false;
}

RefPtr<CVertexBuffer> CTerrainPatchProxy::HardwareTransformPatch_GetVertexBufferPtr()
{
	if (m_pTerrainPatch)
		return m_pTerrainPatch->HardwareTransformPatch_GetVertexBufferPtr();

	return NULL;
}

UINT CTerrainPatchProxy::GetWaterFaceCount()
{
	if (m_pTerrainPatch)
		return m_pTerrainPatch->GetWaterFaceCount();
	
	return 0;
}

void CTerrainPatchProxy::Clear()
{
	m_bUsed = false;
	m_sPatchNum = 0;
	m_byTerrainNum = 0xFF;

	m_pTerrainPatch = NULL;
}
