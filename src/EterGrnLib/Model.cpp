#include "StdAfx.h"
#include "Model.h"
#include "Mesh.h"
#include "qMin32Lib/all.h"

const CGrannyMaterialPalette& CGrannyModel::GetMaterialPalette() const
{
	return m_kMtrlPal;
}

const CGrannyModel::TMeshNode* CGrannyModel::GetMeshNodeList(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType) const
{
	return m_meshNodeLists[eMeshType][eMtrlType];
}

CGrannyMesh * CGrannyModel::GetMeshPointer(int iMesh)
{
	assert(CheckMeshIndex(iMesh));
	assert(m_meshs != NULL);

	return m_meshs + iMesh;
}

const CGrannyMesh* CGrannyModel::GetMeshPointer(int iMesh) const
{
	assert(CheckMeshIndex(iMesh));
	assert(m_meshs != NULL);

	return m_meshs + iMesh;
}

bool CGrannyModel::CanDeformPNTVertices() const
{
	return m_canDeformPNVertices;
}

void CGrannyModel::DeformPNTVertices(void * dstBaseVertices, D3DXMATRIX * boneMatrices, const std::vector<granny_mesh_binding*>& c_rvct_pgrnMeshBinding) const
{
	int meshCount = GetMeshCount();

	for (int iMesh = 0; iMesh < meshCount; ++iMesh)
	{
		assert(iMesh < c_rvct_pgrnMeshBinding.size());

		CGrannyMesh & rMesh = m_meshs[iMesh];
		if (rMesh.CanDeformPNTVertices())
			rMesh.DeformPNTVertices(dstBaseVertices, boneMatrices, c_rvct_pgrnMeshBinding[iMesh]);
	}
}

int CGrannyModel::GetRigidVertexCount() const
{
	return m_rigidVtxCount;
}

int CGrannyModel::GetDeformVertexCount() const
{
	return m_deformVtxCount;
}

int CGrannyModel::GetVertexCount() const
{
	return m_vtxCount;
}

int CGrannyModel::GetMeshCount() const
{
	return m_pgrnModel ? m_pgrnModel->MeshBindingCount : 0;
}

granny_model* CGrannyModel::GetGrannyModelPointer()
{
	return m_pgrnModel;
}

RefPtr<CIndexBuffer> CGrannyModel::GetIndexBuffer() const
{
	return m_idxBuf;
}

RefPtr<CVertexBuffer> CGrannyModel::GetVertexBuffer() const
{
	return m_pntVtxBuf;
}

bool CGrannyModel::LoadVertices()
{
	if (m_rigidVtxCount <= 0)
		return true;

	assert(m_meshs != NULL);

	bool hasPNT2 = false;

	for (int i = 0; i < m_pgrnModel->MeshBindingCount; ++i)
	{
		if (m_meshs[i].IsPNT2())
		{
			hasPNT2 = true;
			break;
		}
	}

	if (hasPNT2)
	{
		m_stride = sizeof(TPNT2Vertex);
		std::vector<TPNT2Vertex> vertices(m_rigidVtxCount);

		for (int i = 0; i < m_pgrnModel->MeshBindingCount; ++i)
			m_meshs[i].LoadVertices(vertices.data());

		m_pntVtxBuf = m_dx->CreateVertexBuffer(vertices.data(), m_stride, m_rigidVtxCount);
	}
	else
	{
		m_stride = sizeof(TPNTVertex);
		std::vector<TPNTVertex> vertices(m_rigidVtxCount);

		for (int i = 0; i < m_pgrnModel->MeshBindingCount; ++i)
			m_meshs[i].LoadVertices(vertices.data());

		m_pntVtxBuf = m_dx->CreateVertexBuffer(vertices.data(), m_stride, m_rigidVtxCount);
	}

	return true;
}

bool CGrannyModel::LoadIndices()
{
	if (m_idxCount <= 0)
		return true;

	std::vector<WORD> m_ownIndices(m_idxCount);

	for (int m = 0; m < m_pgrnModel->MeshBindingCount; ++m)
	{
		CGrannyMesh& rMesh = m_meshs[m];
		rMesh.LoadIndices(m_ownIndices.data());
	}

	m_idxBuf = m_dx->CreateIndexBuffer(m_ownIndices.data(), (UINT)m_ownIndices.size());
	return true;
}

