float4x4 g_mView;
float4x4 g_mProj;
float g_fTexScaleX;
float g_fTexScaleY;
float g_fTime;

struct VS_INPUT
{
    float3 vPos : POSITION;
    float4 vDiffuse : COLOR0;
};

struct VS_OUTPUT
{
    float4 vPos : POSITION;
    float4 vDiffuse : COLOR0;
    float2 vTex0 : TEXCOORD0;
    float2 vTex1 : TEXCOORD1;
    float fFog : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 vWorldPos = float4(input.vPos, 1.0f);
    float4 vViewPos = mul(vWorldPos, g_mView);
    output.vPos = mul(vViewPos, g_mProj);

    float2 vBaseUV = float2(input.vPos.x * g_fTexScaleX, input.vPos.y * g_fTexScaleY);

    float fSpeed1 = 0.04f;
    float fSpeed2 = 0.04f;
    output.vTex0 = vBaseUV * 1.0f + float2(g_fTime * fSpeed1, g_fTime * fSpeed1);
    output.vTex1 = vBaseUV * 1.0f + float2(-g_fTime * fSpeed2, g_fTime * fSpeed2);

    
    output.vDiffuse = input.vDiffuse;
    output.fFog = vViewPos.z;

    return output;
}