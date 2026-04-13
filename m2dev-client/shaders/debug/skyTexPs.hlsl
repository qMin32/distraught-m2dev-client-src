sampler2D g_texFace : register(s0);

struct PS_INPUT
{
    float2 vTex0 : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
    return tex2D(g_texFace, input.vTex0);
}