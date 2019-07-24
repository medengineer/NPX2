/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2019 Allen Institute for Brain Science and Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __NPX2EDITOR_H__
#define __NPX2EDITOR_H__

#include <ProcessorHeaders.h>
#include <EditorHeaders.h>

class NPX2Editor : public GenericEditor
{
public:
	NPX2Editor(GenericProcessor* parentNode, NPX2Thread* thread, bool useDefaultParameterEditors);
	virtual ~NPX2Editor();

private:

	NPX2Thread* thread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NPX2Editor);

};

#endif  // __NPX2EDITOR_H__
