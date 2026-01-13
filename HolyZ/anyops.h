
#ifndef ANYOPS_H
#define ANYOPS_H

#include "builtin.h"
#include <boost/any.hpp>

using namespace boost;
using namespace std;

int LogWarning(const string& warningText);
int any_type(const boost::any& val);

// Gets if any is NullType
bool any_null(const boost::any& val)
{
	return val.empty();
}

// Will convert type 'any' val to a bool
bool AnyAsBool(const boost::any& val)
{
	if (any_null(val))
		return false;
	try // Try converting to bool
	{
		return any_cast<bool>(val);
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to string
		{
			return any_cast<string>(val) == "true";
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to float
			{
				return any_cast<float>(val) == 1.0f;
			}
			catch (boost::bad_any_cast) // Try converting to int
			{
				try
				{
					return any_cast<int>(val) == 1;
				}
				catch (boost::bad_any_cast) // Does not convert, return
				{
					LogWarning("invalid conversion from " + to_string(any_type(val)) + " to type \'bool\'");
					return false;
				}
			}
		}
	}
}

// Will convert type 'any' val to a string
string AnyAsString(const boost::any& val)
{
	if (any_null(val))
		return "";
	try // Try converting to string
	{
		return any_cast<string>(val);
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to int
		{
			return to_string(any_cast<int>(val));
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to float
			{
				return to_string(any_cast<float>(val));
			}
			catch (boost::bad_any_cast) // Try converting to bool
			{
				try
				{
					string i = "false";
					if (any_cast<bool>(val) == true) i = "true";
					return i;
				}
				catch (boost::bad_any_cast) // Does not convert, return
				{
					LogWarning("invalid conversion from " + to_string(any_type(val)) + " to type \'string\'");
					return "";
				}
			}
		}
	}
}

// Will convert type 'any' val to a float
float AnyAsFloat(const boost::any& val)
{
	if (any_null(val))
		return 0.0f;
	try // Try converting to float
	{
		return any_cast<float>(val);
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to int
		{
			return (float)any_cast<int>(val);
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to string, then converting it to float
			{
				return stof(any_cast<string>(val));
			}
			catch (boost::bad_any_cast) // Try converting to bool
			{
				try
				{
					float i = 0.0f;
					if (any_cast<bool>(val) == true) i = 1.0f;
					return i;
				}
				catch (boost::bad_any_cast e) // Does not convert, return
				{
					LogWarning("invalid conversion from " + to_string(any_type(val)) + " to type \'float\'");
					return 0;
				}
			}
		}
	}
}

// Will convert type 'any' val to an integer
int AnyAsInt(const boost::any& val)
{
	if (any_null(val))
		return 0;
	try // Try converting to int
	{
		return any_cast<int>(val);
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to float
		{
			return (int)any_cast<float>(val);
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to string, then converting it to int
			{
				return stoi(any_cast<string>(val));
			}
			catch (boost::bad_any_cast) // Try converting to bool
			{
				try
				{
					int i = 0;
					if (any_cast<bool>(val) == true) i = 1;
					return i;
				}
				catch (boost::bad_any_cast) // Does not convert, return
				{
					LogWarning("invalid conversion from " + to_string(any_type(val)) + " to type \'int\'");
					return 0;
				}
			}
		}
	}
}

// Will get type 'any' val to a Vec2
Vec2 AnyAsVec2(const boost::any& val)
{
	if (any_null(val))
		return Vec2(0, 0);
	try // Try converting to Vec2
	{
		return any_cast<Vec2>(val);
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to float then Vec2
		{
			return Vec2(any_cast<float>(val), any_cast<float>(val));
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to int then Vec2
			{
				return Vec2(any_cast<int>(val), any_cast<int>(val));
			}
			catch (boost::bad_any_cast) // Does not convert, return
			{
				LogWarning("invalid conversion from " + to_string(any_type(val)) + " to type \'Vec2\'");
				return Vec2(0, 0);
			}
		}
	}
}

// Will get type 'any' val to a ClassInstance
ClassInstance AnyAsClassInstance(const boost::any& val)
{
	if (any_null(val))
		return ClassInstance();
	try // Try converting to ClassInstance
	{
		return any_cast<ClassInstance>(val);
	}
	catch (boost::bad_any_cast)
	{
		LogWarning("invalid conversion to type 'ClassInstance'");
		return ClassInstance();
	}
}

// Gets type of 'any' val
// 0 -> int;  1 -> float;  2 -> bool;  3 -> string;  4 -> Sprite; 5 -> Vec2; 6 -> Text; 7 -> ClassInstance;
int any_type(const boost::any& val)
{
	try // Try converting to int
	{
		int i = any_cast<int>(val);
		return 0;
	}
	catch (boost::bad_any_cast)
	{
		try // Try converting to float
		{
			float f = any_cast<float>(val);
			return 1;
		}
		catch (boost::bad_any_cast)
		{
			try // Try converting to bool
			{
				bool b = any_cast<bool>(val);
				return 2;
			}
			catch (boost::bad_any_cast) // Try converting to string
			{
				try
				{
					string s = any_cast<string>(val);
					return 3;
				}
				catch (boost::bad_any_cast) // Try converting to sprite
				{
					try
					{
						Sprite s = any_cast<Sprite>(val);
						return 4;
					}
					catch (boost::bad_any_cast) // Try converting to Vec2
					{
						try
						{
							Vec2 v = any_cast<Vec2>(val);
							return 5;
						}
						catch (boost::bad_any_cast) // Try converting to Text
						{
							try
							{
								Text t = any_cast<Text>(val);
								return 6;
							}
							catch (boost::bad_any_cast) // Does not convert, return
								try
								{
									ClassInstance ci = any_cast<ClassInstance>(val);
									return 7;
								}
								catch (boost::bad_any_cast) // Does not convert, return
								{
#endif

// Gets type of 'any' val as string for runtime type checking
string any_type_name(const boost::any& val)
{
	int typeNum = any_type(val);
	switch (typeNum) {
		case 0: return "int";
		case 1: return "float";
		case 2: return "bool";
		case 3: return "string";
		case 4: return "Sprite";
		case 5: return "Vec2";
		case 6: return "Text";
		case 7: return "object";
		case 8: 
			try {
				any_cast<ResultValue>(val);
				return "Result";
			} catch (...) {}
			break;
		case 9:
			try {
				any_cast<OptionValue>(val);
				return "Option";
			} catch (...) {}
			break;
		default: return "null";
	}
	return "null";
}
