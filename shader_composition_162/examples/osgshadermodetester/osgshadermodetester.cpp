/* OpenSceneGraph example, osgwindows.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Transform>
#include <osg/Material>
#include <osg/NodeCallback>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/TextureCubeMap>
#include <osg/VertexProgram>
#include <osg/PositionAttitudeTransform>
#include <osg/LineWidth>
#include <osg/TextureRectangle>
#include <osg/Camera>
#include <osg/ClipNode>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Multisample>
#include <osg/MatrixTransform>
#include <osg/LightModel>
#include <osg/ShaderGenerator>
#include <osg/Fog>

#include <iostream>

#include <osgGA/TrackballManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/BoundsChecking>

#include <osgGA/GUIEventHandler>
#include <cassert>

#include <osgViewer/TweakableManager>

using namespace osg;
using namespace osgGA;


Node* createQuadScene()
{
	Geode* geode = new Geode;
	geode->setName("QuadGeode");
	
	Geometry* geometry = createTexturedQuadGeometry(Vec3(-10, 0, -7.5), Vec3(20, 0, 0), Vec3(0, 0, 15));
	geometry->setName("QuadGeometry");

	geode->addDrawable(geometry);

	return geode;
}

struct TextureData
{
	TextureData() {
		textureEnabled = false;
		texMatEnabled = false;
		texMat = new osg::TexMat;
		texEnvEnabled = false;
		texEnv = new osg::TexEnv;
		texEnvMode = osg::TexEnv::MODULATE;
		texEnvColor = osg::Vec4(1.0, 1.0, 1.0, 1.0);

		texGenEnabled = false;
		texGen = new osg::TexGen();
		texGenMode = osg::TexGen::OBJECT_LINEAR;
	}

	bool textureEnabled;
	
	bool texMatEnabled;
	osg::ref_ptr<osg::TexMat> texMat;
	
	bool texEnvEnabled;
	osg::ref_ptr<osg::TexEnv> texEnv;
	osg::TexEnv::Mode texEnvMode;
	osg::Vec4 texEnvColor;
	
	bool texGenEnabled;
	osg::ref_ptr<osg::TexGen> texGen;
	osg::TexGen::Mode texGenMode;
};

struct LightData
{
	LightData() {
		lightEnabled = false;
		lightSource = new osg::LightSource;
		lightSource->getLight()->setLightNum(lightNum++);
	}

	static int lightNum;
	bool lightEnabled;
	osg::ref_ptr<osg::LightSource> lightSource;
	osg::Vec4 ambientColor;
	osg::Vec4 diffuseColor;
	osg::Vec4 specularColor;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float spotExponent;
	float spotCutOff;
	osg::Vec3 position;
	osg::Vec3 direction;
};

int LightData::lightNum = 0;

struct ClippingData
{
	ClippingData()
	{
		clippingEnabled = false;
		clipPlane = new osg::ClipPlane(0, osg::Vec4d(0, 0, 1, 0));
		dir = osg::Vec3(0, 0, 1);
		offset = 0;
	}

	bool clippingEnabled;
	osg::Vec3 dir;
	float offset;
	osg::ref_ptr<osg::ClipPlane> clipPlane;
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

	ShaderGenerator* shaderGenerator = new ShaderGenerator();

    // construct the viewer.
    osgViewer::CompositeViewer viewer;
	viewer.setName("MainViewer");
//	osg::DisplaySettings::instance()->setNumMultiSamples( 4 );

    osgViewer::View* view1 = new osgViewer::View;
    view1->setName("ShaderView");
    viewer.addView(view1);
    view1->setUpViewInWindow(50, 50, 500, 500);
    view1->setCameraManipulator(new osgGA::TrackballManipulator);
//    view1->addEventHandler( statesetManipulator.get() );

	osgViewer::View* view2 = new osgViewer::View;
	if (view2)
	{
		view2->setName("FFPView");
		viewer.addView(view2);
		view2->setUpViewInWindow(600, 50, 500, 500);
	}

	osgViewer::TweakableManager* param = new osgViewer::TweakableManager();
	view1->addEventHandler(param);

	view1->addEventHandler(new osgViewer::WindowSizeHandler());
	view1->addEventHandler(new osgViewer::StatsHandler());
	view1->addEventHandler(new osgViewer::ThreadingHandler());
	
	osg::ref_ptr<StateSet> rootstateset = view1->getCamera()->getOrCreateStateSet();
	rootstateset->setName("FlowersStateset");

	osg::Texture2D* texture = new osg::Texture2D(osgDB::readImageFile("Images/blueFlowers.png"));
	rootstateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

	osg::Texture2D* texture1 = new osg::Texture2D(osgDB::readImageFile("Images/osg256.png"));
	rootstateset->setTextureAttributeAndModes(1, texture1, osg::StateAttribute::ON);

	rootstateset->setMode(GL_RESCALE_NORMAL,osg::StateAttribute::ON);

    view1->addEventHandler(new osgGA::StateSetManipulator(rootstateset.get()));
	//view1->setCameraManipulator(new osgGA::TrackballManipulator());
	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	bool shaderPipelineEnabled = false;
	osgViewer::BooleanTweaker* shaderEnabledTweaker = new osgViewer::BooleanTweaker(shaderPipelineEnabled, true);
	param->addTweakable("Shader pipeline enabled", shaderEnabledTweaker);

	// Setup tweakable model
	osgViewer::LookupTweaker<osg::ref_ptr<osg::Node> >::LookupMap models;
	
	osg::ref_ptr<osg::Node> model = createQuadScene();
	models[model] = "Textured quad";
	models[osgDB::readNodeFile("cow.osg")] = "cow.osg";
	models[osgDB::readNodeFile("cessna.osg")] = "cessna.osg";
	models[osgDB::readNodeFile("dumptruck.osg")] = "dumptruck.osg";
	models[osgDB::readNodeFile("spaceship.osg")] = "spaceship.osg";

    // set the scene to render
	model->setStateSet(rootstateset);
	view1->setSceneData(model);
	if (view2)
	{
		view2->setSceneData(model);
	}
	
	osgViewer::LookupTweaker<osg::ref_ptr<osg::Node> >* modelTweaker = new osgViewer::LookupTweaker<osg::ref_ptr<osg::Node> >(models, model);
	param->addTweakable("Model", modelTweaker);

	// Setup tweakable shadertree rootnode
	osgViewer::LookupTweaker<osg::ShaderMode::Type>::LookupMap roots;
	
	osg::ShaderMode::Type rootMode = ShaderModeVisualRoot::type;
	rootstateset->setShaderMode(ShaderModeIndexPair(rootMode, 0), osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
	roots[rootMode] = "FF Visual";
	roots[ShaderModeEyeNormalRoot::type] = "Eye Normal";
	roots[ShaderModeDepthRoot::type] = "Depth";

	osgViewer::LookupTweaker<osg::ShaderMode::Type>* rootTweaker = new osgViewer::LookupTweaker<osg::ShaderMode::Type>(roots, rootMode);
	param->addTweakable("Root", rootTweaker);

	LightData lightData[8];

	int currentLightNumber = 0;
	osgViewer::IntTweaker* currentLightTweaker = new osgViewer::IntTweaker(currentLightNumber, 1, 0, 7, true);
	param->addTweakable("Edit Light", currentLightTweaker);

	for (int i=0; i < 8; i++)
	{
		osgViewer::TweakerEnabler<int>* lightEnabler = new osgViewer::TweakerEnabler<int>(currentLightTweaker, i);
		osgViewer::BooleanTweaker* lightEnabledTweaker = new osgViewer::BooleanTweaker(lightData[i].lightEnabled, true, lightEnabler);
		param->addTweakable("Light enabled", lightEnabledTweaker);

		osgViewer::NumericArrayTweaker<osg::Vec4>* lightAmbientTweaker = 
			new osgViewer::NumericArrayTweaker<osg::Vec4>(lightData[i].ambientColor, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1), false, lightEnabler);
		param->addTweakable("Light ambient", lightAmbientTweaker);

		osgViewer::NumericArrayTweaker<osg::Vec4>* lightDiffuseTweaker = 
			new osgViewer::NumericArrayTweaker<osg::Vec4>(lightData[i].diffuseColor, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1), false, lightEnabler);
		param->addTweakable("Light diffuse", lightDiffuseTweaker);

		osgViewer::NumericArrayTweaker<osg::Vec4>* lightSpecularTweaker = 
			new osgViewer::NumericArrayTweaker<osg::Vec4>(lightData[i].specularColor, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1), false, lightEnabler);
		param->addTweakable("Light specular", lightSpecularTweaker);
	}

	osg::Vec4 materialAmbient = osg::Vec4(0.1, 0.1, 0.1, 0.1);
	osgViewer::NumericArrayTweaker<osg::Vec4>* materialAmbientTweaker = 
		new osgViewer::NumericArrayTweaker<osg::Vec4>(materialAmbient, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1));
	param->addTweakable("Material ambient", materialAmbientTweaker);

	osg::Vec4 materialDiffuse = osg::Vec4(0.7, 0.7, 0.7, 1.0);
	osgViewer::NumericArrayTweaker<osg::Vec4>* materialDiffuseTweaker = 
		new osgViewer::NumericArrayTweaker<osg::Vec4>(materialDiffuse, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1));
	param->addTweakable("Material diffuse", materialDiffuseTweaker);

	osg::Vec4 materialSpecular = osg::Vec4(1.0, 1.0, 1.0, 1.0);
	osgViewer::NumericArrayTweaker<osg::Vec4>* materialSpecularTweaker = 
		new osgViewer::NumericArrayTweaker<osg::Vec4>(materialSpecular, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1));
	param->addTweakable("Material specular", materialSpecularTweaker);

	float materialShininess = 25;
	param->addTweakable("Material shininess", new osgViewer::FloatTweaker(materialShininess, 1, 0.0, 50.0));

	osg::Vec4 materialEmission = osg::Vec4(0.0, 0.0, 0.0, 0.0);
	osgViewer::NumericArrayTweaker<osg::Vec4>* materialEmissionTweaker = 
		new osgViewer::NumericArrayTweaker<osg::Vec4>(materialEmission, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1));
	param->addTweakable("Material emission", materialEmissionTweaker);

	osg::Material* material = new osg::Material();
	material->setAmbient(osg::Material::FRONT_AND_BACK, materialAmbient);
	material->setDiffuse(osg::Material::FRONT_AND_BACK, materialDiffuse);
	material->setSpecular(osg::Material::FRONT_AND_BACK, materialSpecular);
	material->setEmission(osg::Material::FRONT_AND_BACK, materialEmission);
	material->setShininess(osg::Material::FRONT_AND_BACK, materialShininess);
	rootstateset->setAttributeAndModes(material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	// Input parameter the control texture stage that will be updated
	int currentTexture = 0;
	osgViewer::IntTweaker* currentTextureTweaker = new osgViewer::IntTweaker(currentTexture, 1, 0, 7);
	param->addTweakable("Edit Texture", currentTextureTweaker);

	TextureData textureData[4];

	// Input parameter to control texture environment
	for (int i =0; i < 4; i++)
	{
		osgViewer::TweakerEnabler<int>* textureEnabler = new osgViewer::TweakerEnabler<int>(currentTextureTweaker, i);
		osgViewer::BooleanTweaker* textureEnabledTweaker = new osgViewer::BooleanTweaker(textureData[i].textureEnabled, true, textureEnabler);
		param->addTweakable("Texture enabled", textureEnabledTweaker);
//		osgViewer::BooleanTweaker* texMatEnabledTweaker = new osgViewer::BooleanTweaker(textureData[i].texMatEnabled, true, textureEnabler);
//		param->addTweakable("Texture matrix enabled", texMatEnabledTweaker);

		// Setup editable texture environment
		osgViewer::BooleanTweaker* texEnvEnabledTweaker = new osgViewer::BooleanTweaker(textureData[i].texEnvEnabled, true, textureEnabler);
		param->addTweakable("TexEnv enabled", texEnvEnabledTweaker);

		osgViewer::TweakerEnabler<bool>* texEnvEnabler = new osgViewer::TweakerEnabler<bool>(texEnvEnabledTweaker, true, true, textureEnabler);
		osgViewer::LookupTweaker<osg::TexEnv::Mode>::LookupMap texEnvModes;
		texEnvModes[osg::TexEnv::DECAL] = "DECAL";
		texEnvModes[osg::TexEnv::MODULATE] = "MODULATE";
		texEnvModes[osg::TexEnv::BLEND] = "BLEND";
		texEnvModes[osg::TexEnv::REPLACE] = "REPLACE";
		texEnvModes[osg::TexEnv::ADD] = "ADD";
		osgViewer::LookupTweaker<osg::TexEnv::Mode>* texEnvModeTweaker = new osgViewer::LookupTweaker<osg::TexEnv::Mode>(texEnvModes, textureData[i].texEnvMode, false, texEnvEnabler);
		param->addTweakable("TexEnv", texEnvModeTweaker);
	//	osgViewer::LookupTweaker<osg::Vec4>* texEnvColorTweaker = new osgViewer::LookupTweaker<osg::Vec4>(colors, textureData[0].texEnvColor, &texEnvEnabler);
	//	param->addTweakable("TexEnv color", texEnvColorTweaker);

		// Setup editable texture coordinate generation
		osgViewer::BooleanTweaker* texGenEnabledTweaker = new osgViewer::BooleanTweaker(textureData[i].texGenEnabled, true, textureEnabler);
		param->addTweakable("TexGen enabled", texGenEnabledTweaker);

		osgViewer::TweakerEnabler<bool>* texGenEnabler = new osgViewer::TweakerEnabler<bool>(texGenEnabledTweaker, true, true, textureEnabler);

		osgViewer::LookupTweaker<osg::TexGen::Mode>::LookupMap texGenModes;
		texGenModes[osg::TexGen::OBJECT_LINEAR] = "OBJECT_LINEAR";
		texGenModes[osg::TexGen::EYE_LINEAR] = "EYE_LINEAR";
		texGenModes[osg::TexGen::SPHERE_MAP] = "SPHERE_MAP";
		texGenModes[osg::TexGen::NORMAL_MAP] = "NORMAL_MAP";
		texGenModes[osg::TexGen::REFLECTION_MAP] = "REFLECTION_MAP";
		osgViewer::LookupTweaker<osg::TexGen::Mode>* texGenModeTweaker = new osgViewer::LookupTweaker<osg::TexGen::Mode>(texGenModes, textureData[i].texGenMode, false, texGenEnabler);
		param->addTweakable("TexGen", texGenModeTweaker);

		rootstateset->setTextureAttributeAndModes(i, textureData[i].texMat, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
		rootstateset->setTextureAttributeAndModes(i, textureData[i].texEnv, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
		rootstateset->setTextureAttributeAndModes(i, textureData[i].texGen, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
	}

	// Input parameter to control fog
	bool fogEnabled = false;
	osgViewer::BooleanTweaker* fogEnabledTweaker = new osgViewer::BooleanTweaker(fogEnabled);
	param->addTweakable("Fog enabled", fogEnabledTweaker);
	osgViewer::TweakerEnabler<bool>* fogEnabler = new osgViewer::TweakerEnabler<bool>(fogEnabledTweaker, true);

	// Setup editable fog
	osgViewer::LookupTweaker<osg::Fog::Mode>::LookupMap fogModes;
	fogModes[osg::Fog::LINEAR] = "LINEAR";
	fogModes[osg::Fog::EXP] = "EXP";
	fogModes[osg::Fog::EXP2] = "EXP2";

	osg::Fog::Mode fogMode = osg::Fog::LINEAR;
	osgViewer::LookupTweaker<osg::Fog::Mode>* fogModeTweaker = new osgViewer::LookupTweaker<osg::Fog::Mode>(fogModes, fogMode, false, fogEnabler);
	param->addTweakable("Fog mode", fogModeTweaker);

	// Enables parameters that are only used when fogmode is linear
	osgViewer::TweakerEnabler<osg::Fog::Mode>* linearFogEnabler = new osgViewer::TweakerEnabler<osg::Fog::Mode>(fogModeTweaker, osg::Fog::LINEAR);
	osgViewer::TweakerEnabler<osg::Fog::Mode>* exponentialFogEnabler = new osgViewer::TweakerEnabler<osg::Fog::Mode>(fogModeTweaker, osg::Fog::LINEAR, false);

	// Input parameter the control fog start
	float fogStart = 10.0f;
	param->addTweakable("Fog start", new osgViewer::FloatTweaker(fogStart, 1, 0, 100, false, linearFogEnabler));

	// Input parameter the control fog start
	float fogEnd = 50.0f;
	param->addTweakable("Fog end", new osgViewer::FloatTweaker(fogEnd, 1, 0, 100, false, linearFogEnabler));

	// Input parameter the control fog density
	float fogDensity = 0.01f;
	param->addTweakable("Fog density", new osgViewer::FloatTweaker(fogDensity, 0.001, 0.0, 0.1, false, exponentialFogEnabler));

	osg::Fog* fog = new osg::Fog(); 
	
	osg::Vec4 fogColor = osg::Vec4(1.0, 1.0, 1.0, 1.0);
	osgViewer::NumericArrayTweaker<osg::Vec4>* fogColorTweaker = 
		new osgViewer::NumericArrayTweaker<osg::Vec4>(fogColor, osg::Vec4(0.1, 0.1, 0.1, 0.1), osg::Vec4(0, 0, 0, 0), osg::Vec4(1, 1, 1, 1), false, fogEnabler);
	param->addTweakable("Fog color", fogColorTweaker);
//	rootstateset->setAttributeAndModes(fog, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);

	ClippingData clippingData[6];

	// Input parameter the control clip plane that will be updated
	int currentClipplane = 0;
	osgViewer::IntTweaker* currentClipplaneTweaker = new osgViewer::IntTweaker(currentClipplane, 1, 0, 5);
	param->addTweakable("Edit clip plane", currentClipplaneTweaker);

	for (int i=0; i < 6; i++)
	{
		clippingData[i].clipPlane->setClipPlaneNum(i);
		osgViewer::TweakerEnabler<int>* clipplaneEnabler = new osgViewer::TweakerEnabler<int>(currentClipplaneTweaker, i);
		param->addTweakable("Clipplane enabled", new osgViewer::BooleanTweaker(clippingData[i].clippingEnabled, true, clipplaneEnabler));
		param->addTweakable("Clipplane normal", new osgViewer::DirectionTweaker(clippingData[i].dir, 2, 2, clipplaneEnabler));
		param->addTweakable("Clipplane offset", new osgViewer::FloatTweaker(clippingData[i].offset, 0.5, -20, 20, false, clipplaneEnabler));
	}

	view1->getCamera()->setName("MainCamera");
	view1->getCamera()->getGraphicsContext()->getState()->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);

 

	viewer.realize();

	

	while (!viewer.done())
	{
		viewer.advance();
		viewer.eventTraversal();
		viewer.updateTraversal();

		if (rootTweaker->hasChanged())
		{
			rootstateset->setShaderMode(ShaderModeIndexPair(ShaderModeVisualRoot::type, 0), osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			rootstateset->setShaderMode(ShaderModeIndexPair(ShaderModeEyeNormalRoot::type, 0), osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			rootstateset->setShaderMode(ShaderModeIndexPair(ShaderModeDepthRoot::type, 0), osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			rootstateset->setShaderMode(ShaderModeIndexPair(rootMode, 0), osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
		}

		if (modelTweaker->hasChanged())
		{
			model->setStateSet(rootstateset);
			view1->setSceneData(model);
			if (view2)
			{
				view2->setSceneData(model);
			}
		}

		if (view2)
		{
			view2->getCamera()->setViewMatrix(view1->getCamera()->getViewMatrix());
		}

		material->setAmbient(osg::Material::FRONT_AND_BACK, materialAmbient);
		material->setDiffuse(osg::Material::FRONT_AND_BACK, materialDiffuse);
		material->setSpecular(osg::Material::FRONT_AND_BACK, materialSpecular);
		material->setEmission(osg::Material::FRONT_AND_BACK, materialEmission);
		material->setShininess(osg::Material::FRONT_AND_BACK, materialShininess);

		if (shaderEnabledTweaker->hasChanged())
		{
			if (shaderPipelineEnabled)
				view1->getCamera()->getGraphicsContext()->getState()->setShaderGenerator(shaderGenerator);
			else
				view1->getCamera()->getGraphicsContext()->getState()->setShaderGenerator(NULL);
		}

		// Update clipping
		for (int i=0; i < 6; i++)
		{
			if (clippingData[i].clippingEnabled)
			{
				clippingData[i].clipPlane->setClipPlane(osg::Vec4d(clippingData[i].dir, clippingData[i].offset));
				rootstateset->setAttributeAndModes(clippingData[i].clipPlane, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setAttributeAndModes(clippingData[i].clipPlane, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}
		}

		// Update lights
		for (int i=0; i < 8; i++)
		{
			if (lightData[i].lightEnabled)
			{
				lightData[i].lightSource->getLight()->setAmbient(lightData[i].ambientColor);
				lightData[i].lightSource->getLight()->setDiffuse(lightData[i].diffuseColor);
				lightData[i].lightSource->getLight()->setSpecular(lightData[i].specularColor);
				

	//float constantAttenuation;
	//float linearAttenuation;
	//float quadraticAttenuation;
	//float spotExponent;
	//float spotCutOff;

//				lightData[i].lightSource->getLight()->setPosition(lightData[i].position);
//				lightData[i].lightSource->getLight()->setDirection(lightData[i].direction);

				rootstateset->setAttributeAndModes(lightData[i].lightSource->getLight(), osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setAttributeAndModes(lightData[i].lightSource->getLight(), osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}
		}

		// Update texturing
		for (int i=0; i < 4; i++)
		{
			if (textureData[i].textureEnabled)
			{
				rootstateset->setTextureMode(i, GL_TEXTURE_2D, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setTextureMode(i, GL_TEXTURE_2D, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}

			// Update texture matrix
			if (textureData[i].texMatEnabled)
			{
				//texMat->setMatrix();
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texMat, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texMat, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}

			// Update texture environment
			if (textureData[i].texEnvEnabled)
			{
				textureData[i].texEnv->setMode(textureData[i].texEnvMode);
				textureData[i].texEnv->setColor(textureData[i].texEnvColor);
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texEnv, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texEnv, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}
			
			// Update texture generation
			if (textureData[i].texGenEnabled)
			{
				textureData[i].texGen->setMode(textureData[i].texGenMode);
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texGen, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
			}
			else
			{
				rootstateset->setTextureAttributeAndModes(i, textureData[i].texGen, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
			}
		}

		// Update fog
		if (fogEnabled)
		{
			fog->setStart(fogStart);
			fog->setEnd(fogEnd);
			fog->setDensity(fogDensity); 
			fog->setMode(fogMode);
			fog->setColor(fogColor);
			rootstateset->setAttributeAndModes(fog, osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
		}
		else
		{
			rootstateset->setAttributeAndModes(fog, osg::StateAttribute::OFF| osg::StateAttribute::OVERRIDE);
		}
		
		viewer.renderingTraversals();
	}

	return 0;
}

