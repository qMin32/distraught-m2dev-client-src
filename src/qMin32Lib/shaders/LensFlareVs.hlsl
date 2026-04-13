float4x4 g_mWorld;
float4x4 g_mView;
float4x4 g_mProj;

struct VS_INPUT
{
    float3 pos      : POSITION;
    float4 diffuse  : COLOR0;
    float2 tex      : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos      : POSITION;
    float4 diffuse  : COLOR0;
    float2 tex      : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    float4 pos      = float4(IN.pos, 1.0f);
    float4 worldPos = mul(g_mWorld, pos);
    float4 viewPos  = mul(g_mView,  worldPos);
    OUT.pos         = mul(g_mProj,  viewPos);
    OUT.diffuse     = IN.diffuse;
    OUT.tex         = IN.tex;
    return OUT;
}
