///////////////////////////////////////////////////////////////////////  
//	SpeedTreeRT DirectX Example
//
//	(c) 2003 IDV, Inc.
//
//	This example demonstrates how to render trees using SpeedTreeRT
//	and DirectX.  Techniques illustrated include ".spt" file parsing,
//	static lighting, dynamic lighting, LOD implementation, cloning,
//	instancing, and dynamic wind effects.
//
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com

///////////////////////////////////////////////////////////////////////  
//	Includes

#pragma once
#include "SpeedTreeConfig.h"
#include <map>
#include <string>
#include <d3d9.h>


/////////////////////////////////////////////////////////////////////// 
// FVF Branch Vertex Structure

struct SFVFBranchVertex
{
	D3DXVECTOR3		m_vPosition;			// Always Used							
#ifdef WRAPPER_USE_DYNAMIC_LIGHTING			
	D3DXVECTOR3		m_vNormal;				// Dynamic Lighting Only			
#else										     
	DWORD			m_dwDiffuseColor;		// Static Lighting Only	
#endif	
	FLOAT			m_fTexCoords[2];		// Always Used
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	FLOAT			m_fShadowCoords[2];		// Texture coordinates for the shadows
#endif
#ifdef WRAPPER_USE_GPU_WIND		
	FLOAT			m_fWindIndex;			// GPU Only
	FLOAT			m_fWindWeight;			
#endif
};


///////////////////////////////////////////////////////////////////////  
//	Branch/Frond Vertex Program

static const char g_achSimpleVertexProgram[] = 
{
		"vs.1.1\n"												// identity shader version

		"mov		oT0.xy,		v7\n"							// always pass texcoord0 through

	#ifdef WRAPPER_RENDER_SELF_SHADOWS
		"mov		oT1.xy,		v8\n"							// pass shadow texcoords through if enabled
	#endif

		// retrieve and convert wind matrix index
		"mov		a0.x,	v9.x\n"

		// perform wind interpolation
		"m4x4		r1,			v0,			c[54+a0.x]\n"		// compute full wind effect
		"sub		r2,			r1,			v0\n"				// compute difference between full wind and none
		"mov		r3.x,		v9.y\n"							// mad can't access two v's at once, use r3.x as tmp
		"mad		r1,			r2,			r3.x,		v0\n"	// perform interpolation

		"add		r2,			c[52],		r1\n"				// translate to tree's position
		"m4x4		oPos,		r2,			c[0]\n"				// project to screen

	#ifdef WRAPPER_USE_FOG
		"dp4		r1,			r2,			c[2]\n"				// find distance to vertex
		"sub		r2.x,		c[85].y,	r1.z\n"				// linear fogging
		"mul		oFog,		r2.x,		c[85].z\n"			// write to fog register
	#endif

	#ifdef WRAPPER_USE_STATIC_LIGHTING
		"mov		oD0,		v5\n"							// pass color through
	#else
		"mov		r1,			c[74]\n"						// can only use one const register per instruction
		"mul		r5,			c[73],		r1\n"				// diffuse values
	
		"mov		r1,			c[75]\n"						// can only use one const register per instruction
		"mul		r4,			c[72],		r1\n"				// ambient values
			
		"dp3		r2,		v3,			c[71]\n"				// dot light direction with normal
//		"max		r2.x,		r2.x,		c[70].x\n"			// limit it
		"mad		oD0,		r2.x,		r5,			r4\n"	// compute the final color
	#endif
};

/////////////////////////////////////////////////////////////////////// 
// FVF Leaf Vertex Structure

struct SFVFLeafVertex
{
		D3DXVECTOR3		m_vPosition;			// Always Used							
	#ifdef WRAPPER_USE_DYNAMIC_LIGHTING			
		D3DXVECTOR3		m_vNormal;				// Dynamic Lighting Only			
	#else										     
		DWORD			m_dwDiffuseColor;		// Static Lighting Only	
	#endif											
		FLOAT			m_fTexCoords[2];		// Always Used
	#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
		FLOAT			m_fWindIndex;			// Only used when GPU is involved
		FLOAT			m_fWindWeight;					
		FLOAT			m_fLeafPlacementIndex;
		FLOAT			m_fLeafScalarValue;
	#endif
};


