/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * Author: Roland Smeenk
 */

#include <osg/ShaderModeFog>
#include <osg/ShaderModeLight>
#include <osg/ShaderModeLighting>
#include <osg/ShaderModeLightModel>
#include <osg/ShaderModeMaterial>
#include <osg/ShaderModePointSprite>
#include <osg/ShaderModeRescaleNormal>
#include <osg/ShaderModeRoot>
#include <osg/ShaderModeShadeModel>
#include <osg/ShaderModeTexEnv>
#include <osg/ShaderModeTexGen>
#include <osg/ShaderModeTexture2D>

using namespace osg;

ShaderModeVisualRoot::ShaderModeVisualRoot() :
	ShaderMode("FFVisualRoot", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN, false)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 0.0, 1.0, 1.0"));
}

void ShaderModeVisualRoot::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "gl_Position = ftransform();" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTINIT)
	{
	    str.indent() << "vec4 fragColor = vec4(1.0);" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		str.indent() << "gl_FragColor = fragColor;" << str.eol();
	}
}

ShaderModeEyeNormalRoot::ShaderModeEyeNormalRoot() :
	ShaderMode("FFEyeNormalRoot", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN, false)
{ 
	_inputs.push_back(ShaderMode::Variable("vec3", "transformedNormal", "0.0, 0.0, 0.0"));
}

void ShaderModeEyeNormalRoot::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXINIT)
	{
		str.indent() << "varying vec3 vNormal;" << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "vNormal = transformedNormal;" << str.eol();
		str.indent() << "gl_Position = ftransform();" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTINIT)
	{
	    str.indent() << "varying vec3 vNormal;" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		str.indent() << "gl_FragColor = vec4(vNormal * 0.5 + 0.5, 1.0);" << str.eol();
	}
}

ShaderModeDepthRoot::ShaderModeDepthRoot() :
	ShaderMode("FFDepthRoot", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN, false)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "0.0, 0.0, 0.0, 0.0"));
}

void ShaderModeDepthRoot::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXINIT)
	{
		str.indent() << "varying vec3 vViewVec;" << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		// TODO make fixed scale factor more flexible
		str.indent() << "vViewVec = -vec3(0.01 * ecPosition);" << str.eol();
		str.indent() << "gl_Position = ftransform();" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTINIT)
	{
	    str.indent() << "varying vec3 vViewVec;" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		str.indent() << "gl_FragColor = vec4( length(vViewVec));" << str.eol();
	}
}

ShaderModeBase::ShaderModeBase() :
	ShaderMode("FFBase", ShaderMode::VERTEXMAIN, false)
{ 
	_outputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "1.0, 1.0, 1.0, 1.0"));
	_outputs.push_back(ShaderMode::Variable("vec3", "transformedNormal", "1.0, 1.0, 1.0"));
	_outputs.push_back(ShaderMode::Variable("vec3", "ecPosition3", "1.0, 1.0, 1.0"));
}

void ShaderModeBase::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXINIT)
	{
		str.indent() << "vec4 ecPosition = vec4(0.0);" << str.eol();
		str.indent() << "vec3 transformedNormal = vec3(1.0);" << str.eol();
		str.indent() << "vec3 ecPosition3 = vec3(0.0);" << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "ecPosition = gl_ModelViewMatrix * gl_Vertex;" << str.eol();				
		str.indent() << "transformedNormal   = gl_NormalMatrix * gl_Normal;" << str.eol();
		str.indent() << "ecPosition3 = (vec3 (ecPosition)) / ecPosition.w;" << str.eol();
	}
}


ShaderModeMaterial::ShaderModeMaterial() :
		ShaderMode("FFMaterial", ShaderMode::VERTEXMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("float", "Dummy", "0.0"));
	_outputs.push_back(ShaderMode::Variable("float", "Dummy", "0.0"));

	// Note:
	// Does not contribute shader code, but only passes through material values from uniforms
	//gl_FrontMaterial.ambient
	//gl_FrontMaterial.diffuse
	//gl_FrontMaterial.specular
	//gl_FrontMaterial.shininess

	//gl_BackMaterial.ambient
	//gl_BackMaterial.diffuse
	//gl_BackMaterial.specular
	//gl_BackMaterial.shininess
}

