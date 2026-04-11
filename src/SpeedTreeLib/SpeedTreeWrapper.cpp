
#pragma warning(disable:4786)

///////////////////////////////////////////////////////////////////////  
//	Include Files
#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include "EterBase/Debug.h"
#include "EterBase/Timer.h"
#include "EterBase/Filename.h"
#include "EterLib/ResourceManager.h"
#include "EterLib/Camera.h"
#include "EterLib/StateManager.h"

#include "SpeedTreeConfig.h"
#include "SpeedTreeForestDirectX.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

#include <filesystem>
#include "qMin32Lib/All.h"

using namespace std;

LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_pLeafVertexShader = nullptr;
bool CSpeedTreeWrapper::ms_bSelfShadowOn = true;

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::CSpeedTreeWrapper
CSpeedTreeWrapper::CSpeedTreeWrapper() :
	m_pSpeedTree(new CSpeedTreeRT),
	m_bIsInstance(false),
	m_pInstanceOf(NULL),
	m_pGeometryCache(NULL),
	m_usNumLeafLods(0),
	m_pLeavesUpdatedByCpu(NULL),
	m_unBranchVertexCount(0),
	m_unFrondVertexCount(0),
	m_pTextureInfo(NULL)
{
	// set initial position
	m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;

	m_pSpeedTree->SetWindStrength(1.0f);
	m_pSpeedTree->SetLocalMatrices(0, 4);
}

void CSpeedTreeWrapper::SetVertexShaders(LPDIRECT3DVERTEXSHADER9 pVertexShader)
{
	ms_pLeafVertexShader = pVertexShader;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
	if (!!ms_pLeafVertexShader)
		CSpeedTreeForestDirectX::Instance().EnsureVertexShaders();

	CSpeedTreeForestDirectX::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);

	m_pSpeedTree->SetLodLevel(1.0f);
	//Advance();

	CSpeedTreeForestDirectX::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), mat.view, mat.proj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	DWORD dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	DWORD dwAlphaBlendEnable = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	// choose fixed function pipeline or custom shader for fronds and branches
	m_dx->SetVertexDeclaration(VD_BRANCH);

	{
		LPDIRECT3DTEXTURE9 lpd3dTexture;

		// set texture map
		if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
			STATEMANAGER.SetTexture(0, lpd3dTexture);

		if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
		{
			// activate the branch vertex buffer
			m_dx->SetVertexBuffer(m_pBranchVertexBuffer);
			// set the index buffer
			m_dx->SetIndexBuffer(m_pBranchIndexBuffer);
		}
	}

	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	{
		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
		{
			// activate the frond vertex buffer
			m_dx->SetVertexBuffer(m_pFrondVertexBuffer);
			// set the index buffer
			m_dx->SetIndexBuffer(m_pFrondIndexBuffer);
		}
	}
	RenderFronds();

	if (ms_pLeafVertexShader)
	{
		m_dx->SetVertexDeclaration(VD_LEAF);

		STATEMANAGER.SaveVertexShader(ms_pLeafVertexShader);

		{
			UploadLeafTables(c_nVertexShader_LeafTables);

			if (!m_CompositeImageInstance.IsEmpty())
				STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
		}
		RenderLeaves();
		STATEMANAGER.RestoreVertexShader();
	}
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}

