Texture2D txDiffuse : register(t0);
TextureCube txSkyBox : register(t1);
Texture2D txNormal : register(t2);
Texture2D txSpecular : register(t3);
Texture2D txAmbient : register(t4);
Texture2D txEmissive : register(t5);
Texture2D txOpacity : register(t6);
Texture2D txShadow : register(t7);
Texture2D txMetalic : register(t8);
Texture2D txRoughness : register(t9);
TextureCube txEnvironmentMap : register(t10);
TextureCube txIrradianceMap : register(t11);
TextureCube txPrefilteredMap : register(t12);
Texture2D txLookUpTexture : register(t13);
Texture2D txHDRSceneTexture : register(t14);
Texture2D txGBufferBaseColor : register(t15);
Texture2D txGBufferNormal : register(t16);
Texture2D txGBufferPosition : register(t17);
Texture2D txDepth : register(t18);
Texture2D txBillBoard : register(t19);


SamplerState samLinear : register(s0);
SamplerState samClamp : register(s1);


cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float3 vDirectionalLight;
    float4 vDirectionalColor;
    
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    
    float4 DiffuseMaterial;
    float4 AmbientMaterial;
    float4 SpecularMaterial;
    
    float3 camerapos;
    float lightPower;
    
    uint IsRigid;
    uint RefBoneIndex;
    float Roughness;
    float Metalness;
    
    bool OverrideMaterial;
    bool HasNormalMap;
    float MaxHDRNits;
    float Exposure;
}

cbuffer ShadowConstantBuffer : register(b2)
{
    matrix ShadowView;
    matrix ShadowProjection;
}

cbuffer ModelBoneMatrix : register(b3)
{
    matrix BonePose[128];
}

cbuffer ModelOffsetMatrix : register(b4)
{
    matrix BoneOffset[128];
}

cbuffer DirectionalLightBuffer : register(b5)
{
    
}

cbuffer PointLightBuffer : register(b6)
{
    
}

cbuffer BillBoardBuffer : register(b7)
{
    matrix BillBoardWorld;
    matrix BillBoardView;
    matrix BillBoardProjection;
    float2 UVScale;
    float2 UVOffset;
}

struct VS_INPUT
{
    float4 pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeights : BLENDWEIGHT;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldpos : TEXCOORD0;
    float2 Tex : TEXCOORD1;
    float3 norm : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
    float4 shadowpos : TEXCOORD5;
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

struct VS_SHADOW_INPUT
{
    float4 pos : POSITION;
};

struct PS_SHADOW_INPUT
{
    float4 pos : SV_Position;
}; 

struct VS_QUAD_INPUT
{
    float4 pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_QUAD_INPUT
{
    float4 pos : SV_Position;
    float2 UV : TEXCOORD0;
};

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return saturate(x * (a * x + b) / (x * (c * x + d) + e));
}

struct VS_GBUFFER_INPUT
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeights : BLENDWEIGHT;
};

struct VS_GBUFFER_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 worldpos : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 norm : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
    float4 shadowpos : TEXCOORD5;
};

struct VS_BILLBOARDINPUT
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct PS_BILLBOARDINPUT
{
    float4 Position : SV_POSITION;
    float4 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
};