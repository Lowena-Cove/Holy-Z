
#include <iostream>
#include <fstream>
#include <string>
//bool DEVELOPER_MESSAGES = true;
#define DEVELOPER_MESSAGES false
#define EXAMPLE_PROJECT false
#define NAMEVERSION "Holy Z v2.2.0-alpha (Holy C Enhanced)"

#if defined(__unix__)
#define UNIX true
#define WINDOWS false
#elif defined(_MSC_VER)
#define UNIX false
#define WINDOWS true
#endif

#include <regex>
#include <limits>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <boost/any.hpp>
#include <unordered_map>
#include <stdio.h>
#include <codecvt>
#include <thread>

#if UNIX
#include <unistd.h>
#include <filesystem>
#elif WINDOWS
#include <windows.h>
#endif

#include "eval.h"
#include "strops.h"
#include "builtin.h"
#include "main.h"
#include "anyops.h"

#include "ZS.h"

using namespace std;
using namespace boost;

// Helper function for case-insensitive keyword comparison
inline bool IsKeyword(const string& token, const string& keyword) {
	return toLower(token) == toLower(keyword);
}

unordered_map<string, boost::any> globalVariableValues;
unordered_map<string, vector<vector<string>>> functionValues;

// Memory heap for dynamic allocation
MemoryHeap globalMemoryHeap;

// Immutability tracking - Rust-inspired
unordered_map<string, bool> immutableVariables;  // true if variable is immutable (let)
unordered_map<string, bool> borrowedVariables;   // true if variable is borrowed (borrowed reference)

// Holy C mode flags
bool holyCMode = false;
bool shellMode = false;

// Current execution context for 'this' keyword
ClassInstance* currentThisContext = nullptr;

boost::any GetVariableValue(const string& varName, const unordered_map<string, boost::any>& variableValues)
{
	string classSubComponent;
	string baseName = trim(varName);

	if (count(varName, '.') > 0)
	{
		classSubComponent = trim(varName.substr(indexInStr(varName, '.') + 1, -1));
		baseName = trim(split(varName, '.')[0]);
	}

	// Handle 'this' keyword
	if (baseName == "this")
	{
		if (currentThisContext != nullptr)
		{
			if (classSubComponent.empty())
				return *currentThisContext;
			else
				return GetClassAttribute(*currentThisContext, classSubComponent);
		}
		else
		classSubComponent = trim(varName.substr(indexInStr(varName, '.') + 1, -1));
		baseName = trim(split(varName, '.')[0]);
	}
	}

	boost::any outputValue = nullType;

	auto iA = variableValues.find(baseName);
	auto iB = globalVariableValues.find(baseName);
	if (iA != variableValues.end())
		outputValue = iA->second;
	else if (iB != globalVariableValues.end())
		outputValue = iB->second;
	else
		outputValue = varName;

	if (count(varName, '.') > 0 && !outputValue.empty())
		return GetClassSubComponent(outputValue, classSubComponent);
	else
		return outputValue;
}

// Check if there is a variable with the specified name
bool IsVar(const string& varName, const unordered_map<string, boost::any>& variableValues)
{
	// Handle 'this' keyword
	if (split(varName, '.')[0] == "this")
		return currentThisContext != nullptr;
	
	// Handle class static access (ClassName.staticMember)
	string baseName = split(varName, '.')[0];
	if (globalClassDefinitions.find(baseName) != globalClassDefinitions.end())
		return true;
	
	if (split(varName, '.')[0] == "ZS")
		return false;

	if (variableValues.find(split(varName, '.')[0]) != variableValues.end())
		return true;

	return false;
}

// Return a vector of values that correspond to a vector of input variable names
vector<boost::any> VarValues(const vector<string>& varNames, unordered_map<string, boost::any>& variableValues)
{
	vector<boost::any> realValues;

	for (int varIndex = 0; varIndex < varNames.size(); varIndex++)
	{
		string varName = trim(varNames.at(varIndex));

		//realValues.push_back(EvalExpression(varName, variableValues));
		auto iA = variableValues.find(varName);
		if (iA != variableValues.end())
		{
			realValues.push_back(iA->second);
		}
		else
		{
			auto iB = globalVariableValues.find(varName);
			if (iB != globalVariableValues.end())
				realValues.push_back(iB->second);
			else
				realValues.push_back(EvalExpression(varName, variableValues));
		}
	}

	return realValues;
}

void printVarValues(const vector<string>& vec, unordered_map<string, boost::any>& variableValues)
{
	cout << "<";
	for (int i = 0; i < vec.size(); i++)
	{
		cout << AnyAsString(GetVariableValue(vec.at(i), globalVariableValues));
		cout << AnyAsString(GetVariableValue(vec.at(i), variableValues));
		cout << " | ";
	}
	cout << ">";
	cout << endl;
}


bool IsFunction(const string& funcName)
{
	if (functionValues.find(funcName) != functionValues.end())
		return true;
	else
		return false;
}
bool IsClass(const string& className)
{
	if (globalClassDefinitions.find(className) != globalClassDefinitions.end())
		return true;
	else
		return false;
}
bool IsZSFunction(const string& funcName)
{
	return startsWith(funcName, "ZS.");
}

bool IsHolyCFunction(const string& funcName)
{
	// List of built-in Holy C functions
	static const vector<string> holyCFunctions = {
		"ToInt", "ToFloat", "ToStr", "ToBool",
		"typeof", "TypeOf", "typecheck", "TypeCheck", "istype", "IsType",
		"malloc", "Malloc", "free", "Free", "addressof", "AddressOf", "ptr",
		"deref", "Deref", "dereference", "setvalue", "SetValue",
		"send", "Send", "hasmethod", "HasMethod", "getmethod", "GetMethod",
		// Result/Option functions (Rust-like)
		"Ok", "Err", "Some", "None", "isOk", "IsOk", "isErr", "IsErr", 
		"isSome", "IsSome", "isNone", "IsNone", "unwrap", "Unwrap",
		"expect", "Expect", "unwrapOr", "UnwrapOr", "match", "Match"
	};
	
	for (const auto& func : holyCFunctions) {
		if (toLower(funcName) == toLower(func))
			return true;
	}
	return false;
}

// Forward declarations
boost::any ExecuteHolyCFunction(const string& functionName, const vector<boost::any>& args);
void RunREPL();