void ShaderModeMaterial::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index)
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "// Material " << str.eol();
	}
}

bool ShaderModeLight::_typeTraversed = false;

ShaderModeLight::ShaderModeLight(int lightNum) :
	MultiShaderMode("FFLight", lightNum, ShaderMode::VERTEXMAIN)
{
	_inputs.push_back(ShaderMode::Variable("vec3", "transformedNormal", "1.0, 1.0, 1.0"));
	_inputs.push_back(ShaderMode::Variable("vec3", "ecPosition3", "1.0, 1.0, 1.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "AmbientLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "DiffuseLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "SpecularLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "AmbientLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "DiffuseLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "SpecularLightIntensity", "0.0, 0.0, 0.0, 0.0"));
}

void ShaderModeLight::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if ((location == ShaderMode::VERTEXFUNCTION) && !_typeTraversed)
	{
		_typeTraversed = true;

		str.indent() << "// Fixed function light functions" << str.eol();
		str.indent() << "void pointLight(in int i, in vec3 normal, in vec3 eye, in vec3 ecPosition3)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    float nDotVP;       // normal . light direction" << str.eol();
		str.indent() << "    float nDotHV;       // normal . light half vector" << str.eol();
		str.indent() << "    float pf;           // power factor" << str.eol();
		str.indent() << "    float attenuation;  // computed attenuation factor" << str.eol();
		str.indent() << "    float d;            // distance from surface to light source" << str.eol();
		str.indent() << "    vec3  VP;           // direction from surface to light position" << str.eol();
		str.indent() << "    vec3  halfVector;   // direction of maximum highlights" << str.eol();
		str.indent() << "    " << str.eol();
		str.indent() << "    // Compute vector from surface to light position" << str.eol();
		str.indent() << "    VP = vec3 (gl_LightSource[i].position) - ecPosition3;" << str.eol();
		str.indent() << "    " << str.eol();
		str.indent() << "    // Compute distance between surface and light position" << str.eol();
		str.indent() << "    d = length(VP);" << str.eol();
		str.indent() << "    " << str.eol();
		str.indent() << "    // Normalize the vector from surface to light position" << str.eol();
		str.indent() << "    VP = normalize(VP);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    // Compute attenuation" << str.eol();
		str.indent() << "    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +" << str.eol();
		str.indent() << "                         gl_LightSource[i].linearAttenuation * d +" << str.eol();
		str.indent() << "                         gl_LightSource[i].quadraticAttenuation * d * d);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    halfVector = normalize(VP + eye);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    nDotVP = max(0.0, dot(normal, VP));" << str.eol();
		str.indent() << "    nDotHV = max(0.0, dot(normal, halfVector));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    if (nDotVP == 0.0)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = 0.0;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = pow(nDotHV, gl_FrontMaterial.shininess);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    AmbientLightIntensity  += gl_LightSource[i].ambient * attenuation;" << str.eol();
		str.indent() << "    DiffuseLightIntensity  += gl_LightSource[i].diffuse * nDotVP * attenuation;" << str.eol();
		str.indent() << "    SpecularLightIntensity += gl_LightSource[i].specular * pf * attenuation;" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << str.eol();

		str.indent() << "void spotLight(in int i, in vec3 normal, in vec3 eye, in vec3 ecPosition3)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    float nDotVP;            // normal . light direction" << str.eol();
		str.indent() << "    float nDotHV;            // normal . light half vector" << str.eol();
		str.indent() << "    float pf;                // power factor" << str.eol();
		str.indent() << "    float spotDot;           // cosine of angle between spotlight" << str.eol();
		str.indent() << "    float spotAttenuation;   // spotlight attenuation factor" << str.eol();
		str.indent() << "    float attenuation;       // computed attenuation factor" << str.eol();
		str.indent() << "    float d;                 // distance from surface to light source" << str.eol();
		str.indent() << "    vec3  VP;                // direction from surface to light position" << str.eol();
		str.indent() << "    vec3  halfVector;        // direction of maximum highlights" << str.eol();
		str.indent() << str.eol();
		str.indent() << "    // Compute vector from surface to light position" << str.eol();
		str.indent() << "    VP = vec3 (gl_LightSource[i].position) - ecPosition3;" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    // Compute distance between surface and light position" << str.eol();
		str.indent() << "    d = length(VP);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    // Normalize the vector from surface to light position" << str.eol();
		str.indent() << "    VP = normalize(VP);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    // Compute attenuation" << str.eol();
		str.indent() << "    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +" << str.eol();
		str.indent() << "    gl_LightSource[i].linearAttenuation * d +" << str.eol();
		str.indent() << "    gl_LightSource[i].quadraticAttenuation * d * d);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    // See if point on surface is inside cone of illumination" << str.eol();
		str.indent() << "    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    if (spotDot < gl_LightSource[i].spotCosCutoff)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        spotAttenuation = 0.0; // light adds no contribution" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    // Combine the spotlight and distance attenuation." << str.eol();
		str.indent() << "    attenuation *= spotAttenuation;" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    halfVector = normalize(VP + eye);" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    nDotVP = max(0.0, dot(normal, VP));" << str.eol();
		str.indent() << "    nDotHV = max(0.0, dot(normal, halfVector));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    if (nDotVP == 0.0)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = 0.0;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = pow(nDotHV, gl_FrontMaterial.shininess);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    AmbientLightIntensity  += gl_LightSource[i].ambient * attenuation;" << str.eol();
		str.indent() << "    DiffuseLightIntensity  += gl_LightSource[i].diffuse * nDotVP * attenuation;" << str.eol();
		str.indent() << "    SpecularLightIntensity += gl_LightSource[i].specular * pf * attenuation;" << str.eol();
		str.indent() << "} " << str.eol();
		str.indent() << str.eol();

		str.indent() << "void infiniteSpotLight(in int i, in vec3 normal)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    float nDotVP;         // normal . light direction" << str.eol();
		str.indent() << "    float nDotHV;         // normal . light half vector" << str.eol();
		str.indent() << "    float pf;             // power factor" << str.eol();
		str.indent() << "    float spotAttenuation;" << str.eol();
		str.indent() << "    vec3  Ppli;" << str.eol();
		str.indent() << "    vec3  Sdli;" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position))));" << str.eol();
		str.indent() << "    nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector)));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    Ppli = -normalize(vec3(gl_LightSource[i].position));" << str.eol();
		str.indent() << "    Sdli = normalize(vec3(gl_LightSource[i].spotDirection));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    spotAttenuation = pow(dot(Ppli, Sdli), gl_LightSource[i].spotExponent);" << str.eol();
		str.indent() << "    if (nDotVP == 0.0)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = 0.0;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = pow(nDotHV, gl_FrontMaterial.shininess);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    AmbientLightIntensity  += gl_LightSource[i].ambient * spotAttenuation;" << str.eol();
		str.indent() << "    DiffuseLightIntensity  += gl_LightSource[i].diffuse * nDotVP * spotAttenuation;" << str.eol();
		str.indent() << "    SpecularLightIntensity += gl_LightSource[i].specular * pf * spotAttenuation;" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << str.eol();

		str.indent() << "void directionalLight(in int i, in vec3 normal)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    float nDotVP;         // normal . light direction" << str.eol();
		str.indent() << "    float nDotHV;         // normal . light half vector" << str.eol();
		str.indent() << "    float pf;             // power factor" << str.eol();
		str.indent() << str.eol();
		str.indent() << "    nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position))));" << str.eol();
		str.indent() << "    nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector)));" << str.eol();
		str.indent() << "" << str.eol();
		str.indent() << "    if (nDotVP == 0.0)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = 0.0;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        pf = pow(nDotHV, gl_FrontMaterial.shininess);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    AmbientLightIntensity  += gl_LightSource[i].ambient;" << str.eol();
		str.indent() << "    DiffuseLightIntensity  += gl_LightSource[i].diffuse * nDotVP;" << str.eol();
		str.indent() << "    SpecularLightIntensity += gl_LightSource[i].specular * pf;" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << str.eol();

		str.indent() << "void fflight(in int i)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    if (gl_LightSource[i].position.w == 0.0)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        if (gl_LightSource[i].spotCutoff <= 90.0)" << str.eol();
		str.indent() << "            infiniteSpotLight(i, transformedNormal);" << str.eol();
		str.indent() << "        else" << str.eol();
		str.indent() << "            directionalLight(i, transformedNormal);" << str.eol();				
		str.indent() << "    }" << str.eol();
		str.indent() << "    else" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        if (gl_LightSource[i].spotCutoff <= 90.0)" << str.eol();
		str.indent() << "            spotLight(i, transformedNormal, eye, ecPosition3);" << str.eol();
		str.indent() << "        else" << str.eol();
		str.indent() << "            pointLight(i, transformedNormal, eye, ecPosition3);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "fflight(" << _index << ");" << str.eol();
	}
}

