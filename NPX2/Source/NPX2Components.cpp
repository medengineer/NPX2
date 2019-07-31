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

#include <stdlib.h>
#include <chrono>
#include <thread>

#include "NPX2Components.h"

#define MAXLEN 50

np::NP_ErrorCode errorCode;

NeuropixComponent::NeuropixComponent() : serial_number(-1), part_number(""), version("")
{
}

void Neuropix2API::getInfo()
{
	uint8_t version_major;
	uint8_t version_minor;
	np::getAPIVersion(&version_major, &version_minor);

	version = String(version_major) + "." + String(version_minor);
}

void Basestation::getInfo()
{

	uint8_t version_major;
	uint8_t version_minor;
	uint16_t version_build;

	errorCode = np::getBSBootVersion(slot, &version_major, &version_minor, &version_build);

	boot_version = String(version_major) + "." + String(version_minor);

	if (version_build != NULL)
	{
		boot_version += ".";
		boot_version += String(version_build);
	}
}


void BasestationConnectBoard::getInfo()
{

	uint8_t version_major;
	uint8_t version_minor;
	uint16_t version_build;

	errorCode = np::getBSCBootVersion(basestation->slot, &version_major, &version_minor, &version_build);

	boot_version = String(version_major) + "." + String(version_minor);

	if (version_build != NULL)
	{
		boot_version += ".";
		boot_version += String(version_build);
	}

	errorCode = np::getBSCVersion(basestation->slot, &version_major, &version_minor);

	version = String(version_major) + "." + String(version_minor);

	errorCode = np::readBSCSN(basestation->slot, &serial_number);

	char pn[MAXLEN];
	np::readBSCPN(basestation->slot, pn, MAXLEN);

	part_number = String(pn);

}

void Headstage::getInfo()
{

	uint8_t version_major;
	uint8_t version_minor;

	errorCode = np::getHSVersion(probe->basestation->slot, probe->port, &version_major, &version_minor);

	version = String(version_major) + "." + String(version_minor);

	errorCode = np::readHSSN(probe->basestation->slot, probe->port, &serial_number);

	char pn[MAXLEN];
	errorCode = np::readHSPN(probe->basestation->slot, probe->port, pn, MAXLEN);

	part_number = String(pn);

}


void Flex::getInfo()
{

	uint8_t version_major;
	uint8_t version_minor;

	errorCode = np::getFlexVersion(probe->basestation->slot, probe->port, probe->dock, &version_major, &version_minor);

	version = String(version_major) + "." + String(version_minor);

	char pn[MAXLEN];
	errorCode = np::readFlexPN(probe->basestation->slot, probe->port, probe->dock, pn, MAXLEN);

	part_number = String(pn);

}


void Probe::getInfo()
{

	errorCode = np::readProbeSN(basestation->slot, port, dock, &serial_number);

	char pn[MAXLEN];
	errorCode = np::readProbePN(basestation->slot, port, dock, pn, MAXLEN);

	part_number = String(pn);
	
}

Probe::Probe(Basestation* bs, int port, int dock) : Thread("probe_" + String(port)), 
	basestation(bs), port(port), dock(dock), shank(0)
{

	setStatus(0);
	setSelected(false);

	flex = new Flex(this);
	headstage = new Headstage(this);

	getInfo();

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		channelMap.add(np::electrodebanks_t::None);
	}

	fifoFillPercentage = 0.0f;

}

void Probe::setStatus(int status)
{
	this->status = status;
}

void Probe::setSelected(bool isSelected)
{
	this->isSelected = isSelected;
}

void Probe::setChannels(Array<int> channelStatus)
{

	//Reset all channel connections to None. 
	/*
	for (int channel = 0; channel < channelMap.size(); channel++)
	{
		errorCode = np::selectElectrode(basestation->slot, port, dock, channel, shank, np::electrodebanks_t::None);
	}

	//Use channel (UI) status to set channel connection to proper electrode by bank
	np::electrodebanks_t bank;
	for (int channel = 0; channel < channelMap.size(); channel++)
	{

		//TODO: There should be a check here if certain electrode is being used as a ref or not. 

		if (channelStatus[channel])
		{
			bank = np::electrodebanks_t::BankA;
		}
		else if (channelStatus[channel + 1 * 384])
		{
			bank = np::electrodebanks_t::BankB;
		}
		else if (channelStatus[channel + 2 * 384])
		{
			bank = np::electrodebanks_t::BankC;
		}
		else if (channelStatus[channel + 3 * 384])
		{
			bank = np::electrodebanks_t::BankD;
		}
		else
		{
			bank = np::electrodebanks_t::None;
		}

		channelMap.set(channel, bank);

		errorCode = np::selectElectrode(basestation->slot, port, dock, channel, shank, bank);

	}

	std::cout << "Writing settings for probe on slot: " << basestation->slot << " port: " << port << " dock: " << dock << std::endl;

	bool readCheck = false;
	errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, readCheck);
	if (!errorCode == np::SUCCESS)
		std::cout << "Failed to write channel config " << std::endl;
	else
		std::cout << "Successfully wrote channel config " << std::endl;
	*/

}

