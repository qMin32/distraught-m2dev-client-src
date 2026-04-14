#include "StdAfx.h"
#include "Eterlib/StateManager.h"
#include "ModelInstance.h"
#include "Model.h"
#include "../qMin32Lib/All.h"
#include "EterLib/Camera.h"

CNewLight      CGrannyModelInstance::ms_meshLight;

void CGrannyModelInstance::DeformNoSkin(const D3DXMATRIX* c_pWorldMatrix)
{
	if (IsEmpty())
		return;

	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);
}

void CGrannyModelInstance::BeginShaderRender(const RefPtr<CShaders>& shader)
{
	if (!m_pModel)
		return;

	m_dx->SetShader(shader);

	auto vsConst = shader->GetConstantVs();
	vsConst.SetMatrix("g_mView", &mat.view);
	vsConst.SetMatrix("g_mProj", &mat.proj);

	auto* cam = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 pos = cam->GetEye();

	auto psConst = shader->GetConstantPs();
	psConst.SetVector("g_vCameraPos", &pos);
	ms_meshLight.SetToShader(shader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// With One Texture
void CGrannyModelInstance::RenderWithOneTexture()
{
	if (IsEmpty())
		return;

	m_dx->SetVertexDeclaration(VD_PNT);

	// WORK
	auto lpd3dDeformPNTVtxBuf = GetDeformableVertexBuffer();
	// END_OF_WORK

	auto lpd3dRigidPNTVtxBuf = m_pModel->GetVertexBuffer();

	if (lpd3dDeformPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dDeformPNTVtxBuf);
		RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
	}
	if (lpd3dRigidPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dRigidPNTVtxBuf);
		RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
	}
	m_dx->SetShader(nullptr);
}

void CGrannyModelInstance::BlendRenderWithOneTexture()
{
	if (IsEmpty())
		return;

	// WORK
	auto lpd3dDeformPNTVtxBuf = GetDeformableVertexBuffer();
	// END_OF_WORK
	auto lpd3dRigidPNTVtxBuf = m_pModel->GetVertexBuffer();

	m_dx->SetVertexDeclaration(VD_PNT);

	if (lpd3dDeformPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dDeformPNTVtxBuf);
		RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
	}

	if (lpd3dRigidPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dRigidPNTVtxBuf);
		RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
	}
	m_dx->SetShader(nullptr);
}

// With Two Texture
void CGrannyModelInstance::RenderWithTwoTexture()
{
	// FIXME : Deform, Render, BlendRender를 묶어 상위에서 걸러주는 것이 더 나을 듯 - [levites]
	if (IsEmpty())
		return;

	m_dx->SetVertexDeclaration(VD_PNT2);

	// WORK
	auto lpd3dDeformPNTVtxBuf = GetDeformableVertexBuffer();
	// END_OF_WORK
	auto lpd3dRigidPNTVtxBuf = m_pModel->GetVertexBuffer();

	if (lpd3dDeformPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dDeformPNTVtxBuf);
		RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
	}
	if (lpd3dRigidPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dRigidPNTVtxBuf);
		RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
	}
	m_dx->SetShader(nullptr);
}

void CGrannyModelInstance::BlendRenderWithTwoTexture()
{
	if (IsEmpty())
		return;

	// WORK
	auto lpd3dDeformPNTVtxBuf = GetDeformableVertexBuffer();
	// END_OF_WORK
	auto lpd3dRigidPNTVtxBuf = m_pModel->GetVertexBuffer();

	m_dx->SetVertexDeclaration(VD_PNT2);

	if (lpd3dDeformPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dDeformPNTVtxBuf);
		RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
	}

	if (lpd3dRigidPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dRigidPNTVtxBuf);
		RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
	}
	m_dx->SetShader(nullptr);
}

void CGrannyModelInstance::RenderWithoutTexture()
{
	if (IsEmpty())
		return;

	m_dx->SetVertexDeclaration(VD_PNT);
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

	// WORK
	auto lpd3dDeformPNTVtxBuf = GetDeformableVertexBuffer();
	// END_OF_WORK
	auto lpd3dRigidPNTVtxBuf = m_pModel->GetVertexBuffer();

	if (lpd3dDeformPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dDeformPNTVtxBuf);
		RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
	}

	if (lpd3dRigidPNTVtxBuf)
	{
		m_dx->SetVertexBuffer(lpd3dRigidPNTVtxBuf);
		RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
	}
	m_dx->SetShader(nullptr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render Mesh List
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// With One Texture
void CGrannyModelInstance::RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	auto lpd3dIdxBuf = m_pModel->GetIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode* pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

	while (pMeshNode)
	{
		const CGrannyMesh* pMesh = pMeshNode->pMesh;
		int vtxMeshBasePos = pMesh->GetVertexBasePosition();
		m_dx->SetIndexBuffer(lpd3dIdxBuf);
		auto shader = m_dx->GetShaderContained()->GetMeshPnt();
		auto vsConst = shader->GetConstantVs();
		vsConst.SetMatrix("g_mWorld", &m_meshMatrices[pMeshNode->iMesh]);
		BeginShaderRender(shader);

		/////
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		int vtxCount = pMesh->GetVertexCount();

		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;

			// MR-12: Fix specular isolation issue
			CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);

			rkMtrl.SetConstantsToShader(shader);
			rkMtrl.ApplyRenderState();
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtxMeshBasePos, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
			rkMtrl.RestoreRenderState();

			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}
		/////

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}

// With Two Texture
void CGrannyModelInstance::RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	auto lpd3dIdxBuf = m_pModel->GetIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode* pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

	while (pMeshNode)
	{
		const CGrannyMesh* pMesh = pMeshNode->pMesh;
		int vtxMeshBasePos = pMesh->GetVertexBasePosition();
		m_dx->SetIndexBuffer(lpd3dIdxBuf);
		auto shader = m_dx->GetShaderContained()->GetMeshPnt2();
		auto vsConst = shader->GetConstantVs();
		vsConst.SetMatrix("g_mWorld", &m_meshMatrices[pMeshNode->iMesh]);
		BeginShaderRender(shader);

		/////
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		int vtxCount = pMesh->GetVertexCount();
		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;

			CGrannyMaterial& rkMtrl = m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
			rkMtrl.SetConstantsToShader(shader);
			STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
			STATEMANAGER.SetTexture(1, rkMtrl.GetD3DTexture(1));

			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtxMeshBasePos, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}
		/////

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}

// Without Texture
void CGrannyModelInstance::RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	auto lpd3dIdxBuf = m_pModel->GetIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode* pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);

	while (pMeshNode)
	{
		const CGrannyMesh* pMesh = pMeshNode->pMesh;
		int vtxMeshBasePos = pMesh->GetVertexBasePosition();

		m_dx->SetIndexBuffer(lpd3dIdxBuf);
		auto shader = m_dx->GetShaderContained()->GetMeshPnt();
		auto vsConst = shader->GetConstantVs();
		vsConst.SetMatrix("g_mWorld", &m_meshMatrices[pMeshNode->iMesh]);
		BeginShaderRender(shader);

		/////
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		int vtxCount = pMesh->GetVertexCount();

		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtxMeshBasePos, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}
		/////

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}