boost::any EvalExpression(const string& ex, unordered_map<string, boost::any>& variableValues)
{
	string expression = trim(ex);
	bool inQuotes = false;

	string expression = trim(ex);
	bool inQuotes = false;

#if DEVELOPER_MESSAGES == true
	//InterpreterLog("	old expression: |" + expression + "|");
#endif

	bool isFunc = IsFunction(split(expression, '(')[0]);
	bool isZS = split(expression, '.')[0] == "ZS";
	// If no operations are applied, then return self
	if ((countOutsideParenthesis(expression, '+') == 0 && countOutsideParenthesis(expression, '-') == 0 && countOutsideParenthesis(expression, '*') == 0 && countOutsideParenthesis(expression, '/') == 0 && countOutsideParenthesis(expression, '^') == 0) || split(expression, '.')[0] == "ZS")
	{
		//bool isFunc = IsFunction(split(expression, '(')[0]);
		if (isFunc && !inQuotes)
		{
			// start -> FuncCall(0, x, OtherFunc(a))
			// changeto -> 0, x, OtherFunc(a)
			string insideFunArgs = betweenChars(expression, '(', ')');
			vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
			cout << split(expression, '(')[0] << "  [" << unWrapVec(argList) << "]" << endl;
			printVarValues(argList, variableValues);
#endif
			vector<boost::any> funcArgs = VarValues(argList, variableValues);
			return ExecuteFunction(split(expression, '(')[0], funcArgs);
		}
		else if (IsHolyCFunction(split(expression, '(')[0]) && !inQuotes)
		{
			string insideFunArgs = betweenChars(expression, '(', ')');
			vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
			cout << split(expression, '(')[0] << "  [" << unWrapVec(argList) << "]" << endl;
#endif
			vector<boost::any> funcArgs = VarValues(argList, variableValues);
			return ExecuteHolyCFunction(split(expression, '(')[0], funcArgs);
		}
		else if (isZS && !inQuotes)
		{
			// start -> FuncCall(0, x, OtherFunc(a))
			// changeto -> 0, x, OtherFunc(a)
			string insideFunArgs = betweenChars(expression, '(', ')');
			vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
			cout << split(expression, '(')[0] << "  [" << unWrapVec(argList) << "]" << endl;
			printVarValues(argList, variableValues);
#endif
			vector<boost::any> funcArgs = VarValues(argList, variableValues);
			return ZSFunction(split(expression, '(')[0], funcArgs);
		}
		else
			return GetVariableValue(expression, variableValues);
	}

	string newExpression = "";
	inQuotes = false;

	for (int i = 0; i < expression.size(); i++)
	{
		if (expression[i] == '\"' && !isEscaped(newExpression, i))
			inQuotes = !inQuotes;

		if (isalpha(expression[i]) || expression[i] == '_')
		{
			string name = "";

			while (i < expression.size() && (isalpha(expression[i]) || expression[i] == '.' || expression[i] == '_'))
			{
				name += expression[i];
				i++;
			}

			//string varVal = GetVariableValue(name, variables, variableValues);
			bool isFunc = IsFunction(name);
			if (isFunc && !inQuotes)
			{
				// start -> FuncCall(0, x, OtherFunc(a))
				// changeto -> 0, x, OtherFunc(a)
				string insideFunArgs = betweenChars(expression, '(', ')');
				vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
				cout << "[" << unWrapVec(argList) << "]" << endl;
				printVarValues(argList, variableValues);
#endif
				vector<boost::any> funcArgs = VarValues(argList, variableValues);
				string returnVal = AnyAsString(ExecuteFunction(split(expression, '(')[0], funcArgs));
				newExpression += returnVal;
			}
			else if (split(name, '.')[0] == "ZS" && !inQuotes)
			{
				// start -> FuncCall(0, x, OtherFunc(a))
				// changeto -> 0, x, OtherFunc(a)
				string insideFunArgs = betweenChars(expression, '(', ')');
				vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
				cout << "[" << unWrapVec(argList) << "]" << endl;
				printVarValues(argList, variableValues);
#endif
				vector<boost::any> funcArgs = VarValues(argList, variableValues);
				string returnVal = AnyAsString(ZSFunction(split(expression, '(')[0], funcArgs));
				newExpression += returnVal;
			}
			else
			{
				if (inQuotes)
					newExpression += name;
				else
					newExpression += AnyAsString(GetVariableValue(name, variableValues));
			}

			i--;
		}
		else
		{
			newExpression += expression[i];
		}
	}
#if DEVELOPER_MESSAGES == true
	//InterpreterLog("	new expression: |" + newExpression + "|");
#endif

	bool addStrings = false;
	for (int i = 0; i < (int)newExpression.size(); i++)
		if (isalpha(newExpression[i]) || (newExpression[i] == '\"' && !isEscaped(newExpression, i)))
		{
			addStrings = true;
			break;
		}
	if (addStrings)
	{
		inQuotes = false;
		string withoutParenthesis = "";
		for (int i = 0; i < (int)newExpression.size(); i++)
		{
			if (newExpression[i] == '\"' && !isEscaped(newExpression, i))
			{
				inQuotes = !inQuotes;
				continue;
			}
			if (inQuotes)
				withoutParenthesis += newExpression[i];
			if (!inQuotes && newExpression[i] != '(' && newExpression[i] != ')' && newExpression[i] != '+' && newExpression[i] != ' ')
				withoutParenthesis += newExpression[i];
		}

		//cout << "NewSTRING = " << Quoted(withoutParenthesis) << endl;
		return withoutParenthesis;
	}
	else
		return evaluate(newExpression);
}

bool BooleanLogic(const string& valA, const string& comparer, const string& valB, unordered_map<string, boost::any>& variableValues)
{
	boost::any valARealValue;
	boost::any valBRealValue;
	if (valA != "")
		valARealValue = EvalExpression(valA, variableValues);
	if (valB != "")
		valBRealValue = EvalExpression(valB, variableValues);
#if DEVELOPER_MESSAGES == true
	InterpreterLog(AnyAsString(valARealValue) + " " + comparer + " " + AnyAsString(valBRealValue) + " : " + AnyAsString(valA) + " " + comparer + " " + AnyAsString(valB) + " : " + to_string(AnyAsString(valARealValue) == AnyAsString(valBRealValue)));
#endif
	if (comparer == "==")
		return any_compare(valARealValue, valBRealValue);
	else if (comparer == "!=")
		return !any_compare(valARealValue, valBRealValue);
	else if (comparer == ">=")
		return AnyAsFloat(valARealValue) >= AnyAsFloat(valBRealValue);
	else if (comparer == "<=")
		return AnyAsFloat(valARealValue) <= AnyAsFloat(valBRealValue);
	else if (comparer == ">")
		return AnyAsFloat(valARealValue) > AnyAsFloat(valBRealValue);
	else if (comparer == "<")
		return AnyAsFloat(valARealValue) < AnyAsFloat(valBRealValue);
	else if (comparer == "")
		return AnyAsBool(valARealValue) == true;
	else
		LogWarning("unrecognized comparer \'" + comparer + "\'");

	return false;
}