void CSpeedTreeWrapper::OnRender()
{
	if (!ms_pLeafVertexShader)
		CSpeedTreeForestDirectX::Instance().EnsureVertexShaders();

	CSpeedTreeForestDirectX::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);

	m_pSpeedTree->SetLodLevel(1.0f);

	CSpeedTreeForestDirectX::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), mat.view, mat.proj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);

	m_dx->SetVertexDeclaration(VD_BRANCH);
	SetupBranchForTreeType();
	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	SetupFrondForTreeType();
	RenderFronds();

	if (ms_pLeafVertexShader)
	{
		m_dx->SetVertexDeclaration(VD_LEAF);

		STATEMANAGER.SaveVertexShader(ms_pLeafVertexShader);

		SetupLeafForTreeType();
		RenderLeaves();
		STATEMANAGER.RestoreVertexShader();
	}
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper()
{
	// if this is not an instance, clean up
	if (!m_bIsInstance)
	{
		for (short i = 0; i < m_usNumLeafLods; ++i)
		{
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

			if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
			{
				if (i >= 0 && (size_t)i < m_pLeafVertexBuffer.size())
					m_pLeafVertexBuffer[(size_t)i].reset();
			}
		}

		SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);

		SAFE_DELETE(m_pTextureInfo);

		SAFE_DELETE(m_pGeometryCache);
	}

	// always delete the speedtree
	SAFE_DELETE(m_pSpeedTree);

	Clear();
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::LoadTree
bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, const BYTE* c_pbBlock, unsigned int uiBlockSize, UINT nSeed, float fSize, float fSizeVariance)
{
	bool bSuccess = false;

	m_pSpeedTree->SetTextureFlip(true);

	// load the tree file
	if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
	{
		if (!m_pSpeedTree->LoadTree(pszSptFile))
		{
			TraceError("SpeedTreeRT Error: %s", CSpeedTreeRT::GetCurrentError());
			return false;
		}
	}

	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);

	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);

	m_pSpeedTree->SetNumLeafRockingGroups(1);

	// override the size, if necessary
	if (fSize >= 0.0f && fSizeVariance >= 0.0f)
		m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

	// generate tree geometry
	if (m_pSpeedTree->Compute(NULL, nSeed, false))
	{
		// get the dimensions
		m_pSpeedTree->GetBoundingBox(m_afBoundingBox);

		// make the leaves rock in the wind
		m_pSpeedTree->SetLeafRockingState(true);

		// billboard setup
		CSpeedTreeRT::SetDropToBillboard(true);

		// query & set materials
		m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial());
		m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial());
		m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial());

		// adjust lod distances
		float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
		m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);

		// query textures
		m_pTextureInfo = new CSpeedTreeRT::STextures;
		m_pSpeedTree->GetTextures(*m_pTextureInfo);

		std::filesystem::path path = pszSptFile;
		path = path.parent_path();

		auto branchTexture = path / m_pTextureInfo->m_pBranchTextureFilename;
		branchTexture.replace_extension(".dds");

		// load branch textures
		LoadTexture(branchTexture.generic_string().c_str(), m_BranchImageInstance);

		auto selfShadowTexture = path / m_pTextureInfo->m_pSelfShadowFilename;
		selfShadowTexture.replace_extension(".dds");

		if (m_pTextureInfo->m_pSelfShadowFilename != NULL)
			LoadTexture(selfShadowTexture.generic_string().c_str(), m_ShadowImageInstance);

		auto compositeTexture = path / m_pTextureInfo->m_pCompositeFilename;
		compositeTexture.replace_extension(".dds");

		if (m_pTextureInfo->m_pCompositeFilename)
			LoadTexture(compositeTexture.generic_string().c_str(), m_CompositeImageInstance);

		// setup the index and vertex buffers
		SetupBuffers();

		// everything appeared to go well
		bSuccess = true;
	}
	else // tree failed to compute
		fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError());

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
	// read all the geometry for highest LOD into the geometry cache (just a precaution, it's updated later)
	m_pSpeedTree->SetLodLevel(1.0f);

	if (m_pGeometryCache == NULL)
		m_pGeometryCache = new CSpeedTreeRT::SGeometry;

	m_pSpeedTree->GetGeometry(*m_pGeometryCache);

	// setup the buffers for each part
	SetupBranchBuffers();
	SetupFrondBuffers();
	SetupLeafBuffers();
}

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);
	CSpeedTreeRT::SGeometry::SIndexed* pBranches = &m_pGeometryCache->m_sBranches;

	m_unBranchVertexCount = pBranches->m_usVertexCount;
	if (m_unBranchVertexCount <= 1)
		return;

	std::vector<SFVFBranchVertex> vtx;
	vtx.resize(m_unBranchVertexCount);

	SFVFBranchVertex* pVertexBuffer = vtx.data();

	for (UINT i = 0; i < m_unBranchVertexCount; ++i)
	{
		std::memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

		pVertexBuffer->m_dwDiffuseColor = pBranches->m_pColors[i];

		pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
		pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

		pVertexBuffer->m_fShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
		pVertexBuffer->m_fShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];

		++pVertexBuffer;
	}

	m_pBranchVertexBuffer = m_dx->CreateVertexBuffer(nullptr, sizeof(SFVFBranchVertex), m_unBranchVertexCount, D3DUSAGE_WRITEONLY);
	if (!m_pBranchVertexBuffer)
		return;

	if (!m_pBranchVertexBuffer->Update(vtx.data(), m_unBranchVertexCount))
		return;

	const uint32_t unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();
	m_branchStripOffsets.clear();
	m_branchStripLengths.clear();
	if (unNumLodLevels > 0)
		m_branchStripLengths.resize(unNumLodLevels);

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);
	pBranches = &m_pGeometryCache->m_sBranches;

	const uint32_t stripCount = pBranches->m_usNumStrips;
	uint32_t totalIndexCount = 0;

	if (stripCount > 0)
	{
		m_branchStripOffsets.resize(stripCount);
		for (uint32_t s = 0; s < stripCount; ++s)
		{
			m_branchStripOffsets[s] = totalIndexCount;
			totalIndexCount += pBranches->m_pStripLengths[s];
		}
	}

	for (uint32_t i = 0; i < unNumLodLevels; ++i)
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, i);
		pBranches = &m_pGeometryCache->m_sBranches;

		auto& lengths = m_branchStripLengths[i];
		lengths.assign(stripCount, 0);

		const uint32_t lodStripCount = pBranches->m_usNumStrips;
		for (uint32_t s = 0; s < stripCount && s < lodStripCount; ++s)
		{
			lengths[s] = pBranches->m_pStripLengths[s];
		}
	}

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);
	pBranches = &m_pGeometryCache->m_sBranches;

	if (totalIndexCount > 0)
	{
		std::vector<uint16_t> tmp;
		tmp.resize(totalIndexCount);

		uint32_t cursor = 0;
		for (uint32_t s = 0; s < stripCount; ++s)
		{
			const uint32_t length = pBranches->m_pStripLengths[s];
			std::memcpy(tmp.data() + cursor, pBranches->m_pStrips[s], length * sizeof(uint16_t));
			cursor += length;
		}

		m_pBranchIndexBuffer = m_dx->CreateIndexBuffer(tmp.data(), (UINT)tmp.size());
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
	// reference to frond structure
	CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
	m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

	if (m_unFrondVertexCount <= 1)
		return;

	std::vector<SFVFBranchVertex> vtx;
	vtx.resize(m_unFrondVertexCount);

	SFVFBranchVertex* pVertexBuffer = vtx.data();

	for (UINT i = 0; i < m_unFrondVertexCount; ++i)
	{
		// position
		std::memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

		pVertexBuffer->m_dwDiffuseColor = pFronds->m_pColors[i];

		// texcoords for layer 0
		pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
		pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

		// texcoords for layer 1 (if enabled)
		pVertexBuffer->m_fShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
		pVertexBuffer->m_fShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];

		++pVertexBuffer;
	}
	m_pFrondVertexBuffer = m_dx->CreateVertexBuffer(nullptr, sizeof(SFVFBranchVertex), m_unFrondVertexCount, D3DUSAGE_WRITEONLY);
	if (!m_pFrondVertexBuffer)
		return;

	if (!m_pFrondVertexBuffer->Update(vtx.data(), m_unFrondVertexCount))
		return;
	const uint32_t unNumLodLevels = m_pSpeedTree->GetNumFrondLodLevels();
	m_frondStripOffsets.clear();
	m_frondStripLengths.clear();
	if (unNumLodLevels > 0)
		m_frondStripLengths.resize(unNumLodLevels);

	// set LOD0 for strip offsets/index buffer sizing
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
	const uint32_t stripCount = pFronds->m_usNumStrips;
	uint32_t totalIndexCount = 0;
	if (stripCount > 0)
	{
		m_frondStripOffsets.resize(stripCount);
		for (uint32_t s = 0; s < stripCount; ++s)
		{
			m_frondStripOffsets[s] = totalIndexCount;
			totalIndexCount += pFronds->m_pStripLengths[s];
		}
	}

	for (uint32_t j = 0; j < unNumLodLevels; ++j)
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, j);
		auto& lengths = m_frondStripLengths[j];
		lengths.assign(stripCount, 0);
		const uint32_t lodStripCount = pFronds->m_usNumStrips;
		for (uint32_t s = 0; s < stripCount && s < lodStripCount; ++s)
		{
			lengths[s] = pFronds->m_pStripLengths[s];
		}
	}
	// go back to highest LOD for buffer fill
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);

	if (totalIndexCount > 0)
	{
		std::vector<uint16_t> tmp;
		tmp.resize(totalIndexCount);

		uint32_t cursor = 0;
		for (uint32_t s = 0; s < stripCount; ++s)
		{
			const uint32_t length = pFronds->m_pStripLengths[s];
			std::memcpy(tmp.data() + cursor, pFronds->m_pStrips[s], length * sizeof(uint16_t));
			cursor += length;
		}
		m_pFrondIndexBuffer = m_dx->CreateIndexBuffer(tmp.data(), (UINT)tmp.size());
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
	const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };

	m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();

	m_pLeafVertexBuffer.clear();
	m_pLeafVertexBuffer.resize(m_usNumLeafLods);

	delete[] m_pLeavesUpdatedByCpu;
	m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods] {};

	for (UINT unLod = 0; unLod < m_usNumLeafLods; ++unLod)
	{
		m_pLeavesUpdatedByCpu[unLod] = false;
		m_pLeafVertexBuffer[unLod].reset();

		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, unLod);

		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &m_pGeometryCache->m_sLeaves0;
		const unsigned short usLeafCount = pLeaf->m_usLeafCount;
		if (usLeafCount < 1)
			continue;

		std::vector<SFVFLeafVertex> vtx;
		vtx.resize((size_t)usLeafCount * 6);

		SFVFLeafVertex* pVertex = vtx.data();

		for (UINT unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
		{
			const float cx = pLeaf->m_pCenterCoords[unLeaf * 3 + 0];
			const float cy = pLeaf->m_pCenterCoords[unLeaf * 3 + 1];
			const float cz = pLeaf->m_pCenterCoords[unLeaf * 3 + 2];

#ifndef WRAPPER_USE_GPU_LEAF_PLACEMENT
			const float s = 0.05f * m_pSpeedTree->GetLeafLodSizeAdjustments()[unLod];
			D3DXVECTOR3 corners[4] =
			{
				D3DXVECTOR3(cx - s, cy - s, cz),
				D3DXVECTOR3(cx + s, cy - s, cz),
				D3DXVECTOR3(cx + s, cy + s, cz),
				D3DXVECTOR3(cx - s, cy + s, cz),
			};
#endif

			for (UINT unVert = 0; unVert < 6; ++unVert)
			{
#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
				pVertex->m_vPosition.x = cx;
				pVertex->m_vPosition.y = cy;
				pVertex->m_vPosition.z = cz;
#else
				const short ci = anVertexIndices[unVert];
				pVertex->m_vPosition = corners[ci];
#endif

#ifdef WRAPPER_USE_DYNAMIC_LIGHTING
				if (pLeaf->m_pNormals)
					memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
				else
					pVertex->m_vNormal = D3DXVECTOR3(0, 0, 1);
#else
				pVertex->m_dwDiffuseColor = pLeaf->m_pColors ? pLeaf->m_pColors[unLeaf] : 0xffffffff;
#endif

				if (pLeaf->m_pLeafMapTexCoords)
				{
					memcpy(pVertex->m_fTexCoords,
						&(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]),
						2 * sizeof(float));
				}
				else
				{
					pVertex->m_fTexCoords[0] = 0.0f;
					pVertex->m_fTexCoords[1] = 0.0f;
				}

#ifdef WRAPPER_USE_GPU_WIND
				if (pLeaf->m_pWindMatrixIndices)
					pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
				else
					pVertex->m_fWindIndex = 0.0f;

				if (pLeaf->m_pWindWeights)
					pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
				else
					pVertex->m_fWindWeight = 0.0f;
#endif

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
				if (pLeaf->m_pLeafClusterIndices)
					pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
				else
					pVertex->m_fLeafPlacementIndex = (float)c_nVertexShader_LeafTables;

				pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments()[unLod];
#endif

				++pVertex;
			}
		}

