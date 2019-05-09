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
 * 
 * Author: Roland Smeenk
 */

#include <osg/Notify>
#include <osg/ShaderGenerator>
#include <osg/ShaderModeRoot>

#include <algorithm>

using namespace osg;

#define DEBUG_SHADERGENERATOR 1


ShaderGenerator::ShaderGenerator() 
{
	_utilShaderModes.insert(new ShaderModeBase());

	// Precreate the default program to indicate that a valid shadermode root is missing
	_noRootProgram = new Program;

	SourceOutput vert;
	vert.indent() << "void main (void)" << vert.eol();
	vert.indent() << "{" << vert.eol();
	vert.indent() << "    gl_Position = ftransform();" << vert.eol();
	vert.indent() << "}" << vert.eol();
	_noRootProgram->addShader(new osg::Shader(osg::Shader::VERTEX, vert.str()));

	SourceOutput frag;
	frag.indent() << "void main (void)" << frag.eol();
	frag.indent() << "{" << frag.eol();
	frag.indent() << "    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);" << frag.eol();
	frag.indent() << "}" << frag.eol();
	_noRootProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag.str()));
}

void ShaderGenerator::removeAllShaderModes() 
{	
	_shaderModes.clear(); 
	_shaderModes.insert(_utilShaderModes.begin(), _utilShaderModes.end());
}

void ShaderGenerator::addShaderMode(osg::ref_ptr<ShaderMode> shaderMode)
{
	_shaderModes.insert(shaderMode);
}

osg::Program* ShaderGenerator::getOrCreateProgram()
{
	// Check shader cache
	// if cache hit, return cached shader program
	// TODO Current code only returns exact hit, but cache needs to be able to 
	// find the best match that contains all required shadermodes
	ShaderModeListProgramMap::iterator iter = _programCache.find(_shaderModes);
	if (iter != _programCache.end())
	{
		return iter->second;
	}

	// Remove previous ShaderMode tree data
	_rootModes.clear();
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		iitr_sm->get()->removeAllChildren();
	}

	notify(WARN) << "Collected ShaderModes:" << std::endl;
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		notify(WARN) << iitr_sm->get()->getName() << std::endl;
	}
	notify(WARN) << std::endl;

	// Collect shader rootmodes, at least one is needed to be able to generate a tree
	// Multiple shader rootmodes is only usefull if they write to different targets 
	// otherwise they will just overwrite eachother
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		if (iitr_sm->get()->isRootNode())
		{
			notify(WARN) << "Adding root ShaderMode " << iitr_sm->get()->getName() << std::endl;
			_rootModes.insert(iitr_sm->get());
		}
	}

	if (!_rootModes.empty())
	{
		// Build new ShaderMode tree data
		for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
		{
			notify(WARN) << "Processing ShaderMode " << iitr_sm->get()->getName() << std::endl;

			// For each input find the ShaderMode that provides the required output.
			for(ShaderMode::VariableList::iterator iitr = iitr_sm->get()->getInputVariables().begin();
				iitr != iitr_sm->get()->getInputVariables().end();
				++iitr)
			{
				notify(WARN) << " Searching for input '" << (*iitr)._name << "'" << std::endl;
				bool resolved = false;

				for (ShaderModeList::iterator jitr_sm = _shaderModes.begin(); jitr_sm != _shaderModes.end(); ++jitr_sm)
				{
					notify(WARN) << " Trying '" << jitr_sm->get()->getName() << "'" << std::endl;
					if (iitr_sm->get() != jitr_sm->get())
					{
						for(ShaderMode::VariableList::const_iterator oitr = jitr_sm->get()->getOutputVariables().begin();
							oitr != jitr_sm->get()->getOutputVariables().end();
							++oitr)
						{
							if (((*iitr)._name.compare((*oitr)._name) ==0) && ((*iitr)._type.compare((*oitr)._type) == 0))
							{
								iitr_sm->get()->addChild(jitr_sm->get());

								notify(WARN) << "  Linking ShaderMode '" << iitr_sm->get()->getName() << "' to ShaderMode '" << jitr_sm->get()->getName() << "'" << std::endl;
								resolved = true;
							}
						}
					}
				}
				if (!resolved)
				{
					notify(WARN) << "  Input variable " << (*iitr)._name << " could not be resolved." << std::endl
										   <<  "  Falling back to default value." << std::endl;
					
				}

				(*iitr)._useDefault = !resolved;
			}
		}

		// For debugging purposes
	#if DEBUG_SHADERGENERATOR
		SourceOutput graphOutput;
		writeShaderModeGraph(graphOutput);
		notify(NOTICE) << graphOutput.str() << std::endl;
	#endif

		// Build actual shader program
		_compositedProgram = new Program;

		// Collect all Enabled uniforms
		for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
		{
			Uniform* uniform = iitr_sm->get()->getEnabledUniform();
			if (uniform)
			{
				StateAttribute::UniformList::iterator itr=_uniformList.find(uniform->getName());
				if (itr==_uniformList.end())
				{
					// new entry.
					StateAttribute::RefUniformPair& up = _uniformList[uniform->getName()];
					up.first = uniform;
					up.second = 1&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
		            
	//				uniform->addParent(this);
				}
				else
				{
					if (itr->second.first==uniform)
					{
						// changing just override
						itr->second.second = 1&(StateAttribute::OVERRIDE|StateAttribute::PROTECTED);
					}
					else
					{
						// Should never end up here, because ShaderMode Enabled uniform should be unique and
						// only one ShaderMode exists
					}
				}
			}
		}


		notify(NOTICE) << "Building Vertex Shader" << std::endl << std::endl;
	
		SourceOutput vert;
		vert.indent() << "// Generated vertex code" << vert.eol();

		// Traverse shadermode graph to write needed initialization
		writeShaderPart(vert, ShaderMode::VERTEXINIT);
		// Traverse shadermode graph to write needed functions
		// Note: Functions need to be written after global variables that they may use
		writeShaderPart(vert, ShaderMode::VERTEXFUNCTION);
		vert.indent() << "void main (void)" << vert.eol();
		vert.indent() << "{" << vert.eol();
		vert.moveIn();
		// Write all default output variables and where needed default input variables
		writeShaderPart(vert, ShaderMode::VERTEXMAINVARS);
		// Write actual vertex shader code
		writeShaderPart(vert, ShaderMode::VERTEXMAIN);
		vert.moveOut();
		vert.indent() << "}" << vert.eol();

		_compositedProgram->addShader(new osg::Shader(osg::Shader::VERTEX, vert.str()));

#if DEBUG_SHADERGENERATOR
		notify(NOTICE) << vert.str() << std::endl;
#endif

		notify(NOTICE) << "Building Geometry Shader" << std::endl << std::endl;

		SourceOutput geom;
		geom.indent() << "// Generated geometry code" << geom.eol();

		// Traverse shadermode graph to write needed initialization
		writeShaderPart(geom, ShaderMode::GEOMETRYINIT);
		// Traverse shadermode graph to write needed functions
		// Note: Functions need to be written after global variables that they may use
		writeShaderPart(geom, ShaderMode::GEOMETRYFUNCTION);
		geom.indent() << "void main (void)" << geom.eol();
		geom.indent() << "{" << geom.eol();
		geom.moveIn();
		// Write all default output variables and where needed default input variables
		writeShaderPart(geom, ShaderMode::GEOMETRYMAINVARS);
		// Write actual geometry shader code
		writeShaderPart(geom, ShaderMode::GEOMETRYMAIN);
		geom.moveOut();
		geom.indent() << "}" << geom.eol();

//		TODO Implement geometry shadermodes and default mode
//		if (SourceOutput::EndOfLineWithLineCount::lineCount > 4)
//			_compositedProgram->addShader(new osg::Shader(osg::Shader::GEOMETRY, geom.str()));

#if DEBUG_SHADERGENERATOR
		notify(NOTICE) << geom.str() << std::endl;
#endif

		notify(NOTICE) << "Building Fragment Shader" << std::endl << std::endl;

		SourceOutput frag;
		frag.indent() << "// Generated fragment code" << frag.eol();

		// Traverse shadermode graph to write needed initialization
		writeShaderPart(frag, ShaderMode::FRAGMENTINIT);
		// Traverse shadermode graph to write needed functions
		// Note: Functions need to be written after global variables that they may use
		writeShaderPart(frag, ShaderMode::FRAGMENTFUNCTION);
		frag.indent() << "void main (void)" << frag.eol();
		frag.indent() << "{" << frag.eol();
		frag.moveIn();
		// Write all default output variables and where needed default input variables
		writeShaderPart(frag, ShaderMode::FRAGMENTMAINVARS);
		// Write actual fragment shader code
		writeShaderPart(frag, ShaderMode::FRAGMENTMAIN);
		frag.moveOut();
		frag.indent() << "}" << frag.eol();

		_compositedProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, frag.str()));

