//
// C++ Implementation: can_i7565
//
// Description: 
// i7565 CAN to USB/serial converter driver
// Implements ICANInterface
//
//
// Author: Andrzej Godziuk <gdr@go2.pl>, (C) Marcontrel 2008
//
// Depends on libSerial [http://libserial.sourceforge.net/]
//
//
#include "can_i7565.h"

#include <exception>
#include <iostream>
#include <assert.h>

#include <SerialPort.h>

CAN_i7565::CAN_i7565()
{
	this->port = "/dev/ttyUSB0";	// TODO: get from prefs

	try
	{
		this->serial = new SerialPort(port);
		serial->Open(	SerialPort::BAUD_921600, 
						SerialPort::CHAR_SIZE_8,
						SerialPort::PARITY_NONE,
						SerialPort::STOP_BITS_1,
						SerialPort::FLOW_CONTROL_NONE
						);
	}
	catch(SerialPort::AlreadyOpen &e)
	{
		cout << "Serial port already opened" << endl;
		exit(0);
	}
	catch(SerialPort::OpenFailed &e)
	{
		cout << "Couldn't open serial port because:" << endl << e.what() << endl << "----- exiting -----" << endl;
		exit(0);
	}
	catch(invalid_argument &e)
	{
		cout << "Invalid argument while opening serial port" << endl << e.what() << endl << "----- exiting -----" << endl;
		exit(0);
	}

	cout << "Serial port opened successfully" << endl;

	this->Reset();
}


CAN_i7565::~CAN_i7565()
{
	this->serial->Close();
	delete this->serial;
}

int CAN_i7565::SendCommand(string &cmd)
{
	serial->Write(cmd);
	serial->WriteByte(13);

	// TODO: remove it
	cout << "Sending CAN msg: " << cmd << endl;

	try
	{
		// 10 ms timeout, <CR> is the line terminator
    	string reply = serial->ReadLine(10, 13); 

		// TODO: remove it
		cout << "CAN reply: " << reply << endl;

		if(reply[0] == '?')
		{
			return reply[1] - '0';
		}
		else
		{
			lineBuffer = reply;
			return 0;
		}
	}
	catch(SerialPort::ReadTimeout &e)
	{
		return 0;
	}

}

void CAN_i7565::Reset()
{
	static string ra("RA");

	SendCommand(ra);
}

const char CAN_i7565::hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

string CAN_i7565::VectorToHex(vector<char> &vec)
{
	string str;
	vector<char>::iterator i;

	for(i = vec.begin(); i != vec.end(); i++)
	{
		str += hex[(*i) >> 4];
		str += hex[(*i) & 0x0f];
	}


	return str;
}

int CAN_i7565::SendStandardFrame(unsigned int toId, vector<char> &data)
{
	string req;

	req = "t";

	// 11-bits identifier
	assert(toId <= 0x7ff);
	req += hex[(toId >> 8) & 0x07];
	req += hex[(toId >> 4) & 0x0f];
	req += hex[toId & 0x0f];

	// Data length
	int size = data.size();
	assert(size <= 8);
	req += hex[size];

	// The data itself
	req += VectorToHex(data);

    // Off we go!
	return SendCommand(req);
}

int CAN_i7565::SendStandardRemoteFrame(unsigned int toId, unsigned int dlc)
{
	string req;

	req = "T";

	// 11-bits identifier
	assert(toId <= 0x7ff);
	req += hex[(toId >> 8) & 0x07];
	req += hex[(toId >> 4) & 0x0f];
	req += hex[toId & 0x0f];

	// Data length
	assert(dlc <= 8);
	req += hex[dlc];

    // Off we go!
	return SendCommand(req);
}
int CAN_i7565::SendExtendedFrame(unsigned int toId, vector<char> &data) 
{
	string req;

	req = "e";

	// 29-bits identifier
	assert(toId <= 0x1fffffff);
	req += hex[(toId >> 28) & 0x01];
	req += hex[(toId >> 24) & 0x0f];
	req += hex[(toId >> 20) & 0x0f];
	req += hex[(toId >> 16) & 0x0f];
	req += hex[(toId >> 12) & 0x0f];
	req += hex[(toId >> 8) & 0x07];
	req += hex[(toId >> 4) & 0x0f];
	req += hex[toId & 0x0f];

	// Data length
	int size = data.size();
	assert(size <= 8);
	req += hex[size];

	// The data itself
	req += VectorToHex(data);

    // Off we go!
	return SendCommand(req);
}

