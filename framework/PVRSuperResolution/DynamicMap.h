/*!
\brief Dictionary struct to allow client applications communicate with the techniques in the library
\file PVRSuperResolution/DynamicMap.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include <map>
#include <string>
#include <any>

namespace pvr {

/// <summary>Dictionary implementation which can hold different data types which can be queried through a string or name.</summary>
class DynamicMap
{
	std::map<std::string, std::any> _map;

public:
	template<typename T>
	T getValue(const std::string& key, T defaultValue) const
	{
		auto it = _map.find(key);
		if (it == _map.end())
		{
			return defaultValue;
		}

		return std::any_cast<T>(it->second);
	};

	template<typename T>
	void setValue(const std::string& key, T value)
	{
		_map[key] = value;
	};
};

} // namespace pvr
