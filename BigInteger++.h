#include <stdio.h>
#include <vector>
#include <string>

#pragma once

#ifndef BIG_INTEGER
#define BIG_INTEGER

// CONSTANTS:
// Each of the following was replaced from [#define name value] by MS-VS
constexpr auto E9			= 1000000000;
constexpr auto E18			= 1000000000000000000LL;
constexpr auto LOW_DWORD	= 0x00000000FFFFFFFFLL;
constexpr auto HIGH_DWORD	= 0xFFFFFFFF00000000LL;
constexpr auto LOW_WORD		= 0x0000FFFF;
constexpr auto HIGH_WORD	= 0xFFFF0000;

constexpr auto ABSOLUTE_MAX_SIZE = 134217728;

class unsignedBigInteger
{
//=========================================================================================================================
// Constructors and Destructor
//=========================================================================================================================
public:
	unsignedBigInteger();
	unsignedBigInteger(const unsignedBigInteger&);
	unsignedBigInteger(unsigned long long);

	unsignedBigInteger(std::string, int base = 10); // Allowing the base to be decimal or hexadecimal

	~unsignedBigInteger();

//=========================================================================================================================
// Assign and Subscript Operators(=, []) :
//=========================================================================================================================
public:
	unsignedBigInteger& operator=(const unsignedBigInteger&);
	unsignedBigInteger& operator=(unsigned long long);

	// This function accesses the binaryContents by reference (simpler code)
	inline unsigned long long& operator[](unsigned int);

//=========================================================================================================================
// Arithmatic Operators (+, -, *, /, %, +=, -=, *=, /=, %=, ++, --) and Arithmatic Functions:
//=========================================================================================================================
public:
	unsignedBigInteger operator+(unsignedBigInteger&);
	unsignedBigInteger operator-(unsignedBigInteger&);
	unsignedBigInteger operator*(unsignedBigInteger&);
	unsignedBigInteger operator/(unsignedBigInteger&);
	unsignedBigInteger operator%(unsignedBigInteger&);

	unsignedBigInteger operator+(unsigned long long);
	unsignedBigInteger operator-(unsigned long long);
	unsignedBigInteger operator*(unsigned long long);
	unsignedBigInteger operator/(unsigned long long);
	unsignedBigInteger operator%(unsigned long long);

	unsignedBigInteger& operator+=(unsignedBigInteger&);
	unsignedBigInteger& operator-=(unsignedBigInteger&);
	unsignedBigInteger& operator*=(unsignedBigInteger&);
	unsignedBigInteger& operator/=(unsignedBigInteger&);
	unsignedBigInteger& operator%=(unsignedBigInteger&);

	unsignedBigInteger& operator+=(unsigned long long);
	unsignedBigInteger& operator-=(unsigned long long);
	unsignedBigInteger& operator*=(unsigned long long);
	unsignedBigInteger& operator/=(unsigned long long);
	unsignedBigInteger& operator%=(unsigned long long);

	unsignedBigInteger& operator++();
	unsignedBigInteger& operator--();

	// Divide functions return whether the operation was successful
	friend bool Divide(unsignedBigInteger& dividend, unsignedBigInteger& divisor,
		unsignedBigInteger& quotient, unsignedBigInteger& remainder);
	friend bool Divide(unsignedBigInteger& dividend, unsigned int& divisor,
		unsignedBigInteger& quotient, unsigned int& remainder);

	unsignedBigInteger& FastPower(unsigned long long exponent);
	unsignedBigInteger& FastPower(unsignedBigInteger& exponent);

//=========================================================================================================================
// Comparison Operators (<, <=, >, >=, ==, !=) and Comparison Functions:
//=========================================================================================================================
public:
	bool operator< (unsignedBigInteger&);
	bool operator<=(unsignedBigInteger&);
	bool operator> (unsignedBigInteger&);
	bool operator>=(unsignedBigInteger&);
	bool operator==(unsignedBigInteger&);
	bool operator!=(unsignedBigInteger&);

