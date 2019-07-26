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

NPX2Thread::NPX2Thread(SourceNode* sn) : DataThread(sn)
{
    
    np::NP_ErrorCode ec; 

    uint32_t availableSlotMask;

    np::getAvailableSlots(&availableSlotMask);

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
                //basestations[i]->setSyncAsInput();
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

    np::setParameter(np::NP_PARAM_BUFFERSIZE, MAXSTREAMBUFFERSIZE);
    np::setParameter(np::NP_PARAM_BUFFERCOUNT, MAXSTREAMBUFFERCOUNT);
}

void NPX2Thread::closeConnection()
{
    //TODO: Properly close all connections...
}

bool NPX2Thread::foundInputSource()
{
    //Colors the plugin orange indicating input source is ready...
	return basestationAvailable;
}

bool NPX2Thread::updateBuffer()
{
	return false;
}

/** Initializes data transfer.*/
bool NPX2Thread::startAcquisition()
{
    return true;
}

/** Stops data transfer.*/
bool NPX2Thread::stopAcquisition()
{
    return false;
}

void NPX2Thread::setDefaultChannelNames()
{
}

bool NPX2Thread::usesCustomNames() const
{
	return true;
}


/** Returns the number of virtual subprocessors this source can generate */
unsigned int NPX2Thread::getNumSubProcessors() const
{
	return 1;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int NPX2Thread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{
	return 1;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int NPX2Thread::getNumTTLOutputs(int subProcessorIdx) const 
{
	return 8;
}

/** Returns the sample rate of the data source.*/
float NPX2Thread::getSampleRate(int subProcessorIdx) const
{
	return 30000;
}

/** Returns the volts per bit of the data source.*/
float NPX2Thread::getBitVolts(const DataChannel* chan) const
{
	//std::cout << "BIT VOLTS == 0.195" << std::endl;
	return 0.1950000f;
}

void NPX2Thread::setTriggerMode(bool trigger)
{
    //TODO
}

void NPX2Thread::setAutoRestart(bool restart)
{
	//TODO
}
