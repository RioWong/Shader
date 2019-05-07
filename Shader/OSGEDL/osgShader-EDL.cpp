#include "../Common/Common.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Camera>
#include <osg/Program>
#include <iostream>

//ͨ��һ
static const char* edlshaderVert = {
    "void main()                           \n"
    "{                                       \n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0; \n"
    "    gl_Position = ftransform();         \n"
    "}                                       \n"
};
//ƬԪ��ɫ��
static const char* edlshaderFrag = {
    "uniform	sampler2D	s1_color;                                                                                            \n"
    "uniform	sampler2D	s2_depth;                                                                                            \n"
    "                                                                                                                            \n"
    "uniform float		Pix_scale;		    //	(relative) pixel scale in image                                                  \n"
    "uniform vec2		Neigh_pos_2D[8];	//	array of neighbors (2D positions)                                                \n"
    "uniform float		Exp_scale;			//	exponential scale factor (for computed AO)                                       \n"
    "                                                                                                                            \n"
    "uniform float		Zm;					//	minimal depth in image                                                           \n"
    "uniform float		ZM;					//	maximal depth in image                                                           \n"
    "                                                                                                                            \n"
    "uniform float		Sx;                                                                                                      \n"
    "uniform float		Sy;                                                                                                      \n"
    "                                                                                                                            \n"
    "uniform float		Zoom;				// image display zoom (so as to always use - approximately - the same pixels)        \n"
    "uniform int			PerspectiveMode;	// whether perspective mode is enabled (1) or not (0) - for z-Buffer compensation\n"
    "                                                                                                                            \n"
    "uniform vec3		Light_dir;                                                                                               \n"
    "/**************************************************/                                                                        \n"
    "                                                                                                                            \n"
    "                                                                                                                            \n"
    "//  Obscurance (pseudo angle version)                                                                                       \n"
    "//	z		neighbour relative elevation                                                                                     \n"
    "//	dist	distance to the neighbourx                                                                                       \n"
    "float obscurance(float z, float dist)                                                                                       \n"
    "{                                                                                                                           \n"
    "    return max(0.0, z) / dist;                                                                                              \n"
    "}                                                                                                                           \n"
    "                                                                                                                            \n"
    "float ztransform(float z_b)                                                                                                 \n"
    "{                                                                                                                           \n"
    "    if (PerspectiveMode == 1)                                                                                               \n"
    "    {                                                                                                                       \n"
    "        //'1/z' depth-buffer transformation correction                                                                      \n"
    "        float z_n = 2.0 * z_b - 1.0;                                                                                        \n"
    "        z_b = 2.0 * Zm / (ZM + Zm - z_n * (ZM - Zm));                                                                       \n"
    "        z_b = z_b * ZM / (ZM - Zm);                                                                                         \n"
    "    }                                                                                                                       \n"
    "                                                                                                                            \n"
    "    return clamp(1.0 - z_b, 0.0, 1.0);                                                                                      \n"
    "}                                                                                                                           \n"
    "                                                                                                                            \n"
    "float computeObscurance(float depth, float scale)                                                                           \n"
    "{                                                                                                                           \n"
    "    // Light-plane point                                                                                                    \n"
    "    vec4 P = vec4(Light_dir.xyz, -dot(Light_dir.xyz, vec3(0.0, 0.0, depth)));                                               \n"
    "                                                                                                                            \n"
    "    float sum = 0.0;                                                                                                        \n"
    "                                                                                                                            \n"
    "    // contribution of each neighbor                                                                                        \n"
    "    for (int c = 0; c < 8; c++)                                                                                             \n"
    "    {                                                                                                                       \n"
    "        vec2 N_rel_pos = scale * Zoom / vec2(Sx, Sy) * Neigh_pos_2D[c];	//neighbor relative position                     \n"
    "        vec2 N_abs_pos = gl_TexCoord[0].st + N_rel_pos;					//neighbor absolute position                     \n"
    "                                                                                                                            \n"
    "        //version with background shading                                                                                   \n"
    "        float Zn = ztransform(texture2D(s2_depth, N_abs_pos).r);		//depth of the real neighbor                         \n"
    "        float Znp = dot(vec4(N_rel_pos, Zn, 1.0), P);				//depth of the in-plane neighbor                         \n"
    "                                                                                                                            \n"
    "        sum += obscurance(Znp, scale);                                                                                      \n"
    "    }                                                                                                                       \n"
    "                                                                                                                            \n"
    "    return	sum;                                                                                                             \n"
    "}                                                                                                                           \n"
    "                                                                                                                            \n"
    "void main(void)                                                                                                             \n"
    "{                                                                                                                           \n"
    "    //ambient occlusion                                                                                                     \n"
    "    vec3 rgb = texture2D(s1_color, gl_TexCoord[0].st).rgb;                                                                  \n"
    "    float depth = ztransform(texture2D(s2_depth, gl_TexCoord[0].st).r);                                                     \n"
    "                                                                                                                            \n"
    "    if (depth > 0.01)                                                                                                       \n"
    "    {                                                                                                                       \n"
    "        float f = computeObscurance(depth, Pix_scale);                                                                      \n"
    "        f = exp(-Exp_scale*f);                                                                                              \n"
    "                                                                                                                            \n"
    "        gl_FragData[0] = vec4(f*rgb, 1.0);                                                                                  \n"
    "    }                                                                                                                       \n"
    "    else                                                                                                                    \n"
    "    {                                                                                                                       \n"
    "        gl_FragData[0] = vec4(rgb, 1.0);                                                                                    \n"
    "    }                                                                                                                       \n"
    "}                                                                                                                           \n"
};
//ͨ����
static const char* edlmixVert = {
    "void main()                            \n"
    "{                                      \n"
    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "    gl_Position = ftransform();        \n"
    "}                                      \n"
};
//ƬԪ��ɫ��
static const char* edlmixFrag = {
    "uniform sampler2D	s2_I1;	//	X1 scale                              \n"
    "uniform sampler2D	s2_I2;	//	X2 scale                              \n"
    "uniform sampler2D	s2_I4;	//	X4 scale                              \n"
    "uniform sampler2D	s2_D;	// initial depth texture                  \n"
    "//                                                                   \n"
    "uniform float		A0;                                               \n"
    "uniform float		A1;                                               \n"
    "uniform float		A2;                                               \n"
    "uniform int		absorb;                                           \n"
    "/**************************************************/                 \n"
    "                                                                     \n"
    "void main(void)                                                      \n"
    "{                                                                    \n"
    "    float d = texture2D(s2_D, gl_TexCoord[0].st).r;                  \n"
    "                                                                     \n"
    "    //gray level only version                                        \n"
    "    /*float t = 1.0;                                                 \n"
    "    if(d>0.99 && absorb==1)                                          \n"
    "    {                                                                \n"
    "    t = texture2D(s2_I1,gl_TexCoord[0].st).r;                        \n"
    "    }                                                                \n"
    "    else                                                             \n"
    "    {                                                                \n"
    "    float t1 = texture2D(s2_I1,gl_TexCoord[0].st).r;                 \n"
    "    float t2 = texture2D(s2_I2,gl_TexCoord[0].st).r;                 \n"
    "    float t4 = texture2D(s2_I4,gl_TexCoord[0].st).r;                 \n"
    "    t = (A0*t1 + A1*t2 + A2*t4) /(A0+A1+A2);                         \n"
    "    }                                                                \n"
    "                                                                     \n"
    "    gl_FragData[0] = vec4(t,t,t,1.);                                 \n"
    "    */                                                               \n"
    "                                                                     \n"
    "    if (d > 0.99)                                                    \n"
    "    {                                                                \n"
    "        gl_FragData[0].rgb = texture2D(s2_I1, gl_TexCoord[0].st).rgb;\n"
    "        gl_FragData[0].a = 1.;                                       \n"
    "        return;                                                      \n"
    "    }                                                                \n"
    "                                                                     \n"
    "    //color version                                                  \n"
    "    vec3 C;                                                          \n"
    "    if (d>0.99 && absorb == 1)                                       \n"
    "    {                                                                \n"
    "        C = texture2D(s2_I1, gl_TexCoord[0].st).rgb;                 \n"
    "    }                                                                \n"
    "    else                                                             \n"
    "    {                                                                \n"
    "        vec3 C1 = texture2D(s2_I1, gl_TexCoord[0].st).rgb;           \n"
    "        vec3 C2 = texture2D(s2_I2, gl_TexCoord[0].st).rgb;           \n"
    "        vec3 C4 = texture2D(s2_I4, gl_TexCoord[0].st).rgb;           \n"
    "        C = (A0*C1 + A1*C2 + A2*C4) / (A0 + A1 + A2);                \n"
    "    }                                                                \n"
    "                                                                     \n"
    "    gl_FragData[0] = vec4(C.x, C.y, C.z, 1.);                        \n"
    "}                                                                    \n"
};



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
    //ͨ��һ
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

    //��RTT
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
        std::cout << "ERRIR" << std::endl;
    }
    quadFirst->addChild(screenquad);
    osg::StateSet* stateset = quadFirst->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(1, textureFirst2, osg::StateAttribute::ON);
    stateset->setTextureAttributeAndModes(2, textureFirst, osg::StateAttribute::ON);

    osg::Program* programFirst = new osg::Program;
    programFirst->addShader(new osg::Shader(osg::Shader::VERTEX, edlshaderVert));//��һ��shader
    programFirst->addShader(new osg::Shader(osg::Shader::FRAGMENT, edlshaderFrag));
    stateset->setAttributeAndModes(programFirst, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("s1_color", 1));
    stateset->addUniform(new osg::Uniform("s2_depth", 0));
    //stateset->addUniform(new osg::Uniform("Sx", static_cast<float>(m_screenWidth)));
    //stateset->addUniform(new osg::Uniform("Sy", static_cast<float>(m_screenHeight)));
    //float lightMod = perspectiveMode ? 3.0f : static_cast<float>(std::sqrt(2.0 * std::max(parameters.zoom, 0.7))); //1.41
    /*
            //ccEDLFilter.cpp
            m_EDLShader->setUniformValue("s1_color", 1);
            m_EDLShader->setUniformValue("s2_depth", 0);
            m_EDLShader->setUniformValue("Sx", static_cast<float>(m_screenWidth));
            m_EDLShader->setUniformValue("Sy", static_cast<float>(m_screenHeight));
            m_EDLShader->setUniformValue("Zoom", lightMod);
            m_EDLShader->setUniformValue("PerspectiveMode", perspectiveMode);
            m_EDLShader->setUniformValue("Pix_scale", static_cast<float>(scale));
            m_EDLShader->setUniformValue("Exp_scale", m_expScale);
            m_EDLShader->setUniformValue("Zm", static_cast<float>(parameters.zNear));
            m_EDLShader->setUniformValue("ZM", static_cast<float>(parameters.zFar));
            m_EDLShader->setUniformValueArray("Light_dir", reinterpret_cast<const GLfloat*>(m_lightDir), 1, 3);
            m_EDLShader->setUniformValueArray("Neigh_pos_2D", reinterpret_cast<const GLfloat*>(m_neighbours), 8, 2);
    */
    passFirst->addChild(quadFirst);

    //---------------------------------------------------------------------------
    //ͨ����
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
    programsecond->addShader(new osg::Shader(osg::Shader::VERTEX, edlmixVert));//��һ��shader
    programsecond->addShader(new osg::Shader(osg::Shader::FRAGMENT, edlmixFrag));
    stateset->setAttributeAndModes(programsecond, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("RT", 3));
    stateset->addUniform(new osg::Uniform("sampleDist0", 0.0198f));
    secondPass->addChild(quadsecond);
    //-----------------------------------------------------------------
    //---------------------------------------------------------------------------
    ////ͨ����
    //osg::Group* thirdPass = new osg::Group;
    ////RTT
    //osg::Texture2D* textureThird = new osg::Texture2D;
    //textureThird->setTextureSize(128, 128);
    //textureThird->setInternalFormat(GL_RGBA);
    //textureThird->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    //textureThird->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    //textureThird->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    //textureThird->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    //textureThird->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    //osg::Camera* cameraThird = new osg::Camera;
    //cameraThird->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    //cameraThird->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //cameraThird->setViewport(0, 0, 128, 128);
    //cameraThird->setRenderTargetImplementation(rm);
    //cameraThird->attach(osg::Camera::COLOR_BUFFER, textureThird);
    //cameraThird->addChild(secondPass);
    //thirdPass->addChild(cameraThird);

    //osg::Group* quadthird = new osg::Group;
    //quadthird->addChild(screenquad);
    //stateset = quadthird->getOrCreateStateSet();
    //stateset->setTextureAttributeAndModes(4, textureThird, osg::StateAttribute::ON);

    //osg::Program* programThird = new osg::Program;
    //programThird->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderThird));
    //programThird->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderThird));
    //stateset->setAttributeAndModes(programThird, osg::StateAttribute::ON);

    //stateset->addUniform(new osg::Uniform("RT1", 4));
    //stateset->addUniform(new osg::Uniform("sampleDist1", 0.0192f));
    //thirdPass->addChild(quadthird);
    ////-----------------------------------------------------------------

    ////ͨ����
    //osg::Group* forthPass = new osg::Group;
    ////RTT
    //osg::Texture2D* textureForth = new osg::Texture2D;
    //textureForth->setTextureSize(512, 512);
    //textureForth->setInternalFormat(GL_RGBA);
    //textureForth->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    //textureForth->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    //textureForth->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    //textureForth->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    //textureForth->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

    //osg::Camera* cameraForth = new osg::Camera;
    //cameraForth->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    //cameraForth->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //cameraForth->setViewport(0, 0, 512, 512);
    //cameraForth->setRenderTargetImplementation(rm);
    //cameraForth->attach(osg::Camera::COLOR_BUFFER, textureForth);
    //cameraForth->addChild(thirdPass);
    //forthPass->addChild(cameraForth);

    //osg::Group* quadForth = new osg::Group;
    //quadForth->addChild(screenquad);
    //stateset = quadForth->getOrCreateStateSet();
    //stateset->setTextureAttributeAndModes(5, textureFirst2, osg::StateAttribute::ON);
    //stateset->setTextureAttributeAndModes(6, textureForth, osg::StateAttribute::ON);

    //osg::Program* programForth = new osg::Program;
    //programForth->addShader(new osg::Shader(osg::Shader::VERTEX, vertShaderForth));
    //programForth->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShaderForth));
    //stateset->setAttributeAndModes(programForth, osg::StateAttribute::ON);

    //stateset->addUniform(new osg::Uniform("tex0", 5));
    //stateset->addUniform(new osg::Uniform("tex1", 6));
    //forthPass->addChild(quadForth);


    viewer->setSceneData(secondPass);
    viewer->addEventHandler(new CameraEvent);
    viewer->addEventHandler(new ChangeWindow);
    viewer->run();
    return 0;
}