///////////////////////////////////////////////////////////////////////  
//	Leaf Vertex Program

static const char g_achLeafVertexProgram[] = 
{
		"vs.1.1\n"											// identity shader version

		"dcl_position v0\n"

#ifdef WRAPPER_USE_STATIC_LIGHTING
		"dcl_color v5\n"
#else
		"dcl_normal v3\n"
#endif
		"dcl_texcoord0 v7\n"
#if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
		"dcl_texcoord2 v9\n"
#endif

		"mov		oT0.xy,	v7\n"							// always pass texcoord0 through
		
	#ifdef WRAPPER_USE_GPU_WIND
		// retrieve and convert wind matrix index
		"mov		a0.x,	v9.x\n"

		// perform wind interpolation
		"m4x4		r1,		v0,			c[54+a0.x]\n"		// compute full wind effect
		"sub		r2,		r1,			v0\n"				// compute difference between full wind and none
		"mov		r3.x,	v9.y\n"							// mad can't access two v's at once, use r3.x as tmp
		"mad		r0,		r2,			r3.x,		v0\n"	// perform interpolation
	#else
		"mov		r0,		v0\n"							// wind already handled, pass the vertex through
	#endif

	#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
		"mov		a0.x,	v9.z\n"							// place the leaves
		"mul		r1,		c[a0.x],	v9.w\n"
		"add		r0,		r1,			r0\n"	
	#endif

		"add		r0,		c[52],		r0\n"				// translate to tree's position
		"m4x4		oPos,	r0,			c[0]\n"				// project to screen

	#ifdef WRAPPER_USE_FOG
		"dp4		r1,			r0,			c[2]\n"			// find distance to vertex
		"sub		r2.x,		c[85].y,	r1.z\n"			// 
		"mul		oFog,		r2.x,		c[85].z\n"
	#endif

	#ifdef WRAPPER_USE_STATIC_LIGHTING
		"mov		oD0,	v5\n"							// pass color through
	#else
		"mov		r1,		c[74]\n"						// can only use one const register per instruction
		"mul		r5,		c[73],		r1\n"				// diffuse values

		"mov		r1,		c[75]\n"						// can only use one const register per instruction
		"mul		r4,		c[72],		r1\n"				// ambient values
		
		"dp3		r2.x,   v3,			c[71]\n"			// dot light direction with normal
		"max		r2.x,	r2.x,		c[70].x\n"			// limit it
		"mad		oD0,	r2.x,		r5,			r4\n"	// compute the final color
	#endif
};


///////////////////////////////////////////////////////////////////////  
//	LoadLeafShader

static void LoadLeafShader(LPDIRECT3DDEVICE9 pDx, LPDIRECT3DVERTEXSHADER9& pVertexShader)
{
	if (!pDx)
	{
		TraceError("Failed to load leaf shader: null D3D device.");
		return;
	}

	LPDIRECT3DVERTEXSHADER9 pNewVertexShader = nullptr;
	LPD3DXBUFFER pCode = nullptr, pError = nullptr;
	const HRESULT hrAssemble = D3DXAssembleShader(g_achLeafVertexProgram, sizeof(g_achLeafVertexProgram) - 1, nullptr, nullptr, 0, &pCode, &pError);
	if (SUCCEEDED(hrAssemble) && pCode)
	{
		const HRESULT hrCreateShader = pDx->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &pNewVertexShader);
		if (FAILED(hrCreateShader))
			TraceError("Failed to create leaf vertex shader (hr=0x%08X).", hrCreateShader);
	}
	else
	{
		TraceError("Failed to assemble leaf vertex shader (hr=0x%08X). The error reported is [ %s ].", hrAssemble, pError ? pError->GetBufferPointer() : "unknown");
	}

	if (pNewVertexShader)
	{
		SAFE_RELEASE(pVertexShader);
		pVertexShader = pNewVertexShader;
	}
	else
	{
		SAFE_RELEASE(pNewVertexShader);
	}

	SAFE_RELEASE(pCode);
	SAFE_RELEASE(pError);
}