int CAN_i7565::SendExtendedRemoteFrame(unsigned int toId, unsigned int dlc) 
{
	string req;

	req = "e";

	// 29-bits identifier
	assert(toId <= 0x1fffffff);
	req += hex[(toId >> 28) & 0x01];
	req += hex[(toId >> 24) & 0x0f];
	req += hex[(toId >> 20) & 0x0f];
	req += hex[(toId >> 16) & 0x0f];
	req += hex[(toId >> 12) & 0x0f];
	req += hex[(toId >> 8) & 0x07];
	req += hex[(toId >> 4) & 0x0f];
	req += hex[toId & 0x0f];

	// Data length
	assert(dlc <= 8);
	req += hex[dlc];

    // Off we go!
	return SendCommand(req);
}

/* Translates errorId returned by other methods */
string &CAN_i7565::GetErrorString(int errorId)
{
	static string errors[] = 
	{
		"Errno out of range",
		"Invalid header",
		"Invalid length",
		"Invalid checksum",
		"wtf!?", // reserved, should not occur
		"Timeout",
	};
	
	if(errorId < 1 || errorId > 5)
	{
		return errors[0];
	}
	else
	{
		return errors[errorId];
	}
}

void CAN_i7565::SetCANBaudRate(CANBaudRates b)
{
	string req;

	req = "P1";
	req += b;

	SendCommand(req);
}

/* Registers incoming data listener (observer), *FrameListener are interfaces */
void CAN_i7565::AddStandardFrameListener(IStandardFrameListener *l)
{
}

void CAN_i7565::AddExtendedFrameListener(IExtendedFrameListener *l)
{
	extendedListeners.push_back(l);
}

void CAN_i7565::RemoveStandardFrameListener(IStandardFrameListener *l)
{
}

void CAN_i7565::RemoveExtendedFrameListener(IExtendedFrameListener *l)
{
	extendedListeners.remove(l);
}

void CAN_i7565::Tick()
{
	string line;
	int maxFrames = 10;

	line = lineBuffer;
	lineBuffer = "";

	if(line.size() == 0)
	{
		try
		{
			line = serial->ReadLine(10, 13);

		}
		catch(SerialPort::ReadTimeout &e)
		{
			return;
		}
	}

	do
	{
		// Process received frame
		switch(line[0])
		{
			/* std frame */
			case 't':
			// TODO: implement standard frame parser
				break;
			/* std remote frame */
			case 'T':
			// TODO: implement standard remote frame parser
				break;
			/* ext frame */
			case 'e':
				ProcessExtendedFrame(line);
				break;
			/* ext remote frame */
			case 'E':
				ProcessExtendedFrame(line);
				break;
			default:
				cout << "Unsupported frame recv'd: " << line << endl;
				break;
		}

		// Read next frame
		try
		{
			line = serial->ReadLine(3, 13);
		}
		catch(SerialPort::ReadTimeout &e)
		{
			break;
		}
	}
	while(line.size() && (--maxFrames));
}

void CAN_i7565::NotifyExtendedFrameListeners(int fromId, vector<char> &data)
{
	list<IExtendedFrameListener *>::iterator i;

	for(i = extendedListeners.begin(); i != extendedListeners.end(); i++)
	{
		(*i)->OnExtendedFrameReceived(fromId, data);
	}
}


void CAN_i7565::ProcessExtendedFrame(string &line)
{
	assert(line[0] == 'e' || line[0] == 'E');

	// determine sender id
	string from = line.substr(1, 8);
	unsigned int fromId = (unsigned int)strtol(from.c_str(), NULL, 16);
	assert(fromId <= 0x1fffffff);

	// determine length
	unsigned int len = line[9] - '0';
	assert(len <= 8);

	// get data
	string byte;
	vector<char> data;
	for(unsigned int i = 0; i < len; i++)
	{
		int idx = 10 + 2*i;
		byte = line.substr(idx, 2);
		data.push_back((char)strtol(byte.c_str(), NULL, 16));
	}

	NotifyExtendedFrameListeners(fromId, data);
}