void Probe::setReferences(np::channelreference_t ref, np::electrodebanks_t bank)
{
	
	for (int channel = 0; channel < NUM_CHANNELS; channel++)
		errorCode = np::setReference(basestation->slot, port, dock, channel, shank, ref, bank);

	bool readCheck = false;
	errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, readCheck);

	std::cout << "Wrote reference " << int(ref) << ", " << int(bank) << " with error code " << errorCode << std::endl;
}

void Probe::run()
{

	while (!threadShouldExit())
	{

		errorCode = np::readPacket(
			basestation->slot,
			port,
			dock,
			static_cast<np::streamsource_t>(0), 
			&pckinfo[0],
			&data[0],
			samplesToRead,
			&actualRead);

		if (errorCode == np::SUCCESS && actualRead > 0)
		{

			eventCode = pckinfo->Status >> 6; //TODO: Confirm event code is same bit...

			int64 npx_timestamp = pckinfo->Timestamp;

			float samples[NUM_CHANNELS];

			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				samples[i] = 100.0f * float(data[i]) / 8192; //TODO: Confirm scale factor...
			}

			stream->addToBuffer(samples, &npx_timestamp, &eventCode, 1);

			size_t packetsAvailable;
			size_t headroom;

			errorCode = np::getPacketFifoStatus(
				basestation->slot,
				port,
				dock,
				static_cast<np::streamsource_t>(0),
				&packetsAvailable,
				&headroom);

			fifoFillPercentage = float(packetsAvailable) / float(packetsAvailable + headroom);

		}

	}

}

Headstage::Headstage(Probe* probe_) : probe(probe_)
{
	getInfo();
}

Flex::Flex(Probe* probe_) : probe(probe_)
{
	getInfo();
}

BasestationConnectBoard::BasestationConnectBoard(Basestation* bs) : basestation(bs)
{
	getInfo();
}


Basestation::Basestation(int slot_number) : probesInitialized(false)
{

	slot = slot_number;

	errorCode = np::openBS(slot);

	if (errorCode == np::SUCCESS)
	{

		std::cout << "  Opened BS on slot " << slot << std::endl;

		getInfo();
		basestationConnectBoard = new BasestationConnectBoard(this);

		savingDirectory = File();

		for (int port = 1; port <= NUM_PORTS; port++)
		{
			for (int dock = 1; dock <= NUM_DOCKS; dock++)
			{
				errorCode = np::openProbe(slot, port, dock);

				if (errorCode == np::SUCCESS)
				{
					Probe* probe = new Probe(this, port, dock);
					if (probe->serial_number / NPX2_MIN_PROBE_SERIAL > 0)
					{
						probes.add(probe);
						probes[probes.size() - 1]->setStatus(2); 
					}
					else
					{
						delete probe;
					}
				}
			}
		}
		std::cout << "Found " << String(probes.size()) << (probes.size() == 1 ? " probe." : " probes.") << std::endl;
	}

	syncFrequencies.add(1);
	syncFrequencies.add(10);
}

void Basestation::init()
{

	for (int i = 0; i < probes.size(); i++)
	{
		std::cout << "Initializing probe " << String(i + 1) << "/" << String(probes.size()) << "...";

		errorCode = np::init(this->slot, probes[i]->port, probes[i]->dock);
		if (errorCode != np::SUCCESS)
			std::cout << "  FAILED!." << std::endl;
		else
		{
			probes[i]->setStatus(1);
			std::cout << "  Success!" << std::endl;
		}
	}

}

Basestation::~Basestation()
{
	for (int i = 0; i < probes.size(); i++)
	{
		errorCode = np::closePort(slot, probes[i]->port);
	}
	errorCode = np::closeBS(slot);
}

void Basestation::setSyncAsInput()
{

	errorCode = np::setParameter(np::NP_PARAM_SYNCMASTER, slot);
	if (errorCode != np::SUCCESS)
	{
		printf("Failed to set slot %d as sync master!\n");
		return;
	}

	errorCode = np::setParameter(np::NP_PARAM_SYNCSOURCE, np::SIGNALLINE_SMA);
	if (errorCode != np::SUCCESS)
		printf("Failed to set slot %d SMA as sync source!\n");

}

Array<int> Basestation::getSyncFrequencies()
{
	return syncFrequencies;
}

void Basestation::setSyncAsOutput(int freqIndex)
{

	//TODO: Can't find the corresponding calls in NPX2 API
	/*
	np1_error = np::setParameter(np::NP_PARAM_SYNCMASTER, slot);
	if (np1_error != np::SUCCESS)
	{
		printf("Failed to set slot %d as sync master!\n", slot);
		return;
	} 

	np1_error = np::setParameter(np::NP_PARAM_SYNCSOURCE, np::TRIGIN_SYNCCLOCK);
	if (np1_error != np::SUCCESS)
	{
		printf("Failed to set slot %d internal clock as sync source!\n", slot);
		return;
	}

	int freq = syncFrequencies[freqIndex];

	printf("Setting slot %d sync frequency to %d Hz...\n", slot, freq);
	np1_error = setParameter(np::NP_PARAM_SYNCFREQUENCY_HZ, freq);
	if (np1_error != np::SUCCESS)
	{
		printf("Failed to set slot %d sync frequency to %d Hz!\n", slot, freq);
		return;
	}
	*/

}

