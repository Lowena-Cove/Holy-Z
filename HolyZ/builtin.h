#ifndef BUILTIN_H
#define BUILTIN_H

#if defined(__unix__)
#define UNIX true
#define WINDOWS false
#elif defined(_MSC_VER)
#define UNIX false
#define WINDOWS true
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <boost/any.hpp>
#ifdef HOLYZ_GRAPHICS_ENABLED
#include <SDL.h>
#endif
#include <ctime>
#include <math.h>
#include <sys/stat.h>
#include <cstdlib> // for console command printing

#include "strops.h"
#ifdef HOLYZ_GRAPHICS_ENABLED
#include "graphics.h"
#endif
#include "anyops.h"
#if WINDOWS
#include "color.hpp"
#endif

//#define DEVELOPER_MESSAGES false

using namespace std;
using namespace boost;

// ============================================================
// Holy C / Zen-C Style Type Aliases
// ============================================================
// Inspired by Holy C's type system from TempleOS
typedef void U0;           // Zero-size type (true void)
typedef int8_t I8;         // 8-bit signed integer
typedef uint8_t U8;        // 8-bit unsigned integer
typedef int16_t I16;       // 16-bit signed integer
typedef uint16_t U16;      // 16-bit unsigned integer
typedef int32_t I32;       // 32-bit signed integer
typedef uint32_t U32;      // 32-bit unsigned integer
typedef int64_t I64;       // 64-bit signed integer
typedef uint64_t U64;      // 64-bit unsigned integer
typedef double F64;        // 64-bit floating point

#ifdef HOLYZ_GRAPHICS_ENABLED
vector<string> types = { "int", "float", "string", "bool", "void", "null", "Sprite", "Vec2", "Text", "Result", "Option", 
                         "I8", "U8", "I16", "U16", "I32", "U32", "I64", "U64", "F64", "U0" };
#else
vector<string> types = { "int", "float", "string", "bool", "void", "null", "Result", "Option",
                         "I8", "U8", "I16", "U16", "I32", "U32", "I64", "U64", "F64", "U0" };
#endif
// Forward declarations for Holy C classes
class ClassDefinition;
class ClassInstance;
class ClassMethod;
class ClassAttribute;
class ResultValue;
class OptionValue;

// Global class definitions map
extern unordered_map<string, ClassDefinition> globalClassDefinitions;

// Class definitions for Holy C support
class ClassAttribute {
public:
	string name;
	boost::any value;
	bool isStatic;
	
	ClassAttribute() : isStatic(false) {}
	ClassAttribute(const string& n, const boost::any& v, bool stat = false) 
		: name(n), value(v), isStatic(stat) {}
};

class ClassMethod {
public:
	string name;
	vector<string> parameters;
	vector<vector<string>> body;
	bool isStatic;
	
	ClassMethod() : isStatic(false) {}
	ClassMethod(const string& n, const vector<string>& params, bool stat = false)
		: name(n), parameters(params), isStatic(stat) {}
	ClassMethod(const string& n, const vector<string>& params, const vector<vector<string>>& b, bool stat = false)
		: name(n), parameters(params), body(b), isStatic(stat) {}
};

class ClassDefinition {
public:
	string className;
	string superClassName;
	vector<ClassAttribute> attributes;
	vector<ClassMethod> methods;
	unordered_map<string, boost::any> staticAttributes;
	
	ClassDefinition() {}
	ClassDefinition(const string& name) : className(name) {}
};

class ClassInstance {
public:
	string className;
	unordered_map<string, boost::any> instanceAttributes;
	
	ClassInstance() {}
	ClassInstance(const string& name) : className(name) {}
};

// Pointer support for direct memory manipulation
class Pointer {
public:
	void* address;
	string pointedType;  // Type that the pointer points to
	
	Pointer() : address(nullptr), pointedType("void") {}
	Pointer(void* addr, const string& type = "void") 
		: address(addr), pointedType(type) {}
	
	// Dereference operator simulation
	boost::any dereference() const {
		if (address == nullptr) return boost::any();
		// Type-specific dereferencing would be handled at runtime
		return boost::any();
	}
};

