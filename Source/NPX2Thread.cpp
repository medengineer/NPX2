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

class NPX2Thread;

DataThread* NPX2Thread::createDataThread(SourceNode *sn)
{
	return new NPX2Thread(sn);
}

GenericEditor* NPX2Thread::createEditor(SourceNode* sn)
{
    return new NPX2Editor(sn, this, true);
}

NPX2Thread::NPX2Thread(SourceNode* sn) : DataThread(sn), recordingTimer(this)
{
    
    api.getInfo();

    basestationAvailable = false;
    probesInitialized = false;
    isRecording = false;
    recordingNumber = 0;

    np::NP_ErrorCode ec; 

    uint32_t availableSlotMask;

    np::getAvailableSlots(&availableSlotMask);

    totalProbes = 0;
    for (int slot = 0; slot < MAX_NUM_SLOTS; slot++)
    {
        if ((availableSlotMask >> slot) & 1)
        {
            basestations.add(new Basestation(slot));
        }
    }

}

NPX2Thread::~NPX2Thread()
{
    closeConnection();
}

int NPX2Thread::getNumBasestations()
{
    return basestations.size();
}

int NPX2Thread::getSlotNumberFor(int slotIndex)
{
    return basestations[slotIndex]->slot;
}

void NPX2Thread::setMasterSync(int slotIndex)
{
    basestations[slotIndex]->setSyncAsInput();
}

void NPX2Thread::setSyncOutput(int slotIndex)
{
    basestations[slotIndex]->setSyncAsOutput(0);
}

Array<int> NPX2Thread::getSyncFrequencies()
{
    return basestations[0]->getSyncFrequencies();
}


void NPX2Thread::setSyncFrequency(int slotIndex, int freqIndex)
{
    basestations[slotIndex]->setSyncAsOutput(freqIndex);
}

void NPX2Thread::openConnection()
{

    bool foundSync = false;

    for (int i = 0; i < basestations.size(); i++)
    {

        basestations[i]->init();

        if (basestations[i]->getProbeCount() > 0)
        {
            totalProbes += basestations[i]->getProbeCount();

            basestationAvailable = true;

            if (!foundSync)
            {
                basestations[i]->setSyncAsInput();
                selectedSlot = basestations[i]->slot;
                selectedPort = basestations[i]->probes[0]->port;
                selectedDock = basestations[i]->probes[0]->dock;
                foundSync = true;
            }

            for (int probe_num = 0; probe_num < basestations[i]->getProbeCount(); probe_num++)
            {
                std::cout << "Creating buffer for slot: " << int(basestations[i]->slot) 
                << ", port: " << int(basestations[i]->probes[probe_num]->port) 
                << ", dock: " << int(basestations[i]->probes[probe_num]->dock) << std::endl;
                sourceBuffers.add(new DataBuffer(384, 10000));  // full-band buffer
                basestations[i]->probes[probe_num]->stream = sourceBuffers.getLast();

                CoreServices::sendStatusMessage("Initializing probe " + String(probe_num + 1) + "/" + String(basestations[i]->getProbeCount()) + 
                    " on Basestation " + String(i + 1) + "/" + String(basestations.size()));
                
                basestations[i]->initializeProbes();
            }
                
        }
            
    }

    /* MAXSTREAMBUFFERSIZE, MAXSTREAMBUFFERCOUNT are not inclued in API 2.8
    //np::setParameter(np::NP_PARAM_BUFFERSIZE, MAXSTREAMBUFFERSIZE);
    //np::setParameter(np::NP_PARAM_BUFFERCOUNT, MAXSTREAMBUFFERCOUNT);

}

void NPX2Thread::closeConnection()
{
    //TODO: Properly close all connections...
}

void NPX2Thread::updateProbeSettingsQueue()
{
    probeSettingsUpdateQueue.add(this->p_settings);
}

void NPX2Thread::applyProbeSettingsQueue()
{
    for (auto settings : probeSettingsUpdateQueue)
    {
        
        selectElectrodes(settings.slot, settings.port, settings.dock, settings.channelStatus);
        setAllReferences(settings.slot, settings.port, settings.dock, settings.refChannelIndex);
        /*
        setAllGains(settings.slot, settings.port, settings.apGainIndex, settings.lfpGainIndex);
        setFilter(settings.slot, settings.port, settings.disableHighPass);
        */

    }
}