#if DEBUG_SHADERGENERATOR
		notify(NOTICE) << frag.str() << std::endl;
#endif
	}
	else
	{
		// warn against empty root nodes, create purple warning shader
		notify(WARN) << "No ShaderMode root node found." << std::endl << std::endl;
		_compositedProgram = _noRootProgram;
	}

	_programCache[_shaderModes] = _compositedProgram;

	return _compositedProgram.get();
}

void ShaderGenerator::writeShaderPart(SourceOutput& str, ShaderMode::SourceInjectLocation location, unsigned int index)
{
	// Reset shadermode instance traversal flags
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		iitr_sm->get()->resetTraversed();
	}

	// Traverse shader graph for all available root nodes
	for (ShaderModeList::iterator iitr_rt = _rootModes.begin(); iitr_rt != _rootModes.end(); ++iitr_rt)
	{
		iitr_rt->get()->writeSource(str, location, index);
	}

}

void ShaderGenerator::writeShaderModeGraph(SourceOutput& str)
{
	// Reset shadermode traversal flags
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		iitr_sm->get()->resetTraversed();
	}

	// Traverse shader graph for all available root nodes
	for (ShaderModeList::iterator iitr_rt = _rootModes.begin(); iitr_rt != _rootModes.end(); ++iitr_rt)
	{
		iitr_rt->get()->writeShaderModeDebugInfo(str);
	}

	// Write out all shader modes that did not end up the graph
	str << std::endl << "Orphaned ShaderModes:" << std::endl;
	for (ShaderModeList::iterator iitr_sm = _shaderModes.begin(); iitr_sm != _shaderModes.end(); ++iitr_sm)
	{
		if (!iitr_sm->get()->getTraversed())
		{
			str << iitr_sm->get()->getName() << std::endl;
		}
	}
	str << std::endl;
}

//////////////////////////////////////////////////////////////////////////

/** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
int ShaderGenerator::ShaderModeList::compare(const ShaderModeList& rhs) const
{
	if (size() < rhs.size())
		return -1;
	if (size() > rhs.size())
		return 1;

	// Compare ShaderModes in ShaderModeList
	ShaderModeList::const_iterator iitr_rhs = rhs.begin();

	// Sort by shader mode (ShaderMode < operator)
	for (ShaderModeList::const_iterator iitr = begin(); iitr != end(); ++iitr, ++iitr_rhs)
	{
		if (**iitr != **iitr_rhs)
		{
			if (**iitr < **iitr_rhs)
				return -1;
			else 
				return 1;
		}
	}

	return 0;
}