// Memory heap for allocating values (simulates dynamic allocation)
class MemoryHeap {
private:
	unordered_map<long long, boost::any> heap;
	long long nextAddress = 1000;
	
public:
	Pointer allocate(const boost::any& value) {
		heap[nextAddress] = value;
		Pointer ptr(reinterpret_cast<void*>(nextAddress));
		nextAddress += sizeof(value);
		return ptr;
	}
	
	void deallocate(const Pointer& ptr) {
		heap.erase(reinterpret_cast<long long>(ptr.address));
	}
	
	boost::any dereference(const Pointer& ptr) {
		auto it = heap.find(reinterpret_cast<long long>(ptr.address));
		if (it != heap.end()) {
			return it->second;
		}
		return boost::any();
	}
	
	void write(const Pointer& ptr, const boost::any& value) {
		heap[reinterpret_cast<long long>(ptr.address)] = value;
	}
};

// Global memory heap instance
extern MemoryHeap globalMemoryHeap;

// Global class definitions
unordered_map<string, ClassDefinition> globalClassDefinitions;

// Implementation of AnyAsClassInstance (defined after ClassInstance is complete)
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

// Compare two boost::any values for equality
bool any_compare(const boost::any& a, const boost::any& b)
{
	// Handle null/empty cases
	if (a.empty() && b.empty()) return true;
	if (a.empty() || b.empty()) return false;
	
	// Try int comparison
	try {
		return any_cast<int>(a) == any_cast<int>(b);
	} catch (boost::bad_any_cast) {}
	
	// Try float comparison
	try {
		return any_cast<float>(a) == any_cast<float>(b);
	} catch (boost::bad_any_cast) {}
	
	// Try bool comparison
	try {
		return any_cast<bool>(a) == any_cast<bool>(b);
	} catch (boost::bad_any_cast) {}
	
	// Try string comparison
	try {
		return any_cast<string>(a) == any_cast<string>(b);
	} catch (boost::bad_any_cast) {}
	
	// Default: not equal if types don't match
	return false;
}

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
#ifdef HOLYZ_GRAPHICS_ENABLED
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
							catch (boost::bad_any_cast) // Try Class Instance
							{
								try
								{
									ClassInstance ci = any_cast<ClassInstance>(val);
									return 7;
								}
								catch (boost::bad_any_cast) // Does not convert, return
								{
									return -1; // Unknown type
								}
							}
						}
					}
#else
					// Graphics disabled, try ClassInstance directly
					try
					{
						ClassInstance ci = any_cast<ClassInstance>(val);
						return 7;
					}
					catch (boost::bad_any_cast) // Does not convert, return
					{
						return -1; // Unknown type
					}
#endif
				}
			}
		}
	}
}

//unordered_map<string, vector<vector<string>>> builtinFunctionValues;
//unordered_map<string, boost::any> builtinVarVals;

// Foreground colors
const std::string blackFGColor = "\x1B[30m";
const std::string redFGColor = "\x1B[31m";
const std::string greenFGColor = "\x1B[32m";
const std::string yellowFGColor = "\x1B[33m";
const std::string blueFGColor = "\x1B[34m";
const std::string magentaFGColor = "\x1B[35m";
const std::string cyanFGColor = "\x1B[36m";
const std::string whiteFGColor = "\x1B[37m";
const std::string brightBlackFGColor = "\x1B[90m";
const std::string brightRedFGColor = "\x1B[91m";
const std::string brightGreenFGColor = "\x1B[92m";
const std::string brightYellowFGColor = "\x1B[93m";
const std::string brightBlueFGColor = "\x1B[94m";
const std::string brightMagentaFGColor = "\x1B[95m";
const std::string brightCyanFGColor = "\x1B[96m";
const std::string brightWhiteFGColor = "\x1B[97m";
//Background colors
const std::string blackBGColor = "\x1B[40m";
const std::string redBGColor = "\x1B[41m";
const std::string greenBGColor = "\x1B[42m";
const std::string yellowBGColor = "\x1B[43m";
const std::string blueBGColor = "\x1B[44m";
const std::string magentaBGColor = "\x1B[45m";
const std::string cyanBGColor = "\x1B[46m";
const std::string whiteBGColor = "\x1B[47m";
const std::string brightBlackBGColor = "\x1B[100m";
const std::string brightRedBGColor = "\x1B[101m";
const std::string brightGreenBGColor = "\x1B[102m";
const std::string brightYellowBGColor = "\x1B[103m";
const std::string brightBlueBGColor = "\x1B[104m";
const std::string brightMagentaBGColor = "\x1B[105m";
const std::string brightCyanBGColor = "\x1B[106m";
const std::string brightWhiteBGColor = "\x1B[107m";
// Reset color
const std::string resetColor = "\033[0m";