void NPX2Thread::setAllReferences(int slot, int port, int dock, int refId)
{
 
    np::NP_ErrorCode ec;
    np::channelreference_t ref;
    np::electrodebanks_t bank;

    if (refId == 0) // external reference
    {
        ref = np::EXT_REF;
        bank = np::None;
    }
    else if (refId == 1) // tip reference
    {
        ref = np::TIP_REF;
        bank = np::None;
    }
    else if (refId > 1) // internal reference
    {
        ref = np::INT_REF;
        bank = static_cast<np::electrodebanks_t>(refId - 1);
    }

    for (int i = 0; i < basestations.size(); i++)
    {
        basestations[i]->setReferences(slot, port, dock, ref, bank);
    }

}

bool NPX2Thread::foundInputSource()
{
	return basestationAvailable;
}

void NPX2Thread::setDirectoryForSlot(int slotIndex, File directory)
{
    std::cout << "Thread setting directory for slot " << slotIndex << " to " << directory.getFileName() << std::endl;

    if (slotIndex < basestations.size())
    {
        basestations[slotIndex]->setSavingDirectory(directory);
    }
}

File NPX2Thread::getDirectoryForSlot(int slotIndex)
{
    if (slotIndex < basestations.size())
    {
        return basestations[slotIndex]->getSavingDirectory();
    }
    else {
        return File::getCurrentWorkingDirectory();
    }
}

XmlElement NPX2Thread::getInfoXml()
{

    XmlElement neuropix_info("NEUROPIX-PXI");

    XmlElement* api_info = new XmlElement("API");
    api_info->setAttribute("version", api.version);
    neuropix_info.addChildElement(api_info);

    for (int i = 0; i < basestations.size(); i++)
    {
        XmlElement* basestation_info = new XmlElement("BASESTATION");
        basestation_info->setAttribute("index", i + 1);
        basestation_info->setAttribute("slot", int(basestations[i]->slot));
        basestation_info->setAttribute("firmware_version", basestations[i]->boot_version);
        basestation_info->setAttribute("bsc_firmware_version", basestations[i]->basestationConnectBoard->boot_version);
        basestation_info->setAttribute("bsc_part_number", basestations[i]->basestationConnectBoard->part_number);
        basestation_info->setAttribute("bsc_serial_number", String(basestations[i]->basestationConnectBoard->serial_number));

        for (int j = 0; j < basestations[i]->getProbeCount(); j++)
        {
            XmlElement* probe_info = new XmlElement("PROBE");
            probe_info->setAttribute("port", int(basestations[i]->probes[j]->port));
            probe_info->setAttribute("dock", int(basestations[i]->probes[j]->dock));
            probe_info->setAttribute("probe_serial_number", String(basestations[i]->probes[j]->serial_number));
            probe_info->setAttribute("hs_serial_number", String(basestations[i]->probes[j]->headstage->serial_number));
            probe_info->setAttribute("hs_part_number", basestations[i]->probes[j]->headstage->part_number);
            probe_info->setAttribute("hs_version", basestations[i]->probes[j]->headstage->version);
            probe_info->setAttribute("flex_part_number", basestations[i]->probes[j]->flex->part_number);
            probe_info->setAttribute("flex_version", basestations[i]->probes[j]->flex->version);

            basestation_info->addChildElement(probe_info);

        }

        neuropix_info.addChildElement(basestation_info);

    }

    return neuropix_info;

}


