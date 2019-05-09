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
#include <osg/State>
#include <osg/Fog>
#include <osg/Notify>

using namespace osg;

#ifndef GL_FOG_COORDINATE_SOURCE
    #define GL_FOG_COORDINATE_SOURCE    0x8450
#endif

Fog::Fog()
{
    _mode = EXP;
    _density = 1.0f;
    _start   = 0.0f;
    _end     = 1.0f;
    _color.set( 0.0f, 0.0f, 0.0f, 0.0f);
    _fogCoordinateSource = FRAGMENT_DEPTH;
	_fogModeUniform = new Uniform("fogMode", 1);
	addUniform(_fogModeUniform);
	_fogCoordinateSourceUniform = new Uniform("fogCoordinateSource", 1);
	addUniform(_fogCoordinateSourceUniform);
}


Fog::~Fog()
{
}

void Fog::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE

#ifdef OSG_GLES1_AVAILABLE
    #define glFogi glFogx
#endif

    glFogi( GL_FOG_MODE,     _mode );

    glFogf( GL_FOG_DENSITY,  _density );
    glFogf( GL_FOG_START,    _start );
    glFogf( GL_FOG_END,      _end );
    glFogfv( GL_FOG_COLOR,    (GLfloat*)_color.ptr() );
    
    static bool fogCoordExtensionSuppoted = osg::isGLExtensionSupported(state.getContextID(),"GL_EXT_fog_coord");
    if (fogCoordExtensionSuppoted)
    {
        glFogi(GL_FOG_COORDINATE_SOURCE,_fogCoordinateSource);
    }
#else
    OSG_NOTICE<<"Warning: Fog::apply(State&) - not supported."<<std::endl;
#endif

	switch (_mode)
	{
	case LINEAR:
		_fogModeUniform->set(0);
		break;
	case EXP2:
		_fogModeUniform->set(2);
		break;
	case EXP:
		// Fall through
	default:
		_fogModeUniform->set(1);
	}

	switch (_fogCoordinateSource)
	{
	case FOG_COORDINATE:
		_fogCoordinateSourceUniform->set(0);
		break;
	case FRAGMENT_DEPTH:
		// Fall through
	default:
		_fogCoordinateSourceUniform->set(1);
	}
}
