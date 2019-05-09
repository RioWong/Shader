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
*/
#include <osg/GLExtensions>
#include <osg/TexEnv>
#include <osg/State>
#include <osg/Notify>

using namespace osg;

osg::ref_ptr<Uniform> TexEnv::_modeUniform = new Uniform(Uniform::INT, "TexEnvMode", 8);

TexEnv::TexEnv(Mode mode)
{
    _mode = mode;
    _color.set(0.0f,0.0f,0.0f,0.0f);

	addUniform(_modeUniform);
}


TexEnv::~TexEnv()
{
}

void TexEnv::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    if (_mode==ADD)
    {
        static bool isTexEnvAddSupported = isGLExtensionSupported(state.getContextID(),"GL_ARB_texture_env_add");
        if (isTexEnvAddSupported)
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, ADD);
        else // fallback on OpenGL default.
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, MODULATE);
    }
    else
    {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
        if (_mode==TexEnv::BLEND)
        {
            glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, _color.ptr());
        }
    }
#else
    OSG_NOTICE<<"Warning: TexEnv::apply(State&) - not supported."<<std::endl;
#endif

	unsigned int unit = state.getActiveTextureUnit();
	
	switch (_mode)
	{
	case DECAL:
		_modeUniform->setElement(unit, 2);
		break;
	case BLEND:
		_modeUniform->setElement(unit, 3);
		break;
	case REPLACE:
		_modeUniform->setElement(unit, 0);
		break;
	case ADD:
		_modeUniform->setElement(unit, 4);
		break;
	case MODULATE:
		// Fall through
	default:
		_modeUniform->setElement(unit, 1);
	}
}
