#include "../Common/Common.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Camera>
#include <osg/Program>
#include <iostream>

//通道一
static const char* vertShaderFirst = {
    "varying vec2 texCoord;           \n"
    "void main(void)                  \n"
    "{                                \n"
    "	vec2 p = sign(gl_Vertex.xy);    \n"
    "	gl_Position = vec4(p, 0.0, 1.0);\n"
    "	texCoord = p*0.5 + 0.5;         \n"
    "}                                \n"
};
//片元着色器
static const char* fragShaderFirst = {
    "	uniform sampler2D RT;                                                     \n"
    "	uniform sampler2D LUM;                                                    \n"
    "	                                                                          \n"
    "	varying vec2 texCoord;                                                    \n"
    "	vec4 saturate(vec4 inValue)                                               \n"
    "	{                                                                         \n"
    "		vec4  outValue;                                                         \n"
    "		outValue.x = clamp(inValue.x, 0.0, 1.0);                                \n"
    "		outValue.y = clamp(inValue.y, 0.0, 1.0);                                \n"
    "		outValue.z = clamp(inValue.z, 0.0, 1.0);                                \n"
    "		outValue.w = clamp(inValue.w, 0.0, 1.0);                                \n"
    "                                                                           \n"
    "		return outValue;                                                        \n"
    "	}                                                                         \n"
    "                                                                           \n"
    "	void main(void)                                                           \n"
    "	{                                                                         \n"
    "		vec4 tex;                                                               \n"
    "		vec4 vTex0 = texture2D(LUM, (texCoord + vec2(-0.5, -0.5)));             \n"
    "		vec4 vTex1 = texture2D(LUM, (texCoord + vec2(-0.5, 0.5)));              \n"
    "		vec4 vTex2 = texture2D(LUM, (texCoord + vec2(0.5, -0.5)));              \n"
    "		vec4 vTex3 = texture2D(LUM, (texCoord + vec2(0.5, -0.5)));              \n"
    "                                                                           \n"
    "		vec4 vAvg = saturate(vTex0*0.25 + vTex1*0.25 + vTex2*0.25 + vTex3*0.25);\n"
    "		vAvg = 4*vAvg*vAvg;                                                     \n"
    "                                                                           \n"
    "		vec4 vGlareAmount;                                                      \n"
    "		vGlareAmount.xyz = dot(vec3(0.3, 0.59, 0.11), vAvg.xyz);                \n"
    "		vGlareAmount.w = 0.025;                                                 \n"
    "                                                                           \n"
    "		vec4 vScreen = texture2D(RT, texCoord);                                 \n"
    "                                                                           \n"
    "		vec4 vBrightest = saturate(vScreen.xyzw  - vec4(0.4, 0.4, 0.4, 0.4))*3.0;    \n"
    "		vBrightest.xyz = 4*vBrightest.xyz*vBrightest.xyz;                       \n"
    "                                                                           \n"
    "		vec4 finalCol;                                                          \n"
    "		finalCol.xyz = vBrightest.xyz*(1-vGlareAmount.xyz);                     \n"
    "		finalCol.w = 1;                                                         \n"
    "		gl_FragColor = finalCol;                                                \n"
    "	}                                                                         \n"
};
//通道二
static const char* vertShaderSecond = {
    "varying vec2 vTexCoord;           \n"
    "void main(void)                   \n"
    "{                                 \n"
    "	vec2 Pos = sign(gl_Vertex.xy);   \n"
    "	gl_Position = vec4(Pos.xy, 0, 1);\n"
    "	vTexCoord.x = 0.5*(1.0+Pos.x);   \n"
    "	vTexCoord.y = 0.5*(1.0+Pos.y);   \n"
    "}                                 \n"
};
//片元着色器
static const char* fragShaderSecond = {
    "	uniform float sampleDist0;                      \n"
    "	uniform sampler2D RT;                           \n"
    "                                                 \n"
    "	varying vec2 vTexCoord;                         \n"
    "	void main(void)                                 \n"
    "	{                                               \n"
    "		vec2 samples00 = vec2(-0.326212, -0.405805);  \n"
    "		vec2 samples01 = vec2(-0.840144, -0.073580);  \n"
    "		vec2 samples02 = vec2(-0.695914,  0.457137);  \n"
    "		vec2 samples03 = vec2(-0.203345,  0.620716);  \n"
    "		vec2 samples04 = vec2( 0.962340, -0.194983);  \n"
    "		vec2 samples05 = vec2( 0.473434, -0.480026);  \n"
    "		vec2 samples06 = vec2( 0.519456,  0.767022);  \n"
    "		vec2 samples07 = vec2( 0.185461, -0.893124);  \n"
    "		vec2 samples08 = vec2( 0.507431,  0.064425);  \n"
    "		vec2 samples09 = vec2( 0.896420,  0.412458);  \n"
    "		vec2 samples10 = vec2(-0.321940, -0.932615);  \n"
    "		vec2 samples11 = vec2(-0.791559, -0.597705);  \n"
    "		                                              \n"
    "		vec2 newCoord;                                \n"
    "		vec4 sum = texture2D(RT, vTexCoord);          \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples00; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples01; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples02; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples03; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples04; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples05; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples06; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples07; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples08; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples09; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples10; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist0*samples11; \n"
    "		sum += texture2D(RT, newCoord);              \n"
    "                                                 \n"
    "		sum /= 13.0;                                  \n"
    "		gl_FragColor = vec4(sum);                     \n"
    "	}                                               \n"
};

