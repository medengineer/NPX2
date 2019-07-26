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

#ifndef __NPX2COMPONENTS_H__
#define __NPX2COMPONENTS_H__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>

#include "npx2-api/NeuropixAPI.h"

#define MAX_NUM_SLOTS 32
#define NUM_CHANNELS 384
#define NUM_ELECTRODES 1280
#define NUM_REF_ELECTRODES 5
#define NUM_PORTS 4
#define NUM_DOCKS 2
#define SAMPLECOUNT 64

class BasestationConnectBoard;
class Flex;
class Headstage;
class Probe;

class NeuropixComponent
{
public:
	NeuropixComponent();

	String version;
	uint64_t serial_number;
	String part_number;

	virtual void getInfo() = 0;
};

class NeuropixAPI : public NeuropixComponent
{
public:
	void getInfo();
};

class Basestation : public NeuropixComponent
{
public:
	Basestation(int slot);
	~Basestation();

	int slot;
	String boot_version;
	void updateFirmware();

	ScopedPointer<BasestationConnectBoard> basestationConnectBoard;
	OwnedArray<Probe> probes;

	void init();

	float getTemperature();

	int getProbeCount();
	void initializeProbes();

	void setChannels(int slot, int port, int dock, Array<int> channelStatus);
	void setReferences(int slot, int port, int dock, np::channelreference_t refId, np::electrodebanks_t bank);
	void setGains(int slot, int port, int dock, unsigned char apGain, unsigned char lfpGain);
	void setApFilterState(int slot, int port, int dock, bool filterState);

	void setSyncAsInput();
	void setSyncAsOutput(int freqIndex);
	Array<int> getSyncFrequencies();

	void startAcquisition();
	void stopAcquisition();

	void setSavingDirectory(File);
	File getSavingDirectory();

	float getFillPercentage();

	void getInfo();
	
private:

	bool probesInitialized;
	Array<int> syncFrequencies;
	File savingDirectory;
};

class BasestationConnectBoard : public NeuropixComponent
{
public:
	BasestationConnectBoard(Basestation*);
	String boot_version;

	bool updateFirmware();

	Basestation* basestation;

	void getInfo();
};

class Probe : public NeuropixComponent, public Thread
{
public:
	Probe(Basestation* bs, int port, int dock);

	Basestation* basestation;
	int port;
	int dock;
	int shank;

	DataBuffer* stream;
	int64 timestamp;

	ScopedPointer<Headstage> headstage;
	ScopedPointer<Flex> flex;

	int reference;
	int ap_gain;
	int lfp_gain;
	bool highpass_on;

	void init();

	void setChannels(Array<int> channelStatus);
	Array<np::electrodebanks_t> channelMap;

	Array<int> apGains;
	Array<int> lfpGains;

	void setApFilterState(bool);
	void setReferences(np::channelreference_t refId, np::electrodebanks_t refBank);
	void setGains(unsigned char apGain, unsigned char lfpGain);

	void calibrate();

	void setStatus(int status_);
	int status;

	void setSelected(bool isSelected_);
	bool isSelected;

	void getInfo();

	int channel_count;

	float fifoFillPercentage;

	String name;

	void run();

	uint64 eventCode;

private:
	 
	Array<int> gains;

	np::PacketInfo* pckinfo;
	int16_t* data;
	np::electrodePacket packet[SAMPLECOUNT];

};

class Headstage : public NeuropixComponent
{
public:
	Headstage::Headstage(Probe*);
	Probe* probe;
	void getInfo();
};

class Flex : public NeuropixComponent
{
public:
	Flex::Flex(Probe*);
	Probe* probe;
	void getInfo();
};

#endif  // __NEUROPIXCOMPONENTS_H_2C4C2D67__