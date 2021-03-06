[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
	Texture = "textures/common/white.tga";
};

// Contexts
OpenGL4
{
	context LIGHTING
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		PixelShader = compile GLSL FS_LIGHTING_GL4;
		
		ZWriteEnable = false;
		BlendMode = Add;
	}
	
	context AMBIENT
	{
		VertexShader = compile GLSL VS_GENERAL_GL4;
		PixelShader = compile GLSL FS_AMBIENT_GL4;
	}
}


[[VS_GENERAL_GL4]]
// =================================================================================================
#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec3 viewerPos;

layout( location = 0 ) in vec3 vertPos;
layout( location = 1 ) in vec3 normal;
layout( location = 5 ) in vec2 texCoords0;

out vec4 pos, vsPos;
out vec2 texCoords;
out vec3 tsbNormal;

void main( void )
{
	// Calculate normal
	//vec3 _normal = normalize( calcWorldVec( normal ) );
	//tsbNormal = _normal;

	// Calculate world space position
	pos = calcWorldPos( vec4( vertPos, 1.0 ) );
	vsPos = calcViewPos( pos );
	
	// Calculate texture coordinates and clip space position
	texCoords = texCoords0;
	gl_Position = viewProjMat * pos;
}

[[FS_ATTRIBPASS]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragDeferredWrite.glsl" 

uniform vec3 viewerPos;
uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	setMatID( 1.0 );
	setPos( newPos - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( albedo.rgb );
	setSpecParams( matSpecParams.rgb, matSpecParams.a );
}

[[FS_ATTRIBPASS_GL4]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragDeferredWriteGL4.glsl" 

uniform vec3 viewerPos;
uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

in vec4 pos;
in vec2 texCoords;

#ifdef _F02_NormalMapping
	in mat3 tsbMat;
#else
	in vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	in vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	setMatID( 1.0 );
	setPos( newPos - viewerPos );
	setNormal( normalize( normal ) );
	setAlbedo( albedo.rgb );
	setSpecParams( matSpecParams.rgb, matSpecParams.a );
}
	
[[VS_SHADOWMAP]]
// =================================================================================================
	
#include "shaders/utilityLib/vertCommon.glsl"
#include "shaders/utilityLib/vertSkinning.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;
attribute vec3 vertPos;
varying vec3 lightVec;

#ifdef _F05_AlphaTest
	attribute vec2 texCoords0;
	varying vec2 texCoords;
#endif

void main( void )
{
#ifdef _F01_Skinning	
	vec4 pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ) ) );
#else
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

#ifdef _F05_AlphaTest
	texCoords = texCoords0;
#endif

	lightVec = lightPos.xyz - pos.xyz;
	gl_Position = viewProjMat * pos;
}
	
[[VS_SHADOWMAP_GL4]]
// =================================================================================================
	
#include "shaders/utilityLib/vertCommon.glsl"
#include "shaders/utilityLib/vertSkinningGL4.glsl"

uniform mat4 viewProjMat;
uniform vec4 lightPos;

layout( location = 0 ) in vec3 vertPos;
out vec3 lightVec;

#ifdef _F05_AlphaTest
	layout( location = 5 ) in vec2 texCoords0;
	out vec2 texCoords;
#endif

void main( void )
{
#ifdef _F01_Skinning	
	vec4 pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ) ) );
#else
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

#ifdef _F05_AlphaTest
	texCoords = texCoords0;
#endif

	lightVec = lightPos.xyz - pos.xyz;
	gl_Position = viewProjMat * pos;
}

	
[[FS_SHADOWMAP]]
// =================================================================================================

uniform vec4 lightPos;
uniform float shadowBias;
varying vec3 lightVec;

#ifdef _F05_AlphaTest
	uniform vec4 matDiffuseCol;
	uniform sampler2D albedoMap;
	varying vec2 texCoords;
#endif

void main( void )
{
#ifdef _F05_AlphaTest
	vec4 albedo = texture2D( albedoMap, texCoords * vec2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	float dist = length( lightVec ) / lightPos.w;
	gl_FragDepth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	//gl_FragDepth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}

[[FS_SHADOWMAP_GL4]]
// =================================================================================================

uniform vec4 lightPos;
uniform float shadowBias;
in vec3 lightVec;

#ifdef _F05_AlphaTest
	uniform vec4 matDiffuseCol;
	uniform sampler2D albedoMap;
	in vec2 texCoords;
#endif

void main( void )
{
#ifdef _F05_AlphaTest
	vec4 albedo = texture( albedoMap, texCoords * vec2( 1, -1 ) ) * matDiffuseCol;
	if( albedo.a < 0.01 ) discard;
#endif
	
	float dist = length( lightVec ) / lightPos.w;
//	gl_FragDepth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	gl_FragDepth = dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}


[[FS_LIGHTING]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragLighting.glsl" 

uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	gl_FragColor.rgb =
		calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
		                    matSpecParams.a, -vsPos.z, 0.3 );
}

[[FS_LIGHTING_GL4]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragLightingGL4.glsl" 

uniform vec4 matDiffuseCol;
uniform vec4 matSpecParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

in vec4 pos, vsPos;
in vec2 texCoords;

#ifdef _F02_NormalMapping
	in mat3 tsbMat;
#else
	in vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	in vec3 eyeTS;
#endif

out vec4 fragColor;

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture( albedoMap, newCoords.st ) * matDiffuseCol;
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	fragColor.rgb = calcPhongSpotLight( newPos, normalize( normal ), albedo.rgb, matSpecParams.rgb,
										matSpecParams.a, -vsPos.z, 0.3 );
}


[[FS_AMBIENT]]	
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/fragLighting.glsl" 

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

#ifdef _F04_EnvMapping
	uniform samplerCube envMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st * vec2( 1, -1 ) );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	// Flip texture vertically to match the GL coordinate system
	newCoords.t *= -1.0;

	vec4 albedo = texture2D( albedoMap, newCoords.st );
	
#ifdef _F05_AlphaTest
	if( albedo.a < 0.01 ) discard;
#endif
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif
	
	gl_FragColor.rgb = albedo.rgb * textureCube( ambientMap, normal ).rgb;
	
#ifdef _F04_EnvMapping
	vec3 refl = textureCube( envMap, reflect( pos.xyz - viewerPos, normalize( normal ) ) ).rgb;
	gl_FragColor.rgb = refl * 1.5;
#endif
}

[[FS_AMBIENT_GL4]]	
// =================================================================================================

in vec2 texCoords;
uniform sampler2D albedoMap;
out vec4 fragColor;


void main( void )
{
	vec2 newCoords = texCoords;
	newCoords.y *= -1.0;
	fragColor = texture(albedoMap, newCoords);
}