ShaderModeFog::ShaderModeFog() :
	ShaderMode("FFFog", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN, false)
{ 

	_inputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "1.0, 1.0, 1.0, 1.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeFog::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXINIT)
	{
		str.indent() << "const int FOG_COORDINATE = 0;" << str.eol();
		str.indent() << "uniform int fogCoordinateSource;" << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "if (fogCoordinateSource == FOG_COORDINATE)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    gl_FogFragCoord = gl_FogCoord;" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << "else // FRAGMENT_DEPTH" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    gl_FogFragCoord =(abs(ecPosition.z));" << str.eol();
		str.indent() << "}" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTINIT)
	{
		str.indent() << "const int LINEAR = 0;" << str.eol();
		str.indent() << "const int EXP = 1;" << str.eol();
		str.indent() << "const int EXP2 = 2;" << str.eol();
		str.indent() << "uniform int fogMode;" << str.eol();

		str.indent() << "const float LOG2E = 1.442695;" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		// TODO precompute gl_Fog.density * LOG2E on CPU and set as uniform
		// TODO precompute gl_Fog.density * gl_Fog.density * LOG2E on CPU and set as uniform
		str.indent() << "float fog = 0.0;" << str.eol();
		str.indent() << "if (fogMode == LINEAR)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    fog = (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale;" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << "else if (fogMode == EXP)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    fog = exp2(-gl_Fog.density * gl_FogFragCoord * LOG2E);" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << "else // EXP2" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    fog = exp2(-gl_Fog.density * gl_Fog.density * gl_FogFragCoord * gl_FogFragCoord * LOG2E);" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << "fog = clamp(fog, 0.0, 1.0);" << str.eol();
		str.indent() << "fragColor = vec4(mix( vec3(gl_Fog.color), vec3(fragColor), fog), fragColor.a);" << str.eol();
	}
}

