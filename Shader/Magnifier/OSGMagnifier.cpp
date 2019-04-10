/************************************************************************/
/*                                                                      */
/* \                                                                    */
/* \date:20190410                                                       */
/* \author:WLL                                                          */
/* \Contact: Wlvliang@gmail.com                                         */
/* \                                                                    */
/* \brief:A test for osg shader、FBO、RTT.It's used like a magnifier!   */
/* \                                                                    */
/* TODO: long description                                               */
/*                                                                      */
/* \note:All copyright reserved by author!                              */
/*       Any commerical use please contact the author!                  */
/************************************************************************/
#define GLUT_DISABLE_ATEXIT_HACK
#include <stdlib.h>

#include <gl/glut.h>
#include <gl/GL.h>
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/Texture2D"
#include "osgDB/ReadFile"
#include "osgViewer/Viewer"
#include "osg/Drawable"
#include "osgViewer/Viewer"
#include "osg/RenderInfo"
#include "osg/StateSet"
#include "osg/StateAttribute"
#include "osg/TexEnv"
#include "osg/StateSet"
#include "osg/Matrix"
#include "osg/TexGen"
#include "osg/ShapeDrawable"
#include "osg/PositionAttitudeTransform"
#include "osg/Camera"
#include <Windows.h>
#include "osgGA/GUIEventAdapter"
#include "osgGA/GUIEventHandler"
#include "osg/NodeCallback"
#include "osg/Matrixtransform"
#include "osg/Shader"
#include "osg/Program"
//#include <Windows.h>



static int g_mouse_x = 0.0, g_mouse_y = 0.0;
static float g_move_x = 0.0, g_move_y = 0.0;
static float times = 5.0f;
float cameratimes = 16.0f;
const static GLfloat red_color[] = { 1.0f, 0.0f, 0.0f, 0.5f };
const static GLfloat green_color[] = { 0.0f, 1.0f, 0.0f, 0.5f };
const static GLfloat blue_color[] = { 0.0f, 0.0f, 1.0f, 0.5f };

//shader
float g_radisX = 0.02f;
float g_radisXX = 0.06f;


void setMaterial(const GLfloat mat_diffuse[4], GLfloat mat_shininess)
{
    static const GLfloat mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    static const GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);//-------------------------------------------------------------------------
    //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    //glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    //glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}


class Teapot :public osg::Drawable
{
public:
    Teapot(){}
    Teapot(const Teapot& T, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) :
        osg::Drawable(T, copyop) {}
    ~Teapot()
    {

    }
    META_Object(osg, Teapot)
        virtual void drawImplementation(osg::RenderInfo & renderInfo) const
    {
        //glColor3f(1.0f,0.0f,0.0f);
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        //glDepthMask(GL_FALSE);
        //setMaterial(red_color, 30.0f);//-----------------------------------
        //glTranslatef(-2.5f,0.0f,0.0f);
        //glutSolidTeapot(0.3f);//-----------------------------------
        //-------------------------------------------------------------------------
        osg::ref_ptr<osg::TessellationHints> hits = new osg::TessellationHints;
        //值越小精度也就越小
        //hits->setDetailRatio(10.0f);

        osg::ref_ptr<osg::Cylinder> cy = new osg::Cylinder;
        //直接用几何对象初始化实例
        osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable(cy);
        cy->setCenter(osg::Vec3(50.0, 0.0, 0.0));
        cy->setHeight(30);
        cy->setRadius(30);

        //sd->setTessellationHints(hits);
        //：https://blog.csdn.net/wb175208/article/details/87934377 

        osg::ref_ptr<osg::Box> box = new osg::Box;
        box->setCenter(osg::Vec3(-50.0, 0.0, 0.0));
        box->setHalfLengths(osg::Vec3(30.0, 30.0, 30.0));
        osg::ref_ptr<osg::ShapeDrawable> sd2 = new osg::ShapeDrawable(box.get());

        //osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        //geode->addDrawable(sd.get());
        //geode->addDrawable(sd2.get());
        //-------------------------------------------------------------------------
    }
     //protected:
     //	virtual ~Teapot();
};