class NullType {
public:
	string type = "NULL";
};
class BREAK {
public:
	string type = "BREAK";
};

// Rust-like Result type: either Ok(value) or Err(error)
class ResultValue {
public:
	bool isOk;  // true for Ok, false for Err
	boost::any value;  // The success value
	string error;  // The error message
	string errorType;  // Type of error (e.g., "IOError", "ParseError")
	
	ResultValue() : isOk(true), error(""), errorType("") {}
	ResultValue(const boost::any& val) : isOk(true), value(val), error(""), errorType("") {}
	ResultValue(const string& err, const string& errType = "Error") 
		: isOk(false), error(err), errorType(errType) {}
	
	string toString() const {
		if (isOk) return "Ok(value)";
		return "Err(" + errorType + ": " + error + ")";
	}
};

// Rust-like Option type: either Some(value) or None
class OptionValue {
public:
	bool isSome;  // true for Some, false for None
	boost::any value;  // The contained value
	
	OptionValue() : isSome(false) {}
	OptionValue(const boost::any& val) : isSome(true), value(val) {}
	
	// Factory methods
	static OptionValue Some(const boost::any& val) {
		return OptionValue(val);
	}
	
	static OptionValue None() {
		return OptionValue();
	}
	
	string toString() const {
		if (isSome) return "Some(value)";
		return "None";
	}
};