String NPX2Thread::getInfoString()
{

    String infoString;

    infoString += "API Version: ";
    infoString += api.version;
    infoString += "\n";
    infoString += "\n";
    infoString += "\n";

    for (int i = 0; i < basestations.size(); i++)
    {
        infoString += "Basestation ";
        infoString += String(i + 1);
        infoString += "\n";
        infoString += "  Firmware version: ";
        infoString += basestations[i]->boot_version;
        infoString += "\n";
        infoString += "  BSC firmware version: ";
        infoString += basestations[i]->basestationConnectBoard->boot_version;
        infoString += "\n";
        infoString += "  BSC part number: ";
        infoString += basestations[i]->basestationConnectBoard->part_number;
        infoString += "\n";
        infoString += "  BSC serial number: ";
        infoString += String(basestations[i]->basestationConnectBoard->serial_number);
        infoString += "\n";
        infoString += "\n";

        for (int j = 0; j < basestations[i]->getProbeCount(); j++)
        {
            infoString += "    Port ";
            infoString += String(basestations[i]->probes[j]->port);
            infoString += "\n";
            infoString += "\n";
            infoString += "    Dock ";
            infoString += String(basestations[i]->probes[j]->dock);
            infoString += "\n";
            infoString += "\n";
            infoString += "    Probe serial number: ";
            infoString += String(basestations[i]->probes[j]->serial_number);
            infoString += "\n";
            infoString += "\n";
            infoString += "    Headstage serial number: ";
            infoString += String(basestations[i]->probes[j]->headstage->serial_number);
            infoString += "\n";
            infoString += "    Headstage part number: ";
            infoString += basestations[i]->probes[j]->headstage->part_number;
            infoString += "\n";
            infoString += "    Headstage version: ";
            infoString += basestations[i]->probes[j]->headstage->version;
            infoString += "\n";
            infoString += "\n";
            infoString += "    Flex part number: ";
            infoString += basestations[i]->probes[j]->flex->part_number;
            infoString += "\n";
            infoString += "    Flex version: ";
            infoString += basestations[i]->probes[j]->flex->version;
            infoString += "\n";
            infoString += "\n";
            infoString += "\n";

        }
        infoString += "\n";
        infoString += "\n";
    }


    return infoString;

}
void NPX2Thread::selectElectrodes(int slot, int port, int dock, Array<int> channelStatus)
{

    for (int i = 0; i < basestations.size(); i++)
    {
        basestations[i]->setChannels(slot, port, dock, channelStatus);
    }

}

bool NPX2Thread::runBist(int slot, int port, int dock, int bistIndex)
{
    bool returnValue = false;

    for (int i = 0; i < basestations.size(); i++)
    {
        returnValue = basestations[i]->runBist(slot, port, dock, bistIndex);
    }

    return returnValue;
}

bool NPX2Thread::updateBuffer()
{

    bool shouldRecord = CoreServices::getRecordingStatus();

    if (!isRecording && shouldRecord)
    {
        isRecording = true;
        recordingTimer.startTimer(1000);
    }
    else if (isRecording && !shouldRecord)
    {
        isRecording = false;
        stopRecording();
    }

    return true;
}

bool NPX2Thread::startAcquisition()
{

    counter = 0;
    maxCounter = 0;

    last_npx_timestamp = 0;

    startTimer(500 * totalProbes); // wait for signal chain to be built //?
    return true;
}

bool NPX2Thread::stopAcquisition()
{

    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    for (int i = 0; i < basestations.size(); i++)
    {
        basestations[i]->stopAcquisition();
    }

    return true;
}

void NPX2Thread::timerCallback()
{

    for (int i = 0; i < basestations.size(); i++)
    {
        basestations[i]->startAcquisition();
    }

    startThread();
    stopTimer();


}

float NPX2Thread::getFillPercentage(int slot)
{

    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->slot == slot)
        {
            return basestations[i]->getFillPercentage();
        }
            
    }

    return 0.0f;
}

void NPX2Thread::startRecording()
{
    recordingNumber++;

    File rootFolder = CoreServices::RecordNode::getRecordingPath();
    String pathName = rootFolder.getFileName();
    
    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->getProbeCount() > 0)
        {
            File savingDirectory = basestations[i]->getSavingDirectory();

            if (!savingDirectory.getFullPathName().isEmpty())
            {
                File fullPath = savingDirectory.getChildFile(pathName);

                if (!fullPath.exists())
                {
                    fullPath.createDirectory();
                }

                File npxFileName = fullPath.getChildFile("recording_slot" + String(basestations[i]->slot) + "_" + String(recordingNumber) + ".npx2");

                np::setFileStream(basestations[i]->slot, npxFileName.getFullPathName().getCharPointer());
                np::enableFileStream(basestations[i]->slot, true);

                std::cout << "Basestation " << i << " started recording." << std::endl;
            }
            
        }
    }

}

void NPX2Thread::stopRecording()
{
    for (int i = 0; i < basestations.size(); i++)
    {
        np::enableFileStream(basestations[i]->slot, false);
    }

    std::cout << "NeuropixThread stopped recording." << std::endl;
}

