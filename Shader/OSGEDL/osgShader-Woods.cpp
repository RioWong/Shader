#include "../Common/Common.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/Shader>
#include <osg/Texture3D>

#if 0 
//顶点着色器
static const char* vertShader = {
	"uniform float scale; \n"

	"varying vec3 vScaledPosition;\n"
	"varying vec3 vNormalES; \n"
	"varying vec3 vViewVec; \n"

	"void main()\n"
	"{\n"
	"	gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	vScaledPosition = scale * gl_Vertex.xyz;\n"
	"   vNormalES = gl_NormalMatrix* gl_Normal; \n"
	"   vViewVec = - vec3(gl_ModelViewMatrix*gl_Vertex); \n"
	"}\n"
};

//片元着色器
static const char* fragShader = {
	"uniform vec4 darkWood;\n"
	"uniform vec4 liteWood;\n"
	"uniform vec4 lightDir; \n"

	"uniform float frequency; \n"
	"uniform float noiseScale; \n"
	"uniform float ringScale; \n"
	"uniform sampler3D Noise; \n"

	"varying vec3 vScaledPosition;\n"
	"varying vec3 vNormalES; \n"
	"varying vec3 vViewVec; \n"
	

	"void main()\n"
	"{\n"
	"	float snoise = 2.0 * texture3D(Noise, vScaledPosition).x - 1.0; \n"
	"	float ring = fract(frequency * vScaledPosition.z + noiseScale * snoise); "
	"	ring *= 4.0*(1.0-ring);\n"
	"	float lrp = pow(ring, ringScale) + snoise; \n"

	"	vec4 base = mix(darkWood, liteWood, lrp);\n"
	"	vec3 normal = normalize(vNormalES);"
	"	float diffuse = 0.5 + 0.5*dot(lightDir.xyz, normal);\n"
	"   float specular = pow(clamp(dot(reflect(-normalize(vViewVec), normal), lightDir.xyz), 0.0, 1.0), 12.0);\n"
	"	gl_FragColor = base * diffuse * 0.6 + specular * 0.4;\n"
	"} \n"
};


static osg::Texture3D* make3DNoiseTexture()
{
	osg::Texture3D* noiseTexture = new osg::Texture3D;
	noiseTexture->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR);
	noiseTexture->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR);
	noiseTexture->setWrap(osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT);
	noiseTexture->setWrap(osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT);
	noiseTexture->setWrap(osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT);
	noiseTexture->setImage(osgDB::readImageFile("../Images/Noise.png"));
	return noiseTexture;
}


int main()
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	osg::Node* ele = osgDB::readNodeFile("../ElephantBody.3ds");
	osg::StateSet* ss = ele->getOrCreateStateSet();
	osg::Program *program = new osg::Program;
	osg::Texture3D *noiseTexture = make3DNoiseTexture();
	ss->setTextureAttribute(1, noiseTexture);


	ss->addUniform(new osg::Uniform("darkWood", osg::Vec4(0.4, 0.2, 0.0, 1.0)));
	ss->addUniform(new osg::Uniform("lightDir", osg::Vec4(0.4, 0.4, 0.8, 1.0)));
	ss->addUniform(new osg::Uniform("liteWood", osg::Vec4(0.9, 0.5, 0.1, 1.0)));
	ss->addUniform(new osg::Uniform("scale", 0.025f));
	ss->addUniform(new osg::Uniform("frequency", 6.0f));
	ss->addUniform(new osg::Uniform("noiseScale", 2.6f));
	ss->addUniform(new osg::Uniform("ringScale", 2.0f));
	ss->addUniform(new osg::Uniform("Noise",1.0f));


	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertShader));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	viewer->setSceneData(ele);
	viewer->addEventHandler(new ChangeWindow);
	viewer->run();
	return 0;
}
#endif