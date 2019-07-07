#ifndef _EBSDEVICE_H_
#define _EBSDEVICE_H_

#include <iostream>

#include "device.hpp"
#include "context.hpp"

/*
*
* class EBSDevice
* An abstract class
*
*/
class EBSDevice : public Device
{

private:
	bool registerEBSDevice();

	AccessType aType;
public:
	EBSDevice()
	{
		setDeviceType(EBSDEVICE);
		devSignum = SIGRTMIN + 4;
		aType = STORE;
		std::cout << "EBSDevice()" << std::endl;
	}
	EBSDevice(AccessType type)
	{
		aType = type;
	}
	~EBSDevice()
	{
		std::cout << "~EBSDevice()" << std::endl;
	}

	//Pure virtual function to be imeplemented by tool
	int registerThisDevice(struct perf_event_attr *attr);
	void unRegisterThisDevice();
	void deviceOnEvent();
};


#endif
