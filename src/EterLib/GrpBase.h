#pragma once

#include "Ray.h"
#include <vector>
#include "../qMin32Lib/Core.h"
#include "../qMin32Lib/BaseClass.h"

// to remove ms_matIdentity we use only MatIdentity()
static inline const D3DXMATRIX& MatIdentity()
{
	static const D3DXMATRIX mat(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	return mat;
}

//what directx need for start is world,view and projection matrix so we can use this struct to set them at once.
struct BaseMatrix
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX proj;
};

void PixelPositionToD3DXVECTOR3(const D3DXVECTOR3& c_rkPPosSrc, D3DXVECTOR3* pv3Dst);
void D3DXVECTOR3ToPixelPosition(const D3DXVECTOR3& c_rv3Src, D3DXVECTOR3* pv3Dst);

class CGraphicTexture;

typedef struct SFace
{
	WORD indices[3];
} TFace;

// a lot of structs have the same size so i will delete some of them 
typedef struct SPDVertex
{
	float x, y, z;
	DWORD color;
} TPDVertex;

typedef struct SPTVertex
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 texCoord;
} TPTVertex;

typedef struct SPDTVertex
{
	D3DXVECTOR3	position;
	DWORD		diffuse;
	D3DXVECTOR2 texCoord;
	SPDTVertex() : position(0.0f, 0.0f, 0.0f), diffuse(0x80339CFF), texCoord(0.0f, 0.0f) {}
	SPDTVertex(D3DXVECTOR3 pos, DWORD col, D3DXVECTOR2 uv) : position(pos), diffuse(col), texCoord(uv) {}
	SPDTVertex(float x, float y, float z, DWORD color, float pu, float pv) : position(x, y, z), diffuse(color), texCoord(pu, pv) {}
} TPDTVertex;

typedef struct SPNTVertex
{
	D3DXVECTOR3	position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2	texCoord;
} TPNTVertex;

typedef struct SPNT2Vertex
{
	D3DXVECTOR3	position;
	D3DXVECTOR3	normal;
	D3DXVECTOR2 texCoord;
	D3DXVECTOR2 texCoord2;
} TPNT2Vertex;

typedef struct SBoundBox
{
	float sx, sy, sz;
	float ex, ey, ez;
	int meshIndex;
	int boneIndex;
} TBoundBox;

const WORD c_FillRectIndices[6] = { 0, 2, 1, 2, 3, 1 };

class CGraphicBase
{
	public:
		static DWORD GetAvailableTextureMemory();
		static const D3DXMATRIX& GetViewMatrix();

		enum
		{			
			DEFAULT_IB_LINE, 
			DEFAULT_IB_LINE_TRI, 
			DEFAULT_IB_LINE_RECT, 
			DEFAULT_IB_LINE_CUBE, 
			DEFAULT_IB_FILL_TRI,
			DEFAULT_IB_FILL_RECT,
			DEFAULT_IB_FILL_CUBE,
			DEFAULT_IB_NUM,
		};

	public:
		CGraphicBase();
		virtual	~CGraphicBase();

		void		SetSimpleCamera(float x, float y, float z, float pitch, float roll);
		void		SetEyeCamera(float xEye, float yEye, float zEye, float xCenter, float yCenter, float zCenter, float xUp, float yUp, float zUp);
		void		SetAroundCamera(float distance, float pitch, float roll, float lookAtZ = 0.0f);
		void		SetPositionCamera(float fx, float fy, float fz, float fDistance, float fPitch, float fRotation);

		void		GetTargetPosition(float * px, float * py, float * pz);
		void		GetCameraPosition(float * px, float * py, float * pz);
		void		SetOrtho2D(float hres, float vres, float zres);
		void		SetOrtho3D(float hres, float vres, float zmin, float zmax);
		void		SetPerspective(float fov, float aspect, float nearz, float farz);
		float		GetFOV();
		void		GetClipPlane(float * fNearY, float * fFarY)
		{
			*fNearY = ms_fNearY;
			*fFarY = ms_fFarY;
		}

		////////////////////////////////////////////////////////////////////////
		void		PushMatrix();

