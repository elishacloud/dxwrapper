/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3d8to9.hpp"
#include <assert.h>

AddressLookupTable::AddressLookupTable(Direct3DDevice8 *Device) :
	Device(Device)
{
	// Do nothing
}
AddressLookupTable::~AddressLookupTable()
{
	for (UINT i = 0; i < 8; i++)
	{
		while (AddressCache[i].size())
		{
			auto it = AddressCache[i].begin();

			it->second->DeleteMe();

			it = AddressCache[i].erase(it);
		}
	}
}