string any_type_name(const boost::any& val)
{
	int typeNum = any_type(val);
	switch (typeNum) {
		case 0: return "int";
		case 1: return "float";
		case 2: return "bool";
		case 3: return "string";
#ifdef HOLYZ_GRAPHICS_ENABLED
		case 4: return "Sprite";
		case 5: return "Vec2";
		case 6: return "Text";
#endif
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

// Trait support - improved trait system
class TraitDefinition {
public:
	string traitName;
	vector<pair<string, vector<string>>> methods;  // method name and parameters
	unordered_map<string, vector<vector<string>>> defaultImplementations;  // default method bodies
	
	TraitDefinition() {}
	TraitDefinition(const string& name) : traitName(name) {}
	
	void addMethod(const string& methodName, const vector<string>& params) {
		methods.push_back({methodName, params});
	}
	
	void setDefaultImplementation(const string& methodName, const vector<vector<string>>& body) {
		defaultImplementations[methodName] = body;
	}
	
	bool hasDefaultImpl(const string& methodName) const {
		return defaultImplementations.find(methodName) != defaultImplementations.end();
	}
};

// Trait implementation for a type
class TraitImplementation {
public:
	string typeName;
	string traitName;
	unordered_map<string, vector<vector<string>>> implementations;  // method implementations
	
	TraitImplementation() {}
	TraitImplementation(const string& type, const string& trait) 
		: typeName(type), traitName(trait) {}
};

// Global trait definitions and implementations
extern unordered_map<string, TraitDefinition> globalTraits;
extern vector<TraitImplementation> globalTraitImpls;

boost::any nullType;
boost::any breakReOp;

float clamp(float v, float min, float max)
{
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void PrintColored(std::string text, std::string fgColor, std::string bgColor, bool isError)
{
#if WINDOWS
	if (fgColor == blackFGColor)
		cerr << hue::black;
	else if (fgColor == redFGColor)
		cerr << hue::red;
	else if (fgColor == greenFGColor)
		cerr << hue::green;
	else if (fgColor == yellowFGColor)
		cerr << hue::yellow;
	else if (fgColor == blueFGColor)
		cerr << hue::blue;
	else if (fgColor == magentaFGColor)
		cerr << hue::purple;
	else if (fgColor == cyanFGColor)
		cerr << hue::aqua;
	else if (fgColor == whiteFGColor)
		cerr << hue::white;

	if (bgColor == blackBGColor)
		cerr << hue::on_black;
	else if (bgColor == redBGColor)
		cerr << hue::on_red;
	else if (bgColor == greenBGColor)
		cerr << hue::on_green;
	else if (bgColor == yellowBGColor)
		cerr << hue::on_yellow;
	else if (bgColor == blueBGColor)
		cerr << hue::on_blue;
	else if (bgColor == magentaBGColor)
		cerr << hue::on_purple;
	else if (bgColor == cyanBGColor)
		cerr << hue::on_aqua;
	else if (bgColor == whiteBGColor)
		cerr << hue::on_white;

	std::cerr << text << hue::reset;
#else
	if (!isError)
		cout << fgColor + bgColor + text + resetColor;
	else
		cerr << fgColor + bgColor + text + resetColor;
#endif
}

int LogWarning(const string& warningText)
{
	PrintColored("WARNING: ", yellowFGColor, "", true);
	PrintColored(escaped(warningText), yellowFGColor, "", true);
	cerr << std::endl;
	//cout << "\x1B[33mWARNING: " << warningText << "\033[0m\t\t" << endl;
	return 1;
}

int InterpreterLog(const string& logText)
{
	int Hour = 0;
	int Min = 0;
	int Sec = 0;

	time_t timer = time(0);

	tm bt{};
#if UNIX
	time_t currentTime;
	struct tm* localTime;

	time(&currentTime);                   // Get the current time
	localTime = localtime(&currentTime);  // Convert the current time to the local time

	Hour = localTime->tm_hour;
	Min = localTime->tm_min;
	Sec = localTime->tm_sec;
#elif WINDOWS
	localtime_s(&bt, &timer);
	Hour = bt.tm_hour;
	Min = bt.tm_min;
	Sec = bt.tm_sec;
#else
	static mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *localtime(&timer);
#endif

	PrintColored("[" + to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec) + "] ", blueFGColor, "", true);
	PrintColored("HolyZ: ", yellowFGColor, "", true);
	PrintColored(escaped(logText), greenFGColor, "", true);
	cout << std::endl;
	//cout << "\x1B[34m[" + to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec) + "] \x1B[33mHolyZ: \x1B[32m" << logText << "\033[0m\t\t" << endl;
	return 1;
}

int LogCriticalError(const string& errorText)
{
	int Hour = 0;
	int Min = 0;
	int Sec = 0;
	time_t timer = time(0);

	tm bt{};
#if UNIX
	time_t currentTime;
	struct tm* localTime;

	time(&currentTime);                   // Get the current time
	localTime = localtime(&currentTime);  // Convert the current time to the local time

	Hour = localTime->tm_hour;
	Min = localTime->tm_min;
	Sec = localTime->tm_sec;
#elif WINDOWS
	localtime_s(&bt, &timer);
	Hour = bt.tm_hour;
	Min = bt.tm_min;
	Sec = bt.tm_sec;
#else
	static mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	bt = *localtime(&timer);
#endif

	PrintColored("[" + to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec) + "] ", blueFGColor, "", true);
	PrintColored("HolyZ: ", yellowFGColor, "", true);
	PrintColored(escaped(errorText), redFGColor, "", true);
	cerr << std::endl;
	InterpreterLog("Press Enter to Exit...");
	cin.ignore();
	exit(1);
	//cerr << "\x1B[34m[" + to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec) + "] \x1B[33mHolyZ: \x1B[31mERROR: " << errorText << "\033[0m\t\t" << endl;
	//exit(EXIT_FAILURE);
	return 2;
}

#ifdef HOLYZ_GRAPHICS_ENABLED
boost::any GetClassSubComponent(boost::any value, string subComponentName)
{
	// If a Sprite Class
	if (any_type(value) == 4)
	{
		return any_cast<Sprite>(value).SubComponent(subComponentName);
	}
	// If a Vec2 Class
	if (any_type(value) == 5)
	{
		return any_cast<Vec2>(value).SubComponent(subComponentName);
	}
	// If a Text Class
	if (any_type(value) == 6)
	{
		return any_cast<Text>(value).SubComponent(subComponentName);
	}
	return nullType;
}
#else
boost::any GetClassSubComponent(boost::any value, string subComponentName)
{
	return nullType;
}
#endif // HOLYZ_GRAPHICS_ENABLED

#ifdef HOLYZ_GRAPHICS_ENABLED
boost::any EditClassSubComponent(boost::any value, string oper, boost::any otherVal, string subComponentName)
{
	// If a Sprite Class
	if (any_type(value) == 4)
	{
		Sprite v = any_cast<Sprite>(value);
		v.EditSubComponent(subComponentName, oper, otherVal);
		return v;
	}
	// If a Vec2 Class
	if (any_type(value) == 5)
	{
		Vec2 v = any_cast<Vec2>(value);
		v.EditSubComponent(subComponentName, oper, otherVal);
		return v;
	}
	// If a Text Class
	if (any_type(value) == 6)
	{
		Text t = any_cast<Text>(value);
		t.EditSubComponent(subComponentName, oper, otherVal);
		return t;
	}
	return nullType;
}

bool AxisAlignedCollision(const Sprite& a, const Sprite& b) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = a.position.x + a.scale.x / 2 >= b.position.x - b.scale.x / 2 &&
		b.position.x + b.scale.x / 2 >= a.position.x - a.scale.x;
	// collision y-axis?
	bool collisionY = a.position.y + a.scale.y / 2 >= b.position.y - b.scale.y / 2 &&
		b.position.y + b.scale.y / 2 >= a.position.y - a.scale.y / 2;

	//// collision x-axis?
	//bool collisionX = a.position.x - a.scale.x / 2 >= b.position.x + b.scale.x / 2 ||
	//	a.position.x + a.scale.x / 2 <= b.position.x - b.scale.x / 2;
	//// collision y-axis?
	//bool collisionY = a.position.y - a.scale.y / 2 >= b.position.y + b.scale.y / 2 ||
	//	a.position.y + a.scale.y / 2 <= b.position.y - b.scale.y / 2;

	// collision only if on both axes
	return collisionX && collisionY;
}
#else
boost::any EditClassSubComponent(boost::any value, string oper, boost::any otherVal, string subComponentName)
{
	return nullType;
}
#endif // HOLYZ_GRAPHICS_ENABLED