int varOperation(const vector<string>& str, unordered_map<string, boost::any>& variableValues)
{
	if (IsVar(str.at(0), variableValues))
	{
		// Checks if type is simple, like int or string
		if (any_type(variableValues[str.at(0)]) <= 3)
		{
			if (str.at(1) == "=")
				variableValues[str.at(0)] = EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues);
			else if (str.at(1) == "+=")
				variableValues[str.at(0)] = EvalExpression(str.at(0) + "+(" + unWrapVec(vector<string>(str.begin() + 2, str.end())) + ")", variableValues);
			else if (str.at(1) == "-=")
				variableValues[str.at(0)] = AnyAsFloat(variableValues[str.at(0)]) - AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else if (str.at(1) == "*=")
				variableValues[str.at(0)] = AnyAsFloat(variableValues[str.at(0)]) * AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else if (str.at(1) == "/=")
				variableValues[str.at(0)] = AnyAsFloat(variableValues[str.at(0)]) / AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else
				LogWarning("unrecognized operator \'" + str.at(1) + "\'");
		}
		// Else it is a Vec2. No other complex class can be operated on it's base form (ex. you can't do: Sprite += Sprite)
		else if (any_type(variableValues[str.at(0)]) == 5)
		{
			boost::any otherExpression = EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues);
			if (str.at(1) == "=")
				variableValues[str.at(0)] = otherExpression;
			else if (str.at(1) == "+=")
				variableValues[str.at(0)] = AnyAsVec2(variableValues[str.at(0)]) + AnyAsVec2(otherExpression);
			else if (str.at(1) == "-=")
				variableValues[str.at(0)] = AnyAsVec2(variableValues[str.at(0)]) - AnyAsVec2(otherExpression);
			else if (str.at(1) == "*=")
				variableValues[str.at(0)] = AnyAsVec2(variableValues[str.at(0)]) * AnyAsFloat(otherExpression);
			else if (str.at(1) == "/=")
				variableValues[str.at(0)] = AnyAsVec2(variableValues[str.at(0)]) / AnyAsFloat(otherExpression);
			else
				LogWarning("unrecognized operator \'" + str.at(1) + "\'");
		}
		return 0;
	}
	else if (IsVar(str.at(0), globalVariableValues))
	{
		// Checks if type is simple, like int or string
		if (any_type(globalVariableValues[str.at(0)]) <= 3)
		{
			if (str.at(1) == "=")
				globalVariableValues[str.at(0)] = EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues);
			else if (str.at(1) == "+=")
				globalVariableValues[str.at(0)] = EvalExpression(str.at(0) + "+(" + unWrapVec(vector<string>(str.begin() + 2, str.end())) + ")", variableValues);
			else if (str.at(1) == "-=")
				globalVariableValues[str.at(0)] = AnyAsFloat(globalVariableValues[str.at(0)]) - AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else if (str.at(1) == "*=")
				globalVariableValues[str.at(0)] = AnyAsFloat(globalVariableValues[str.at(0)]) * AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else if (str.at(1) == "/=")
				globalVariableValues[str.at(0)] = AnyAsFloat(globalVariableValues[str.at(0)]) / AnyAsFloat(EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues));
			else
				LogWarning("unrecognized operator \'" + str.at(1) + "\'");
		}
		// Else it is a Vec2. No other complex class can be operated on it's base form (ex. you can't do: Sprite += Sprite)
		else if (any_type(globalVariableValues[str.at(0)]) == 5)
		{
			boost::any otherExpression = EvalExpression(unWrapVec(vector<string>(str.begin() + 2, str.end())), variableValues);
			if (str.at(1) == "=")
				globalVariableValues[str.at(0)] = otherExpression;
			else if (str.at(1) == "+=")
				globalVariableValues[str.at(0)] = AnyAsVec2(globalVariableValues[str.at(0)]) + AnyAsVec2(otherExpression);
			else if (str.at(1) == "-=")
				globalVariableValues[str.at(0)] = AnyAsVec2(globalVariableValues[str.at(0)]) - AnyAsVec2(otherExpression);
			else if (str.at(1) == "*=")
				globalVariableValues[str.at(0)] = AnyAsVec2(globalVariableValues[str.at(0)]) * AnyAsFloat(otherExpression);
			else if (str.at(1) == "/=")
				globalVariableValues[str.at(0)] = AnyAsVec2(globalVariableValues[str.at(0)]) / AnyAsFloat(otherExpression);
			else
				LogWarning("unrecognized operator \'" + str.at(1) + "\'");
		}
		return 0;
	}
	LogWarning("uninitialized variable or typo in \'" + str.at(0) + "\'");
	return 1;
}