osg::Geode* createTeapot()
{
    osg::Geode* geode = new osg::Geode;

    osg::Texture2D* KLN89FaceTexture = new osg::Texture2D;
    KLN89FaceTexture->setDataVariance(osg::Object::DYNAMIC);
    osg::Image* klnFace = osgDB::readImageFile(".//ground.bmp");
    if (!klnFace)
    {
        std::cout << "couldn't find texture,quiting." << std::endl;
        //exit(0);
        return 0;
    }
    KLN89FaceTexture->setImage(klnFace);

    //blend
    osg::StateSet* blendStateSet = new osg::StateSet();
    osg::TexEnv* blendTexEnv = new osg::TexEnv();
    blendTexEnv->setMode(osg::TexEnv::REPLACE);
    blendStateSet->setTextureAttributeAndModes(0, KLN89FaceTexture, osg::StateAttribute::ON);
    blendStateSet->setTextureAttribute(0, blendTexEnv);
    geode->addDrawable(new Teapot());
    //geode->setStateSet(blendStateSet);
    return geode;
}
osg::Geode* createMagnify()
{
    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vv = new osg::Vec3Array;
    geom->setVertexArray(vv);
    vv->push_back(osg::Vec3(-0.5f, -0.2f, -0.5f));
    vv->push_back(osg::Vec3(0.5f, -0.2f, -0.5f));
    vv->push_back(osg::Vec3(0.5f, -0.2f, 0.5f));
    vv->push_back(osg::Vec3(-0.5f, -0.2f, 0.5f));

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
    osg::Geode* geod = new osg::Geode;
    geod->addDrawable(geom);

    osg::Texture2D* KLN89FaceTexture = new osg::Texture2D;
    KLN89FaceTexture->setDataVariance(osg::Object::DYNAMIC);
    osg::Image* klnFace = osgDB::readImageFile("C:\\Users\\HIVE\\Desktop\\X.bmp");
    if (!klnFace)
    {
        std::cout << "couldn't find texture,quiting." << std::endl;
        //exit(0);
        return 0;
    }
    KLN89FaceTexture->setImage(klnFace);

    //blend
    osg::StateSet* blendStateSet = new osg::StateSet();
    osg::TexEnv* blendTexEnv = new osg::TexEnv();
    blendTexEnv->setMode(osg::TexEnv::DECAL);
    blendStateSet->setTextureAttributeAndModes(0, KLN89FaceTexture, osg::StateAttribute::ON);
    blendStateSet->setTextureAttribute(0, blendTexEnv);


    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0.0f, 0.0f));
    texcoords->push_back(osg::Vec2(1.0f, 0.0f));
    texcoords->push_back(osg::Vec2(1.0f, 1.0f));
    texcoords->push_back(osg::Vec2(0.0f, 1.0f));
    geom->setTexCoordArray(0, texcoords);
    //geom->setTexCoordArray(1,texcoords);
    geod->setStateSet(blendStateSet);
    //geod->setStateSet(stateSet1);

    return geod;
}
osg::Texture* createRttTexture(int texWidth, int texHeight)
{
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    texture->setTextureSize(texWidth, texHeight);
    return texture;
}
osg::Camera* createRttCamera(int texWidth, int texHeight)
{
    osg::Camera* rttCamera = new osg::Camera;
    rttCamera->setClearColor(osg::Vec4(0.0f, 0.3f, 0.3f, 1.0f));
    rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    rttCamera->setViewport(0, 0, 256, 256);
    rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);
    rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    rttCamera->setProjectionMatrixAsPerspective(times, 1.0f, 1.0f, 100.0f);

    osg::Vec3d upDirect(0.0f, 0.0f, 1.0f);
    osg::Vec3d center(g_move_x, 0.0f, g_move_y);
    osg::Vec3d viewDirect(g_move_x, -3.0f, g_move_y);

    rttCamera->setViewMatrixAsLookAt(viewDirect, center, upDirect);
    return rttCamera;
}
class Pickhandler :public osgGA::GUIEventHandler
{
public:
    Pickhandler()
    {
        g_rotate = false;
        temp_x = 0;
        temp_y = 0;
    }

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer =
            dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer)
            return false;
        switch (ea.getEventType())
        {
            case osgGA::GUIEventAdapter::MOVE:
                {
                    g_move_x = 2.5*ea.getXnormalized();
                    g_move_y = 2.5*ea.getYnormalized();
                    return false;
                }
            case osgGA::GUIEventAdapter::DRAG:
                {
                    g_mouse_x = (g_mouse_x + (int)(ea.getX() - temp_x)) % 360;
                    g_mouse_y = (g_mouse_y + (int)(ea.getY() - temp_y)) % 360;
                    temp_x = ea.getX();
                    temp_y = ea.getY();
                }
            case osgGA::GUIEventAdapter::PUSH:
                {
                    temp_x = ea.getX();
                    temp_y = ea.getY();
                }



        }
        return true;
    }