//// Initial script processing, which loads variables and functions from builtin
//int GetBuiltins(std::string s)
//{
//	std::string script = replace(s, "    ", "\t");
//
//	vector<string> lines = split(script, '\n');
//	vector<vector<string>> words;
//	for (int i = 0; i < (int)lines.size(); i++)
//	{
//		words.push_back(split(lines.at(i), ' '));
//	}
//
//	// Go through entire script and iterate through all types to see if line is a
//	// function declaration, then store it with it's value
//	for (int lineNum = 0; lineNum < (int)words.size(); lineNum++)
//	{
//		//Checks if it is function
//		if (words.at(lineNum).at(0) == "func")
//		{
//			vector<vector<string>> functionContents;
//
//			string functName = split(words.at(lineNum).at(1), '(')[0];
//
//#if DEVELOPER_MESSAGES == true
//			InterpreterLog("Load builtin function " + functName + "...");
//#endif
//
//			string args = "";
//			for (int w = 1; w < (int)words.at(lineNum).size(); w++) // Get all words from the instantiation line: these are the args
//			{
//				args += replace(replace(words.at(lineNum).at(w), "(", " "), ")", "");
//			}
//
//			args = replace(args, functName + " ", "");
//			functionContents.push_back(split(args, ','));
//
//			int numOfBrackets = 1;
//			for (int p = lineNum + 2; p < (int)words.size(); p++)
//			{
//				numOfBrackets += countInVector(words.at(p), "{") - countInVector(words.at(p), "}");
//				if (numOfBrackets == 0)
//					break;
//				functionContents.push_back(removeTabs(words.at(p), 1));
//			}
//			builtinFunctionValues[functName] = functionContents;
//			//cout << functName << " is \n" << Vec2Str(functionContents) << endl << endl;
//		}
//		else
//		{
//			if (words.at(lineNum).at(0) == "string")
//			{
//				builtinVarVals[words.at(lineNum).at(1)] = StringRaw(words.at(lineNum).at(3));
//#if DEVELOPER_MESSAGES == true
//				InterpreterLog("Load builtin variable " + words.at(lineNum).at(1) + "...");
//#endif
//			}
//			else if (words.at(lineNum).at(0) == "int")
//			{
//				builtinVarVals[words.at(lineNum).at(1)] = stoi(words.at(lineNum).at(3));
//#if DEVELOPER_MESSAGES == true
//				InterpreterLog("Load builtin variable " + words.at(lineNum).at(1) + "...");
//#endif
//			}
//			else if (words.at(lineNum).at(0) == "float")
//			{
//				builtinVarVals[words.at(lineNum).at(1)] = stof(words.at(lineNum).at(3));
//#if DEVELOPER_MESSAGES == true
//				InterpreterLog("Load builtin variable " + words.at(lineNum).at(1) + "...");
//#endif
//			}
//			else if (words.at(lineNum).at(0) == "bool")
//			{
//				builtinVarVals[words.at(lineNum).at(1)] = stob(words.at(lineNum).at(3));
//#if DEVELOPER_MESSAGES == true
//				InterpreterLog("Load builtin variable " + words.at(lineNum).at(1) + "...");
//#endif
//			}
//			//else
//			//	LogWarning("unrecognized type \'" + words[lineNum][0] + "\' on line: " + to_string(lineNum));
//		}
//	}
//
//	return 0;
//}

