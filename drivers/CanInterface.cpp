/*
 * CanInterface.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "CanInterface.h"

CanInterface::CanInterface(CanInterfaceProvider *provider)
  : _provider(provider)
{
}

CanInterface::~CanInterface() {
}

CanInterfaceProvider* CanInterface::getProvider() {
	return _provider;
}

void CanInterface::open() {
}

void CanInterface::close() {
}