// Holy C type conversion functions
boost::any ExecuteHolyCFunction(const string& functionName, const vector<boost::any>& args)
{
	if (functionName == "ToInt")
		return AnyAsInt(args.at(0));
	else if (functionName == "ToFloat")
		return AnyAsFloat(args.at(0));
	else if (functionName == "ToStr")
		return AnyAsString(args.at(0));
	else if (functionName == "ToBool")
		return AnyAsBool(args.at(0));
	else if (functionName == "typeof" || functionName == "TypeOf")
	{
		// Runtime type checking - returns the type of a value as a string
		if (args.empty())
			return "null";
		return any_type_name(args.at(0));
	}
	else if (functionName == "typecheck" || functionName == "TypeCheck")
	{
		// Runtime type checking - returns true if value matches expected type
		if (args.size() < 2)
			return false;
		string expectedType = AnyAsString(args.at(0));
		string actualType = any_type_name(args.at(1));
		return actualType == expectedType;
	}
	else if (functionName == "istype" || functionName == "IsType")
	{
		// Alias for typecheck
		if (args.size() < 2)
			return false;
		string expectedType = AnyAsString(args.at(0));
		string actualType = any_type_name(args.at(1));
		return actualType == expectedType;
	}
	else if (functionName == "malloc" || functionName == "Malloc")
	{
		// Dynamic memory allocation
		if (args.empty())
			return nullType;
		Pointer ptr = globalMemoryHeap.allocate(args.at(0));
		return ptr;
	}
	else if (functionName == "free" || functionName == "Free")
	{
		// Free allocated memory
		if (args.empty())
			return nullType;
		try {
			Pointer ptr = any_cast<Pointer>(args.at(0));
			globalMemoryHeap.deallocate(ptr);
			return true;
		}
		catch (boost::bad_any_cast) {
			LogWarning("free() requires a pointer argument");
			return false;
		}
	}
	else if (functionName == "addressof" || functionName == "AddressOf" || functionName == "ptr")
	{
		// Get address of a variable (reference/pointer)
		// This creates a pointer to the variable
		if (args.empty())
			return nullType;
		// Allocate in heap to create a persistent reference
		Pointer ptr = globalMemoryHeap.allocate(args.at(0));
		return ptr;
	}
	else if (functionName == "deref" || functionName == "Deref" || functionName == "dereference")
	{
		// Dereference a pointer to get its value
		if (args.empty())
			return nullType;
		try {
			Pointer ptr = any_cast<Pointer>(args.at(0));
			return globalMemoryHeap.dereference(ptr);
		}
		catch (boost::bad_any_cast) {
			LogWarning("deref() requires a pointer argument");
			return nullType;
		}
	}
	else if (functionName == "setvalue" || functionName == "SetValue")
	{
		// Set value at pointer address
		if (args.size() < 2)
			return nullType;
		try {
			Pointer ptr = any_cast<Pointer>(args.at(0));
			globalMemoryHeap.write(ptr, args.at(1));
			return true;
		}
		catch (boost::bad_any_cast) {
			LogWarning("setvalue() requires a pointer and value argument");
			return false;
		}
	}
	else if (functionName == "send" || functionName == "Send")
	{
		// Message passing - send a message to an object
		// Usage: send(object, "methodName", arg1, arg2, ...)
		if (args.size() < 2)
			return nullType;
		try {
			ClassInstance obj = any_cast<ClassInstance>(args.at(0));
			string methodName = AnyAsString(args.at(1));
			
			// Collect remaining arguments for the method
			vector<boost::any> methodArgs;
			for (size_t i = 2; i < args.size(); i++) {
				methodArgs.push_back(args.at(i));
			}
			
			// Look up method in class definition
			if (globalClassDefinitions.find(obj.className) != globalClassDefinitions.end()) {
				ClassDefinition& classDef = globalClassDefinitions[obj.className];
				for (auto& method : classDef.methods) {
					if (method.name == methodName) {
						// Execute the method with proper context
						ClassInstance* oldThisContext = currentThisContext;
						currentThisContext = &obj;
						
						try {
							unordered_map<string, boost::any> methodVariables;
							// Set up method parameters
							for (size_t i = 0; i < method.parameters.size() && i < methodArgs.size(); i++) {
								methodVariables[method.parameters[i]] = methodArgs[i];
							}
							
							// Execute method body
							for (size_t lineIdx = 1; lineIdx < method.body.size(); lineIdx++) {
								int line = lineIdx;
								ProcessLine(method.body, line, methodVariables);
							}
						}
						catch (const std::exception& e) {
							LogWarning("Error executing method: " + string(e.what()));
						}
						
						currentThisContext = oldThisContext;
						return true;
					}
				}
				LogWarning("Method '" + methodName + "' not found in class '" + obj.className + "'");
				return false;
			}
			return false;
		}
		catch (boost::bad_any_cast) {
			LogWarning("send() requires an object as first argument");
			return false;
		}
	}
	else if (functionName == "hasmethod" || functionName == "HasMethod")
	{
		// Check if object has a method
		if (args.size() < 2)
			return false;
		try {
			ClassInstance obj = any_cast<ClassInstance>(args.at(0));
			string methodName = AnyAsString(args.at(1));
			
			if (globalClassDefinitions.find(obj.className) != globalClassDefinitions.end()) {
				ClassDefinition& classDef = globalClassDefinitions[obj.className];
				for (auto& method : classDef.methods) {
					if (method.name == methodName) {
						return true;
					}
				}
			}
			return false;
		}
		catch (boost::bad_any_cast) {
			return false;
		}
	}
	else if (functionName == "getmethod" || functionName == "GetMethod")
	{
		// Get method reference/info from object
		if (args.size() < 2)
			return nullType;
		try {
			ClassInstance obj = any_cast<ClassInstance>(args.at(0));
			string methodName = AnyAsString(args.at(1));
			
			if (globalClassDefinitions.find(obj.className) != globalClassDefinitions.end()) {
				ClassDefinition& classDef = globalClassDefinitions[obj.className];
				for (auto& method : classDef.methods) {
					if (method.name == methodName) {
						// Return method name (in a real system, would return method reference)
						return methodName;
					}
				}
			}
			return nullType;
		}
		catch (boost::bad_any_cast) {
			return nullType;
		}
	}
	// Rust-like Result operations
	else if (functionName == "Ok")
	{
		// Create a successful Result: Ok(value)
		if (args.empty())
			return ResultValue();
		return ResultValue(args.at(0));
	}
	else if (functionName == "Err")
	{
		// Create an error Result: Err(error_message, error_type)
		if (args.empty())
			return ResultValue("Unknown error");
		string errorMsg = AnyAsString(args.at(0));
		string errorType = (args.size() > 1) ? AnyAsString(args.at(1)) : "Error";
		return ResultValue(errorMsg, errorType);
	}
	else if (functionName == "isOk" || functionName == "IsOk")
	{
		// Check if Result is Ok
		if (args.empty())
			return false;
		try {
			ResultValue res = any_cast<ResultValue>(args.at(0));
			return res.isOk;
		}
		catch (boost::bad_any_cast) {
			return false;
		}
	}
	else if (functionName == "isErr" || functionName == "IsErr")
	{
		// Check if Result is Err
		if (args.empty())
			return true;
		try {
			ResultValue res = any_cast<ResultValue>(args.at(0));
			return !res.isOk;
		}
		catch (boost::bad_any_cast) {
			return false;
		}
	}
	else if (functionName == "unwrap" || functionName == "Unwrap")
	{
		// Unwrap Result - panics if Err
		if (args.empty())
			return nullType;
		try {
			ResultValue res = any_cast<ResultValue>(args.at(0));
			if (!res.isOk) {
				LogWarning("Unwrap called on Err: " + res.error);
				return nullType;
			}
			return res.value;
		}
		catch (boost::bad_any_cast) {
			return nullType;
		}
	}
	else if (functionName == "expect" || functionName == "Expect")
	{
		// Unwrap with custom panic message
		if (args.empty())
			return nullType;
		try {
			ResultValue res = any_cast<ResultValue>(args.at(0));
			if (!res.isOk) {
				string msg = (args.size() > 1) ? AnyAsString(args.at(1)) : res.error;
				LogWarning("Expect failed: " + msg);
				return nullType;
			}
			return res.value;
		}
		catch (boost::bad_any_cast) {
			return nullType;
		}
	}
	else if (functionName == "unwrapOr" || functionName == "UnwrapOr")
	{
		// Unwrap Result with default value
		if (args.empty())
			return nullType;
		try {
			ResultValue res = any_cast<ResultValue>(args.at(0));
			if (!res.isOk && args.size() > 1) {
				return args.at(1);  // Return default value
			}
			return res.isOk ? res.value : nullType;
		}
		catch (boost::bad_any_cast) {
			return (args.size() > 1) ? args.at(1) : nullType;
		}
	}
	// Rust-like Option operations
	else if (functionName == "Some")
	{
		// Create an Option with a value: Some(value)
		if (args.empty())
			return OptionValue::None();
		return OptionValue::Some(args.at(0));
	}
	else if (functionName == "None")
	{
		// Create an empty Option: None
		return OptionValue::None();
	}
	else if (functionName == "isSome" || functionName == "IsSome")
	{
		// Check if Option is Some
		if (args.empty())
			return false;
		try {
			OptionValue opt = any_cast<OptionValue>(args.at(0));
			return opt.isSome;
		}
		catch (boost::bad_any_cast) {
			return false;
		}
	}
	else if (functionName == "isNone" || functionName == "IsNone")
	{
		// Check if Option is None
		if (args.empty())
			return true;
		try {
			OptionValue opt = any_cast<OptionValue>(args.at(0));
			return !opt.isSome;
		}
		catch (boost::bad_any_cast) {
			return true;
		}
	}
	else
	{
		LogWarning("Holy C function '" + functionName + "' does not exist.");
		return nullType;
	}
}

