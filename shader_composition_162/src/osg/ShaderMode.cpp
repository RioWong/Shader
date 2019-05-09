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

#include <osg/ShaderMode>

using namespace osg;

int SourceOutput::EndOfLineWithLineCount::lineCount = 0;
long SourceOutput::EndOfLineWithLineCount::lastPos = 0;
        
/** Add shader source to given stream, but wrapped in enabled flag **/
void ShaderMode::writeSource(SourceOutput& str, SourceInjectLocation location, unsigned int index) 
{
	_traversed = true;

	// First traverse into children (postorder graph traversal)
	writeChildren(str, location, index);

	if (((location == ShaderMode::VERTEXINIT) && (_injectionMask &  ShaderMode::VERTEXMAIN)) ||
		((location == ShaderMode::GEOMETRYINIT) && (_injectionMask &  ShaderMode::GEOMETRYMAIN)) ||
		((location == ShaderMode::FRAGMENTINIT) && (_injectionMask &  ShaderMode::FRAGMENTMAIN)))
	{	
		// Write out default values for all unresolved inputs
		for (ShaderMode::VariableList::const_iterator iitr = _inputs.begin();
			iitr != _inputs.end();
			++iitr)
		{
			if ((*iitr)._useDefault)
			{
				str.indent() << (*iitr)._type << " " << (*iitr)._name << " = " << (*iitr)._type << "(" << (*iitr)._default << ");" << str.eol();
			}
		}

		if (isSwitchable())
		{
			str.indent() << "uniform bool " << _name << "Enabled;" << str.eol();
		}

		writeSourceImplementation(str, location, index);
	}
	else if (((location == ShaderMode::VERTEXMAIN) && (_injectionMask &  ShaderMode::VERTEXMAIN)) ||
			 ((location == ShaderMode::GEOMETRYMAIN) && (_injectionMask &  ShaderMode::GEOMETRYMAIN)) ||
			 ((location == ShaderMode::FRAGMENTMAIN) && (_injectionMask &  ShaderMode::FRAGMENTMAIN)))
	{
		// Wrap source code in enabled branch
		// if branch is disabled the default output will still be available for depending shadermodes
		if (isSwitchable())
		{
			str.indent() << "if (" << getName() << "Enabled)" << str.eol();
		}

		// If not switchable we still keep the scoping brackets to prevent collision of local variables
		str.indent() << "{" << str.eol();
		str.moveIn();
	
		writeSourceImplementation(str, location, index);

		str.moveOut();
		str.indent() << "}" << str.eol();
	}
	else		
	{
		writeSourceImplementation(str, location, index);
	}
}

/** Add shader source to given stream, but wrapped in enabled flag **/
void ShaderMode::writeShaderModeDebugInfo(SourceOutput& str) 
{
	_traversed = true;

	str.indent() << getName() << std::endl;
/*
	for (unsigned int i=0; i < _outputs.size(); i++)
	{
		str.indent() << "    // out: " << _outputs[i] << ""  << std::endl;
	}
		
	for (unsigned int i=0; i < _inputs.size(); i++)
	{
		str.indent() << "    // in: " << _inputs[i] << "" << std::endl;
	}	
*/
	str.moveIn();
	
	// Traverse children 
	for (size_t i=0; i < _children.size(); i++)
	{
		// Only let child write debug not already done
		if (!_children[i]->getTraversed())
		{
			_children[i]->writeShaderModeDebugInfo(str);
		}
		else
		{
			str.indent() << _children[i]->getName() << " already traversed." << std::endl;
		}
	}

	str.moveOut();
}


int ShaderMode::compare(const ShaderMode& rhs) const
{
	if (this==&rhs) return 0;
	if (getType() < rhs.getType())
		return -1;
	if (getType() > rhs.getType())
		return 1;
	return 0; 
}


int MultiShaderMode::compare(const ShaderMode& rhs) const
{
	if (this==&rhs) return 0;
	if (getType() < rhs.getType())
		return -1;
	if (getType() > rhs.getType())
		return 1;
    const MultiShaderMode& rhs_ms = static_cast<const MultiShaderMode&>(rhs);
	if (_index < rhs_ms._index)
		return 1;
	if (_index > rhs_ms._index)
		return -1;
	return 0; 
}

