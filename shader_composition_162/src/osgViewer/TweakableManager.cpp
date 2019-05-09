#include <osgViewer/TweakableManager>
#include <osgViewer/Renderer>
#include <osg/PolygonMode>

using namespace osgViewer;

TweakableManager::TweakableManager() :
	_helpEnabled(false),
	_initialized(false)
{
	_currentTweakable = -1;
	_camera = new osg::Camera;
	_camera->setRenderer(new osgViewer::Renderer(_camera.get()));
	_camera->setRenderOrder(osg::Camera::POST_RENDER, 11);
}

TweakableManager::~TweakableManager()
{
}

bool TweakableManager::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
    if (!view) return false;
	osgViewer::ViewerBase* viewer = view->getViewerBase();

    if (ea.getHandled()) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

		int key = ea.getKey() ;

        switch(key)
        {
			case osgGA::GUIEventAdapter::KEY_Right:
				next();
				return true;
				break;
            case osgGA::GUIEventAdapter::KEY_Left:
				previous();
                return true;
                break;
            case osgGA::GUIEventAdapter::KEY_Up:
				increase();
                return true;
                break;
            case osgGA::GUIEventAdapter::KEY_Down:
				decrease();
                return true;
				break;
			case 'u':
				if (!_initialized)
				{
					setUpHUDCamera(viewer);
					setUpScene();
				}

				_helpEnabled = !_helpEnabled;

				if (_helpEnabled)
				{
					_camera->setNodeMask(0xffffffff);
				}
				else
				{
					_camera->setNodeMask(0);
				}
				return true;
				break;
        }
    }
    return false;
}

void TweakableManager::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("//","Toggle tweakable editing");
    usage.addKeyboardMouseBinding("arrow right","Next tweakable");
    usage.addKeyboardMouseBinding("arrow left","Previous tweakable");
    usage.addKeyboardMouseBinding("arrow up","Increase tweakable value");
	usage.addKeyboardMouseBinding("arrow down","Decrease tweakable value");
}

void TweakableManager::update() 
{
	if (_currentTweakable != -1)
	{
		std::ostringstream os;
		os << (_currentTweakable + 1) << "/" << _tweakableVector.size() << " " <<
		_tweakableVector[_currentTweakable].name << ": " <<
		_tweakableVector[_currentTweakable].tweakable->getValueAsString();
		
		if (_label.get())
			_label->setText(os.str());
	}
	else
	{
		if (_label.get())
			_label->setText("No tweakables");
	}
}

/// Select next tweakable to be the active one
void TweakableManager::next() 
{
	if (_currentTweakable != -1)
	{
		// First see if current tweakable wants to handle the next
		if (!_tweakableVector[_currentTweakable].tweakable->next())
		{
			int prevTweakable = _currentTweakable;
			_currentTweakable++;
			// Wrap next tweakable to begin
			if (_currentTweakable == _tweakableVector.size())
				_currentTweakable = 0;

			// Search for next enabled tweakable, but prevent endless loop
			while (!_tweakableVector[_currentTweakable].tweakable->isEnabled() && (prevTweakable != _currentTweakable))
			{
				_currentTweakable++;
				if (_currentTweakable == _tweakableVector.size())
					_currentTweakable = 0;
			}
		}
		update();
	}
}

/// Select the previous tweakable to be the active one
void TweakableManager::previous()
{
	if (_currentTweakable != -1)
	{
		// First see if current tweakable wants to handle the previous
		if (!_tweakableVector[_currentTweakable].tweakable->previous())
		{
			int prevTweakable = _currentTweakable;
			_currentTweakable--;
			// Wrap previous tweakable to end
			if (_currentTweakable == -1)
				_currentTweakable = _tweakableVector.size()-1;

			// Search for next enabled tweakable, but prevent endless loop
			while (!_tweakableVector[_currentTweakable].tweakable->isEnabled() && (prevTweakable != _currentTweakable))
			{
				_currentTweakable--;
				if (_currentTweakable == -1)
					_currentTweakable = _tweakableVector.size()-1;
			}
		}
		update();		
	}
}

/// Increase the current tweakable
void TweakableManager::increase()
{
	if (_currentTweakable != -1)
	{
		_tweakableVector[_currentTweakable].tweakable->increase();
		update();
	}
}

/// Decrease the current tweakable
void TweakableManager::decrease()
{
	if (_currentTweakable != -1)
	{
		_tweakableVector[_currentTweakable].tweakable->decrease();
		update();
	}
}


void TweakableManager::addTweakable(const std::string& name, Tweakable* pTweakable)
{
	TweakableEntry newEntry;
	newEntry.name = name;
	newEntry.tweakable = pTweakable;

	_tweakableVector.push_back(newEntry);

	if (_currentTweakable == -1)
	{	_currentTweakable = 0;
	}
}

void TweakableManager::setUpHUDCamera(osgViewer::ViewerBase* viewer)
{
	osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());

	if (!window)
	{    
		osgViewer::Viewer::Windows windows;
		viewer->getWindows(windows);

		if (windows.empty()) return;

		window = windows.front();

		_camera->setGraphicsContext(window);
	}

	_camera->setGraphicsContext(window);
	_camera->setViewport(0, 0, window->getTraits()->width, window->getTraits()->height);

	_camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
	_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_camera->setViewMatrix(osg::Matrix::identity());

	// only clear the depth buffer
	_camera->setClearMask(0);

	_initialized = true;
}

void TweakableManager::setUpScene()
{
	_switch = new osg::Switch;

	_camera->addChild(_switch.get());

	osg::StateSet* stateset = _switch->getOrCreateStateSet();
	stateset->setName("TweakerHudState");
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
	stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	stateset->setAttribute(new osg::PolygonMode(), osg::StateAttribute::PROTECTED);
	std::string font("fonts/arial.ttf");

	float characterSize = 30.0f;

	osg::Vec3 pos(30.0f, 30.0f,0.0f);
	osg::Vec4 color(1.0f,1.0f,1.0f,1.0f);

	osg::Geode* geode = new osg::Geode();
	_switch->addChild(geode, true);

	// application description
	_label = new osgText::Text;
	_label->setDataVariance(osg::Object::DYNAMIC);
	geode->addDrawable( _label.get() );

	_label->setColor(color);
//	_label->setBackdropType(osgText::Text::OUTLINE);
	_label->setFont(font);
	_label->setCharacterSize(characterSize);
	_label->setPosition(pos);
	update();

	pos.x() = _label->getBound().xMax();
	pos.y() -= characterSize*2.5f;
}

