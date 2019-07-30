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

#include "NPX2Thread.h"
#include "NPX2Editor.h"

EditorBackground::EditorBackground(int numBasestations, bool freqSelectEnabled)
    : numBasestations(numBasestations), freqSelectEnabled(freqSelectEnabled) {}

void EditorBackground::setFreqSelectAvailable(bool isAvailable)
{
    freqSelectEnabled = isAvailable;
}

void EditorBackground::paint(Graphics& g)
{

    float x;
    float y = 13;
    float width = 32; 
    float height = 98;
    float cornerSize = 4;
    float lineThickness = 3;

    for (int i = 0; i < numBasestations; i++)
    {

        x = 90 * i + 32;

        //Basestation outline
        g.setColour(Colours::lightgrey);
        g.drawRoundedRectangle(x, y, width, height, cornerSize, lineThickness);
        g.setColour(Colours::darkgrey);
        lineThickness = 1;
        g.drawRoundedRectangle(x, y, width, height, cornerSize, lineThickness);

        //Basestation number label
        width = 10;
        height = 10;
        g.setColour(Colours::darkgrey);
        g.setFont(20);
        g.drawText(String(i + 1), x + 40, y + 2, width, height, Justification::centred);

        //Fifo monitor percentage labels
        x = x + 55;
        width = 50;
        height = 10; 
        g.setFont(8);
        g.drawText(String("0"), x, y + 87, width, height, Justification::centredLeft);
        g.drawText(String("100"), x, y + 47, width, height, Justification::centredLeft);
        g.drawText(String("%"), x, y + 67, width, height, Justification::centredLeft);

        //Number labels for each port
        x = x - 65;
        width = height;
        for (int j = 0; j < 4; j++)
        {
            g.setFont(10);
            g.drawText(String(j + 1), x, 90 - j * 22, width, height, Justification::centredLeft);
        }
    }

    //Sync configuration labels
    x = 90 * (numBasestations) + 32; 
    width = 100;
    height = 10;
    g.setColour(Colours::darkgrey);
    g.setFont(10);
    g.drawText(String("MASTER SYNC"), x, 13, width, height, Justification::centredLeft);
    g.drawText(String("CONFIG AS"), x, 46, width, height, Justification::centredLeft);
    if (freqSelectEnabled)
        g.drawText(String("WITH FREQ"), x, 79, width, height, Justification::centredLeft);

}

FifoMonitor::FifoMonitor(int id_, NPX2Thread* thread_) : id(id_), thread(thread_), fillPercentage(0.0)
{
    startTimer(500); // update fill percentage every 0.5 seconds
}

void FifoMonitor::timerCallback()
{
    //std::cout << "Checking fill percentage for monitor " << id << ", slot " << int(slot) << std::endl;

    if (slot != 255)
    {
        setFillPercentage(thread->getFillPercentage(slot));
    }
}


void FifoMonitor::setSlot(int slot)
{
    this->slot = slot;
}

void FifoMonitor::setFillPercentage(float fill_)
{
    fillPercentage = fill_;

    repaint();
}

void FifoMonitor::paint(Graphics& g)
{
    g.setColour(Colours::grey);
    g.fillRoundedRectangle(0, 0, this->getWidth(), this->getHeight(), 4);
    g.setColour(Colours::lightslategrey);
    g.fillRoundedRectangle(2, 2, this->getWidth()-4, this->getHeight()-4, 2);
    
    g.setColour(Colours::yellow);
    float barHeight = (this->getHeight() - 4) * fillPercentage;
    g.fillRoundedRectangle(2, this->getHeight()-2-barHeight, this->getWidth() - 4, barHeight, 2);
}

ProbeButton::ProbeButton(int id_, NPX2Thread* thread_) : id(id_), thread(thread_), status(0), selected(false)
{
    connected = false;

    setRadioGroupId(979);

    startTimer(10);

}

void ProbeButton::setSlotAndPortAndDock(int slot, int port, int dock)
{
    this->slot = slot;
    this->port = port;
    this->dock = dock;

    std::cout << "Setting button " << id << " to " << slot << ":" << port << ":" << dock << std::endl;

    if (slot == 255 || port == -1)        
        connected = false;
    else
        connected = true;
}

void ProbeButton::setSelectedState(bool state)
{
    selected = state;
}

void ProbeButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver && connected)
        g.setColour(Colours::antiquewhite);
    else
        g.setColour(Colours::darkgrey);
    g.fillEllipse(0, 0, 15, 15);

    ///g.setGradientFill(ColourGradient(Colours::lightcyan, 0, 0, Colours::lightskyblue, 10,10, true));

    if (status == 1)
    {
        if (selected)
        {
            if (isMouseOver)
                g.setColour(Colours::lightgreen);
            else
                g.setColour(Colours::lightgreen);
        }
        else {
            if (isMouseOver)
                g.setColour(Colours::green);
            else
                g.setColour(Colours::green);
        }
    }
    else if (status == 2)
    {
        if (selected)
        {
            if (isMouseOver)
                g.setColour(Colours::lightsalmon);
            else
                g.setColour(Colours::lightsalmon);
        }
        else {
            if (isMouseOver)
                g.setColour(Colours::orange);
            else
                g.setColour(Colours::orange);
        }
    }
    else {
        g.setColour(Colours::lightgrey);
    }
        
    g.fillEllipse(2, 2, 11, 11);
}