#ifndef WRAPPER_USE_CPU_LEAF_PLACEMENT
		DWORD vbUsage = D3DUSAGE_WRITEONLY;
#else
		DWORD vbUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
#endif

		m_pLeafVertexBuffer[unLod] =
			m_dx->CreateVertexBuffer(vtx.data(), sizeof(SFVFLeafVertex), usLeafCount * 6, vbUsage);

		if (!m_pLeafVertexBuffer[unLod] || !m_pLeafVertexBuffer[unLod]->Get())
		{
			m_pLeafVertexBuffer[unLod].reset();
			continue;
		}
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
	m_pSpeedTree->ComputeLodLevel();
	m_pSpeedTree->SetLodLevel(1.0f);
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::MakeInstance
CSpeedTreeWrapper::SpeedTreeWrapperPtr CSpeedTreeWrapper::MakeInstance()
{
	auto spInstance = std::make_shared<CSpeedTreeWrapper>();

	// make an instance of this object's SpeedTree
	spInstance->m_bIsInstance = true;

	SAFE_DELETE(spInstance->m_pSpeedTree);
	spInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance();

	if (spInstance->m_pSpeedTree)
	{
		// use the same materials
		spInstance->m_cBranchMaterial = m_cBranchMaterial;
		spInstance->m_cLeafMaterial = m_cLeafMaterial;
		spInstance->m_cFrondMaterial = m_cFrondMaterial;
		spInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
		spInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());

		if (!m_ShadowImageInstance.IsEmpty())
			spInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());

		spInstance->m_pTextureInfo = m_pTextureInfo;

		// use the same geometry cache
		spInstance->m_pGeometryCache = m_pGeometryCache;

		// use the same buffers
		spInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
		spInstance->m_branchStripOffsets = m_branchStripOffsets;
		spInstance->m_branchStripLengths = m_branchStripLengths;
		spInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
		spInstance->m_unBranchVertexCount = m_unBranchVertexCount;

		spInstance->m_pFrondIndexBuffer = m_pFrondIndexBuffer;
		spInstance->m_frondStripOffsets = m_frondStripOffsets;
		spInstance->m_frondStripLengths = m_frondStripLengths;
		spInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
		spInstance->m_unFrondVertexCount = m_unFrondVertexCount;

		spInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
		spInstance->m_usNumLeafLods = m_usNumLeafLods;
		spInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;

		// new stuff
		memcpy(spInstance->m_afPos, m_afPos, 3 * sizeof(float));
		memcpy(spInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
		spInstance->m_pInstanceOf = shared_from_this();
		m_vInstances.push_back(spInstance);
	}
	else
	{
		fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError());
		spInstance.reset();
	}

	return spInstance;
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::GetInstances
std::vector <CSpeedTreeWrapper::SpeedTreeWrapperPtr> CSpeedTreeWrapper::GetInstances(UINT& nCount)
{
	std::vector <SpeedTreeWrapperPtr> kResult;

	nCount = m_vInstances.size();
	if (nCount)
	{
		for (auto it : m_vInstances)
		{
			kResult.push_back(it);
		}
	}

	return kResult;
}

