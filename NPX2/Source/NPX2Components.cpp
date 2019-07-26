/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2018 Allen Institute for Brain Science and Open Ephys

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

#include "NPX2Components.h"

#define MAXLEN 50

np::NP_ErrorCode errorCode;

NeuropixComponent::NeuropixComponent() : serial_number(-1), part_number(""), version("")
{
}

void NeuropixAPI::getInfo()
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

	for (int i = 0; i < 384; i++)
	{
		apGains.add(3); // default = 500
		lfpGains.add(2); // default = 250
		channelMap.add(np::electrodebanks_t::None);
	}

	gains.add(50.0f);
	gains.add(125.0f);
	gains.add(250.0f);
	gains.add(500.0f);
	gains.add(1000.0f);
	gains.add(1500.0f);
	gains.add(2000.0f);
	gains.add(3000.0f);

	fifoFillPercentage = 0.0f;

}

void Probe::setStatus(int status_)
{
	status = status_;
}

void Probe::setSelected(bool isSelected_)
{
	isSelected = isSelected_;
}

void Probe::calibrate()
{
	File baseDirectory = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
	File calibrationDirectory = baseDirectory.getChildFile("CalibrationInfo");
	File probeDirectory = calibrationDirectory.getChildFile(String(serial_number));

	std::cout << probeDirectory.getFullPathName() << std::endl;

	if (probeDirectory.exists())
	{
		String adcFile = probeDirectory.getChildFile(String(serial_number) + "_ADCCalibration.csv").getFullPathName();
		String gainFile = probeDirectory.getChildFile(String(serial_number) + "_gainCalValues.csv").getFullPathName();
		std::cout << adcFile << std::endl;
		
		errorCode = np::setADCCalibration(basestation->slot, port, adcFile.toRawUTF8());

		if (errorCode == 0)
			std::cout << "Successful ADC calibration." << std::endl;
		else
			std::cout << "Unsuccessful ADC calibration, failed with error code: " << errorCode << std::endl;

		std::cout << gainFile << std::endl;
		
		errorCode = np::setGainCalibration(basestation->slot, port, dock, gainFile.toRawUTF8());

		if (errorCode == 0)
			std::cout << "Successful gain calibration." << std::endl;
		else
			std::cout << "Unsuccessful gain calibration, failed with error code: " << errorCode << std::endl;

		errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, false);
	}
	else {
		// show popup notification window
		String message = "Missing calibration files for probe serial number " + String(serial_number) + ". ADC and Gain calibration files must be located in 'CalibrationInfo\\<serial_number>' folder in the directory where the Open Ephys GUI was launched. The GUI will proceed without calibration.";
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Calibration files missing", message, "OK");
	}
}

void Probe::setChannels(Array<int> channelStatus)
{

	//Reset all channel connections to None. 
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

}

void Probe::setApFilterState(bool disableHighPass)
{
	/*
	for (int channel = 0; channel < 384; channel++)
		errorCode = np::setAPCornerFrequency(basestation->slot, port, dock, channel, disableHighPass);

	errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, false);

	std::cout << "Wrote filter " << int(disableHighPass) << " with error code " << errorCode << std::endl;
	*/
	CoreServices::sendStatusMessage("Set AP Bandwidth not valid for NPX2 probes!");
}

void Probe::setGains(unsigned char apGain, unsigned char lfpGain)
{
	/*
	for (int channel = 0; channel < 384; channel++)
	{
		errorCode = np2::setGain(basestation->slot, port, channel, dock, apGain, lfpGain);
		apGains.set(channel, int(apGain));
		lfpGains.set(channel, int(lfpGain));
	}
		
	errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, false);

	std::cout << "Wrote gain " << int(apGain) << ", " << int(lfpGain) << " with error code " << errorCode << std::endl;
	*/
	CoreServices::sendStatusMessage("Set AP Gain not valid for NPX2 probes!");
}


void Probe::setReferences(np::channelreference_t refId, np::electrodebanks_t refBank)
{
	for (int channel = 0; channel < 384; channel++)
		errorCode = np::setReference(basestation->slot, port, dock, channel, shank, refId, refBank);

	bool readCheck = false;
	errorCode = np::writeProbeConfiguration(basestation->slot, port, dock, readCheck);

	std::cout << "Wrote reference " << int(refId) << ", " << int(refBank) << " with error code " << errorCode << std::endl;
}

