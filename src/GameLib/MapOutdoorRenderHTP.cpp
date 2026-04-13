#include "StdAfx.h"
#include "MapOutdoor.h"

#include "EterLib/StateManager.h"

//step 3. include all new class 
#include "qMin32Lib/All.h"

//step 4. create new function 
void CMapOutdoor::BeginTerrainSplat(const float4x4& matTex0, const float4x4& matTex1,
	LPDIRECT3DTEXTURE9 diffuseTex, LPDIRECT3DTEXTURE9 alphaTex)
{
	//first thing to do is to set vertexdeclaration
	m_dx->SetVertexDeclaration(VD_PN);

	//secound step is to set the shaders
	auto shader = m_dx->GetShaderContained()->GetTerrain();
	m_dx->SetShader(shader);

	//after we set the shader we can send the light to shader
	m_light.SetToShader(shader);

	//now we can start to send the constant for vertexshader
	auto vsConstant = shader->GetConstantVs(); //get vertex shader constant table
	vsConstant.SetMatrix("g_mView", &mat.view); //send view matrix
	vsConstant.SetMatrix("g_mProj", &mat.proj); //send projection matrix
	vsConstant.SetMatrix("g_mTex0Transform", &matTex0); //send transform matrix for diffuse
	vsConstant.SetMatrix("g_mTex1Transform", &matTex1); //set transfor matrix for alpha

	//before we start with pixel shader constant we need to get fog , 
	float fogNear = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogNearDistance() : 5000.0f;
	float fogFar = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogFarDistance() : 10000.0f;

	//now let's make pixel shader constant 
	auto psConstant = shader->GetConstantPs(); //get pixel shader constant table
	psConstant.SetFloat("g_fFogNear", &fogNear); //send fog near value
	psConstant.SetFloat("g_fFogFar", &fogFar); //send fog far value
	D3DXCOLOR fogColor = mc_pEnvironmentData ?
		D3DXCOLOR(mc_pEnvironmentData->FogColor) : D3DXCOLOR(1, 1, 1, 1); //get color for fog -> white

	ColorStruct fogCol(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
	psConstant.SetColor("g_vFogColor", &fogCol); //send fog 

	//set light
	m_light.SetType((LightType)mc_pEnvironmentData->DirLights->Type);
	m_light.SetDirection(mc_pEnvironmentData->DirLights->Direction);
	m_light.SetPosition(mc_pEnvironmentData->DirLights->Position);
	m_light.SetDiffuse(mc_pEnvironmentData->DirLights->Diffuse);
	m_light.SetAmbient(mc_pEnvironmentData->DirLights->Ambient);
	m_light.SetSpecular(mc_pEnvironmentData->DirLights->Specular);

	//set texture for pixel shader 
	STATEMANAGER.SetTexture(0, diffuseTex);
	STATEMANAGER.SetTexture(1, alphaTex);
}

void CMapOutdoor::BeginTerrainShadow(const float4x4& staticShadow, const float4x4& dynamicShadow, LPDIRECT3DTEXTURE9 staticShadowTex, LPDIRECT3DTEXTURE9 dynamicShadowTex, bool useDynamicShadow)
{
	//like the function before,the same order 
	m_dx->SetVertexDeclaration(VD_PN);

	auto shader = m_dx->GetShaderContained()->GetShadow();
	m_dx->SetShader(shader);


	auto vsConstant = shader->GetConstantVs();
	vsConstant.SetMatrix("g_mView", &mat.view);
	vsConstant.SetMatrix("g_mProj", &mat.proj);
	vsConstant.SetMatrix("g_mStaticShadow", &staticShadow);
	vsConstant.SetMatrix("g_mDynamicShadow", &dynamicShadow);

	float fogNear = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogNearDistance() : 5000.0f;
	float fogFar = mc_pEnvironmentData ? mc_pEnvironmentData->GetFogFarDistance() : 10000.0f;
	D3DXCOLOR fogColor = mc_pEnvironmentData ?
		D3DXCOLOR(mc_pEnvironmentData->FogColor) : D3DXCOLOR(1, 1, 1, 1);

	ColorStruct fogCol(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
	auto psConstant = shader->GetConstantPs();
	psConstant.SetFloat("g_fFogNear", &fogNear);
	psConstant.SetFloat("g_fFogFar", &fogFar);
	psConstant.SetColor("g_vFogColor", &fogCol);
	int flag = useDynamicShadow ? 1 : 0;
	psConstant.SetInt("g_useDynamicShadow", &flag);
	STATEMANAGER.SetTexture(0, staticShadowTex);
	STATEMANAGER.SetTexture(1, useDynamicShadow ? dynamicShadowTex : nullptr);
}

void CMapOutdoor::EndTerrainShader()
{
	m_dx->SetShader(nullptr); //set shader to null after we finish to render terrain

	//set textures to null after we finish to render terrain
	STATEMANAGER.SetTexture(0, nullptr);
	STATEMANAGER.SetTexture(1, nullptr);
}

// in shaders we don t need Set/Get/SaveTransform, or Set/Get/SaveTextureStageState 
// because shader will handle those things for us, but we still need to set some render state
// let's start to delete them 
void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch()
{
	DWORD dwFogColor;
	float fFogFarDistance;
	float fFogNearDistance;
	if (mc_pEnvironmentData)
	{
		dwFogColor=mc_pEnvironmentData->FogColor;
		fFogNearDistance=mc_pEnvironmentData->GetFogNearDistance();
		fFogFarDistance=mc_pEnvironmentData->GetFogFarDistance();
	}
	else
	{
		dwFogColor=0xffffffff;
		fFogNearDistance=5000.0f;
		fFogFarDistance=10000.0f;
	}
	
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);

	CSpeedTreeWrapper::ms_bSelfShadowOn = true;
	STATEMANAGER.SetBestFiltering(0);
	STATEMANAGER.SetBestFiltering(1);

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;

	m_iRenderedSplatNumSqSum = 0;
	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;
	m_RenderedTextureNumVector.clear();

	std::pair<float, long> fog_far(fFogFarDistance+1600.0f, 0);
	std::pair<float, long> fog_near(fFogNearDistance-3200.0f, 0);

	if (mc_pEnvironmentData && mc_pEnvironmentData->bDensityFog)
		fog_far.first = 1e10f;

	std::vector<std::pair<float ,long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(),m_PatchVector.end(),fog_far);
	std::vector<std::pair<float ,long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(),m_PatchVector.end(),fog_near);

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();


	for(; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}
		
		__HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);

		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;
		
 		if (m_bDrawWireFrame)
			DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
	}

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for(it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for(it = far_it; it != m_PatchVector.end(); ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
 				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	std::sort(m_RenderedTextureNumVector.begin(),m_RenderedTextureNumVector.end());

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

	//now here we need to use EndTerrainShaders()
	EndTerrainShader();
}


void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplat(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain* pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	DWORD dwFogColor;
	if (mc_pEnvironmentData)
		dwFogColor = mc_pEnvironmentData->FogColor;
	else
		dwFogColor = 0xffffffff;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
	m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);

	D3DXMATRIX matTexTransform, matSplatAlphaTexTransform;

	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);

	auto pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	m_dx->SetVertexBuffer(pkVB);

	int iPrevRenderedSplatNum = m_iRenderedSplatNum;

	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	// before draw function we need to set shader,but we will use BeginTerrainSplat/Shadow() 
	// function to set shader and set constant for shader, and also set texture for shader,
	// so we need to call BeginTerrainSplat/Shadow()  before we call DrawIndexedPrimitive() function

	for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j)
	{
		TTerainSplat& rSplat = rTerrainSplatPatch.Splats[j];

		if (!rSplat.Active)
			continue;

		if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
			continue;

		const TTerrainTexture& rTexture = m_TextureSet.GetTexture(j);

		float4x4 matSplatColorTexTransform;

		D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);

		//here for terrain
		BeginTerrainSplat(matSplatColorTexTransform, matSplatAlphaTexTransform, rTexture.pd3dTexture, rSplat.pd3dTexture);

		STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

		if (std::find(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end(), (int)j) == m_RenderedTextureNumVector.end())
			m_RenderedTextureNumVector.push_back(j);

		++m_iRenderedSplatNum;
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;

	}

	if (m_bDrawShadow)
	{
		D3DXMATRIX matShadowTexTransform;
		D3DXMatrixMultiply(&matShadowTexTransform, &matTexTransform, &m_matStaticShadow);
		
		//here for shadow
		BeginTerrainShadow(matShadowTexTransform, m_matDynamicShadow, pTerrain->GetShadowTexture(), m_lpCharacterShadowMapTexture, m_bDrawChrShadow);

		STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

		STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

		STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		++m_iRenderedPatchNum;
	}
	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;
}

// now terrain is render with shader,
// so we don't need to care about render state and texture stage state for terrain,
// next step is to remove all the remain code for light and fog

void CMapOutdoor::__HardwareTransformPatch_RenderPatchNone(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL!=m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchNone");
	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	
	if (!pTerrainPatchProxy->isUsed())
		return;

	auto pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	m_dx->SetVertexBuffer(pkVB);

	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
}