ShaderModeLighting::ShaderModeLighting() :
	ShaderMode("FFLighting", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "AmbientLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "DiffuseLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "SpecularLightIntensity", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "0.0, 0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec3", "transformedNormal", "0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec3", "ecPosition3", "0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeLighting::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index)
{
	if (location == ShaderMode::VERTEXINIT)
	{
		str.indent() << "vec3 eye = vec3 (0.0, 0.0, 1.0);" << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "vec4 color = gl_FrontLightModelProduct.sceneColor +" << str.eol();
		str.indent() << "             AmbientLightIntensity  * gl_FrontMaterial.ambient +" << str.eol();
		str.indent() << "             DiffuseLightIntensity  * gl_FrontMaterial.diffuse;" << str.eol();
		str.indent() << "color += SpecularLightIntensity * gl_FrontMaterial.specular;" << str.eol();
		str.indent() << "color = clamp( color, 0.0, 1.0 );" << str.eol();
		str.indent() << "gl_FrontColor = color;" << str.eol();
	}	
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		str.indent() << "fragColor = gl_Color;" << str.eol();
	}
}

ShaderModeLightModel::ShaderModeLightModel() :
	ShaderMode("FFLightModel", ShaderMode::VERTEXMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("float", "Dummy", "0.0"));
	_outputs.push_back(ShaderMode::Variable("float", "Dummy", "0.0"));
}


void ShaderModeLightModel::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << " // light Model" << str.eol();
	}
}