private:
    bool g_rotate;
    float temp_x;
    float temp_y;
};
class CSpaceShipCallback : public osg::NodeCallback
{
public:
    CSpaceShipCallback() :m_Angle(0){}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::ref_ptr<osg::MatrixTransform> pMT = dynamic_cast<osg::MatrixTransform*>(node);
        osg::Matrix Matrix;
        Matrix.makeTranslate(g_move_x, 0.0f, g_move_y);
        pMT->setMatrix(Matrix);
        traverse(node, nv);
    }

private:
    double m_Angle;
};

class CTeapotCallback : public osg::NodeCallback
{
public:
    CTeapotCallback() :m_Angle(0){}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::ref_ptr<osg::MatrixTransform> pMT = dynamic_cast<osg::MatrixTransform*>(node);
        osg::Matrix Matrix1, Matrix2;
        Matrix1.makeRotate((float)g_mouse_x / 10, osg::Vec3(-1.0f, 0.0f, 0.0f));
        Matrix2.makeRotate((float)g_mouse_y / 10, osg::Vec3(0.0f, 0.0f, -1.0f));
        pMT->setMatrix(Matrix2*Matrix1);
        traverse(node, nv);
    }

private:
    double m_Angle;
};
// class PosXCallback:public osg::Uniform::Callback
// {
// public:
// 	PosXCallback();
// 	virtual void operator() (osg::Uniform* uniform, osg::NodeVisitor* nv)
// 	{
// 		if(!uniform) return;
// 		uniform->set(g_move_x);
// 	}
// };
// class PosYCallback:public osg::Uniform::Callback
// {
// public:
// 	PosYCallback();
// 	virtual void operator() (osg::Uniform* uniform,osg::NodeVisitor* nv)
// 	{
// 		if(!uniform) return;
// 		uniform->set(g_move_y);
//         
// 	}
// };

void createShaders(osg::StateSet& ss)
{
    osg::Shader* vertShader = new osg::Shader(osg::Shader::VERTEX);
    //vertShader->loadShaderSourceFromFile(".\\Magnifier\\Shaders\\vp.glsl");
    vertShader->loadShaderSourceFromFile("Shaders//vp.glsl");
    osg::Shader* fragShader = new osg::Shader(osg::Shader::FRAGMENT);
    //fragShader->loadShaderSourceFromFile(".\\Magnifier\\Shaders\\fp.glsl");
    fragShader->loadShaderSourceFromFile("Shaders//fp.glsl");
    osg::Program* pro = new osg::Program;
    pro->addShader(vertShader);
    pro->addShader(fragShader);
    osg::Uniform*  g_textureImg1 = new osg::Uniform("g_textureImg1", 0);
    //osg::Uniform*  g_textureImg2=new osg::Uniform("g_textureImg2",1);
    osg::Uniform*  g_centerPosX = new osg::Uniform("g_centerPosX", g_move_x);
    osg::Uniform*  g_centerPosY = new osg::Uniform("g_centerPosY", g_move_y);
    osg::Uniform*  g_radis = new osg::Uniform("g_radis", g_radisX);

    //   	g_centerPosY->setUpdateCallback(new PosYCallback);
    //   	g_centerPosX->setUpdateCallback(new PosXCallback);

    ss.addUniform(g_textureImg1);
    ss.addUniform(g_centerPosX);
    ss.addUniform(g_centerPosY);
    ss.addUniform(g_radis);

    ss.setAttributeAndModes(pro);
}