//通道三
static const char* vertShaderThird = {
    "varying vec2 vTexCoord;           \n"
    "void main(void)                   \n"
    "{                                 \n"
    "	vec2 Pos = sign(gl_Vertex.xy);   \n"
    "	gl_Position = vec4(Pos.xy, 0, 1);\n"
    "	vTexCoord.x = 0.5*(1.0+Pos.x);   \n"
    "	vTexCoord.y = 0.5*(1.0+Pos.y);   \n"
    "}                                 \n"
};
//片元着色器
static const char* fragShaderThird = {
    "	uniform float sampleDist1;                      \n"
    "	uniform sampler2D RT1;                           \n"
    "                                                 \n"
    "	varying vec2 vTexCoord;                         \n"
    "	void main(void)                                 \n"
    "	{                                               \n"
    "		vec2 samples00 = vec2(-0.326212, -0.405805); \n"
    "		vec2 samples01 = vec2(-0.840144, -0.073580); \n"
    "		vec2 samples02 = vec2(-0.695914,  0.457137); \n"
    "		vec2 samples03 = vec2(-0.203345,  0.620716); \n"
    "		vec2 samples04 = vec2( 0.962340, -0.194983); \n"
    "		vec2 samples05 = vec2( 0.473434, -0.480026); \n"
    "		vec2 samples06 = vec2( 0.519456,  0.767022); \n"
    "		vec2 samples07 = vec2( 0.185461, -0.893124); \n"
    "		vec2 samples08 = vec2( 0.507431,  0.064425); \n"
    "		vec2 samples09 = vec2( 0.896420,  0.412458); \n"
    "		vec2 samples10 = vec2(-0.321940, -0.932615); \n"
    "		vec2 samples11 = vec2(-0.791559, -0.597705); \n"
    "		                                              \n"
    "		vec2 newCoord;                                \n"
    "		vec4 sum = texture2D(RT1, vTexCoord);          \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples00; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples01; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples02; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples03; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples04; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples05; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples06; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples07; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples08; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples09; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples10; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		newCoord = vTexCoord + sampleDist1*samples11; \n"
    "		sum += texture2D(RT1, newCoord);              \n"
    "                                                 \n"
    "		sum /= 13.0;                                  \n"
    "		gl_FragColor = vec4(sum);                     \n"
    "	}                                               \n"

};
//通道四
static const char* vertShaderForth = {
    "	varying vec2 texCoord;          \n"
    "	void main(void)                 \n"
    "	{                               \n"
    "		vec2 P = sign(gl_Vertex.xy);  \n"
    "		gl_Position = vec4(P, 0, 1.0);\n"
    "		texCoord = P*0.5 + 0.5;       \n"
    "	}                               \n"
};

//片元着色器
static const char* fragShaderForth = {
    "	uniform sampler2D tex0;                                                \n"
    "	uniform sampler2D tex1;                                                \n"
    "                                                                        \n"
    "	varying vec2 texCoord;                                                 \n"
    "	void main(void)                                                        \n"
    "	{                                                                      \n"
    "		vec4 vScreenMap = texture2D(tex0, texCoord);                         \n"
    "		vec4 vGlareMap = texture2D(tex1, texCoord);                          \n"
    "		vec4 finalColor;                                                     \n"
    "		finalColor.xyz = (vScreenMap.xyz + vGlareMap.xyz*(1-vScreenMap.xyz));\n"
    "		finalColor.w = 1.0;                                                  \n"
    "		gl_FragColor = finalColor;                                           \n"
    "	}                                                                      \n"
};



#if 0
osg::Camera* cameraFirst = new osg::Camera;
osg::Camera* cameraFirst2 = new osg::Camera;


class CameraEvent : public osgGA::GUIEventHandler
{
public:
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osg::Vec3 eye;
        osg::Vec3 center;
        osg::Vec3 up;