void ProbeButton::setProbeStatus(int status)
{
    this->status = status;
    repaint();
}

int ProbeButton::getProbeStatus()
{
    return status;
}

void ProbeButton::timerCallback()
{

    printf("***ProbeButton: timerCallback: status: %d\n", status); fflush(stdout);
    if (slot != 255)
    {
        int status = thread->getProbeStatus(slot, port, dock);
        setProbeStatus(status);
        //std::cout << "Setting for slot: " << String(slot) << " port: " << String(port) << " status: " << String(status) << " selected: " << String(selected) << std::endl;
    }
}

BackgroundLoader::BackgroundLoader(NPX2Thread* thread, NPX2Editor* editor) 
    : Thread("NPX2_Loader"), np(thread), ed(editor)
{
}

BackgroundLoader::~BackgroundLoader()
{
}

void BackgroundLoader::run()
{
    /* Open the NPX-PXI probe connections in the background to prevent this plugin from blocking the main GUI*/
    np->openConnection();

    /* Apply any saved settings */
    CoreServices::sendStatusMessage("Restoring saved probe settings...");
    np->applyProbeSettingsQueue();

    /* Remove button callback timers and select first avalable probe by default*/
    bool selectedFirstAvailableProbe = false;
    for (auto button : ed->probeButtons)
    {
        if (button->getProbeStatus() == 1 && (!selectedFirstAvailableProbe))
        {
            ed->buttonEvent(button);
            selectedFirstAvailableProbe = true;
        }
        button->stopTimer();
    }

    /* Let the main GUI know the plugin is done initializing */
    MessageManagerLock mml;
    CoreServices::updateSignalChain(ed);
    CoreServices::sendStatusMessage("NPX2 plugin ready for acquisition!");

}


NPX2Editor::NPX2Editor(GenericProcessor* parentNode, NPX2Thread* t, bool useDefaultParameterEditors)
 : GenericEditor(parentNode, useDefaultParameterEditors)
{

    thread = t;
    //canvas = nullptr;

    //tabText = "Neuropix PXI";

    int numBasestations = t->getNumBasestations();

    for (int i = 0; i < 2*4*numBasestations; i++)
    {
        int slotIndex = i / 8;
        int portIndex = i / 2 + 1;
        int dockIndex = i % 2; 

        int x_pos = slotIndex * 90 + 33 + dockIndex * 15;
        int y_pos = 125 - portIndex * 22;

        ProbeButton* p = new ProbeButton(i, thread);
        p->setBounds(x_pos, y_pos, 15, 15);
        p->addListener(this);
        p->setSlotAndPortAndDock(t->getSlotNumberFor(slotIndex), portIndex, dockIndex + 1);

        probeButtons.add(p);

        addAndMakeVisible(p);

    }

    for (int i = 0; i < numBasestations; i++)
    {
        int x_pos = i * 90 + 70;
        int y_pos = 50;

        UtilityButton* b = new UtilityButton("", Font("Small Text", 13, Font::plain));
        b->setBounds(x_pos, y_pos, 30, 20);
        b->addListener(this);
        addAndMakeVisible(b);
        directoryButtons.add(b);

        savingDirectories.add(File());

        FifoMonitor* f = new FifoMonitor(i, thread);
        f->setBounds(x_pos + 2, 75, 12, 50);
        addAndMakeVisible(f);
        f->setSlot(0);
        fifoMonitors.add(f);
    }

    masterSelectBox = new ComboBox("MasterSelectComboBox");
    masterSelectBox->setBounds(90 * (numBasestations)+32, 39, 38, 20);
    for (int i = 0; i < numBasestations; i++)
    {
        masterSelectBox->addItem(String(i + 1), i+1);
    }
    masterSelectBox->setSelectedItemIndex(0, false);
    masterSelectBox->addListener(this);
    addAndMakeVisible(masterSelectBox);

    masterConfigBox = new ComboBox("MasterConfigComboBox");
    masterConfigBox->setBounds(90 * (numBasestations)+32, 72, 78, 20);
    masterConfigBox->addItem(String("INPUT"), 1);
    masterConfigBox->addItem(String("OUTPUT"), 2);
    masterConfigBox->setSelectedItemIndex(0, false);
    masterConfigBox->addListener(this);
    addAndMakeVisible(masterConfigBox);

    Array<int> syncFrequencies = t->getSyncFrequencies();

    freqSelectBox = new ComboBox("FreqSelectComboBox");
    freqSelectBox->setBounds(90 * (numBasestations)+32, 105, 70, 20);
    for (int i = 0; i < syncFrequencies.size(); i++)
    {
        freqSelectBox->addItem(String(syncFrequencies[i])+String(" Hz"), i+1);
    }
    freqSelectBox->setSelectedItemIndex(0, false);
    freqSelectBox->addListener(this);
    addChildComponent(freqSelectBox);
    freqSelectBox->setVisible(false);

    desiredWidth = 100 * numBasestations + 120;

    background = new EditorBackground(numBasestations, false);
    background->setBounds(0, 15, 500, 150);
    addAndMakeVisible(background);
    background->toBack();
    background->repaint();

    uiLoader = new BackgroundLoader(t, this);
    uiLoader->startThread();
    
}

