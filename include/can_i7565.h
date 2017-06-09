//
// C++ Interface: can_i7565
//
// Description: 
// i7565 CAN to USB/serial converter driver
// Implements ICANInterface
//
//
// Author: Andrzej Godziuk <gdr@go2.pl>, (C) Marcontrel 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CAN_I7565_H
#define CAN_I7565_H

#include <icaninterface.h>
#include <list>
#include <SerialPort.h>	// libSerial
#include <vector>

#include <iostream>
#include <fstream>

/**
	@author GDR!
*/
class CAN_i7565 : public ICANInterface {
private:
	ofstream f;
	SerialPort *serial;
	string port;
	int SendCommand(string &cmd);
	string VectorToHex(vector<unsigned char> &vec);
	static const char hex[16];
	vector<string> lineBuffer; // we put read but not processed strings here
	list<IExtendedFrameListener *> extendedListeners;
	void NotifyExtendedFrameListeners(unsigned int fromId, vector<unsigned char> &data);
	void ProcessExtendedFrame(string &line);
public:
	void Reset();

	/* Send standard CAN data frame, returns error code or 0 if OK */
	virtual int SendStandardFrame(unsigned int toId, vector<unsigned char> &data); 
	virtual int SendStandardRemoteFrame(unsigned int toId, unsigned int dlc); 
	virtual int SendExtendedFrame(unsigned int toId, vector<unsigned char> &data); 
	virtual int SendExtendedRemoteFrame(unsigned int toId, unsigned int dlc); 

	/* Translates errorId returned by other methods */
	virtual string &GetErrorString(int errorId);

	/* Registers incoming data listener (observer), *FrameListener are interfaces */
	virtual void AddStandardFrameListener(IStandardFrameListener *l);
	virtual void AddExtendedFrameListener(IExtendedFrameListener *l);
	virtual void RemoveStandardFrameListener(IStandardFrameListener *l);
	virtual void RemoveExtendedFrameListener(IExtendedFrameListener *l);

    CAN_i7565();
    ~CAN_i7565();

	enum CANBaudRates
	{
		CB_10k = 0,
		CB_20k = 1,
		CB_50k = 2,
		CB_100k = 3,
		CB_125k = 4,
		CB_250k = 5,
		CB_500k = 6,
		CB_800k = 7,
		CB_1000k = 8,
	};

	/* Setting baud rate writes it into EEPROM and triggers converter reboot */
	void SetCANBaudRate(CANBaudRates b);

	/* Process incoming messages, should be called often */
	virtual void Tick();
};

#endif
