/*!
\brief Represents formatted user data.
\file PVRAssets/model/FormattedUserData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/types/Types.h"
#include "PVRCore/math/MathUtils.h"
#include <map>

namespace pvr {
namespace assets {
	class CustomData
	{
	public:
		enum class Type
		{
			NONE = 0,
			NUMBER,
			INT,
			BOOL,
			STRING,
			ARRAY,
			BINARY,
			OBJECT
		} ;

		typedef std::vector<CustomData> Array;
		typedef std::map<std::string, CustomData> Object;

		CustomData() : type(Type::NONE) {}

		explicit CustomData(bool b) : type(Type::BOOL) { booleanValue = b; }
		explicit CustomData(int i) : type(Type::INT) { intValue = i; }
		explicit CustomData(double n) : type(Type::NUMBER) { numberValue = n; }
		explicit CustomData(const std::string& s) : type(Type::STRING) { stringValue = s; }
		explicit CustomData(const unsigned char* p, size_t n) : type(Type::BINARY)
		{
			binaryValue.resize(n);
			memcpy(binaryValue.data(), p, n);
		}
		explicit CustomData(const Array& a) : type(Type::ARRAY) { arrayValue = Array(a); }
		explicit CustomData(const Object& o) : type(Type::OBJECT) { objectValue = Object(o); }

		Type GetType() const { return type; }

		bool IsBool() const { return (type == Type::BOOL); }
		bool IsInt() const { return (type == Type::INT); }
		bool IsNumber() const { return (type == Type::NUMBER); }
		bool IsString() const { return (type == Type::STRING); }
		bool IsBinary() const { return (type == Type::BINARY); }
		bool IsArray() const { return (type == Type::ARRAY); }
		bool IsObject() const { return (type == Type::OBJECT); }

		// Accessor
		template<typename T>
		const T& Get() const;
		template<typename T>
		T& Get();

		bool GetBool() const 
		{ 
			if (IsInt()) return intValue != 0;
			return booleanValue;
		}

		int GetInt() const
		{
			if (IsNumber()) return static_cast<int>(glm::round(numberValue));
			return intValue;
		}
		double GetDouble() const
		{
			if (IsInt()) return static_cast<double>(intValue);
			return numberValue;
		}

		inline const std::string& GetString() const
		{
			return stringValue;
		}
		inline const Array& GetArray() const
		{
			return arrayValue;
		}

		// Lookup value from an array
		const CustomData& Get(std::size_t idx) const
		{
			static CustomData null_value;
			assert(IsArray());
			assert(idx >= 0);
			return (idx < arrayValue.size()) ? arrayValue[idx] : null_value;
		}

		// Lookup value from a key-value pair
		const CustomData& Get(const std::string& key) const
		{
			static CustomData null_value;
			assert(IsObject());
			Object::const_iterator it = objectValue.find(key);
			return (it != objectValue.end()) ? it->second : null_value;
		}
		bool GetFromPath(const std::string& path, CustomData& out) const
		{ 
			std::size_t p = path.find_first_of('/');
			if (p == std::string::npos) // No separator found, try to fetch the attribute for the key
			{
				Object::const_iterator it = objectValue.find(path);
				if (it != objectValue.end())
					out = it->second;
				else
					return false;
				
			}
			else // Keep going through the path
			{
				const std::string currentName = path.substr(0, p);
				Object::const_iterator it = objectValue.find(currentName);
				if (it != objectValue.end())
				{
					const std::string nextName = path.substr(p + 1);
					return it->second.GetFromPath(nextName, out);
				}
				else
				{
					return false;
				}
			}

			return true;
		}

		std::size_t ArrayLen() const
		{
			if (!IsArray()) return 0;
			return arrayValue.size();
		}

		// Valid only for object type.
		bool Has(const std::string& key) const
		{
			if (!IsObject()) return false;
			Object::const_iterator it = objectValue.find(key);
			return (it != objectValue.end()) ? true : false;
		}

		// List keys
		std::vector<std::string> Keys() const
		{
			std::vector<std::string> keys;
			if (!IsObject()) return keys; // empty

			for (Object::const_iterator it = objectValue.begin(); it != objectValue.end(); ++it) { keys.push_back(it->first); }

			return keys;
		}

		std::size_t Size() const { return (IsArray() ? ArrayLen() : Keys().size()); }

	protected:
		Type type = Type::NONE;

		int intValue = -1;
		double numberValue = 0.0;
		std::string stringValue = std::string();
		std::vector<uint8_t> binaryValue;
		Array arrayValue;
		Object objectValue;
		bool booleanValue = false;
		uint8_t pad[3] = {0,0,0};

		int pad0 = 0;
	};
} // namespace assets
} // namespace pvr
