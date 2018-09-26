struct DirectionalLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 direction;
	float pad;
};

struct PointLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 att;
	float pad;
};

struct SpotLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 direction;
	float spot;

	float3 att;
	float pad;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 reflect;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProjM : WORLDVIEWPROJECTION;
    float4x4 gTexTransform;
	Material gMaterial;
};

Texture2D gDiffuseMap;

cbuffer cbPerFrame : register(b1)
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;
};

RasterizerState WireframeRS
{
	FillMode = Wireframe;
	CullMode = Back;
	FrontCounterClockwise = false;
};

RasterizerState SolidRS
{
	FillMode = Solid;
	CullMode = Back;
	FrontCounterClockwise = false;
};

SamplerState anisotropicFilter
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;
    AdressU = MIRROR;
    AdressV = MIRROR;
};

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = -L.direction;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = L.position - pos;

    float d = length(lightVec);

	if(d > L.range)
    {
        return;
    }

    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = L.position - pos;

    float d = length(lightVec);

	if(d > L.range)
        return;

    lightVec /= d;

    ambient = mat.ambient * L.ambient;

    float diffuseFactor = dot(lightVec, normal);

	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.spot);

    float att = spot / dot(L.att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}

struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
    float2 texW : TEXCOORD;
};

VertexOut VS(VertexIn In)
{
	VertexOut Out;

	Out.posW = mul(float4(In.pos, 1.0f), gWorld).xyz;
	Out.normalW = mul(In.normal, (float3x3)gWorldInvTranspose);
	Out.posH = mul(float4(In.pos, 1.0f), gWorldViewProjM);
    Out.texW = mul(float4(In.tex, 0.0f, 1.0f), gTexTransform).xy;

	return Out;
}

float4 PS(VertexOut In) : SV_Target
{
    float4 texcol = gDiffuseMap.Sample(anisotropicFilter, In.texW);

	In.normalW = normalize(In.normalW);

    float3 toEyeW = normalize(gEyePosW - In.posW);

    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 A, S, D;

    ComputeDirectionalLight(gMaterial, gDirLight, In.normalW, toEyeW, A, D, S);

    ambient += A;
    diffuse += D;
    specular += S;

    ComputePointLight(gMaterial, gPointLight, In.posW, In.normalW, toEyeW, A, D, S);

	ambient += A;
    diffuse += D;
    specular += S;

    ComputeSpotLight(gMaterial, gSpotLight, In.posW, In.normalW, toEyeW, A, D, S);

	ambient += A;
    diffuse += D;
    specular += S;

    float4 litColor = texcol * (ambient + diffuse) + specular;

    litColor.a = gMaterial.diffuse.a;

    return litColor;
}

technique11 LightTech
{
	pass lightPass
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));

        SetRasterizerState(SolidRS);
    }
}

technique11 LightTechWire
{
    pass lightPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetRasterizerState(WireframeRS);
    }
}