// Executes 
boost::any ZSFunction(const string& name, const vector<boost::any>& args)
{
	if (name == "ZS.Math.Sin")
		return sin(AnyAsFloat(args.at(0)));
	else if (name == "ZS.Math.Cos")
		return cos(AnyAsFloat(args.at(0)));
	else if (name == "ZS.Math.Tan")
		return tan(AnyAsFloat(args.at(0)));
	else if (name == "ZS.Math.Round")
		return AnyAsInt(args.at(0));
	else if (name == "ZS.Math.Lerp")
		return lerp(AnyAsFloat(args.at(0)), AnyAsFloat(args.at(1)), AnyAsFloat(args.at(2)));
	else if (name == "ZS.Math.Abs")
		return abs(AnyAsFloat(args.at(0)));
#ifdef HOLYZ_GRAPHICS_ENABLED
	else if (name == "ZS.Graphics.Init")
	{
#if DEVELOPER_MESSAGES == true
		InterpreterLog("Init graphics");
#endif
		if (args.size() <= 3)
			initGraphics(StringRaw(AnyAsString(args.at(0))), AnyAsInt(args.at(1)), AnyAsInt(args.at(2)), 1);
		else
			initGraphics(StringRaw(AnyAsString(args.at(0))), AnyAsInt(args.at(1)), AnyAsInt(args.at(2)), AnyAsInt(args.at(3)));
	}
	else if (name == "ZS.Graphics.Sprite")
	{
		string path = StringRaw(AnyAsString(args.at(0)));
		if (count(path, '/') == 0)
			path = "./" + path;
		if (!fileExists(path))
			LogCriticalError("Failed to create 'Sprite' object: \"" + path + "\"");

		Sprite s(StringRaw(AnyAsString(args.at(0))), any_cast<Vec2>(args.at(1)), any_cast<Vec2>(args.at(2)), AnyAsFloat(args.at(3)));
		return s;
	}
	else if (name == "ZS.Graphics.DrawPixel")
	{
		SDL_SetRenderDrawColor(gRenderer, AnyAsInt(args.at(2)), AnyAsInt(args.at(3)), AnyAsInt(args.at(4)), 255);
		SDL_RenderDrawPoint(gRenderer, AnyAsInt(args.at(0)), AnyAsInt(args.at(1)));
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
	}
		//DrawPixel(AnyAsInt(args.at(0)), AnyAsInt(args.at(1)), AnyAsInt(args.at(2)), AnyAsInt(args.at(3)), AnyAsInt(args.at(4)));
	else if (name == "ZS.Graphics.Draw")
		any_cast<Sprite>(args.at(0)).Draw();
	else if (name == "ZS.Graphics.Load")
		any_cast<Sprite>(args.at(0)).Load();
	else if (name == "ZS.Graphics.Text")
	{
		string path = StringRaw(AnyAsString(args.at(1)));
		if (count(path, '/') == 0)
			path = "./" + path;
		if (!fileExists(path))
			LogCriticalError("Failed to create 'Text' object: \"" + path + "\"");

		if (args.size() <= 8)
		{
			Text t(StringRaw(AnyAsString(args.at(0))), path, any_cast<Vec2>(args.at(2)), AnyAsFloat(args.at(3)), AnyAsFloat(args.at(4)), (Uint8)AnyAsFloat(args.at(5)), (Uint8)AnyAsFloat(args.at(6)), (Uint8)AnyAsFloat(args.at(7)), true);
			return t;
		}
		else
		{
			Text t(StringRaw(AnyAsString(args.at(0))), path, any_cast<Vec2>(args.at(2)), AnyAsFloat(args.at(3)), AnyAsFloat(args.at(4)), (Uint8)AnyAsFloat(args.at(5)), (Uint8)AnyAsFloat(args.at(6)), (Uint8)AnyAsFloat(args.at(7)), AnyAsBool(args.at(8)));
			return t;
		}
	}
	else if (name == "ZS.Graphics.DrawText")
		any_cast<Text>(args.at(0)).Draw();
	else if (name == "ZS.Graphics.LoadText")
		any_cast<Text>(args.at(0)).Load();
	else if (name == "ZS.Physics.AxisAlignedCollision")
	{
		return AxisAlignedCollision(any_cast<Sprite>(args.at(0)), any_cast<Sprite>(args.at(1)));
	}
	else if (name == "ZS.Input.GetKey")
		return KEYS[StringRaw(any_cast<string>(args.at(0)))] == 1;
	else if (name == "ZS.System.Vec2")
	{
		Vec2 v(AnyAsFloat(args.at(0)), AnyAsFloat(args.at(1)));
		return v;
	}
#endif // HOLYZ_GRAPHICS_ENABLED
	else if (name == "ZS.System.Print")
		cout << StringRaw(AnyAsString(args.at(0)));
	else if (name == "ZS.System.PrintLine")
		cout << StringRaw(AnyAsString(args.at(0))) << endl;
	else if (name == "ZS.System.Command"){
		string command = StringRaw(AnyAsString(args.at(0)));
		//int command_len = command.length();
		//char* command_char_arr=new char[command_len + 1];
		//strcpy(command_char_arr, command.c_str()); // string into char arr
		int k = system(command.c_str());
	}
	else
		LogWarning("ZS function \'" + name + "\' does not exist.");

	return nullType;
}
// Class-related function implementations