boost::any ProcessLine(const vector<vector<string>>& words, int& lineNum, unordered_map<string, boost::any>& variableValues)
{
	//// Check if the first two chars are '//', which would make it a comment
	//if (startsWith(words.at(lineNum).at(0), "//"))
	//	return nullType;

	// If print statement (deprecated, now use ZS.System.Print() function)
	if (IsKeyword(words.at(lineNum).at(0), "print"))
	{
		cout << StringRaw(AnyAsString(EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 1, words.at(lineNum).end())), variableValues))) << endl;
		return nullType;
	}
	
	// Check for Holy C mode string literals
	if (holyCMode && words.at(lineNum).size() == 1 && startsWith(words.at(lineNum).at(0), "\"") && endsWith(words.at(lineNum).at(0), "\""))
	{
		// Auto-print string literals in Holy C mode
		string str = words.at(lineNum).at(0);
		str = str.substr(1, str.length() - 2); // Remove quotes
		cout << StringRaw(str) << endl;
		return nullType;
	}
	
	// Check for #holyc pragma
	if (IsKeyword(words.at(lineNum).at(0), "#holyc") && words.at(lineNum).size() > 1 && IsKeyword(words.at(lineNum).at(1), "on"))
	{
		holyCMode = true;
		return nullType;
	}

	// Check if it is a function return
	else if (IsKeyword(words.at(lineNum).at(0), "return"))
		return EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 1, words.at(lineNum).end())), variableValues);

	// Check if it is ZS Builtin function call
	else if (startsWith(words.at(lineNum).at(0), "ZS."))
		return EvalExpression(unWrapVec(words.at(lineNum)), variableValues);

	// Check if it is function call
	else if (IsFunction(split(words.at(lineNum).at(0), '(')[0]))
	{
		// start -> FuncCall(0, x, OtherFunc(a))
		// changeto -> 0, x, OtherFunc(a)
		string insideFunArgs = betweenChars(unWrapVec(words.at(lineNum)), '(', ')');
		vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
		cout << unWrapVec(argList) << endl;
		printVarValues(argList, variableValues);
#endif
		vector<boost::any> funcArgs = VarValues(argList, variableValues);
		ExecuteFunction(split(words.at(lineNum).at(0), '(')[0], funcArgs);
		return nullType;
	}

	// Check if it is a SplitThread call
	else if (startsWith(words.at(lineNum).at(0), "SplitThread"))
	{
		vector<string> lineContents = words.at(lineNum);
		cout << "New Thread: " << words.at(lineNum).at(0) << endl;
		//lineContents.at(0) = betweenChars(lineContents.at(0), '(', ')');

		//cout << "debug: " << lineContents.at(0) << endl;

		//if (betweenChars(lineContents.at(0), '(', ')') == "")
		//	std::thread thread_obj(ExecuteFunction, trim(split(lineContents.at(0), '(')[0]), vector<boost::any>());
		//else
		//	std::thread thread_obj(ExecuteFunction, trim(split(lineContents.at(0), '(')[0]), VarValues(split(RMParenthesis("(" + split(unWrapVec(rangeInVec(lineContents, 0, (int)lineContents.size() - 2)), '(')[1]), ','), variableValues));
		return nullType;
	}

	// Check if global variable declaration
	else if (IsKeyword(words.at(lineNum).at(0), "global"))
	{
		try
		{
			globalVariableValues[words.at(lineNum).at(2)] = EvalExpression(unWrapVec(slice(words.at(lineNum), 4, -1)), variableValues);
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)) + ", couldn't initialize variable.");
		}
		return nullType;
	}

	// Iterate through all types to see if line inits or
	// re-inits a variable then store it with it's value
	else if (countInVector(types, words.at(lineNum).at(0)) > 0)
	{
		try
		{
			variableValues[words.at(lineNum).at(1)] = EvalExpression(unWrapVec(slice(words.at(lineNum), 3, -1)), variableValues);
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)) + ", couldn't initialize variable.");
		}
		return nullType;
	}

	// Check existing variables: If matches, then it means
	// the variables value is getting changed with an operator
	else if (count(words.at(lineNum).at(0), '.') == 0 && (IsVar(words.at(lineNum).at(0), variableValues) || IsVar(words.at(lineNum).at(0), globalVariableValues)))
	{
		// Evaluates what the operator (ex. '=', '+=') does to the value on the left by the value on the right
		varOperation(vector<string>(words.at(lineNum).begin(), words.at(lineNum).end()), variableValues);
		return nullType;
	}

	// Check existing variables: To see if accessing class sub component
	else if (count(words.at(lineNum).at(0), '.') > 0 && IsVar(split(words.at(lineNum).at(0), '.')[0], variableValues) || IsVar(split(words.at(lineNum).at(0), '.')[0], globalVariableValues))
	{
		if (IsVar(split(words.at(lineNum).at(0), '.')[0], variableValues))
			variableValues[split(words.at(lineNum).at(0), '.')[0]] = EditClassSubComponent(variableValues[split(words.at(lineNum).at(0), '.')[0]], words.at(lineNum).at(1), EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 2, words.at(lineNum).end())), variableValues), split(words.at(lineNum).at(0), '.')[1]);
		else if (IsVar(split(words.at(lineNum).at(0), '.')[0], globalVariableValues))
			globalVariableValues[split(words.at(lineNum).at(0), '.')[0]] = EditClassSubComponent(globalVariableValues[split(words.at(lineNum).at(0), '.')[0]], words.at(lineNum).at(1), EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 2, words.at(lineNum).end())), variableValues), split(words.at(lineNum).at(0), '.')[1]);
		return nullType;
	}

	// If declaring a while loop, loop until false
	else if (IsKeyword(words.at(lineNum).at(0), "while"))
	{
		vector<vector<string>> whileContents;
		vector<string> whileParameters;

		// Gather the parameters that must be == true for the loop to run
		int numOfBrackets = 0;
		for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
			if (count(words.at(lineNum).at(w), '{') == 0)
				whileParameters.push_back(words.at(lineNum)[w]);
			else
			{
				whileParameters.push_back(replace(words.at(lineNum)[w], "{", ""));
				numOfBrackets = 1;
				break;
			}
		}

		// If the statement is already false, don't bother gathering the contents
		if (BooleanLogic(whileParameters.at(0), whileParameters.at(1), whileParameters.at(2), variableValues) == false) {
			lineNum++;
			while (lineNum < (int)words.size())
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets == 0)
					break;
				lineNum++;
			}
			return nullType;
		}

		// Gather the contents
		lineNum++;
		while (lineNum < (int)words.size())
		{
			numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			if (numOfBrackets == 0)
				break;
			whileContents.push_back(words.at(lineNum));
			lineNum++;
		}

		//whileContents = removeTabsWdArry(whileContents, 1);

		// Loop while true
		while (BooleanLogic(whileParameters.at(0), whileParameters.at(1), whileParameters.at(2), variableValues))
		{
			//Iterate through all lines in while loop
			for (int lineNum = 0; lineNum < (int)whileContents.size(); lineNum++)
			{
				if (whileContents.at(lineNum).at(0) == "continue")
					break; // Stops iterating through lines and return to beginning
				if (whileContents.at(lineNum).at(0) == "break")
					return nullType; // Stops iterating through lines and leave while loop
				boost::any returnVal = ProcessLine(whileContents, lineNum, variableValues);
				if (!returnVal.empty()) {
					try
					{
						BREAK t = any_cast<BREAK>(returnVal);
						return nullType;
					}
					catch (boost::bad_any_cast)
					{
						return returnVal;
					}
				}
			}
		}
		return nullType;
	}

	// If declaring an if statement, only execute if true
	else if (IsKeyword(words.at(lineNum).at(0), "if"))
	{
		vector<vector<string>> ifContents;
		vector<string> ifParameters;

		// Gather the parameters that must be == true for the loop to run
		int numOfBrackets = 0;
		for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
			if (count(words.at(lineNum).at(w), '{') == 0)
				ifParameters.push_back(words.at(lineNum)[w]);
			else
			{
				ifParameters.push_back(replace(words.at(lineNum)[w], "{", ""));
				numOfBrackets = 1;
				break;
			}
		}

		// If the statement is already false, don't bother gathering the contents
		if (BooleanLogic(ifParameters.at(0), ifParameters.at(1), ifParameters.at(2), variableValues) == false) {
			lineNum++;
			while (lineNum < (int)words.size())
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets == 0)
					break;
				lineNum++;
			}
			return nullType;
		}

		// Gather the contents
		lineNum++;
		while (lineNum < (int)words.size())
		{
			numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			if (numOfBrackets == 0)
				break;
			ifContents.push_back(words.at(lineNum));
			lineNum++;
		}
		//ifContents = removeTabsWdArry(ifContents, 1);

		// Execute if true
		if (BooleanLogic(ifParameters.at(0), ifParameters.at(1), ifParameters.at(2), variableValues))
		{
			//Iterate through all lines in if statement
			for (int l = 0; l < (int)ifContents.size(); l++)
			{
				if (ifContents.at(l).at(0) == "break")
					return breakReOp; // Stops iterating through lines and leave while loop
				boost::any returnVal = ProcessLine(ifContents, l, variableValues);
				if (!returnVal.empty())
					return returnVal;
			}
		}
		else if (words.size() > lineNum)
		{
			if (IsKeyword(words[lineNum].at(0), "else"))
			{
				vector<vector<string>> elseContents;
				vector<string> elseParameters;

				int numOfBrackets = 0;
				for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
					if (count(words.at(lineNum).at(w), '{') != 0)
					{
						numOfBrackets = 1;
						break;
					}
				}

				lineNum++;
				while (lineNum < (int)words.size())
				{
					numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
					if (numOfBrackets == 0)
						break;
					elseContents.push_back(words.at(lineNum));
					lineNum++;
				}

				//elseContents = removeTabsWdArry(elseContents, 1);

				//Iterate through all lines in else statement
				for (int l = 0; l < (int)elseContents.size(); l++)
				{
					boost::any returnVal = ProcessLine(elseContents, l, variableValues);
					if (!returnVal.empty())
						return returnVal;
				}

			}
		}
		return nullType;
	}
	return nullType;
}