void Probe::run()
{

	while (!threadShouldExit())
	{
	
		size_t samplesToRead = SAMPLECOUNT;
		size_t actualRead;

		errorCode = np::readPacket(
			basestation->slot,
			port,
			dock,
			static_cast<np::streamsource_t>(0),  
			pckinfo,
			data,
			samplesToRead,
			&actualRead);

		if (errorCode == np::SUCCESS && actualRead > 0)
		{

			printf("Timestamp: %d, Status: %d, Size: %d", pckinfo->Timestamp, pckinfo->Status, pckinfo->payloadlength);
			/*
			for (int packetNum = 0; packetNum < count; packetNum++)
			{
				for (int i = 0; i < 12; i++)
				{
					eventCode = packet[packetNum].Status[i] >> 6; // AUX_IO<0:13>

					uint32_t npx_timestamp = packet[packetNum].timestamp[i];

					for (int j = 0; j < 384; j++)
					{
						apSamples[j] = float(packet[packetNum].apData[i][j]) * 1.2f / 1024.0f * 1000000.0f / gains[apGains[j]]; // convert to microvolts

						if (i == 0)
							lfpSamples[j] = float(packet[packetNum].lfpData[j]) * 1.2f / 1024.0f * 1000000.0f / gains[lfpGains[j]]; // convert to microvolts
					}

					ap_timestamp += 1;

					apBuffer->addToBuffer(apSamples, &ap_timestamp, &eventCode, 1);

					if (ap_timestamp % 30000 == 0)
					{
						size_t packetsAvailable;
						size_t headroom;

						np2::getElectrodeDataFifoState(
							basestation->slot,
							port,
							dock,
							&packetsAvailable,
							&headroom);

						//std::cout << "Basestation " << int(basestation->slot) << ", probe " << int(port) << ", packets: " << packetsAvailable << std::endl;

						fifoFillPercentage = float(packetsAvailable) / float(packetsAvailable + headroom);
					}


				}
				lfp_timestamp += 1;

				lfpBuffer->addToBuffer(lfpSamples, &lfp_timestamp, &eventCode, 1);

			}
			*/

		}
		else if (errorCode != np::SUCCESS)
		{
			std::cout << "Error code: " << errorCode << "for Basestation " << int(basestation->slot) << ", probe " << int(port) << std::endl;
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
					probes.add(new Probe(this, port, dock));
					probes[probes.size() - 1]->setStatus(2); 
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
			setGains(this->slot, probes[i]->port, probes[i]->dock, 3, 2); // set defaults
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

	//TODO: Can't find the corresponding calls in NPX2 API

    /*
	np1_error = np::setParameter(np::NP_PARAM_SYNCMASTER, slot);
	if (np1_error != np::SUCCESS)
	{
		printf("Failed to set slot %d as sync master!\n");
		return;
	}

	np1_error = np::setParameter(np::NP_PARAM_SYNCSOURCE, np::TRIGIN_SMA);
	if (np1_error != np::SUCCESS)
		printf("Failed to set slot %d SMA as sync source!\n");
	*/

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

	for (int i = 0; i < getProbeCount(); i++)
	{
		//TODO: Verify this makes sense...

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
			bool ledEnable = false;
			errorCode = np::setHSLed(slot, probes[i]->port, ledEnable);

			probes[i]->calibrate();

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
}

void Basestation::setApFilterState(int slot, int port, int dock, bool disableHighPass)
{
	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				probes[i]->setApFilterState(disableHighPass);
				std::cout << "Set all filters to " << int(disableHighPass) << std::endl;
			}
		}
	}
	
}

void Basestation::setGains(int slot, int port, int dock, unsigned char apGain, unsigned char lfpGain)
{
	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				probes[i]->setGains(apGain, lfpGain);
				std::cout << "Set all gains to " << int(apGain) << ":" << int(lfpGain) << std::endl;
			}
		}
	}
	
}

void Basestation::setReferences(int slot, int port, int dock, np::channelreference_t refId, np::electrodebanks_t refBank)
{
	if (slot == this->slot)
	{
		for (int i = 0; i < probes.size(); i++)
		{
			if (probes[i]->port == port && probes[i]->dock == dock)
			{
				probes[i]->setReferences(refId, refBank);
				std::cout << "Set all references to " << refId << ":" << int(refBank) << std::endl;
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