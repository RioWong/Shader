#include "../Common/Common.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Uniform>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>

static const char* vertShader = {
    "varying vec3 lightVec;                              \n"
    "varying vec3 viewVec;                               \n"
    "varying vec2 texCoord;                              \n"
    "attribute vec3 Tangent;                             \n"
    "void main(void)                                     \n"
    "{                                                   \n"
    "	gl_Position = ftransform();                        \n"
    "	texCoord = gl_MultiTexCoord0.xy;                    \n"
    "                                                    \n"
    "	vec3 n = normalize(gl_NormalMatrix*gl_Normal);     \n"
    "	vec3 t = normalize(gl_NormalMatrix * Tangent);     \n"
    "	vec3 b = cross(n, t);                              \n"
    "                                                    \n"
    "	vec3 v;                                            \n"
    "	vec3 vVertex = vec3(gl_ModelViewMatrix*gl_Vertex); \n"
    "	vec3 lVec = gl_LightSource[0].position.xyz-vVertex;\n"
    "	                                                   \n"
    "	v.x = dot(lVec, t);                                \n"
    "	v.y = dot(lVec, b);                                \n"
    "	v.z = dot(lVec, n);                                \n"
    "	lightVec = v;                                      \n"
    "                                                    \n"
    "	vec3 vVec= -vVertex;                               \n"
    "	v.x = dot(vVec,t);                                 \n"
    "	v.y = dot(-vVec, b);                               \n"
    "	v.z = dot(vVec, n);                                \n"
    "	viewVec = v;                                       \n"
    "}                                                   \n"
};

static const char* fragShader = {
    "varying vec3 lightVec;                                                                              \n"
    "varying vec3 viewVec;                                                                               \n"
    "varying vec2 texCoord;                                                                              \n"
    "uniform sampler2D colorMap;                                                                         \n"
    "uniform sampler2D normalMap;                                                                        \n"
    "uniform sampler2D heightMap;                                                                        \n"
    "void main(void)                                                                                     \n"
    "{                                                                                                   \n"
    "	vec3 lVec = normalize(lightVec);                                                                   \n"
    "	vec3 vVec = normalize(viewVec);                                                                    \n"
    "                                                                                                    \n"
    "	float height = texture2D(heightMap, texCoord).x;                                                   \n"
    "	vec2 newTexCoord = texCoord + ((height*0.04-0.02)*vVec.xy);                                        \n"
    "                                                                                                    \n"
    "	vec4 base = texture2D(colorMap, newTexCoord);                                                      \n"
    "	vec3 bump = normalize(texture2D(normalMap, newTexCoord).xyz*2.0-1.0);                              \n"
    "	bump = normalize(bump);                                                                            \n"
    "                                                                                                    \n"
    "	float diffuse = max(dot(lVec, bump), 0.0);                                                         \n"
    "	float specular = pow(clamp(dot(reflect(-vVec, bump), lVec), 0.0, 1.0), gl_FrontMaterial.shininess);\n"
    "                                                                                                    \n"
    "	vec4 vAmbient = gl_LightSource[0].ambient*gl_FrontMaterial.ambient;                                \n"
    "	vec4 vDiffuse = gl_LightSource[0].diffuse*gl_FrontMaterial.diffuse*diffuse;                        \n"
    "	vec4 vSpecular = gl_LightSource[0].specular*gl_FrontMaterial.specular*specular;                    \n"
    "	gl_FragColor = vAmbient*base + (vDiffuse*base+vSpecular);                                          \n"
    "}                                                                                                   \n"
};


#if 0 
int main()
{
    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::PositionAttitudeTransform> pt = new osg::PositionAttitudeTransform;

    osg::Node* ptNode = osgDB::readNodeFile("../ScreenAlignedQuad.3ds");
    pt->addChild(ptNode);
    root->addChild(pt);

    pt->setAttitude(osg::Quat(osg::PI_2, osg::Vec3(1.0f, 0.0f, 0.0f)));

    //
    osg::Texture2D* rockwall = new osg::Texture2D(osgDB::readImageFile("../TuAo/rockwall.tga"));
    rockwall->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    rockwall->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    rockwall->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    rockwall->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

    osg::Texture2D* rockwall_height = new osg::Texture2D(osgDB::readImageFile("../TuAo/rockwall_height.png"));
    rockwall_height->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    rockwall_height->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    rockwall_height->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    rockwall_height->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

    osg::Texture2D* rockwall_normal = new osg::Texture2D(osgDB::readImageFile("../TuAo/rockwall_normal.png"));
    rockwall_normal->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    rockwall_normal->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    rockwall_normal->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    rockwall_normal->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

    osg::StateSet* ss = ptNode->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(0, rockwall, osg::StateAttribute::ON);
    ss->setTextureAttributeAndModes(1, rockwall_height, osg::StateAttribute::ON);
    ss->setTextureAttributeAndModes(2, rockwall_normal, osg::StateAttribute::ON);

    osg::Program* program = new osg::Program;
    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertShader));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);

    ss->addUniform(new osg::Uniform("colorMap", 0));
    ss->addUniform(new osg::Uniform("heightMap", 1));
    ss->addUniform(new osg::Uniform("normalMap", 2));

    //

    viewer->setSceneData(root);
    viewer->addEventHandler(new ChangeWindow);
    return viewer->run();
}
#endif 