boost::any ExecuteFunction(const string& functionName, const vector<boost::any>& inputVarVals)
{
	// Get contents of function from global function map
	std::vector<std::vector<std::string>> words = functionValues[functionName];

	unordered_map<string, boost::any> variableValues = {};

	std::vector<std::string> funcArgs = words.at(0);
	// Set function variables equal to whatever inputs were provided
	for (int i = 0; i < (int)inputVarVals.size(); i++)
	{
		if (i < funcArgs.size())
		{
			variableValues[funcArgs[i]] = inputVarVals[i];
#if DEVELOPER_MESSAGES == true
			cout << "in " << functionName + "  " << funcArgs[i] << " == " << AnyAsString(inputVarVals[i]) << endl;
#endif
		}
	}

	//Iterate through all lines in function
	for (int lineNum = 1; lineNum < (int)words.size(); lineNum++)
	{
		try
		{
			boost::any returnVal = ProcessLine(words, lineNum, variableValues);
			if (!returnVal.empty())
				return returnVal;
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)));
		}
	}
	return nullType;
}

// REPL (Read-Eval-Print Loop) for shell mode
void RunREPL()
{
	cout << "Holy Z Interactive Shell (Holy C Enhanced)" << endl;
	cout << "Type 'exit' to quit, '#holyc on' to enable Holy C mode" << endl;
	cout << ">>> ";
	
	string input;
	unordered_map<string, boost::any> replVariables;
	
	while (getline(cin, input))
	{
		if (input == "exit" || input == "quit")
			break;
			
		if (trim(input).empty())
		{
			cout << ">>> ";
			continue;
		}
		
		try
		{
			// Handle pragma directives
			if (startsWith(trim(input), "#holyc"))
			{
				if (input.find("on") != string::npos)
				{
					holyCMode = true;
					cout << "Holy C mode enabled" << endl;
				}
				else if (input.find("off") != string::npos)
				{
					holyCMode = false;
					cout << "Holy C mode disabled" << endl;
				}
			}
			// Handle expressions and statements
			else
			{
				// Try to evaluate as expression first
				boost::any result = EvalExpression(input, replVariables);
				if (!result.empty())
				{
					string resultStr = AnyAsString(result);
					if (!resultStr.empty() && resultStr != "0")
						cout << resultStr << endl;
				}
			}
		}
		catch (const std::exception& e)
		{
			cout << "Error: " << e.what() << endl;
		}
		catch (...)
		{
			cout << "Unknown error occurred" << endl;
		}
		
		cout << ">>> ";
	}
}

int parseHolyZ(string script)
{
	//script = replace(script, "    ", "\t"); // Replace spaces with tabs (not really required, and will break purposefull whitespace in strings etc.)

	// Split the script by newline, signifying a line ending
	vector<string> beforeProcessLines = split(script, '\n');
	vector<string> lines;
	for (int i = 0; i < (int)beforeProcessLines.size(); i++) { // Then split said lines into indiviual words
		if (!startsWith(trim(beforeProcessLines.at(i)), "//") && trim(beforeProcessLines.at(i)) != "")
		{ // dont include line if it is a comment or if it is blank
			lines.push_back(trim(beforeProcessLines.at(i)));
		}
	}
#if DEVELOPER_MESSAGES
	InterpreterLog("Contents:\n");
#endif
	vector<vector<string>> words;
	for (int i = 0; i < (int)lines.size(); i++) // Then split said lines into indiviual words
	{
		words.push_back(split(lines.at(i), ' '));
#if DEVELOPER_MESSAGES
		cout << unWrapVec(words.at(i)) << endl;
#endif
	}

#if DEVELOPER_MESSAGES
	InterpreterLog("Gather variables & functions...");
#endif
	// Go through entire script and iterate through all types to see if line is a variable
	// or function declaration, then store it with it's value
	for (int lineNum = 0; lineNum < (int)words.size(); lineNum++)
	{
		//Checks if it is function
		if (IsKeyword(words.at(lineNum).at(0), "func"))
		{
			vector<vector<string>> functionContents;

			string functName = split(words.at(lineNum).at(1), '(')[0];
#if DEVELOPER_MESSAGES == true
			InterpreterLog("Load script function " + functName + "...");
#endif

			//string args = "";
			//if (indexInStr(unWrapVec(words.at(lineNum)), ')') - indexInStr(unWrapVec(words.at(lineNum)), '(') > 1)
			//	for (int w = 0; w < (int)words.at(lineNum).size(); w++) // Get all words from the instantiation line: these are the args
			//	{
			//		if (count(words.at(lineNum).at(w), '{') == 0)
			//			args += replace(replace(words.at(lineNum).at(w), "(", " "), ")", "");
			//	}
			string args = betweenChars(unWrapVec(words.at(lineNum)), '(', ')');
			//cout << functName << "<" << args << ">" << endl;

			//args = trim(replace(args, functName, ""));
			functionContents.push_back(split(replace(args, " ", ""), ','));


			int numOfBrackets = countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			// Gather the contents
			lineNum++;
			while (lineNum < (int)words.size())
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets == 0)
					break;
				functionContents.push_back(words.at(lineNum));
				//cout << functName << "<" << args << ">" << endl;
				lineNum++;
			}
			
			functionValues[functName] = functionContents;
		}
		else if (toLower(words.at(lineNum).at(0)) == "class")
		{
			string className = words.at(lineNum).at(1);
			ClassDefinition classDef(className);
			
			// Parse class body for methods and attributes
			int numOfBrackets = countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			lineNum++;
			vector<vector<string>> classBodyLines;
			
			while (lineNum < (int)words.size() && numOfBrackets > 0)
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets > 0) {
					classBodyLines.push_back(words.at(lineNum));
				}
				lineNum++;
			}
			
			// Parse methods and attributes from class body
			for (size_t i = 0; i < classBodyLines.size(); i++) {
				if (classBodyLines[i].size() > 0) {
					// Check if this is a method (contains parentheses)
					string firstWord = unWrapVec(classBodyLines[i]);
					bool isMethod = false;
					vector<string> methodParams;
					string methodName;
					
					for (const auto& word : classBodyLines[i]) {
						if (word.find('(') != string::npos && word.find(')') != string::npos) {
							isMethod = true;
							// Extract method name and parameters
							methodName = word.substr(0, word.find('('));
							string paramStr = betweenChars(word, '(', ')');
							if (!paramStr.empty()) {
								methodParams = split(paramStr, ',');
							}
							break;
						}
					}
					
					if (isMethod && !methodName.empty()) {
						// Collect method body
						vector<vector<string>> methodBody;
						methodBody.push_back(classBodyLines[i]);
						i++;
						int methodBrackets = countInVector(classBodyLines[i-1], "{") - countInVector(classBodyLines[i-1], "}");
						
						while (i < classBodyLines.size() && methodBrackets > 0) {
							methodBrackets += countInVector(classBodyLines[i], "{") - countInVector(classBodyLines[i], "}");
							methodBody.push_back(classBodyLines[i]);
							i++;
						}
						i--; // Adjust for loop increment
						
						ClassMethod method(methodName, methodParams, methodBody);
						classDef.methods.push_back(method);
					} else {
						// This is likely an attribute
						if (classBodyLines[i].size() >= 2) {
							string attrType = classBodyLines[i].at(0);
							string attrName = classBodyLines[i].at(1);
							if (attrName.back() == ';') {
								attrName.pop_back();
							}
							ClassAttribute attr(attrName, attrType);
							classDef.attributes.push_back(attr);
						}
					}
				}
			}
			
			globalClassDefinitions[className] = classDef;
#if DEVELOPER_MESSAGES == true
			InterpreterLog("Load script class " + className + "...");