void ShaderModePointSprite::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	str.indent() << "// Point sprite" << str.eol();
}

void ShaderModeRescaleNormal::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	str.indent() << "// Rescale normal" << str.eol();
}

void ShaderModeShadeModel::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	str.indent() << "// Shade model" << str.eol();
}

ShaderModeTexture2D::ShaderModeTexture2D(int stage) :
	MultiShaderMode("FFTexture2D", stage, ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTINIT | ShaderMode::FRAGMENTMAIN, true)
{ 
	char tmp[256];
	sprintf(tmp,"texCoord%dGenerated", _index);
	_inputs.push_back(ShaderMode::Variable("bool", tmp, "false"));

	sprintf(tmp,"textureColor%d", _index);
	_outputs.push_back(ShaderMode::Variable("vec4", tmp, "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeTexture2D::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() <<	"if (!texCoord" << _index << "Generated)" << str.eol();
		str.indent() <<	"{" << str.eol();
		str.indent() << "    gl_TexCoord[" << _index << "] = gl_MultiTexCoord" << _index << ";" << str.eol();
		str.indent() <<	"}" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTINIT)
	{
		str.indent() << "uniform sampler2D texUnit"<< _index <<";" << str.eol();
		char tmp[256];
		sprintf(tmp,"textureColor%d", _index);
		str.indent() << "vec4 " << tmp << " = vec4(1.0);" << str.eol();
	}
	else if (location == ShaderMode::FRAGMENTMAIN)
	{
		char tmp[256];
		sprintf(tmp,"textureColor%d", _index);
		str.indent() << tmp << " = texture2D( texUnit"<< _index <<", gl_TexCoord["<< _index <<"].xy );" << str.eol();
//		str.indent() << "fragColor = vec4(1.0, 1.0, 0.0, " << tmp << ".w);" << str.eol();

	}
}

bool ShaderModeTexEnv::_typeTraversed = false;

ShaderModeTexEnv::ShaderModeTexEnv(int envNum)	:
		MultiShaderMode("FFTextureEnv", envNum, ShaderMode::FRAGMENTINIT | ShaderMode::FRAGMENTFUNCTION | ShaderMode::FRAGMENTMAIN, true)
{ 
	char tmp[256];
	sprintf(tmp,"textureColor%d", _index);
	_inputs.push_back(ShaderMode::Variable("vec4", tmp, "1.0, 1.0, 1.0, 1.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeTexEnv::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index)
{
	if ((location == ShaderMode::FRAGMENTINIT) && !_typeTraversed)
	{
		_typeTraversed = true;
		str.indent() << "uniform int TexEnvMode[8];" << str.eol();
	}
    else if ((location == ShaderMode::FRAGMENTFUNCTION) && !_typeTraversed)
	{
		_typeTraversed = true;

		str.indent() << "const int REPLACE  = 0;" << str.eol();
		str.indent() << "const int MODULATE = 1;" << str.eol();
		str.indent() << "const int DECAL    = 2;" << str.eol();
		str.indent() << "const int BLEND    = 3;" << str.eol();
		str.indent() << "const int ADD      = 4;" << str.eol();
		str.indent() << "const int COMBINE  = 5;" << str.eol();
		str.indent() << str.eol();
		str.indent() << "void applyTextureEnv(in vec4 texture, in int type, in int index, inout vec4 color)" << str.eol();
		str.indent() << "{" << str.eol();
		str.indent() << "    if (type == REPLACE)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        color = texture;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else if (type == MODULATE)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        color *= texture;" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else if (type == DECAL)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        vec3 temp = mix(color.rgb, texture.rgb, texture.a);" << str.eol();
		str.indent() << "        color = vec4(temp, color.a);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else if (type == BLEND)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << " //       vec3 temp = mix(color.rgb, gl_TextureEnvColor[index].rgb, texture.rgb);" << str.eol();
		str.indent() << "        vec3 temp = mix(color.rgb, gl_TextureEnvColor[0].rgb, texture.rgb);" << str.eol();
		str.indent() << "        color = vec4(temp, color.a * texture.a);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else if (type == ADD)" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        color.rgb += texture.rgb;" << str.eol();
		str.indent() << "        color.a   *= texture.a;" << str.eol();
		str.indent() << "        color = clamp(color, 0.0, 1.0);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "    else // COMBINE" << str.eol();
		str.indent() << "    {" << str.eol();
		str.indent() << "        color = clamp(texture * color, 0.0, 1.0);" << str.eol();
		str.indent() << "    }" << str.eol();
		str.indent() << "}" << str.eol();
		str.indent() << str.eol();
	}
    else if (location == ShaderMode::FRAGMENTMAIN)
	{
		char tmp[256];
		sprintf(tmp,"textureColor%d", _index);
		str.indent() << "applyTextureEnv(" << tmp << ", TexEnvMode[" << _index << "], " << _index << ", fragColor);" << str.eol();
	}
}

bool ShaderModeTexGen::_typeTraversed = false;

ShaderModeTexGen::ShaderModeTexGen(int index)	:
	MultiShaderMode("FFTexGen", index, ShaderMode::VERTEXMAIN, true)
{ 
	_inputs.push_back(ShaderMode::Variable("vec3", "transformedNormal", "0.0, 0.0, 0.0"));
	_inputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "0.0, 0.0, 0.0, 0.0"));

	char tmp[256];
	sprintf(tmp,"texCoord%dGenerated", _index);
	_outputs.push_back(ShaderMode::Variable("bool", tmp, "false"));
}

void ShaderModeTexGen::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXINIT)
	{
		if (!_typeTraversed)
		{
			_typeTraversed = true;
			str.indent() << "uniform int TexGenMode[8];" << str.eol();
		}
		str.indent() << "uniform bool texCoord" << _index << "Generated = false;" << str.eol();
	}
	else if ((location == ShaderMode::VERTEXFUNCTION) && !_typeTraversed)
	{
		_typeTraversed = true;

		str.indent() << "const int OBJECT_LINEAR  = 0;" << str.eol();
		str.indent() << "const int EYE_LINEAR = 1;" << str.eol();
		str.indent() << "const int SPHERE_MAP = 2;" << str.eol();
		str.indent() << "const int NORMAL_MAP = 3;" << str.eol();
		str.indent() << "const int REFLECTION_MAP = 4;" << str.eol();
		str.indent() << str.eol();

		// sphere map helper function
		str.indent() <<	"vec2 sphereMap(in vec3 normal, in vec3 ecPosition3)" << str.eol();
		str.indent() <<	"{" << str.eol();
		str.indent() <<	"    float m;" << str.eol();
		str.indent() <<	"    vec3 r, u;" << str.eol();
		str.indent() <<	"    u = normalize(ecPosition3);" << str.eol();
		str.indent() <<	"    r = reflect(u, normal);" << str.eol();
		str.indent() <<	"    m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));" << str.eol();
		str.indent() <<	"    return vec2 (r.x / m + 0.5, r.y / m + 0.5);" << str.eol();
		str.indent() <<	"}" << str.eol();
		str.indent() << str.eol();

		// reflection map helper function
		str.indent() <<	"vec3 reflectionMap(in vec3 normal, in vec3 ecPosition3)" << str.eol();
		str.indent() <<	"{" << str.eol();
		str.indent() <<	"    float NdotU, m;" << str.eol();
		str.indent() <<	"    vec3 u;" << str.eol();
		str.indent() <<	"    u = normalize(ecPosition3);" << str.eol();
		str.indent() <<	"    return (reflect(u, normal));" << str.eol();
		str.indent() <<	"}" << str.eol();
		str.indent() << str.eol();

		// fixed function texture coordinate generation
		str.indent() <<	"void ftexgen(in int i, in int type, in vec3 normal, in vec4 ecPosition)" << str.eol();
		str.indent() <<	"{" << str.eol();
		str.indent() <<	"    if (type == OBJECT_LINEAR)" << str.eol();
		str.indent() <<	"    {" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].s = dot( gl_Vertex, gl_ObjectPlaneS[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].t = dot( gl_Vertex, gl_ObjectPlaneT[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].p = dot( gl_Vertex, gl_ObjectPlaneR[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].q = dot( gl_Vertex, gl_ObjectPlaneQ[i] );" << str.eol();
		str.indent() <<	"    }" << str.eol();
		str.indent() <<	"    else if (type == EYE_LINEAR)" << str.eol();
		str.indent() <<	"    {" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].s = dot( ecPosition, gl_EyePlaneS[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].t = dot( ecPosition, gl_EyePlaneT[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].p = dot( ecPosition, gl_EyePlaneR[i] );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i].q = dot( ecPosition, gl_EyePlaneQ[i] );" << str.eol();
		str.indent() <<	"    }" << str.eol();
		str.indent() <<	"    else if (type == SPHERE_MAP)" << str.eol();
		str.indent() <<	"    {" << str.eol();
		str.indent() <<	"        vec3 ecPosition3 = (vec3(ecPosition))/ecPosition.w;" << str.eol();
		str.indent() <<	"        vec2 sMap = sphereMap( normal, ecPosition3 );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i] = vec4(sMap, 0.0, 1.0 );" << str.eol();
		str.indent() <<	"    }" << str.eol();
		str.indent() <<	"    else if (type == NORMAL_MAP)" << str.eol();
		str.indent() <<	"    {" << str.eol();
		str.indent() <<	"        gl_TexCoord[i] = vec4( normal, 1.0 );" << str.eol();
		str.indent() <<	"    }" << str.eol();
		str.indent() <<	"    else // REFLECTION_MAP" << str.eol();
		str.indent() <<	"    {" << str.eol();
		str.indent() <<	"        vec3 ecPosition3 = (vec3(ecPosition))/ecPosition.w;" << str.eol();
		str.indent() <<	"        vec3 reflection = reflectionMap( normal, ecPosition3 );" << str.eol();
		str.indent() <<	"        gl_TexCoord[i] = vec4( reflection, 1.0);" << str.eol();
		str.indent() <<	"    }" << str.eol();
		str.indent() <<	"}" << str.eol();
		str.indent() << str.eol();
	}
	else if (location == ShaderMode::VERTEXMAIN)
    {
        str.indent() <<	"ftexgen(" << _index << ", TexGenMode[" << _index << "], transformedNormal, ecPosition);" << str.eol();
		str.indent() <<	"texCoord" << _index << "Generated = true;" << str.eol();
		
    }
}



// Texture coordinate modifier
ShaderModeTexMat::ShaderModeTexMat() :
	ShaderMode("FFTexMat", ShaderMode::VERTEXMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "TexCoord", "0.0, 0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "TexCoord", "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeTexMat::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		str.indent() << "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_TexCoord[0];" << str.eol();
	}
}

// ColorMatrix
ShaderModeColorMatrix::ShaderModeColorMatrix() :
	ShaderMode("FFColorMatrix", ShaderMode::VERTEXMAIN | ShaderMode::FRAGMENTMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "fragColor", "0.0, 0.0, 0.0, 0.0"));
	_outputs.push_back(ShaderMode::Variable("vec4", "fragColor", "1.0, 1.0, 1.0, 1.0"));
}

void ShaderModeColorMatrix::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::FRAGMENTMAIN)
	{
		str.indent() << "fragColor = gl_ColorMatrix * fragColor;" << str.eol();
	}
}

bool ShaderModeClipPlane::_typeTraversed = false;

// Root node for clipping
ShaderModeClipPlane::ShaderModeClipPlane(int planeNumber) :
	MultiShaderMode("FFClipPlane", planeNumber, ShaderMode::VERTEXMAIN)
{ 
	_inputs.push_back(ShaderMode::Variable("vec4", "ecPosition", "0.0, 0.0, 0.0, 0.0"));
}

void ShaderModeClipPlane::writeSourceImplementation(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	if (location == ShaderMode::VERTEXMAIN)
	{
		if (!_typeTraversed)
		{
			_typeTraversed = true;
			str.indent() << "gl_ClipVertex = ecPosition;" << str.eol();
		}
		str.indent() << "gl_ClipCoord[" << _index << "] = dot(ecPosition, gl_ClipPlane[" << _index << "]);" << str.eol();
	}
}