	bool operator< (unsigned long long);
	bool operator<=(unsigned long long);
	bool operator> (unsigned long long);
	bool operator>=(unsigned long long);
	bool operator==(unsigned long long);
	bool operator!=(unsigned long long);

private:
	// Main Comparison Functions: (all comparison operators call them)
	signed int CompareWith(unsignedBigInteger&);
	signed int CompareWith(unsigned long long);
	
//=========================================================================================================================
// Bitwise Operators (|, &, ^, |=, &=, ^=, ~) :
//=========================================================================================================================
public:
	unsignedBigInteger operator|(unsignedBigInteger&);
	unsignedBigInteger operator&(unsignedBigInteger&);
	unsignedBigInteger operator^(unsignedBigInteger&);

	unsignedBigInteger operator|(unsigned long long);
	unsignedBigInteger operator&(unsigned long long);
	unsignedBigInteger operator^(unsigned long long);

	unsignedBigInteger& operator|=(unsignedBigInteger&);
	unsignedBigInteger& operator&=(unsignedBigInteger&);
	unsignedBigInteger& operator^=(unsignedBigInteger&);

	unsignedBigInteger& operator|=(unsigned long long);
	unsignedBigInteger& operator&=(unsigned long long);
	unsignedBigInteger& operator^=(unsigned long long);

	unsignedBigInteger& operator~();

//=========================================================================================================================
// Shift Operators (<<, >>, <<=, >>=) and Shifting Functions:
//=========================================================================================================================
public:
	unsignedBigInteger operator<<(unsigned long long);
	unsignedBigInteger operator>>(unsigned long long);

	unsignedBigInteger operator<<(unsignedBigInteger&);
	unsignedBigInteger operator>>(unsignedBigInteger&);

	unsignedBigInteger& operator<<=(unsigned long long);
	unsignedBigInteger& operator>>=(unsigned long long);

	unsignedBigInteger& operator<<=(unsignedBigInteger&);
	unsignedBigInteger& operator>>=(unsignedBigInteger&);

private:
	// Shift by the number of 64-bit elements
	unsignedBigInteger& ShiftRightBy(unsigned int); 
	unsignedBigInteger& ShiftLeftBy(unsigned int);

//=========================================================================================================================
// Printing Functions:
//=========================================================================================================================
public:
	// Printing Functions:
	bool PrintAsDecimal(char charAfter = '\0');
	bool PrintAsHex(char charAfter = '\0');
	bool PrintAsBinary(char charAfter = '\0');

//=========================================================================================================================
// Sizing Functions:
//=========================================================================================================================
public:
	unsigned int NumberOfDigits();
	unsigned int NumberOfBits();
	inline unsigned unsigned long long Size();
	inline unsigned unsigned long long GetMaximumSize();

private:
	bool Resize(unsigned int newSize, bool extendMaxSize = false);
	bool ShrinkContents();
	std::vector<unsigned long long> GetExpandedContents();  // This function will get a vector of the binaryContents after expansion (every 32-bit in a different element)
	unsigned int RemoveTrailingZeros();				        // return the number of trailing zeros and remove them (helpful for multiplication and division)

//=========================================================================================================================
// Converting Functions:
//=========================================================================================================================
public:
	inline unsigned int ToUInt();
	inline unsigned long long ToULongLong();
	std::string ConvertToString(unsigned int base); // valid values for base are only 10 and 16
	bool ConvertToDecimal();
	bool ConvertFromStringDecimal(std::string);
	bool ConvertFromStringHex(std::string);

//=========================================================================================================================
// All Members:
//=========================================================================================================================
private:
	// Quantity Holders:
	std::vector<unsigned long long> binaryContents;		// each element contains a 64-bit part of the number starting from 0 at least significant
	std::vector<unsigned int> decimalContents;			// each element contains a 9-digit part of the number starting from 0 at least significant

	// Limits:
	unsigned long long MAX_SIZE = 32768;				// up to 32768 x 8		bytes for binaryContents (256 KB)
	/* This is moved to the beginning of the file
	unsigned int ABSOLUTE_MAX_SIZE = 134217728;			// up to 134217728 x 8	bytes for binaryContents ( 1  GB)
	//*/

	// Flags to check what to do with decimalContents: [TODO: Un-implemented functionality]
	bool isConvertedToDecimal = false;		// This will be true if decimalContents actually represent the current number.
	bool alwaysConvertToDecimal = false;	// If this is true, all operations will change the value of decimalContents.
};

#endif //  !BIG_INTEGER
