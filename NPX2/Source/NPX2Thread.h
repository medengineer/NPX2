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

#ifndef __NPX2THREAD_H_DEFINED__
#define __NPX2THREAD_H_DEFINED__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>

#include "NPX2Components.h"

class SourceNode;
class NPX2Thread;

class RecordingTimer : public Timer
{

public:

    RecordingTimer(NPX2Thread* t_);
    void timerCallback();

    NPX2Thread* thread;
};

class NPX2Thread : public DataThread, public Timer
{

public:

        NPX2Thread(SourceNode* sn);
        ~NPX2Thread();

        String getInfoString();

        XmlElement getInfoXml();

        void openConnection();
        void closeConnection();

        int getNumBasestations();
        int getSlotNumberFor(int slotIndex);

        bool foundInputSource() override;
        bool updateBuffer();
        void updateChannels();

        void setMasterSync(int slotIndex);
        void setSyncOutput(int slotIndex);

        Array<int> getSyncFrequencies();
        void setSyncFrequency(int slotIndex, int freqIndex);

        bool startAcquisition() override;
        bool stopAcquisition() override;

        float getFillPercentage(int slot);

        void setRecordMode(bool recordToNpx);
        void timerCallback();
        void startRecording();
        void stopRecording();

        int getProbeStatus(int slot, int port, int dock);
        void setSelectedProbe(int slot, int port, int dock);
        bool isSelectedProbe(int slot, int port, int dock);

        /** Selects which electrode is connected to each channel. */
        void selectElectrodes(int slot, int port, int dock, Array<int> channelStatus);

        bool runBist(int slot, int port, int dock, int bistIndex);

        // DataThread Methods

        /** Returns the number of virtual subprocessors this source can generate */
        unsigned int getNumSubProcessors() const override;

        /** Returns the number of continuous headstage channels the data source can provide.*/
        int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const override;

        /** Returns the number of TTL channels that each subprocessor generates*/
        int getNumTTLOutputs(int subProcessorIdx) const override;

        /** Returns the sample rate of the data source.*/
        float getSampleRate(int subProcessorIdx) const override;

        /** Returns the volts per bit of the data source.*/
        float getBitVolts(const DataChannel* chan) const override;

        /** Used to set default channel names.*/
        void setDefaultChannelNames() override;

        /** Used to override default channel names.*/
        bool usesCustomNames() const override;

        /** Toggles between internal and external triggering. */
        void setTriggerMode(bool trigger);

        /** Toggles between auto-restart setting. */
        void setAutoRestart(bool restart);

        CriticalSection* getMutex()
        {
            return &displayMutex;
        }

        static DataThread* createDataThread(SourceNode* sn);

        GenericEditor* createEditor(SourceNode* sn);

        /* Loading settings */ 
        struct probeSettings {
            unsigned char slot;
            signed char port;
            Array<int> channelStatus;
            int apGainIndex;
            int lfpGainIndex;
            int refChannelIndex;
            bool disableHighPass;
        } p_settings;
        Array<probeSettings> probeSettingsUpdateQueue;

        void updateProbeSettingsQueue();
        void applyProbeSettingsQueue();

        void setDirectoryForSlot(int slotIndex, File directory);
        File getDirectoryForSlot(int slotIndex);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NPX2Thread);
private:

        Neuropix2API api;

        OwnedArray<Basestation> basestations;

        //Initialization
        bool basestationAvailable;
        int totalProbes;
        bool probesInitialized;

        int selectedSlot;
        int selectedPort;
        int selectedDock;

        //Acquisition-related
        bool autoRestart;
        bool internalTrigger;
        int counter; //?
        int maxCounter; //?
        int last_npx_timestamp; //?

        //Recording-related
        int recordingNumber;
        RecordingTimer recordingTimer;
        bool isRecording;
        bool recordToNpx;

        CriticalSection displayMutex;

};
#endif  // NEUROPIX2THREAD_H_DEFINED