// Find a method in a class or its superclasses
ClassMethod* FindMethod(const string& className, const string& methodName, bool isStatic = false)
{
	if (globalClassDefinitions.find(className) == globalClassDefinitions.end())
		return nullptr;
	
	ClassDefinition& classDef = globalClassDefinitions[className];
	
	// Search in current class
	for (auto& method : classDef.methods)
	{
		if (method.name == methodName && method.isStatic == isStatic)
			return &method;
	}
	
	// Search in superclass if not found
	if (!classDef.superClassName.empty())
		return FindMethod(classDef.superClassName, methodName, isStatic);
	
	return nullptr;
}

// Create a new class instance
boost::any CreateClassInstance(const string& className, const vector<boost::any>& constructorArgs)
{
	if (globalClassDefinitions.find(className) == globalClassDefinitions.end())
	{
		LogWarning("Class '" + className + "' not found");
		return nullType;
	}
	
	ClassInstance instance(className);
	
	// Initialize instance attributes from class definition
	ClassDefinition& classDef = globalClassDefinitions[className];
	for (const auto& attr : classDef.attributes)
	{
		if (!attr.isStatic)
			instance.instanceAttributes[attr.name] = attr.value;
	}
	
	// Call constructor if it exists
	ClassMethod* constructor = FindMethod(className, "constructor", false);
	if (constructor != nullptr)
	{
		// Constructor execution is handled in Main.cpp ExecuteHolyCFunction()
		// via the send() function which sets up proper 'this' context
		// and executes the method body with all parameters
	}
	
	return instance;
}