        switch (ea.getEventType())
        {
            case osgGA::GUIEventAdapter::KEYDOWN:
            {
                if (ea.getKey() == 'A' || ea.getKey() == 'a')
                {
                    cameraFirst->getViewMatrixAsLookAt(eye, center, up);
                    center = center + osg::Vec3(0.01, 0, 0);
                    cameraFirst->setViewMatrixAsLookAt(eye, center, up);
                    cameraFirst2->getViewMatrixAsLookAt(eye, center, up);
                    center = center + osg::Vec3(0.01, 0, 0);
                    cameraFirst2->setViewMatrixAsLookAt(eye, center, up);
                }
                else if (ea.getKey() == 'D' || ea.getKey() == 'd')
                {
                    cameraFirst->getViewMatrixAsLookAt(eye, center, up);
                    center = center - osg::Vec3(0.01, 0, 0);
                    cameraFirst->setViewMatrixAsLookAt(eye, center, up);
                    cameraFirst2->getViewMatrixAsLookAt(eye, center, up);
                    center = center - osg::Vec3(0.01, 0, 0);
                    cameraFirst2->setViewMatrixAsLookAt(eye, center, up);
                }
                else if (ea.getKey() == 'W' || ea.getKey() == 'w')
                {
                    cameraFirst->getViewMatrixAsLookAt(eye, center, up);
                    osg::Vec3 delta = center - eye;
                    eye = eye + delta / 2.0;
                    cameraFirst->setViewMatrixAsLookAt(eye, center, up);
                    cameraFirst2->getViewMatrixAsLookAt(eye, center, up);
                    delta = center - eye;
                    eye = eye + delta / 2.0;
                    cameraFirst2->setViewMatrixAsLookAt(eye, center, up);
                }
                else if (ea.getKey() == 'S' || ea.getKey() == 's')
                {
                    cameraFirst->getViewMatrixAsLookAt(eye, center, up);
                    osg::Vec3 delta = center - eye;
                    eye = eye - delta / 2.0;
                    cameraFirst->setViewMatrixAsLookAt(eye, center, up);
                    cameraFirst2->getViewMatrixAsLookAt(eye, center, up);
                    delta = center - eye;
                    eye = eye - delta / 2.0;
                    cameraFirst2->setViewMatrixAsLookAt(eye, center, up);
                }
                else
                {

                }

            }
            break;
            default:
                break;
        }

        return false;

    }
};




