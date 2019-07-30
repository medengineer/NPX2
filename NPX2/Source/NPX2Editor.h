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

class UtilityButton;
class SourceNode;
class NPX2Editor;

class EditorBackground : public Component
{
public:
    EditorBackground(int numBasestations, bool freqSelectEnabled);
    void setFreqSelectAvailable(bool available);

private:
    void paint(Graphics& g);

    int numBasestations;
    bool freqSelectEnabled;

};

class BackgroundLoader : public Thread
{
public:
    BackgroundLoader(NPX2Thread* t, NPX2Editor* e);
    ~BackgroundLoader();
    void run();
private:
    NPX2Thread* np;
    NPX2Editor* ed;
};

class ProbeButton : public ToggleButton, public Timer
{
public:
    ProbeButton(int id, NPX2Thread* thread);

    void setSlotAndPortAndDock(int, int, int);
    void setSelectedState(bool);

    void setProbeStatus(int status);
    void timerCallback();

    int slot;
    int port;
    int dock;
    bool connected;
    NPX2Thread* thread;

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    int id;
    int status;
    bool selected;
};

class FifoMonitor : public Component, public Timer
{
public:
    FifoMonitor(int id, NPX2Thread* thread);

    void setSlot(int);

    void setFillPercentage(float);

    void timerCallback();

    int slot;
private:
    void paint(Graphics& g);

    float fillPercentage;
    NPX2Thread* thread;
    int id;
};



class NPX2Editor : public GenericEditor, public ComboBox::Listener
{
public:
	NPX2Editor(GenericProcessor* parentNode, NPX2Thread* thread, bool useDefaultParameterEditors);
	virtual ~NPX2Editor();

    void comboBoxChanged(ComboBox*);
    void buttonEvent(Button* button);

    void saveEditorParameters(XmlElement*);
    void loadEditorParameters(XmlElement*);

    OwnedArray<ProbeButton> probeButtons;

private:

	NPX2Thread* thread;

    OwnedArray<UtilityButton> directoryButtons;
    OwnedArray<FifoMonitor> fifoMonitors; 

    ScopedPointer<ComboBox> masterSelectBox;
    ScopedPointer<ComboBox> masterConfigBox;
    ScopedPointer<ComboBox> freqSelectBox;

    Array<File> savingDirectories;

    ScopedPointer<BackgroundLoader> uiLoader;
    ScopedPointer<EditorBackground> background;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NPX2Editor);

};

#endif  // __NPX2EDITOR_H__