void NPX2Thread::setDefaultChannelNames()
{
    for (int bs_num = 0; bs_num < basestations.size(); bs_num++)
    {
        for (int i = 0; i < NUM_CHANNELS; i++)
        {
            ChannelCustomInfo info;
            info.name = "CH" + String(i + 1);
            info.gain = NPX2_BITVOLTS;
            channelInfo.set(i, info);
        }
    }
}

bool NPX2Thread::usesCustomNames() const
{
	return true;
}


/** Returns the number of virtual subprocessors this source can generate */
unsigned int NPX2Thread::getNumSubProcessors() const
{
	return totalProbes > 0 ? totalProbes : 1;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int NPX2Thread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{
    if (type == DataChannel::DataChannelTypes::HEADSTAGE_CHANNEL)
    {
	   return NUM_CHANNELS;
    }
    return 0;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int NPX2Thread::getNumTTLOutputs(int subProcessorIdx) const 
{
	return 1;
}

/** Returns the sample rate of the data source.*/
float NPX2Thread::getSampleRate(int subProcessorIdx) const
{
	return SAMPLERATE;
}

/** Returns the volts per bit of the data source.*/
float NPX2Thread::getBitVolts(const DataChannel* chan) const
{
	return NPX2_BITVOLTS;
}

void NPX2Thread::setTriggerMode(bool useInternalTrigger)
{
    //ConfigAccessErrorCode caec = neuropix.neuropix_triggerMode(trigger);
    
    this->internalTrigger = useInternalTrigger;
}

void NPX2Thread::setRecordMode(bool recordToNpx)
{
    this->recordToNpx = recordToNpx;
}

void NPX2Thread::setAutoRestart(bool autoRestart)
{
    this->autoRestart = autoRestart;
}


ProbeStatus NPX2Thread::getProbeStatus(int slot, int port, int dock)
{
    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->slot == slot)
        {
            for (int probe_num = 0; probe_num < basestations[i]->getProbeCount(); probe_num++)
            {
                if (basestations[i]->probes[probe_num]->port == port && basestations[i]->probes[probe_num]->dock == dock)
                {
                    return basestations[i]->probes[probe_num]->status;
                }
            }
        }

    }
    return ProbeStatus::DISCONNECTED;
}

bool NPX2Thread::isSelectedProbe(int slot, int port, int dock)
{
    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->slot == slot)
        {
            for (int probe_num = 0; probe_num < basestations[i]->getProbeCount(); probe_num++)
            {
                if (basestations[i]->probes[probe_num]->port == port && basestations[i]->probes[probe_num]->dock == dock)
                {
                    return basestations[i]->probes[probe_num]->isSelected;
                }
            }
        }

    }
    return false;
}

void NPX2Thread::setSelectedProbe(int slot, int port, int dock)
{
    int currentSlot, currentPort, currentDock;
    int newSlot, newPort, newDock;

    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->slot == slot)
        {
            for (int probe_num = 0; probe_num < basestations[i]->getProbeCount(); probe_num++)
            {
                if (basestations[i]->probes[probe_num]->port == port && basestations[i]->probes[probe_num]->dock == dock)
                {
                    newSlot = i;
                    newPort = probe_num;
                    basestations[i]->probes[probe_num]->setSelected(true);
                }
            }
        }
    }

    for (int i = 0; i < basestations.size(); i++)
    {
        if (basestations[i]->slot == selectedSlot)
        {
            for (int probe_num = 0; probe_num < basestations[i]->getProbeCount(); probe_num++)
            {
                if (basestations[i]->probes[probe_num]->port == selectedPort && basestations[i]->probes[probe_num]->dock == selectedDock)
                {
                    currentSlot = i;
                    currentPort = probe_num;
                    basestations[i]->probes[probe_num]->setSelected(false);
                }
            }
        }
    }

    //basestations[newSlot]->probes[newPort]->ap_timestamp = basestations[currentSlot]->probes[currentPort]->ap_timestamp;
    //basestations[newSlot]->probes[newPort]->lfp_timestamp = basestations[currentSlot]->probes[currentPort]->lfp_timestamp;

    selectedSlot = slot;
    selectedPort = port;
    selectedDock = dock;
}

RecordingTimer::RecordingTimer(NPX2Thread* t_)
{
    thread = t_;
}

void RecordingTimer::timerCallback()
{
    thread->startRecording();
    stopTimer();
}