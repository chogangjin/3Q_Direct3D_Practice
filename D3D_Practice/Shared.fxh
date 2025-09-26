Texture2D txDiffuse : register(t0);
TextureCube SkyBox : register(t1);

SamplerState samLinear : register(s0);


cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float4 vDirectionalLight;
    float4 vDirectionalColor;
    
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    
    float4 DiffuseMaterial;
    float4 AmbientMaterial;
    float4 SpecularMaterial;
    
    float3 camerapos;
    float shininess;
}

//cbuffer LightBuffer : register(b1)
//{
//    float4 vDirectionalLights;
//    float4 vDirectionalColors;
    
//    float4 DiffuseColors;
//    float4 DiffuseMaterials;
     
//    float4 AmbientColors;
//    float4 AmbientMaterials;
    
//    float4 SpecularColors;
//    float4 SpecularMaterials;
    
//    float shininess2;
//    float3 padding;
//}

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldpos : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float2 Tex : TEXCOORD2;
};

struct VS_SKYBOX_INPUT
{
    float4 pos : POSITION;
};

struct PS_SKYBOX_INPUT
{
    float4 pos : SV_Position;
    float3 Tex : TEXCOORD0;
};