int main()
{
    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
    osg::Node* ceep = osgDB::readNodeFile("../ceep.ive");

    //--------------------------------------------------------------------------
    //通道一
    osg::Group* passFirst = new osg::Group;
    //RTT
    osg::Texture2D* textureFirst = new osg::Texture2D;
    textureFirst->setTextureSize(32, 32);
    textureFirst->setInternalFormat(GL_RGBA);
    textureFirst->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    textureFirst->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    textureFirst->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    textureFirst->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    textureFirst->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    cameraFirst->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    cameraFirst->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cameraFirst->setProjectionMatrixAsPerspective(60.0, 1.0, 0.1, 1000.0);
    cameraFirst->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    cameraFirst->setViewMatrixAsLookAt(osg::Vec3(0.0, -245.0, 0.0), osg::Vec3(0.0, -144.72, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    cameraFirst->setViewport(0, 0, 32, 32);
    osg::Camera::RenderTargetImplementation rm = osg::Camera::FRAME_BUFFER_OBJECT;
    cameraFirst->setRenderTargetImplementation(rm);
    cameraFirst->attach(osg::Camera::COLOR_BUFFER, textureFirst);
    cameraFirst->addChild(ceep);
    passFirst->addChild(cameraFirst);

    //再RTT
    osg::Texture2D* textureFirst2 = new osg::Texture2D;
    textureFirst2->setTextureSize(512, 512);
    textureFirst2->setInternalFormat(GL_RGBA);
    textureFirst2->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    textureFirst2->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    textureFirst2->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    textureFirst2->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    textureFirst2->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    cameraFirst2->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    cameraFirst2->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cameraFirst2->setProjectionMatrixAsPerspective(60.0, 1.0, 0.1, 1000.0);
    cameraFirst2->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    cameraFirst2->setViewMatrixAsLookAt(osg::Vec3(0.0, -245.0, 0.0), osg::Vec3(0.0, -144.72, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    cameraFirst2->setViewport(0, 0, 512, 512);
    cameraFirst2->setRenderTargetImplementation(rm);
    cameraFirst2->attach(osg::Camera::COLOR_BUFFER, textureFirst2, 0, 0, false, 4, 0);
    cameraFirst2->addChild(ceep);
    passFirst->addChild(cameraFirst2);


    osg::Group* quadFirst = new osg::Group;
    osg::Node* screenquad = osgDB::readNodeFile("../ScreenAlignedQuad.3ds");
    if (!screenquad)
    {
        std::cout << "ERROR" << std::endl;
    }
    quadFirst->addChild(screenquad);
    osg::StateSet* stateset = quadFirst->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(1, textureFirst2, osg::StateAttribute::ON);
    stateset->setTextureAttributeAndModes(2, textureFirst, osg::StateAttribute::ON);

    osg::Program* programFirst = new osg::Program;
    programFirst->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderFirst));
    programFirst->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderFirst));
    stateset->setAttributeAndModes(programFirst, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("RT", 1));
    stateset->addUniform(new osg::Uniform("LUM", 2));
    passFirst->addChild(quadFirst);

    //---------------------------------------------------------------------------
    //通道二
    osg::Group* secondPass = new osg::Group;
    //RTT
    osg::Texture2D* texturesecond = new osg::Texture2D;
    texturesecond->setTextureSize(128, 128);
    texturesecond->setInternalFormat(GL_RGBA);
    texturesecond->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    texturesecond->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    texturesecond->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texturesecond->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    texturesecond->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    osg::Camera* camerasecond = new osg::Camera;
    camerasecond->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    camerasecond->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camerasecond->setViewport(0, 0, 128, 128);
    camerasecond->setRenderTargetImplementation(rm);
    camerasecond->attach(osg::Camera::COLOR_BUFFER, texturesecond);
    camerasecond->addChild(passFirst);
    secondPass->addChild(camerasecond);

    osg::Group* quadsecond = new osg::Group;
    quadsecond->addChild(screenquad);
    stateset = quadsecond->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(3, texturesecond, osg::StateAttribute::ON);

    osg::Program* programsecond = new osg::Program;
    programsecond->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderSecond));
    programsecond->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderSecond));
    stateset->setAttributeAndModes(programsecond, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("RT", 3));
    stateset->addUniform(new osg::Uniform("sampleDist0", 0.0198f));
    secondPass->addChild(quadsecond);
    //-----------------------------------------------------------------
    //---------------------------------------------------------------------------
    //通道三
    osg::Group* thirdPass = new osg::Group;
    //RTT
    osg::Texture2D* textureThird = new osg::Texture2D;
    textureThird->setTextureSize(128, 128);
    textureThird->setInternalFormat(GL_RGBA);
    textureThird->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    textureThird->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    textureThird->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    textureThird->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    textureThird->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    osg::Camera* cameraThird = new osg::Camera;
    cameraThird->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    cameraThird->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cameraThird->setViewport(0, 0, 128, 128);
    cameraThird->setRenderTargetImplementation(rm);
    cameraThird->attach(osg::Camera::COLOR_BUFFER, textureThird);
    cameraThird->addChild(secondPass);
    thirdPass->addChild(cameraThird);

    osg::Group* quadthird = new osg::Group;
    quadthird->addChild(screenquad);
    stateset = quadthird->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(4, textureThird, osg::StateAttribute::ON);

    osg::Program* programThird = new osg::Program;
    programThird->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderThird));
    programThird->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderThird));
    stateset->setAttributeAndModes(programThird, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("RT1", 4));
    stateset->addUniform(new osg::Uniform("sampleDist1", 0.0192f));
    thirdPass->addChild(quadthird);
    //-----------------------------------------------------------------
    //通道四
    osg::Group* forthPass = new osg::Group;
    //RTT
    osg::Texture2D* textureForth = new osg::Texture2D;
    textureForth->setTextureSize(512, 512);
    textureForth->setInternalFormat(GL_RGBA);
    textureForth->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    textureForth->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    textureForth->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    textureForth->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    textureForth->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    osg::Camera* cameraForth = new osg::Camera;
    cameraForth->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    cameraForth->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cameraForth->setViewport(0, 0, 512, 512);
    cameraForth->setRenderTargetImplementation(rm);
    cameraForth->attach(osg::Camera::COLOR_BUFFER, textureForth);
    cameraForth->addChild(thirdPass);
    forthPass->addChild(cameraForth);

    osg::Group* quadForth = new osg::Group;
    quadForth->addChild(screenquad);
    stateset = quadForth->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(5, textureFirst2, osg::StateAttribute::ON);
    stateset->setTextureAttributeAndModes(6, textureForth, osg::StateAttribute::ON);

    osg::Program* programForth = new osg::Program;
    programForth->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderForth));
    programForth->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderForth));
    stateset->setAttributeAndModes(programForth, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("tex0", 5));
    stateset->addUniform(new osg::Uniform("tex1", 6));
    forthPass->addChild(quadForth);


    viewer->setSceneData(forthPass);
    viewer->addEventHandler(new CameraEvent);
    viewer->addEventHandler(new ChangeWindow);
    viewer->run();
    return 0;
}
#endif