		void		MultMatrix( const D3DXMATRIX* pMat );
		void		MultMatrixLocal( const D3DXMATRIX* pMat );
	
		void		Translate(float x, float y, float z);
		void		Rotate(float degree, float x, float y, float z);
		void		RotateLocal(float degree, float x, float y, float z);
		void		RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll);
		void		Scale(float x, float y, float z);
		void		PopMatrix();		
		void		LoadMatrix(const D3DXMATRIX & c_rSrcMatrix);		
		void		GetMatrix(D3DXMATRIX * pRetMatrix) const;
		const		D3DXMATRIX * GetMatrixPointer() const;


		////////////////////////////////////////////////////////////////////////
		void		InitScreenEffect();
		void		SetScreenEffectWaving(float fDuringTime, int iPower);
		void		SetScreenEffectFlashing(float fDuringTime, const D3DXCOLOR & c_rColor);

		////////////////////////////////////////////////////////////////////////
		DWORD		GetColor(float r, float g, float b, float a = 1.0f);

		DWORD		GetFaceCount();
		void		ResetFaceCount();
		HRESULT		GetLastResult();

		void		UpdateProjMatrix();
		void		UpdateViewMatrix();
		
		void		SetViewport(DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, float fMinZ, float fMaxZ);
		static void		GetBackBufferSize(UINT* puWidth, UINT* puHeight);
		static bool		IsTLVertexClipping();
		static bool		IsFastTNL();
		static bool		IsLowTextureMemory();
		static bool		IsHighTextureMemory();

		static void SetDefaultIndexBuffer(UINT eDefIB);
		static bool SetPDTStream(SPDTVertex* pVertices, UINT uVtxCount);
		
	protected:
		static BaseMatrix				mat;

	protected:
		void		UpdatePipeLineMatrix();

	protected:
		static HRESULT					ms_hLastResult;

		static int						ms_iWidth;
		static int						ms_iHeight;	

		static HWND						ms_hWnd;
		static HDC						ms_hDC;
		static LPDIRECT3D9EX			ms_lpd3d;
		static LPDIRECT3DDEVICE9EX		ms_lpd3dDevice;
		static ID3DXMatrixStack*		ms_lpd3dMatStack;
		static D3DVIEWPORT9				ms_Viewport;

		static DWORD					ms_faceCount;
		static D3DCAPS9					ms_d3dCaps;
		static D3DPRESENT_PARAMETERS	ms_d3dPresentParameter;
		
		static DWORD					ms_dwD3DBehavior;
		static LPDIRECT3DVERTEXDECLARATION9					ms_ptVS;
		static LPDIRECT3DVERTEXDECLARATION9					ms_pntVS;
		static LPDIRECT3DVERTEXDECLARATION9					ms_pnt2VS;

		static D3DXVECTOR3				ms_vtPickRayOrig;
		static D3DXVECTOR3				ms_vtPickRayDir;

		static float					ms_fFieldOfView;
		static float					ms_fAspect;
		static float					ms_fNearY;
		static float					ms_fFarY;

		// Screen Effect - Waving, Flashing and so on..
		static DWORD					ms_dwWavingEndTime;
		static int						ms_iWavingPower;
		static DWORD					ms_dwFlashingEndTime;
		static D3DXCOLOR				ms_FlashingColor;

		// Terrain picking용 Ray... CCamera 이용하는 버전.. 기존의 Ray와 통합 필요...
 		static CRay						ms_Ray;

		// 
		static bool						ms_bSupportDXT;
		static bool						ms_isLowTextureMemory;
		static bool						ms_isHighTextureMemory;

		enum
		{
			PDT_VERTEX_NUM = 16,
			PDT_VERTEXBUFFER_NUM = 100,				
		};
		
	protected:
		static UniquePtr<BaseClass>	m_base;

		static RefPtr<CIndexBuffer>	ms_alpd3dDefIB[DEFAULT_IB_NUM];
		static RefPtr<CVertexBuffer>	ms_alpd3dPDTVB[PDT_VERTEXBUFFER_NUM];

	public:
		static UniquePtr<BaseClass>& GetBase();
};

#define m_dx CGraphicBase::GetBase()
