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
#include <VisualizerEditorHeaders.h>

class NPX2Canvas;
class NPX2Editor;
class NPX2Interface;
class EditorBackground;
class BackgroundLoader;

class FifoMonitor;
class ProbeButton;

class UtilityButton;
class Annotation;
class ColorSelector;
class SourceNode;

class NPX2Editor : public VisualizerEditor, public ComboBox::Listener
{
public:
	NPX2Editor(GenericProcessor* parentNode, NPX2Thread* thread, bool useDefaultParameterEditors);
	virtual ~NPX2Editor();

    void comboBoxChanged(ComboBox*);
    void buttonEvent(Button* button);

    void saveEditorParameters(XmlElement*);
    void loadEditorParameters(XmlElement*);

    Visualizer* createNewCanvas(void);

    OwnedArray<ProbeButton> probeButtons;

private:

	NPX2Thread* thread;
    NPX2Canvas* canvas;
    Viewport* viewport;

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
    int getProbeStatus();
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

class NPX2Canvas : public Visualizer, public Button::Listener
{
public:
    NPX2Canvas(GenericProcessor* p, NPX2Editor*, NPX2Thread*);
    ~NPX2Canvas();

    void paint(Graphics& g);

    void refresh();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float);
    void buttonClicked(Button* button);

    void setSelectedProbe(int, int, int);

    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);

    void resized();

    SourceNode* processor;
    ScopedPointer<Viewport> neuropixViewport;
    OwnedArray<NPX2Interface> neuropixInterfaces;

    int option;

    NPX2Editor* editor;

};

class NPX2Interface : public Component, public Button::Listener, public ComboBox::Listener, public Label::Listener, public Timer
{
public:
    NPX2Interface(XmlElement info, int slot, int port, int dock, NPX2Thread*, NPX2Editor*);
    ~NPX2Interface();

    void paint(Graphics& g);

    void mouseMove(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;
    MouseCursor getMouseCursor();

    void buttonClicked(Button*);
    void comboBoxChanged(ComboBox*);
    void labelTextChanged(Label* l);

    void saveParameters(XmlElement* xml);
    void loadParameters(XmlElement* xml);

    void setAnnotationLabel(String, Colour);
    void updateInfoString();

    void timerCallback();

    int slot;
    int port;
    int dock;
    int shank = 0;

private:
    
    NPX2Thread* thread;
    NPX2Editor* editor;
    DataBuffer* inputBuffer;
    AudioSampleBuffer displayBuffer;

    
    XmlElement neuropix_info;

    ScopedPointer<ComboBox> lfpGainComboBox;
    ScopedPointer<ComboBox> apGainComboBox;
    ScopedPointer<ComboBox> referenceComboBox;
    ScopedPointer<ComboBox> filterComboBox;

    ScopedPointer<ComboBox> bistComboBox;

    ScopedPointer<UtilityButton> enableButton;
    ScopedPointer<UtilityButton> selectAllButton;

    ScopedPointer<Viewport> infoLabelView;
    ScopedPointer<Label> infoLabel;
    ScopedPointer<Label> lfpGainLabel;
    ScopedPointer<Label> apGainLabel;
    ScopedPointer<Label> referenceLabel;
    ScopedPointer<Label> filterLabel;
    ScopedPointer<Label> outputLabel;
    ScopedPointer<Label> bistLabel;
    ScopedPointer<Label> annotationLabelLabel;
    ScopedPointer<Label> annotationLabel;

    ScopedPointer<Label> mainLabel;

    ScopedPointer<UtilityButton> enableViewButton;
    ScopedPointer<UtilityButton> lfpGainViewButton;
    ScopedPointer<UtilityButton> apGainViewButton;
    ScopedPointer<UtilityButton> referenceViewButton;
    ScopedPointer<UtilityButton> outputOnButton;
    ScopedPointer<UtilityButton> outputOffButton;
    ScopedPointer<UtilityButton> annotationButton;
    ScopedPointer<UtilityButton> bistButton;


    ScopedPointer<ColorSelector> colorSelector;
        
    Array<int> channelStatus;
    Array<int> channelReference;
    Array<int> channelApGain;
    Array<int> channelLfpGain;
    Array<int> channelOutput;
    Array<int> channelSelectionState;

    Array<Colour> channelColours;

    bool isOverZoomRegion;
    bool isOverUpperBorder;
    bool isOverLowerBorder;
    bool isOverChannel;

    int zoomHeight;
    int zoomOffset;
    int initialOffset;
    int initialHeight;
    int lowerBound;
    int dragZoneWidth;

    int lowestChan;
    int highestChan;

    float channelHeight;

    int visualizationMode;

//      Rectangle<int> selectionBox;
    bool isSelectionActive;

    MouseCursor::StandardCursorType cursorType;

    Path shankPath;

    String channelInfoString;

    Colour getChannelColour(int chan);
    int getNearestChannel(int x, int y);
    String getChannelInfoString(int chan);

    void drawLegend(Graphics& g);
    void drawAnnotations(Graphics& g);

    Array<Annotation> annotations;

    Array<int> getSelectedChannels();

    int getChannelForElectrode(int);
    int getConnectionForChannel(int);

};

class Annotation
{
public:
    Annotation(String text, Array<int> channels, Colour c);
    ~Annotation();

    Array<int> channels;
    String text;

    float currentYLoc;

    bool isMouseOver;
    bool isSelected;

    Colour colour;

};

class ColorSelector : public Component, public ButtonListener
{
public:
    ColorSelector(NPX2Interface* np);
    ~ColorSelector();

    Array<Colour> standardColors;
    Array<Colour> hoverColors;
    StringArray strings;

    OwnedArray<ShapeButton> buttons;

    void buttonClicked(Button* button);

    void updateCurrentString(String s);

    Colour getCurrentColour();

    NPX2Interface* npi;

    int activeButton;

};

#endif  // __NPX2EDITOR_H__
