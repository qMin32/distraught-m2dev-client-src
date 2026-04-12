float4x4 g_mView;
float4x4 g_mProj;
float4x4 g_mStaticShadow;
float4x4 g_mDynamicShadow;

struct VS_IN
{
	float3 Position : POSITION0;
	float3 Normal	: NORMAL0;
};

struct VS_OUT
{
	float4 Position : POSITION0;
	
	float2 SaticUV	: TEXCOORD0;
	float2 DynUV	: TEXCOORD1;
	float  fogDist	: TEXCOORD2;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	
	float4 worldPos = float4(input.Position, 1.0f);
	float4 viewPos = mul(worldPos, g_mView);
	
	output.Position = mul(viewPos, g_mProj);

	float4 suv = mul(viewPos, g_mStaticShadow);
	float4 duv = mul(viewPos, g_mDynamicShadow);
	
    output.SaticUV = suv.xy;
	output.DynUV = duv.xy;
	output.fogDist = -viewPos.z;
	
	return output;
}