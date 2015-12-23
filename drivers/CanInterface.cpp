/*
 * CanInterface.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "CanInterface.h"

CanInterface::CanInterface(CanInterfaceProvider *provider)
  : _id(-1), _provider(provider)
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

uint16_t CanInterface::getId()
{
    return _id;
}

void CanInterface::setId(uint16_t id)
{
    _id = id;
}
