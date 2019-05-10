#include "../Common/Common.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Uniform>
#include <osg/Shader>




//顶点着色器
static const char* vertShader = {
	"uniform vec3 LightPosition;\n"
	"const float SpecularContribution = 0.3;\n"
	"const float DiffuseContribution = 1.0 - SpecularContribution;\n"
	"varying float LightIntensity;\n"
	"varying vec2 MCposition; \n"
	"void main()\n"
	"{\n"
	"vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
	"vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);\n"
	"vec3 lightVec = normalize(LightPosition - ecPosition);\n"
	"vec3 reflectVec = reflect(-lightVec, tnorm);\n"
	"vec3 viewVec = normalize(-ecPosition);\n"
	"float diffuse = max(dot(lightVec, tnorm), 0.0);\n"
	"float spec = 0.0;\n"
	"if(diffuse > 0.0)\n"
	"{\n"
	"	spec = max(dot(reflectVec, viewVec), 0.0);\n"
	"	spec = pow(spec, 16.0);\n"
	"}\n"
	"LightIntensity = DiffuseContribution*diffuse + SpecularContribution*spec;\n"
	"	MCposition = gl_Vertex.xy;\n"
	"	gl_Position= ftransform();\n"
	"}"
};

//片元着色器
static const char* fragShader = {
	"uniform vec3 BrickColor, MortarColor;\n"
	"uniform vec2 BrickSize;\n"
	"uniform vec2 BrickPct;\n"
	"varying vec2 MCposition;\n"
	"varying float LightIntensity;\n"
	"void main()\n"
	"{\n"
	"	vec3 color;\n"
	"	vec2 position, useBrick;\n"
	"	position = MCposition/BrickSize;\n"
	"	if(fract(position.y*0.5)>0.5)\n"
	"		position.x += 0.5;\n"
	"	position = fract(position);\n"
	"	useBrick = step(position, BrickPct);\n"
	"	color = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);\n"
	"   color *= LightIntensity;\n"
	"	gl_FragColor = vec4(color, 1.0);\n"
	"}"
};

#if 0

int main()
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	osg::Node* glider = osgDB::readNodeFile("cow.osg");
	osg::StateSet* stateset = glider->getOrCreateStateSet();
	osg::Program* program = new osg::Program;

	osg::Uniform* BrickColor = new osg::Uniform("BrickColor", osg::Vec3(1.0, 0.0, 0.0));
	osg::Uniform* MortarColor = new osg::Uniform("MortarColor", osg::Vec3(0.8, 0.8, 0.8));
	osg::Uniform* BrickSize = new osg::Uniform("BrickSize", osg::Vec2(1.1, 1.1));
	osg::Uniform* BrickPct = new osg::Uniform("BrickPct", osg::Vec2(0.9, 0.95));
	osg::Uniform* LightPosition = new osg::Uniform("LightPosition", osg::Vec3(1.0, 1.0, 1.0));
	

	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertShader));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
	stateset->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateset->addUniform(BrickColor);
	stateset->addUniform(MortarColor);
	stateset->addUniform(BrickSize);
	stateset->addUniform(BrickPct);
	stateset->addUniform(LightPosition);

	viewer->setSceneData(glider);
	viewer->addEventHandler(new ChangeWindow);
	viewer->run();
	return 0;
}
#endif