NPX2Editor::~NPX2Editor()
{

}

void NPX2Editor::comboBoxChanged(ComboBox* comboBox)
{

    int slotIndex = masterSelectBox->getSelectedId() - 1;

    if (comboBox == masterSelectBox)
    {
        thread->setMasterSync(slotIndex);
        freqSelectBox->setVisible(false);
        background->setFreqSelectAvailable(false);
        freqSelectBox->setSelectedItemIndex(0, true);
    }
    else if (comboBox == masterConfigBox)
    {
        bool asOutput = masterConfigBox->getSelectedId() == 2;
        if (asOutput)
        {
            thread->setSyncOutput(slotIndex);
            freqSelectBox->setVisible(true);
            background->setFreqSelectAvailable(true);
        }
        else
        {
            thread->setMasterSync(slotIndex);
            freqSelectBox->setVisible(false);
            background->setFreqSelectAvailable(false);
        }
        freqSelectBox->setSelectedItemIndex(0, true);
    }
    else /* comboBox == freqSelectBox */
    {
        int freqIndex = freqSelectBox->getSelectedId() - 1;
        thread->setSyncFrequency(slotIndex, freqIndex);
    }

    background->repaint();

}

void NPX2Editor::buttonEvent(Button* button)
{

    if (probeButtons.contains((ProbeButton*) button))
    {
        for (auto button : probeButtons)
        {
            button->setSelectedState(false);
        }
        ProbeButton* probe = (ProbeButton*)button;
        probe->setSelectedState(true);
        thread->setSelectedProbe(probe->slot, probe->port, probe->dock);

        /*      
        if (canvas != nullptr)
            canvas->setSelectedProbe(probe->slot, probe->port, probe->dock);
        */

        repaint();
    }

    if (!acquisitionIsActive)
    {

        if (directoryButtons.contains((UtilityButton*)button))
        {
            // open file chooser to select the saving location for this basestation
            int slotIndex = directoryButtons.indexOf((UtilityButton*)button);
            File currentDirectory = thread->getDirectoryForSlot(slotIndex);
            FileChooser fileChooser("Select directory for NPX file.", currentDirectory);
            if (fileChooser.browseForDirectory())
            {
                File result = fileChooser.getResult();
                String pathName = result.getFullPathName();
                thread->setDirectoryForSlot(slotIndex, result);

                savingDirectories.set(slotIndex, result);
                UtilityButton* ub = (UtilityButton*)button;
                ub->setLabel(pathName.substring(0, 3));
            }

        }
        
    }
}


void NPX2Editor::saveEditorParameters(XmlElement* xml)
{
    std::cout << "Saving Neuropix editor." << std::endl;

    XmlElement* xmlNode = xml->createNewChildElement("NEUROPIXELS_EDITOR");

    for (int slot = 0; slot < thread->getNumBasestations(); slot++)
    {
        String directory_name = savingDirectories[slot].getFullPathName();
        if (directory_name.length() == 2)
            directory_name += "\\\\";
        xmlNode->setAttribute("Slot" + String(slot) + "Directory", directory_name);
    }

}

void NPX2Editor::loadEditorParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("NEUROPIXELS_EDITOR"))
        {
            std::cout << "Found parameters for Neuropixels editor" << std::endl;

            for (int slot = 0; slot < thread->getNumBasestations(); slot++)
            {
                File directory = File(xmlNode->getStringAttribute("Slot" + String(slot) + "Directory"));
                std::cout << "Setting thread directory for slot " << slot << std::endl;
                thread->setDirectoryForSlot(slot, directory);
                directoryButtons[slot]->setLabel(directory.getFullPathName().substring(0, 2));
                savingDirectories.set(slot, directory);
            }
        }
    }
}

/*
Visualizer* NPX2Editor::createNewCanvas(void)
{
    std::cout << "Button clicked..." << std::endl;
    GenericProcessor* processor = (GenericProcessor*) getProcessor();
    std::cout << "Got processor." << std::endl;
    canvas = new NeuropixCanvas(processor, this, thread);
    std::cout << "Created canvas." << std::endl;
    return canvas;
}
*/