void CSpeedTreeWrapper::DeleteInstance(SpeedTreeWrapperPtr pInstance)
{
	auto itor = m_vInstances.begin();

	while (itor != m_vInstances.end())
	{
		if (*itor == pInstance)
		{
			itor = m_vInstances.erase(itor);
		}
		else
			++itor;
	}
}

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupBranchForTreeType

void CSpeedTreeWrapper::SetupBranchForTreeType(void) const
{
	LPDIRECT3DTEXTURE9 lpd3dTexture;

	// set texture map
	if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(0, lpd3dTexture);

	// bind shadow texture
	if (ms_bSelfShadowOn && (lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(1, lpd3dTexture);
	else
		STATEMANAGER.SetTexture(1, NULL);

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{
		// activate the branch vertex buffer
		m_dx->SetVertexBuffer(m_pBranchVertexBuffer);
		// set the index buffer
		m_dx->SetIndexBuffer(m_pBranchIndexBuffer);
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::RenderBranches

void CSpeedTreeWrapper::RenderBranches(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0 && m_pBranchIndexBuffer && !m_branchStripLengths.empty() && !m_branchStripOffsets.empty())
	{
		const int lod = m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel;
		if (lod < 0 || static_cast<size_t>(lod) >= m_branchStripLengths.size())
			return;

		PositionTree();

		// set alpha test value
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));

		const auto& lengths = m_branchStripLengths[lod];
		const size_t stripCount = lengths.size() < m_branchStripOffsets.size() ? lengths.size() : m_branchStripOffsets.size();
		for (size_t s = 0; s < stripCount; ++s)
		{
			const uint16_t stripLength = lengths[s];
			if (stripLength > 2)
			{
				ms_faceCount += stripLength - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, m_branchStripOffsets[s], stripLength - 2);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupFrondForTreeType

void CSpeedTreeWrapper::SetupFrondForTreeType(void) const
{
	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	// bind shadow texture
	LPDIRECT3DTEXTURE9 lpd3dTexture;

	if ((lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(1, lpd3dTexture);

	if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
	{
		// activate the frond vertex buffer
		m_dx->SetVertexBuffer(m_pFrondVertexBuffer);
		// set the index buffer
		m_dx->SetIndexBuffer(m_pFrondIndexBuffer);
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::RenderFronds

void CSpeedTreeWrapper::RenderFronds(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

	if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0 && m_pFrondIndexBuffer && !m_frondStripLengths.empty() && !m_frondStripOffsets.empty())
	{
		const int lod = m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel;
		if (lod < 0 || static_cast<size_t>(lod) >= m_frondStripLengths.size())
			return;

		PositionTree();

		// set alpha test value
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

		const auto& lengths = m_frondStripLengths[lod];
		const size_t stripCount = lengths.size() < m_frondStripOffsets.size() ? lengths.size() : m_frondStripOffsets.size();
		for (size_t s = 0; s < stripCount; ++s)
		{
			const uint16_t stripLength = lengths[s];
			if (stripLength > 2)
			{
				ms_faceCount += stripLength - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, m_frondStripOffsets[s], stripLength - 2);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetupLeafForTreeType

void CSpeedTreeWrapper::SetupLeafForTreeType(void) const
{
	// pass leaf tables to shader
	UploadLeafTables(c_nVertexShader_LeafTables);

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	// bind shadow texture
	STATEMANAGER.SetTexture(1, NULL);
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::UploadLeafTables

void CSpeedTreeWrapper::UploadLeafTables(UINT uiLocation) const
{
	// query leaf cluster table from RT
	UINT uiEntryCount = 0;
	const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);

	// upload for vertex shader use
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafTables, pTable, uiEntryCount / 4);
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::RenderLeaves

void CSpeedTreeWrapper::RenderLeaves(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

	if (m_pLeafVertexBuffer.empty() || m_usNumLeafLods == 0)
		return;

	const int maxLeafLod = static_cast<int>(m_usNumLeafLods);

	PositionTree();

	for (UINT unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
	{
		const CSpeedTreeRT::SGeometry::SLeaf* pLeaf =
			(unLeafLevel == 0) ? &m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;

		if (!pLeaf || !pLeaf->m_bIsActive || pLeaf->m_usLeafCount == 0)
			continue;

		const int unLod = pLeaf->m_nDiscreteLodLevel;
		if (unLod < 0 || unLod >= maxLeafLod)
			continue;

		if ((size_t)unLod >= m_pLeafVertexBuffer.size())
			continue;

		if (!m_pLeafVertexBuffer[unLod] || !m_pLeafVertexBuffer[unLod]->Get())
			continue;

		m_dx->SetVertexBuffer(m_pLeafVertexBuffer[unLod]);
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

		ms_faceCount += pLeaf->m_usLeafCount * 2;
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
	}
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::EndLeafForTreeType

void CSpeedTreeWrapper::EndLeafForTreeType(void)
{
	if (!m_pLeavesUpdatedByCpu)
		return;

	// reset copy flags for CPU wind
	for (UINT i = 0; i < m_usNumLeafLods; ++i)
		m_pLeavesUpdatedByCpu[i] = false;
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::RenderBillboards

void CSpeedTreeWrapper::RenderBillboards(void) const
{

	if (!m_CompositeImageInstance.IsEmpty())
		STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

	PositionTree();

	struct SBillboardVertex
	{
		float fX, fY, fZ;
		float fU, fV;
	};

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

	if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};

		m_dx->SetVertexDeclaration(VD_PT);
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

	// if tree supports 360 degree billboards, render the second
	if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}
}

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
	if (!m_bIsInstance)
		m_pSpeedTree->DeleteTransientData();
}

///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::PositionTree

void CSpeedTreeWrapper::PositionTree(void) const
{
	D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
	D3DXMATRIX matTranslation;
	D3DXMatrixIdentity(&matTranslation);
	D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);

	// store translation for client-side transformation
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);

	// store translation for use in vertex shader
	D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::LoadTexture

bool CSpeedTreeWrapper::LoadTexture(const char* pFilename, CGraphicImageInstance& rImage)
{
	CResource* pResource = CResourceManager::Instance().GetResourcePointer(pFilename);
	rImage.SetImagePointer(static_cast<CGraphicImage*>(pResource));

	if (rImage.IsEmpty())
		return false;

	//TraceError("SpeedTreeWrapper::LoadTexture: %s", pFilename);
	return true;
}


///////////////////////////////////////////////////////////////////////  
//	CSpeedTreeWrapper::SetShaderConstants

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
	const float afUsefulConstants[] =
	{
		m_pSpeedTree->GetLeafLightingAdjustment(), 0.0f, 0.0f, 0.0f,
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

	const float afMaterial[] =
	{
		pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
			pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
	};

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
	m_afPos[0] = x;
	m_afPos[1] = y;
	m_afPos[2] = z;
	m_pSpeedTree->SetTreePosition(x, y, z);
	CGraphicObjectInstance::SetPosition(x, y, z);
}

bool CSpeedTreeWrapper::GetBoundingSphere(D3DXVECTOR3& v3Center, float& fRadius)
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	v3Center.x = 0.0f;
	v3Center.y = 0.0f;
	v3Center.z = fZ * 0.5f;

	fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f; // 0.9f for reduce size

	D3DXVECTOR3 vec = m_pSpeedTree->GetTreePosition();

	v3Center += vec;

	return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	m_v3BBoxMin.x = -fX / 2.0f;
	m_v3BBoxMin.y = -fY / 2.0f;
	m_v3BBoxMin.z = 0.0f;
	m_v3BBoxMax.x = fX / 2.0f;
	m_v3BBoxMax.y = fY / 2.0f;
	m_v3BBoxMax.z = fZ;

	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX& c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}

// collision detection routines
UINT CSpeedTreeWrapper::GetCollisionObjectCount()
{
	assert(m_pSpeedTree);
	return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	assert(m_pSpeedTree);
	m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}


const float* CSpeedTreeWrapper::GetPosition()
{
	return m_afPos;
}

void CSpeedTreeWrapper::GetTreeSize(float& r_fSize, float& r_fVariance)
{
	m_pSpeedTree->GetTreeSize(r_fSize, r_fVariance);
}

// pscdVector may be null
void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector* /*pscdVector*/)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, m_afPos[0], m_afPos[1], m_afPos[2]);

	/////
	for (UINT i = 0; i < GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType ObjectType;
		CStaticCollisionData CollisionData;

		GetCollisionObject(i, ObjectType, (float*)&CollisionData.v3Position, CollisionData.fDimensions);

		if (ObjectType == CSpeedTreeRT::CO_BOX)
			continue;

		switch (ObjectType)
		{
		case CSpeedTreeRT::CO_SPHERE:
			CollisionData.dwType = COLLISION_TYPE_SPHERE;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

		case CSpeedTreeRT::CO_CYLINDER:
			CollisionData.dwType = COLLISION_TYPE_CYLINDER;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0] /** fSizeRatio*/;
			CollisionData.fDimensions[1] = CollisionData.fDimensions[1] /** fSizeRatio*/;
			//AddCollision(&CollisionData);
			break;

			/*case CSpeedTreeRT::CO_BOX:
			break;*/
		}
		AddCollision(&CollisionData, &mat);
	}
}