int main()
{
    int texWidth = 256, texHeight = 256;
    //osg::Geode* Magnify=new osg::Geode;
    osg::Group* root = new osg::Group;
    osg::Geode* quad = new osg::Geode;
    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor(osg::Vec4(0.0f, 0.3f, 0.3f, 1.0f));
    viewer.getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //ShowCursor(false);
    viewer.addEventHandler(new Pickhandler);

    //fbo
    osg::Camera* rttCamera = createRttCamera(texWidth, texHeight);
    quad->addDrawable(osg::createTexturedQuadGeometry(
        osg::Vec3(g_move_x - g_radisXX, -0.4f, g_move_y - g_radisXX), osg::Vec3(2.0*g_radisXX, 0.0f, 0.0f),
        osg::Vec3(0.0f, 0.0f, 2.0*g_radisXX)));

    osg::Geode* TeaTexture = createTeapot();
    osg::MatrixTransform* pMTea1 = new osg::MatrixTransform();
    pMTea1->setUpdateCallback(new CTeapotCallback);
    pMTea1->addChild(TeaTexture);
    rttCamera->addChild(pMTea1);
    osg::Texture* rttTexture = createRttTexture(texWidth, texHeight);
    rttCamera->attach(osg::Camera::COLOR_BUFFER, rttTexture);
    quad->getOrCreateStateSet()->setTextureAttributeAndModes(0, rttTexture);

    //纹理
    osg::MatrixTransform* pMT = new osg::MatrixTransform();
    pMT->setUpdateCallback(new CSpaceShipCallback);
    pMT->addChild(quad);

    //center茶壶
    osg::Geode* Tea = createTeapot();
    osg::MatrixTransform* pMTea = new osg::MatrixTransform();
    pMTea->setUpdateCallback(new CTeapotCallback);
    pMTea->addChild(Tea);

    root->addChild(pMT);
    root->addChild(rttCamera);
    root->addChild(pMTea);


    createShaders(*(quad->getOrCreateStateSet()));
    viewer.setSceneData(root);
    viewer.setUpViewInWindow(100, 100, 640, 640);
    viewer.getCamera()->setViewport(0, 0, 640, 640);

    viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(0.0f, -15.0f, 0.0f), osg::Vec3(0.0f, 0.0, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
    //ShowCursor(false);
    ShowCursor(true);
    while (!viewer.done())
    {
        viewer.getCamera()->setProjectionMatrixAsPerspective(cameratimes, 1.0f, 1.0f, 100.0f);
        float xx = g_move_x / 2.4f;
        float yy = g_move_y / 2.4f;
        osg::Uniform*  g_centerPosX = new osg::Uniform("g_centerPosX", xx);
        osg::Uniform*  g_centerPosY = new osg::Uniform("g_centerPosY", yy);
        quad->getOrCreateStateSet()->addUniform(g_centerPosX);
        quad->getOrCreateStateSet()->addUniform(g_centerPosY);

        osg::Vec3d upDirect(0.0f, 0.0f, 1.0f);
        osg::Vec3d center(g_move_x, 0.0f, g_move_y);
        osg::Vec3d viewDirect(g_move_x, -8.0f, g_move_y);

        rttCamera->setViewMatrixAsLookAt(viewDirect, center, upDirect);

        viewer.frame();
    }

    //viewer.run();
    return 0;

}