#endif
		}
		else
		{
			if (IsKeyword(words.at(lineNum).at(0), "include"))
			{
				string scriptPath = StringRaw(words.at(lineNum).at(1));
				string scriptTextContents;
#if DEVELOPER_MESSAGES == true
				InterpreterLog("Including from " + words.at(lineNum).at(1) + "...");
#endif
#if UNIX
				// Get script contents as single string
				auto ss = ostringstream{};
				ifstream input_file(scriptPath);
				ss << input_file.rdbuf();
				scriptTextContents = ss.str();
#if DEVELOPER_MESSAGES
				InterpreterLog("Gather script contents...");
#endif
#elif WINDOWS
				// Get script contents as single string
				ifstream script(scriptPath);
				stringstream scriptString;
				scriptString << script.rdbuf();
				scriptTextContents = scriptString.str();
#if DEVELOPER_MESSAGES
				InterpreterLog("Gather script contents...");
#endif
#endif
				parseHolyZ(scriptTextContents);
			}


			else if (IsKeyword(words.at(lineNum).at(0), "string")) {
				globalVariableValues[words.at(lineNum).at(1)] = StringRaw(words.at(lineNum).at(3));
#if DEVELOPER_MESSAGES == true
				InterpreterLog("Load script variable " + words.at(lineNum).at(1) + "...");
#endif
			}

			// Iterate through all types to see if line inits or
			// re-inits a variable then store it with it's value
			else if (countInVector(types, trim(words.at(lineNum).at(0))) > 0)
			{
				//cout << words.at(lineNum).at(1) << "=" << unWrapVec(slice(words.at(lineNum), 3, -1)) << "=" << AnyAsString(EvalExpression(unWrapVec(slice(words.at(lineNum), 3, -1)), variableValues)) << endl;
				globalVariableValues[words.at(lineNum).at(1)] = EvalExpression(unWrapVec(slice(words.at(lineNum), 3, -1)), globalVariableValues);
			}
			//			else if (words.at(lineNum).at(0) == "int") {
			//				globalVariableValues[words.at(lineNum).at(1)] = stoi(words.at(lineNum).at(3));
			//#if DEVELOPER_MESSAGES == true
			//				InterpreterLog("Load script variable " + words.at(lineNum).at(1) + "...");
			//#endif
			//			}
			//			else if (words.at(lineNum).at(0) == "float") {
			//				globalVariableValues[words.at(lineNum).at(1)] = stof(words.at(lineNum).at(3));
			//#if DEVELOPER_MESSAGES == true
			//				InterpreterLog("Load script variable " + words.at(lineNum).at(1) + "...");
			//#endif
			//			}
			//			else if (words.at(lineNum).at(0) == "bool") {
			//				globalVariableValues[words.at(lineNum).at(1)] = stob(words.at(lineNum).at(3));
			//#if DEVELOPER_MESSAGES == true
			//				InterpreterLog("Load script variable " + words.at(lineNum).at(1) + "...");
			//#endif
			//			}
			else
				LogWarning("unrecognized type \'" + words.at(lineNum).at(0) + "\' on line: " + to_string(lineNum));
		}
	}

	return 0;
}
// Holy C type conversion functions
boost::any ExecuteHolyCFunction(const string& functionName, const vector<boost::any>& args)
{
	if (functionName == "ToInt")
		return AnyAsInt(args.at(0));
	else if (functionName == "ToFloat")
		return AnyAsFloat(args.at(0));
	else if (functionName == "ToStr")
		return AnyAsString(args.at(0));
	else if (functionName == "ToBool")
		return AnyAsBool(args.at(0));
	else
	{
		LogWarning("Holy C function '" + functionName + "' does not exist.");
		return nullType;
	}
}