bool CGrannyModel::LoadMeshs()
{
	assert(m_meshs == NULL);
	assert(m_pgrnModel != NULL);

	if (m_pgrnModel->MeshBindingCount <= 0)
		return true;

	granny_skeleton* pgrnSkeleton = m_pgrnModel->Skeleton;

	int vtxRigidPos = 0;
	int vtxDeformPos = 0;
	int vtxPos = 0;
	int idxPos = 0;

	int diffusePNTMeshNodeCount = 0;
	int blendPNTMeshNodeCount = 0;
	int blendPNT2MeshNodeCount = 0;

	int meshCount = GetMeshCount();
	m_meshs = new CGrannyMesh[meshCount];
	m_stride = 0;

	for (int m = 0; m < meshCount; ++m)
	{
		CGrannyMesh& rMesh = m_meshs[m];
		granny_mesh* pgrnMesh = m_pgrnModel->MeshBindings[m].Mesh;

		if (GrannyMeshIsRigid(pgrnMesh))
		{
			if (!rMesh.CreateFromGrannyMeshPointer(pgrnSkeleton, pgrnMesh, vtxRigidPos, idxPos, m_kMtrlPal))
				return false;

			vtxRigidPos += GrannyGetMeshVertexCount(pgrnMesh);
		}
		else
		{
			if (!rMesh.CreateFromGrannyMeshPointer(pgrnSkeleton, pgrnMesh, vtxDeformPos, idxPos, m_kMtrlPal))
				return false;

			vtxDeformPos += GrannyGetMeshVertexCount(pgrnMesh);
			m_canDeformPNVertices |= rMesh.CanDeformPNTVertices();
		}
		m_bHaveBlendThing |= rMesh.HaveBlendThing();

		const granny_data_type_definition* type = pgrnMesh->PrimaryVertexData->VertexType;
		if (!type)
			continue;

		for (int i = 0; type[i].Type != GrannyEndMember; ++i)
		{
			const char* name = type[i].Name;
			if (!name || !name[0])
				continue;

			if (strcmp(name, GrannyVertexPositionName) == 0)
				m_stride += sizeof(float) * 3;
			else if (strcmp(name, GrannyVertexNormalName) == 0)
				m_stride += sizeof(float) * 3;
			else if (strcmp(name, GrannyVertexTextureCoordinatesName "0") == 0)
				m_stride += sizeof(float) * 2;
			else if (strcmp(name, GrannyVertexTextureCoordinatesName "1") == 0)
				m_stride += sizeof(float) * 2;
		}

		vtxPos += GrannyGetMeshVertexCount(pgrnMesh);
		idxPos += GrannyGetMeshIndexCount(pgrnMesh);

		if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_DIFFUSE_PNT))
			++diffusePNTMeshNodeCount;

		if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_BLEND_PNT))
			++blendPNTMeshNodeCount;
	}

	m_meshNodeCapacity = diffusePNTMeshNodeCount + blendPNTMeshNodeCount + blendPNT2MeshNodeCount;
	m_meshNodes = new TMeshNode[m_meshNodeCapacity];

	for (int n = 0; n < meshCount; ++n)
	{
		CGrannyMesh& rMesh = m_meshs[n];
		granny_mesh* pgrnMesh = m_pgrnModel->MeshBindings[n].Mesh;

		CGrannyMesh::EType eMeshType = GrannyMeshIsRigid(pgrnMesh) ? CGrannyMesh::TYPE_RIGID : CGrannyMesh::TYPE_DEFORM;

		if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_DIFFUSE_PNT))
			AppendMeshNode(eMeshType, CGrannyMaterial::TYPE_DIFFUSE_PNT, n);

		if (rMesh.GetTriGroupNodeList(CGrannyMaterial::TYPE_BLEND_PNT))
			AppendMeshNode(eMeshType, CGrannyMaterial::TYPE_BLEND_PNT, n);
	}

	if (sizeof(TPNT2Vertex) == m_stride)
	{
		for (int n = 0; n < meshCount; ++n)
		{
			CGrannyMesh& rMesh = m_meshs[n];
			rMesh.SetPNT2Mesh();
		}
	}

	m_rigidVtxCount = vtxRigidPos;
	m_deformVtxCount = vtxDeformPos;
	m_vtxCount = vtxPos;
	m_idxCount = idxPos;
	return true;
}

BOOL CGrannyModel::CheckMeshIndex(int iIndex) const
{
	if (iIndex < 0)
		return FALSE;
	if (iIndex >= GetMeshCount())
		return FALSE;

	return TRUE;
}

void CGrannyModel::AppendMeshNode(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType, int iMesh)
{
	assert(m_meshNodeSize < m_meshNodeCapacity);

	TMeshNode& rMeshNode = m_meshNodes[m_meshNodeSize++];

	rMeshNode.iMesh = iMesh;
	rMeshNode.pMesh = m_meshs + iMesh;
	rMeshNode.pNextMeshNode = m_meshNodeLists[eMeshType][eMtrlType];
	m_meshNodeLists[eMeshType][eMtrlType] = &rMeshNode;
}

bool CGrannyModel::CreateFromGrannyModelPointer(granny_model* pgrnModel)
{
	assert(IsEmpty());

	m_pgrnModel = pgrnModel;

	if (!LoadMeshs())
		return false;

	if (!LoadVertices())
		return false;

	if (!LoadIndices())
		return false;

	AddReference();

	return true;
}

int CGrannyModel::GetIdxCount()
{
	return m_idxCount;
}

bool CGrannyModel::CreateDeviceObjects()
{
	int meshCount = GetMeshCount();

	for (int i = 0; i < meshCount; ++i)
	{
		CGrannyMesh& rMesh = m_meshs[i];
		rMesh.RebuildTriGroupNodeList();
	}
			
	return true;
}

void CGrannyModel::DestroyDeviceObjects()
{
}

bool CGrannyModel::IsEmpty() const
{
	if (m_pgrnModel)
		return false;

	return true;
}

void CGrannyModel::Destroy()
{	
	m_kMtrlPal.Clear();
	
	if (m_meshNodes)
		delete [] m_meshNodes;

	if (m_meshs)
		delete [] m_meshs;

	Initialize();
}

void CGrannyModel::Initialize()
{
	memset(m_meshNodeLists, 0, sizeof(m_meshNodeLists));
	
	m_pgrnModel = NULL;
	m_meshs = NULL;
	m_meshNodes = NULL;

	m_meshNodeSize = 0;
	m_meshNodeCapacity = 0;

	m_rigidVtxCount = 0;
	m_deformVtxCount = 0;
	m_vtxCount = 0;
	m_idxCount = 0;

	m_canDeformPNVertices = false;

	m_stride = 0;
	m_bHaveBlendThing = false;
}

CGrannyModel::CGrannyModel()
{
	Initialize();
}

CGrannyModel::~CGrannyModel()
{
	Destroy();
}
