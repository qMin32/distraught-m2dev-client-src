sampler2D g_texFlare : register(s0);

struct VS_OUTPUT
{
    float4 pos      : POSITION;
    float4 diffuse  : COLOR0;
    float2 tex      : TEXCOORD0;
};

float4 main(VS_OUTPUT IN) : COLOR
{
    float4 tex = tex2D(g_texFlare, IN.tex);
    return tex * IN.diffuse;
}