int Basestation::getProbeCount()
{
	return probes.size();
}

float Basestation::getFillPercentage()
{
	float perc = 0.0;

	//Use the highest fifo percentage of all probes
	for (int i = 0; i < getProbeCount(); i++)
	{
		if (probes[i]->fifoFillPercentage > perc)
			perc = probes[i]->fifoFillPercentage;
	}

	return perc;
}

void Basestation::initializeProbes()
{
	if (!probesInitialized)
	{
		//TODO: Can't find the corresponding calls in NPX2 API
		//np1_error = np::setTriggerInput(slot, np::TRIGIN_SW);

		for (int i = 0; i < probes.size(); i++)
		{
			errorCode = np::setOPMODE(slot, probes[i]->port, probes[i]->dock, np::probe_opmode_t::RECORDING);

			//TODO: Confirm 2.0 probes DO NOT require calibration.
			//probes[i]->calibrate();

			if (errorCode == np::SUCCESS)
			{
				std::cout << "     Probe initialized." << std::endl;
				probes[i]->timestamp = 0;
				probes[i]->eventCode = 0;
				probes[i]->setStatus(1); //READY
			}
			else {
				std::cout << "     Failed with error code " << errorCode << std::endl;
			}

			bool ledEnable = false;
			errorCode = np::setHSLed(slot, probes[i]->port, ledEnable);

		}

		probesInitialized = true;
	}

	errorCode = np::arm(slot);

}

void Basestation::startAcquisition()
{

	for (int i = 0; i < probes.size(); i++)
	{
		std::cout << "Probe " << int(probes[i]->port) << " setting timestamp to 0" << std::endl;
		probes[i]->timestamp = 0;
		//std::cout << "... and clearing buffers" << std::endl;
		probes[i]->stream->clear();
		std::cout << "  Starting thread." << std::endl;
		probes[i]->startThread();
	}

	errorCode = np::setSWTrigger(slot);

}

void Basestation::stopAcquisition()
{
	for (int i = 0; i < probes.size(); i++)
		probes[i]->stopThread(1000);

	errorCode = np::arm(slot);
}

void Basestation::setChannels(int slot, int port, int dock, Array<int> channelMap)
{
	/*
	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				probes[i]->setChannels(channelMap);
				std::cout << "Set electrode-channel connections " << std::endl;
			}
		}
	}
	*/
}

bool Basestation::runBist(int slot, int port, int dock, int bistIndex)
{

	bool returnValue;

	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				switch (bistIndex)
			    {
			    case BIST_SIGNAL:
			    {
			        //np::NP_ErrorCode errorCode = bistSignal(slot, port, &returnValue, probes[i]->stats);
			        CoreServices::sendStatusMessage("Test not valid for Neuropixels 2.0 probes. ");
			        break;
			    }
			    case BIST_NOISE:
			    {
			        if (np::bistNoise(slot, port, dock) == np::SUCCESS)
			            returnValue = true;
			        break;
			    }
			    case BIST_PSB:
			    {
			        if (np::bistPSB(slot, port, dock) == np::SUCCESS)
			            returnValue = true;
			        break;
			    }
			    case BIST_SR:
			    {
			        if (np::bistSR(slot, port, dock) == np::SUCCESS)
			            returnValue = true;
			        break;
			    }
			    case BIST_EEPROM:
			    {
			        if (np::bistEEPROM(slot, port) == np::SUCCESS)
			            returnValue = true;
			        break;
			    }
			    case BIST_I2C:
			    {
			        if (np::bistI2CMM(slot, port, dock) == np::SUCCESS)
			            returnValue = true;
			        break;
			    }
			    case BIST_SERDES:
			    {
			        int errors;
			        np::bistStartPRBS(slot, port);
			        std::this_thread::sleep_for(std::chrono::milliseconds(200));
			        np::bistStopPRBS(slot, port, &errors);

			        if (errors == 0)
			            returnValue = true;
			        break;
			    }
			    case BIST_HB:
			    {
			        if (np::bistHB(slot, port, dock) == np::SUCCESS)
			            returnValue = true;
			        break;
			    } 
			    case BIST_BS:
			    {
			        if (np::bistBS(slot) == np::SUCCESS)
			            returnValue = true;
			        break;
			    } 
			    default :
			        CoreServices::sendStatusMessage("Test not found.");
			    };
			}
		}
	}

	return returnValue;

}

void Basestation::setReferences(int slot, int port, int dock, np::channelreference_t ref, np::electrodebanks_t bank)
{
	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				probes[i]->setReferences(ref, bank);
				std::cout << "Set all references to " << ref << ":" << bank << std::endl;
			}
		}
	}

}

void Basestation::setSavingDirectory(File directory)
{
	savingDirectory = directory;
}

File Basestation::getSavingDirectory()
{
	return savingDirectory;
}