{
	//// Check if the first two chars are '//', which would make it a comment
	//if (startsWith(words.at(lineNum).at(0), "//"))
	//	return nullType;

	// If print statement (deprecated, now use ZS.System.Print() function)
	if (IsKeyword(words.at(lineNum).at(0), "print"))
	if (holyCMode && words.at(lineNum).size() == 1 && startsWith(words.at(lineNum).at(0), "\"") && endsWith(words.at(lineNum).at(0), "\""))
	{
		// Auto-print string literals in Holy C mode
		string str = words.at(lineNum).at(0);
		str = str.substr(1, str.length() - 2); // Remove quotes
		cout << StringRaw(str) << endl;
		return nullType;
	}
	
	// Check for #holyc pragma
	if (words.at(lineNum).at(0) == "#holyc" && words.at(lineNum).size() > 1 && words.at(lineNum).at(1) == "on")
	{
		holyCMode = true;
		return nullType;
	}
	
	{
		cout << StringRaw(AnyAsString(EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 1, words.at(lineNum).end())), variableValues))) << endl;
		return nullType;
	}

	// Check if it is a function return
	else if (words.at(lineNum).at(0) == "return")
		return EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 1, words.at(lineNum).end())), variableValues);

	// Check if it is ZS Builtin function call
	else if (startsWith(words.at(lineNum).at(0), "ZS."))
		return EvalExpression(unWrapVec(words.at(lineNum)), variableValues);

	// Check if it is function call
	else if (IsFunction(split(words.at(lineNum).at(0), '(')[0]))
	{
		// start -> FuncCall(0, x, OtherFunc(a))
		// changeto -> 0, x, OtherFunc(a)
		string insideFunArgs = betweenChars(unWrapVec(words.at(lineNum)), '(', ')');
		vector<string> argList = splitNoOverlap(insideFunArgs, ',', '(', ')');
#if DEVELOPER_MESSAGES == true
		cout << unWrapVec(argList) << endl;
		printVarValues(argList, variableValues);
#endif
		vector<boost::any> funcArgs = VarValues(argList, variableValues);
		ExecuteFunction(split(words.at(lineNum).at(0), '(')[0], funcArgs);
		return nullType;
	}

	// Check if it is a SplitThread call
	else if (startsWith(words.at(lineNum).at(0), "SplitThread"))
	{
		vector<string> lineContents = words.at(lineNum);
		cout << "New Thread: " << words.at(lineNum).at(0) << endl;
		//lineContents.at(0) = betweenChars(lineContents.at(0), '(', ')');

		//cout << "debug: " << lineContents.at(0) << endl;

		//if (betweenChars(lineContents.at(0), '(', ')') == "")
		//	std::thread thread_obj(ExecuteFunction, trim(split(lineContents.at(0), '(')[0]), vector<boost::any>());
		//else
		//	std::thread thread_obj(ExecuteFunction, trim(split(lineContents.at(0), '(')[0]), VarValues(split(RMParenthesis("(" + split(unWrapVec(rangeInVec(lineContents, 0, (int)lineContents.size() - 2)), '(')[1]), ','), variableValues));
		return nullType;
	}

	// Check if global variable declaration
	else if (IsKeyword(words.at(lineNum).at(0), "global"))
	{
		try
		{
			globalVariableValues[words.at(lineNum).at(2)] = EvalExpression(unWrapVec(slice(words.at(lineNum), 4, -1)), variableValues);
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)) + ", couldn't initialize variable.");
		}
		return nullType;
	}

	// Iterate through all types to see if line inits or
	// re-inits a variable then store it with it's value
	else if (countInVector(types, words.at(lineNum).at(0)) > 0)
	{
		try
		{
			variableValues[words.at(lineNum).at(1)] = EvalExpression(unWrapVec(slice(words.at(lineNum), 3, -1)), variableValues);
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)) + ", couldn't initialize variable.");
		}
		return nullType;
	}

	// Check existing variables: If matches, then it means
	// the variables value is getting changed with an operator
	else if (count(words.at(lineNum).at(0), '.') == 0 && (IsVar(words.at(lineNum).at(0), variableValues) || IsVar(words.at(lineNum).at(0), globalVariableValues)))
	{
		// Evaluates what the operator (ex. '=', '+=') does to the value on the left by the value on the right
		varOperation(vector<string>(words.at(lineNum).begin(), words.at(lineNum).end()), variableValues);
		return nullType;
	}

	// Check existing variables: To see if accessing class sub component
	else if (count(words.at(lineNum).at(0), '.') > 0 && IsVar(split(words.at(lineNum).at(0), '.')[0], variableValues) || IsVar(split(words.at(lineNum).at(0), '.')[0], globalVariableValues))
	{
		if (IsVar(split(words.at(lineNum).at(0), '.')[0], variableValues))
			variableValues[split(words.at(lineNum).at(0), '.')[0]] = EditClassSubComponent(variableValues[split(words.at(lineNum).at(0), '.')[0]], words.at(lineNum).at(1), EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 2, words.at(lineNum).end())), variableValues), split(words.at(lineNum).at(0), '.')[1]);
		else if (IsVar(split(words.at(lineNum).at(0), '.')[0], globalVariableValues))
			globalVariableValues[split(words.at(lineNum).at(0), '.')[0]] = EditClassSubComponent(globalVariableValues[split(words.at(lineNum).at(0), '.')[0]], words.at(lineNum).at(1), EvalExpression(unWrapVec(vector<string>(words.at(lineNum).begin() + 2, words.at(lineNum).end())), variableValues), split(words.at(lineNum).at(0), '.')[1]);
		return nullType;
	}

	// If declaring a while loop, loop until false
	else if (IsKeyword(words.at(lineNum).at(0), "while"))
	{
		vector<vector<string>> whileContents;
		vector<string> whileParameters;

		// Gather the parameters that must be == true for the loop to run
		int numOfBrackets = 0;
		for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
			if (count(words.at(lineNum).at(w), '{') == 0)
				whileParameters.push_back(words.at(lineNum)[w]);
			else
			{
				whileParameters.push_back(replace(words.at(lineNum)[w], "{", ""));
				numOfBrackets = 1;
				break;
			}
		}

		// If the statement is already false, don't bother gathering the contents
		if (BooleanLogic(whileParameters.at(0), whileParameters.at(1), whileParameters.at(2), variableValues) == false) {
			lineNum++;
			while (lineNum < (int)words.size())
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets == 0)
					break;
				lineNum++;
			}
			return nullType;
		}

		// Gather the contents
		lineNum++;
		while (lineNum < (int)words.size())
		{
			numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			if (numOfBrackets == 0)
				break;
			whileContents.push_back(words.at(lineNum));
			lineNum++;
		}

		//whileContents = removeTabsWdArry(whileContents, 1);

		// Loop while true
		while (BooleanLogic(whileParameters.at(0), whileParameters.at(1), whileParameters.at(2), variableValues))
		{
			//Iterate through all lines in while loop
			for (int lineNum = 0; lineNum < (int)whileContents.size(); lineNum++)
			{
				if (whileContents.at(lineNum).at(0) == "continue")
					break; // Stops iterating through lines and return to beginning
				if (whileContents.at(lineNum).at(0) == "break")
					return nullType; // Stops iterating through lines and leave while loop
				boost::any returnVal = ProcessLine(whileContents, lineNum, variableValues);
				if (!returnVal.empty()) {
					try
					{
						BREAK t = any_cast<BREAK>(returnVal);
						return nullType;
					}
					catch (boost::bad_any_cast)
					{
						return returnVal;
					}
				}
			}
		}
		return nullType;
	}

	// If declaring an if statement, only execute if true
	else if (IsKeyword(words.at(lineNum).at(0), "if"))
	{
		vector<vector<string>> ifContents;
		vector<string> ifParameters;

		// Gather the parameters that must be == true for the loop to run
		int numOfBrackets = 0;
		for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
			if (count(words.at(lineNum).at(w), '{') == 0)
				ifParameters.push_back(words.at(lineNum)[w]);
			else
			{
				ifParameters.push_back(replace(words.at(lineNum)[w], "{", ""));
				numOfBrackets = 1;
				break;
			}
		}

		// If the statement is already false, don't bother gathering the contents
		if (BooleanLogic(ifParameters.at(0), ifParameters.at(1), ifParameters.at(2), variableValues) == false) {
			lineNum++;
			while (lineNum < (int)words.size())
			{
				numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
				if (numOfBrackets == 0)
					break;
				lineNum++;
			}
			return nullType;
		}

		// Gather the contents
		lineNum++;
		while (lineNum < (int)words.size())
		{
			numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
			if (numOfBrackets == 0)
				break;
			ifContents.push_back(words.at(lineNum));
			lineNum++;
		}
		//ifContents = removeTabsWdArry(ifContents, 1);

		// Execute if true
		if (BooleanLogic(ifParameters.at(0), ifParameters.at(1), ifParameters.at(2), variableValues))
		{
			//Iterate through all lines in if statement
			for (int l = 0; l < (int)ifContents.size(); l++)
			{
				if (ifContents.at(l).at(0) == "break")
					return breakReOp; // Stops iterating through lines and leave while loop
				boost::any returnVal = ProcessLine(ifContents, l, variableValues);
				if (!returnVal.empty())
					return returnVal;
			}
		}
		else if (words.size() > lineNum)
		{
			if (IsKeyword(words[lineNum].at(0), "else"))
			{
				vector<vector<string>> elseContents;
				vector<string> elseParameters;

				int numOfBrackets = 0;
				for (int w = 1; w < (int)words.at(lineNum).size(); w++) {
					if (count(words.at(lineNum).at(w), '{') != 0)
					{
						numOfBrackets = 1;
						break;
					}
				}

				lineNum++;
				while (lineNum < (int)words.size())
				{
					numOfBrackets += countInVector(words.at(lineNum), "{") - countInVector(words.at(lineNum), "}");
					if (numOfBrackets == 0)
						break;
					elseContents.push_back(words.at(lineNum));
					lineNum++;
				}

				//elseContents = removeTabsWdArry(elseContents, 1);

				//Iterate through all lines in else statement
				for (int l = 0; l < (int)elseContents.size(); l++)
				{
					boost::any returnVal = ProcessLine(elseContents, l, variableValues);
					if (!returnVal.empty())
						return returnVal;
				}

			}
		}
		return nullType;
	}
	return nullType;
}

boost::any ExecuteFunction(const string& functionName, const vector<boost::any>& inputVarVals)
{
	// Get contents of function from global function map
	std::vector<std::vector<std::string>> words = functionValues[functionName];

	unordered_map<string, boost::any> variableValues = {};

	std::vector<std::string> funcArgs = words.at(0);
	// Set function variables equal to whatever inputs were provided
	for (int i = 0; i < (int)inputVarVals.size(); i++)
	{
		if (i < funcArgs.size())
		{
			variableValues[funcArgs[i]] = inputVarVals[i];
#if DEVELOPER_MESSAGES == true
			cout << "in " << functionName + "  " << funcArgs[i] << " == " << AnyAsString(inputVarVals[i]) << endl;
#endif
		}
	}

	//Iterate through all lines in function
	for (int lineNum = 1; lineNum < (int)words.size(); lineNum++)
	{
		try
		{
			boost::any returnVal = ProcessLine(words, lineNum, variableValues);
			if (!returnVal.empty())
				return returnVal;
		}
		catch (const std::exception&)
		{
			LogCriticalError("Error at line: " + to_string(lineNum) + ", " + unWrapVec(words.at(lineNum)));
		}
	}
	return nullType;
}