// Call a method on a class instance
boost::any CallClassMethod(const ClassInstance& instance, const string& methodName, const vector<boost::any>& args)
{
	ClassMethod* method = FindMethod(instance.className, methodName, false);
	if (method == nullptr)
	{
		LogWarning("Method '" + methodName + "' not found in class '" + instance.className + "'");
		return nullType;
	}
	
	// Method execution is handled in Main.cpp ExecuteHolyCFunction()
	// via the send() function which sets up proper 'this' context,
	// extracts method parameters and arguments, and calls ProcessLine()
	// for each line in the method body
	// Note: CallClassMethod is kept for potential reflection, actual execution
	// happens through send()
	return nullType;
}

// Call a static method on a class
boost::any CallStaticMethod(const string& className, const string& methodName, const vector<boost::any>& args)
{
	ClassMethod* method = FindMethod(className, methodName, true);
	if (method == nullptr)
	{
		LogWarning("Static method '" + methodName + "' not found in class '" + className + "'");
		return nullType;
	}
	
	// Static method execution is handled in Main.cpp ExecuteHolyCFunction()
	// Static methods do not require instance context ('this')
	// Execution happens through send() with proper parameter setup
	// Note: CallStaticMethod is kept for potential reflection, actual execution
	// happens through send()
	return nullType;
}

// Get class attribute (static or instance)
boost::any GetClassAttribute(const ClassInstance& instance, const string& attributeName)
{
	// First check instance attributes
	auto it = instance.instanceAttributes.find(attributeName);
	if (it != instance.instanceAttributes.end())
		return it->second;
	
	// Then check static attributes
	if (globalClassDefinitions.find(instance.className) != globalClassDefinitions.end())
	{
		ClassDefinition& classDef = globalClassDefinitions[instance.className];
		auto staticIt = classDef.staticAttributes.find(attributeName);
		if (staticIt != classDef.staticAttributes.end())
			return staticIt->second;
		
		// Check superclass static attributes
		if (!classDef.superClassName.empty())
		{
			ClassInstance tempInstance(classDef.superClassName);
			return GetClassAttribute(tempInstance, attributeName);
		}
	}
	
	LogWarning("Attribute '" + attributeName + "' not found in class '" + instance.className + "'");
	return nullType;
}

// Set class attribute (static or instance)
void SetClassAttribute(ClassInstance& instance, const string& attributeName, const boost::any& value)
{
	// Check if it's an instance attribute
	auto it = instance.instanceAttributes.find(attributeName);
	if (it != instance.instanceAttributes.end())
	{
		instance.instanceAttributes[attributeName] = value;
		return;
	}
	
	// Check if it's a static attribute
	if (globalClassDefinitions.find(instance.className) != globalClassDefinitions.end())
	{
		ClassDefinition& classDef = globalClassDefinitions[instance.className];
		auto staticIt = classDef.staticAttributes.find(attributeName);
		if (staticIt != classDef.staticAttributes.end())
		{
			classDef.staticAttributes[attributeName] = value;
			return;
		}
	}
	
	// If not found, create as instance attribute
	instance.instanceAttributes[attributeName] = value;
}

// Get static attribute from class
boost::any GetStaticAttribute(const string& className, const string& attributeName)
{
	if (globalClassDefinitions.find(className) != globalClassDefinitions.end())
	{
		ClassDefinition& classDef = globalClassDefinitions[className];
		auto it = classDef.staticAttributes.find(attributeName);
		if (it != classDef.staticAttributes.end())
			return it->second;
		
		// Check superclass
		if (!classDef.superClassName.empty())
			return GetStaticAttribute(classDef.superClassName, attributeName);
	}
	
	LogWarning("Static attribute '" + attributeName + "' not found in class '" + className + "'");
	return nullType;
}


#endif
