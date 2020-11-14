#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "BigInteger++.h"

//=========================================================================================================================
// Constructors and Destructor
//=========================================================================================================================

unsignedBigInteger::unsignedBigInteger()
{
	Resize(1);
	binaryContents[0] = 0;
	decimalContents.resize(1);
	decimalContents[0] = 0;
	isConvertedToDecimal = true;
}

unsignedBigInteger::unsignedBigInteger(const unsignedBigInteger& other)
{
	binaryContents = other.binaryContents;

	if (other.isConvertedToDecimal) {
		decimalContents = other.decimalContents;
		isConvertedToDecimal = true;
	}
}

unsignedBigInteger::unsignedBigInteger(unsigned long long other)
{
	Resize(1);
	binaryContents[0] = other;
}

unsignedBigInteger::unsignedBigInteger(std::string str, int base)
{
	if (base == 10)
		if (ConvertFromStringDecimal(str))
			return;
	if (base == 16)
		if (ConvertFromStringHex(str))
			return;
	Resize(1);
	binaryContents[0] = 0;
}

unsignedBigInteger::~unsignedBigInteger()
{
	binaryContents.~vector();
	decimalContents.~vector();
}

//=========================================================================================================================
// Assign and Subscript Operators (=, []) :
//=========================================================================================================================

unsignedBigInteger& unsignedBigInteger::operator=(const unsignedBigInteger& other)
{
	binaryContents = other.binaryContents;

	if (other.isConvertedToDecimal) {
		decimalContents = other.decimalContents;
		isConvertedToDecimal = true;
	}
	else isConvertedToDecimal = false;

	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator=(unsigned long long other)
{
	Resize(1);
	binaryContents[0] = other;
	return *this;
}

// This function accesses the binary contents by reference (easier code)
inline unsigned long long& unsignedBigInteger::operator[](unsigned int binaryContentsIndex)
{
	return binaryContents[binaryContentsIndex];
}

//=========================================================================================================================
// Arithmatic Operators (+, -, *, /, %, +=, -=, *=, /=, %=, ++, --) and Arithmatic Functions:
// 
// Each operator (other than ++, --) has 2 overloads, one with a big integer as an input and
// the other one with a long long (64-bit int) as an input. This is also true for the following 
// types of operators. (comparison, bitwise and shift)
//=========================================================================================================================

// [TODO: Check the resulting size of each arithmatic operation beforehand, at least the addition, multiplication and FastPower function]

unsignedBigInteger unsignedBigInteger::operator+(unsignedBigInteger& other)
{
	// These two pointers will point to (this) and (other) depending on how many elements are there in each of them (the size of binaryContents).
	// The greaterNumber is not necessarily greater if they have the same number of elements, and it does not have to be.
	unsignedBigInteger* greaterNumber;
	unsignedBigInteger* smallerNumber;

	unsigned int resultSize;
	if (Size() >= other.Size()) {
		resultSize = Size();
		greaterNumber = this;
		smallerNumber = &other;
	}
	else {
		resultSize = other.Size();
		greaterNumber = &other;
		smallerNumber = this;
	}

	// Construct a big integer with the maximum possible number of 64-bit elements:
	unsignedBigInteger result;
	result.Resize(resultSize);
	bool carry = 0;

	// Add all the elements up to the size of the smaller number.
	for (unsigned int i = 0; i < smallerNumber->Size(); i++) {
		result[i] = (*greaterNumber)[i] + (*smallerNumber)[i] + carry;
		// if the current element of the result is strictly smaller than any of the 2 inputs',
		// then an overflow has occurred and a carry is added to the next element.
		carry = result[i] < (*greaterNumber)[i] || result[i] < (*smallerNumber)[i] ||
			(result[i] == (*greaterNumber)[i] && carry); // <- This condition is a special case [0xFF + 0xFF + (carry=1) = 0x1FF]
	}
	
	// Add the rest of the elements of the greater number. Keep adding the carry along (e.g. 1+2999 = 3000)
	for (unsigned int i = smallerNumber->Size(); i < greaterNumber->Size(); i++) {
		result[i] = (*greaterNumber)[i] + carry;
		if (result[i] != 0)
			carry = 0;
	}

	// Check if the last calculation had a carry
	if (carry)
		result.binaryContents.push_back(carry);

	return result;
}

unsignedBigInteger unsignedBigInteger::operator-(unsignedBigInteger& other)
{
	if ((*this) < other)
		return unsignedBigInteger(0); // no negative values are allowed.

	unsignedBigInteger result;
	result.Resize(Size());
	bool borrow = 0;
	for (unsigned int i = 0; i < other.Size(); i++) {
		result[i] = binaryContents[i] - other[i] - borrow;
		// if the current element of the result is strictly greater than binaryContents[i], or equal and the borrow is 1,
		// then an overflow has occurred and a borrow is subtracted from the next element.
		borrow = result[i] > binaryContents[i] ||
			(result[i] == binaryContents[i] && borrow); // e.g. [0xYZ - 0xFF - (borrow=1) = 0xYZ] (mod 256)
	}

	for (unsigned int i = other.Size(); i < Size(); i++) {
		result[i] = binaryContents[i] - borrow;
		if (binaryContents[i] != 0)
			borrow = 0;
	}

	// Check if the last calculation had a borrow
	if (borrow) {
		// This means that the subtraction of the last element is negative, so the (other) is less than (this) and the result should be 0 from the start.
		printf("DEBUG: An error occurred in the last transaction!\n");
		result = 0;
	}
	result.ShrinkContents();
	return result;
}

unsignedBigInteger unsignedBigInteger::operator*(unsignedBigInteger& other)
{
	if (other == 0)
		return unsignedBigInteger(0);

	// Each element of these vectors contains a 32-bit integer in a 64-bit container to avoid overflowing (including answer)
	std::vector<unsigned long long> firstNumber = GetExpandedContents(),
		secondNumber = other.GetExpandedContents(),
		answer;

	unsigned int answerSize = firstNumber.size() + secondNumber.size() + 1;
	answerSize += answerSize & 1; // make it even
	answer.resize(answerSize);

	for (unsigned int idx = 0; idx < answerSize - 1; idx++) {
		for (int i = std::min(idx, (unsigned int)(firstNumber.size() - 1)), j = idx - i;
			(i >= 0) && (j < secondNumber.size()); i--, j++) {
			unsigned long long multiply = firstNumber[i] * secondNumber[j];
			answer[idx] += multiply;
			if (answer[idx] < multiply) // overflow occurs from 64-bit (add to idx+2)
				answer[idx + 2]++;
		}
		answer[idx + 1] += (answer[idx] & HIGH_DWORD) >> 32; // total overflow occurred from 32-bit (add to idx+1)
		answer[idx] &= LOW_DWORD;
	}

	unsignedBigInteger result;
	result.Resize(answerSize >> 1);
	for (unsigned int i = 0, j = 0; i < result.Size(); i++, j += 2)
		result[i] = answer[j] | (answer[j + 1] << 32); // combine answer
	result.ShrinkContents();
	return result;
}

unsignedBigInteger unsignedBigInteger::operator/(unsignedBigInteger& other)
{
	unsignedBigInteger quotient, remainder;
	if (Divide(*this, other, quotient, remainder))
		return quotient;
	// If unsuccessful:
	return unsignedBigInteger(0);
}

unsignedBigInteger unsignedBigInteger::operator%(unsignedBigInteger& other)
{
	unsignedBigInteger quotient, remainder;
	if (Divide(*this, other, quotient, remainder))
		return remainder;
	// If unsuccessful:
	return unsignedBigInteger(0);
}

unsignedBigInteger unsignedBigInteger::operator+(unsigned long long other)
{
	unsignedBigInteger result(*this);
	result[0] += other;
	if (result[0] >= other)
		return result; // no carry

	// loop for the carry:
	for (unsigned int i = 1; i < result.Size(); i++)
		if (++result[i] != 0)
			break;

	if (result.binaryContents.back() == 0)
		result.binaryContents.push_back(1);
	return result;
}

unsignedBigInteger unsignedBigInteger::operator-(unsigned long long other)
{
	if (Size() == 1 && binaryContents[0] <= other)
		return unsignedBigInteger(0); // no negative values are allowed.

	unsignedBigInteger result(*this);
	result[0] -= other;
	if (result[0] <= binaryContents[0])
		return result; // no borrow

	// loop for the borrow:
	for (unsigned int i = 1; i < result.Size(); i++)
		if (result[i]-- != 0)
			break;

	result.ShrinkContents();
	return result;
}

unsignedBigInteger unsignedBigInteger::operator*(unsigned long long other)
{
	// other is casted to unsignedBigInteger to avoid copying the code
	unsignedBigInteger otherNumber(other);
	return (*this) * otherNumber;
}

unsignedBigInteger unsignedBigInteger::operator/(unsigned long long other)
{
	unsignedBigInteger quotient;
	if ((other & HIGH_DWORD) == 0) { // check if other fits in 32-bit integer
		unsigned int divisor = other, remainder;
		if (Divide(*this, divisor, quotient, remainder))
			return quotient;
		// If unsuccessful
		return unsignedBigInteger(0);
	}

	unsignedBigInteger divisor(other), remainder;
	if (Divide(*this, divisor, quotient, remainder))
		return quotient;
	// If unsuccessful
	return unsignedBigInteger(0);
}

unsignedBigInteger unsignedBigInteger::operator%(unsigned long long other)
{
	unsignedBigInteger dividend(*this), quotient;
	if ((other & HIGH_DWORD) == 0) { // check if other fits in 32-bit integer
		unsigned int divisor = other, remainder;
		if (Divide(*this, divisor, quotient, remainder))
			return unsignedBigInteger(remainder);
		// If unsuccessful
		return unsignedBigInteger(0);
	}

	unsignedBigInteger divisor(other), remainder;
	if (Divide(*this, divisor, quotient, remainder))
		return remainder;
	// If unsuccessful
	return unsignedBigInteger(0);
}

unsignedBigInteger& unsignedBigInteger::operator+=(unsignedBigInteger& other)
{
	if (Size() < other.Size())
		this->Resize(other.Size());
	
	// Construct a big integer with the maximum possible number of 64-bit elements:
	bool carry = 0;

	// Add all the elements up to the size of other.
	for (unsigned int i = 0; i < other.Size(); i++) {
		binaryContents[i] += other[i] + carry;
		// if the current element of the result is strictly smaller than any of the 2 inputs', then an overflow has occurred and a carry is added to the next element.
		carry = binaryContents[i] < other[i] || binaryContents[i] < carry || 
			(binaryContents[i] == other[i] && carry); // This is a special case [e.g. 0xFF + 0xFF + (carry=1) = 0x1FF]
	}

	// Add the rest of the elements of the greater number. Keep adding the carry along [e.g. 1+2999 = 3000]
	for (unsigned int i = other.Size(); i < this->Size() && carry; i++)
		if (++binaryContents[i] != 0)
			carry = 0;

	// Check if the last calculation had a carry
	if (carry)
		binaryContents.push_back(carry);

	return (*this);
}

unsignedBigInteger& unsignedBigInteger::operator-=(unsignedBigInteger& other)
{
	if ((*this) < other)
		return (*this) = 0;
	
	bool borrow = 0;
	unsigned long long beforeSubtraction;
	for (unsigned int i = 0; i < other.Size(); i++) {
		beforeSubtraction = binaryContents[i];
		binaryContents[i] -= other[i] + borrow;

		// if the current element of the result is strictly greater than "beforeSubtraction", or equal and the borrow is 1, then an overflow has occurred and a borrow is subtracted from the next element.
		borrow = binaryContents[i] > beforeSubtraction ||
			(binaryContents[i] == beforeSubtraction && borrow); // e.g. [0xYZ - 0xFF - (borrow=1) = 0xYZ] (mod 256)
	}

	// Keep the rest of the elements unless there is a need to keep subtracting the carry along (e.g. 3000 - 1 = 2999)
	for (unsigned int i = other.Size(); i < Size() && borrow; i++)
		if (binaryContents[i]-- != 0)
			borrow = 0;


	// Check if the last calculation had a borrow
	if (borrow) {
		// This means that the subtraction of the last element is negative, so the (other) is less than (this) and the result should be 0 from the start.
		printf("DEBUG: An error occurred in the last subtraction!\n");
		(*this) = 0;
	}

	ShrinkContents();
	return (*this);
}

unsignedBigInteger& unsignedBigInteger::operator*=(unsignedBigInteger& other)
{
	unsignedBigInteger firstNumber(*this);
	return (*this) = firstNumber * other;
}

unsignedBigInteger& unsignedBigInteger::operator/=(unsignedBigInteger& other)
{
	unsignedBigInteger dividend(*this), remainder;
	if (Divide(dividend, other, *this, remainder)) // *this as quotient
		return *this;
	// If unsuccessful
	return (*this) = 0;
}

unsignedBigInteger& unsignedBigInteger::operator%=(unsignedBigInteger& other)
{
	unsignedBigInteger dividend(*this), quotient;
	if (Divide(dividend, other, quotient, *this)) // *this as remainder
		return *this;
	// If unsuccessful
	return (*this) = 0;
}

unsignedBigInteger& unsignedBigInteger::operator+=(unsigned long long other)
{
	binaryContents[0] += other;
	if (binaryContents[0] >= other)
		return (*this); // no carry

	// loop for the carry:
	for (unsigned int i = 1; i < Size(); i++)
		if (++binaryContents[i] != 0)
			break;

	if (binaryContents.back() == 0)
		binaryContents.push_back(1);
	return (*this);
}

unsignedBigInteger& unsignedBigInteger::operator-=(unsigned long long other)
{
	unsigned long long beforeSubtraction = binaryContents[0];
	binaryContents[0] -= other;
	if (Size() == 1 && binaryContents[0] > beforeSubtraction)
		return (*this) = 0; // no negative values are allowed.

	if (binaryContents[0] <= beforeSubtraction)
		return (*this); // no borrow

	// loop for the borrow:
	for (unsigned int i = 1; i < Size(); i++)
		if (binaryContents[i]-- != 0)
			break;

	ShrinkContents();
	return (*this);

}

unsignedBigInteger& unsignedBigInteger::operator*=(unsigned long long other)
{
	// other is casted to unsignedBigInteger to avoid copying the code
	unsignedBigInteger firstNumber(*this), otherNumber(other);
	return (*this) = firstNumber * otherNumber;
}

unsignedBigInteger& unsignedBigInteger::operator/=(unsigned long long other)
{
	unsignedBigInteger dividend(*this);
	if ((other & HIGH_DWORD) == 0) { // check if other fits in 32-bit integer
		unsigned int divisor = other, remainder;
		if (Divide(dividend, divisor, *this, remainder)) // *this as quotient
			return *this;
		// If unsuccessful
		return (*this) = 0;
	}

	unsignedBigInteger divisor(other), remainder;
	if (Divide(dividend, divisor, *this, remainder)) // *this as quotient
		return *this;
	// If unsuccessful
	return (*this) = 0;
}

unsignedBigInteger& unsignedBigInteger::operator%=(unsigned long long other)
{
	unsignedBigInteger dividend(*this), quotient;
	if ((other & HIGH_DWORD) == 0) { // check if other fits in 32-bit integer
		unsigned int divisor = other, remainder;
		if (Divide(dividend, divisor, quotient, remainder))
			return (*this) = remainder;
		// If unsuccessful
		return (*this) = 0;
	}

	unsignedBigInteger divisor(other);
	if (Divide(dividend, divisor, quotient, *this)) // *this as remainder
		return *this;
	// If unsuccessful
	return (*this) = 0;
}

unsignedBigInteger& unsignedBigInteger::operator++()
{
	return (*this) += 1;
}

unsignedBigInteger& unsignedBigInteger::operator--()
{
	return (*this) -= 1;
}

bool Divide(unsignedBigInteger& dividend, unsignedBigInteger& divisor,
	unsignedBigInteger& quotient, unsignedBigInteger& remainder)
{
	// Inputs:	dividend, divisor
	// Outputs: quotient, remainder
	// dividend = divisor * quotient + remainder

	if (divisor == 0) {
		printf("DEBUG: An error occurred during division: Division by 0!\n");
		return false;
	}

	if (dividend < divisor) {
		remainder = dividend;
		quotient = 0;
		return true;
	}

	if (divisor <= LOW_DWORD) { // Check if divisor fits in 32-bit integer
		unsigned int divisorInt = divisor.ToUInt(), remainderInt = remainder.ToUInt();
		if (Divide(dividend, divisorInt, quotient, remainderInt)) {
			remainder = remainderInt;
			return true;
		}
		return false;
	}

	quotient = 0;
	unsigned int shift = dividend.NumberOfBits() - divisor.NumberOfBits();
	unsignedBigInteger shifted = divisor << shift;
	unsignedBigInteger product = 0;
	
	// while (shifted >= divisor) { // replaced with the break at the end
	
	while(true) {
		quotient <<= 1;
		if (product + shifted <= dividend) {
			product += shifted;
			quotient |= 1;
		}
		shifted >>= 1;
		if (shift-- == 0)
			break;
	}

	remainder = dividend - product;

	if (remainder >= divisor) {
		printf("DEBUG: An error occurred during division!: The remainder is greator than the divisor!\n");
		return false;
	}
	if (quotient * divisor != product) {
		printf("DEBUG: An error occurred during division!: The product does not equal the multiplication of the quotient and the divisor!\n");
		return false;
	}
	return true;
}

bool Divide(unsignedBigInteger& dividend, unsigned int& divisor,
	unsignedBigInteger& quotient, unsigned int& remainder) 
{
	// Inputs:	dividend, divisor
	// Outputs: quotient, remainder
	// dividend = divisor * quotient + remainder

	if (divisor == 0) {
		printf("DEBUG: An error occurred during division: Division by 0!\n");
		return false;
	}

	if (dividend < divisor) {
		remainder = dividend.ToULongLong();
		quotient = 0;
		return true;
	}

	quotient = 0;
	unsigned long long packet = 1LL << 32, // Equivalent to 2^32
		packetQuotient, packetRemainder,
		remainder64 = 0; // 64-bit place holder to avoid overflow

	unsignedBigInteger Temp;
	packetQuotient = packet / divisor;
	packetRemainder = packet % divisor;

	/*	Let D = (A * B) / C where A = Na * C + Ra, B = Nb * C + Rb
	*	Then D = Na * Nb * C + Na * Rb + Nb * Ra + (Ra * Rb) / C
	*	
	*	Where:
	*	- A is explained below
	*	- B is the packet (2^32)
	*	- C is the divisor
	*	- D is the quotient
	* 
	*	The dividend is represented by elements of 64-bit integers, where each element is divided into two 32-bit integers below
	*	Let each element of these be represented by E_i starting from E_0 as the least significant element and ending at E_(n-1)
	*	Then the dividend can be represented by the following equation
	*	dividend = E_0 * packet^0 + E_1 * packet^1 + E_2 * packet^2 + ... + E_(n-1) * packet^(n-1)
	*			 = E_0 + packet * (E_1 + packet * ( ... (E_(n-2) + packet * E_(n-1)) ... ))
	* 
	*	So the A terms described above will contain the following elements at each step, where B = packet at all of them
	*	A_0 = E_(n-1)
	*	A_1 = E_(n-2) + packet * E_(n-1) = E_(n-2) + packet * A_0
	*	...
	*	A_i = E_(n-1-i) + packet * A_(i-1)
	*	
	*/

	for (unsigned int i = dividend.Size() - 1; ; i--) {
		unsigned long long A[2] = { (dividend[i] & HIGH_DWORD) >> 32, dividend[i] & LOW_DWORD };
		for (unsigned int j = 0; j < 2; j++) {
			quotient += A[j] / divisor;
			remainder64 += A[j] % divisor;
			quotient += remainder64 / divisor;
			remainder64 %= divisor;
			
			if (i == 0 && j == 1) // to avoid multiplying the last element by packet
				break;

			Temp = quotient * packetQuotient;
			Temp *= divisor;

			quotient *= packetRemainder;
			quotient += remainder64 * packetQuotient;
			quotient += Temp;
			
			remainder64 *= packetRemainder;
			quotient += remainder64 / divisor;
			remainder64 %= divisor;
		}
		if (i == 0)
			break;
	}
	remainder = (unsigned int) remainder64;
	return true;
}

unsignedBigInteger& unsignedBigInteger::FastPower(unsigned long long exponent)
{
	if (exponent == 0) {
		*this = 1;
		return *this;
	}
	
	if ((*this) == 0)
		return (*this);

	// [TODO: Check the expected size first]

	unsignedBigInteger base(*this);
	exponent--; // (*this) already contains the first power

	while (exponent > 0) {
		if ((exponent & 1) == 1)
			(*this) *= base;
		base *= base;
		exponent >>= 1;
	}
	return (*this);
}

inline unsignedBigInteger& unsignedBigInteger::FastPower(unsignedBigInteger &exponent)
{
	// Not allowing exponents that do not fit in 64-bit integer
	return this->FastPower(exponent.ToULongLong());
}

//=========================================================================================================================
// Comparison Operators (<, <=, >, >=, ==, !=) and Comaprison Functions:
//=========================================================================================================================

bool unsignedBigInteger::operator< (unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result <  0);
}

bool unsignedBigInteger::operator<=(unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result <= 0);
}

bool unsignedBigInteger::operator> (unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result >  0);
}

bool unsignedBigInteger::operator>=(unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result >= 0);
}

bool unsignedBigInteger::operator==(unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result == 0);
}

bool unsignedBigInteger::operator!=(unsignedBigInteger& other)
{
	signed int result = CompareWith(other);
	return (result != 0);
}

bool unsignedBigInteger::operator< (unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result <  0);
}

bool unsignedBigInteger::operator<=(unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result <= 0);
}

bool unsignedBigInteger::operator> (unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result >  0);
}

bool unsignedBigInteger::operator>=(unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result >= 0);
}

bool unsignedBigInteger::operator==(unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result == 0);
}

bool unsignedBigInteger::operator!=(unsigned long long other)
{
	signed int result = CompareWith(other);
	return (result != 0);
}

// This is the function that actually compares two big integers. All the comparison operator call this.
// 0 = equals, +1 = greater, -1 = smaller
signed int unsignedBigInteger::CompareWith(unsignedBigInteger& other)
{
	ShrinkContents();
	other.ShrinkContents();
	if (Size() > other.Size())
		return 1;
	if (Size() < other.Size())
		return -1;
	for (unsigned int i = Size() - 1; ; i--) {
		if (binaryContents[i] > other[i])
			return 1;
		if (binaryContents[i] < other[i])
			return -1;
		if (i == 0)
			break;
	}
	return 0;
}

signed int unsignedBigInteger::CompareWith(unsigned long long other)
{
	ShrinkContents();
	if (Size() > 1)
		return 1;
	if (binaryContents[0] > other)
		return 1;
	if (binaryContents[0] < other)
		return -1;
	return 0;
}

//=========================================================================================================================
// Bitwise Operators (|, &, ^, |=, &=, ^=, ~) :
//=========================================================================================================================

unsignedBigInteger unsignedBigInteger::operator|(unsignedBigInteger& other)
{
	// These two pointers will point to (this) and (other) depending on how many elements are there in each of them (the size of binaryContents).
	// The greaterNumber is not necessarily greater if they have the same number of elements, and it does not have to be.
	unsignedBigInteger* greaterNumber;
	unsignedBigInteger* smallerNumber;

	if (Size() >= other.Size()) {
		greaterNumber = this;
		smallerNumber = &other;
	}
	else {
		greaterNumber = &other;
		smallerNumber = this;
	}

	// Construct a big integer from the greater number (in terms of size):
	unsignedBigInteger result(*greaterNumber);
	for (unsigned int i = 0; i < smallerNumber->Size(); i++)
		result[i] |= (*smallerNumber)[i];
	return result;
}

unsignedBigInteger unsignedBigInteger::operator&(unsignedBigInteger& other)
{
	// These two pointers will point to (this) and (other) depending on how many elements are there in each of them (the size of binaryContents).
	// The greaterNumber is not necessarily greater if they have the same number of elements, and it does not have to be.
	unsignedBigInteger* greaterNumber;
	unsignedBigInteger* smallerNumber;

	if (Size() >= other.Size()) {
		greaterNumber = this;
		smallerNumber = &other;
	}
	else {
		greaterNumber = &other;
		smallerNumber = this;
	}

	// Construct a big integer from the smaller number (in terms of size):
	unsignedBigInteger result(*smallerNumber);
	for (unsigned int i = 0; i < smallerNumber->Size(); i++)
		result[i] &= (*greaterNumber)[i];
	return result;
}

unsignedBigInteger unsignedBigInteger::operator^(unsignedBigInteger& other)
{
	// These two pointers will point to (this) and (other) depending on how many elements are there in each of them (the size of binaryContents).
	// The greaterNumber is not necessarily greater if they have the same number of elements, and it does not have to be.
	unsignedBigInteger* greaterNumber;
	unsignedBigInteger* smallerNumber;

	if (Size() >= other.Size()) {
		greaterNumber = this;
		smallerNumber = &other;
	}
	else {
		greaterNumber = &other;
		smallerNumber = this;
	}

	// Construct a big integer from the greater number (in terms of size):
	unsignedBigInteger result(*greaterNumber);
	for (unsigned int i = 0; i < smallerNumber->Size(); i++)
		result[i] ^= (*smallerNumber)[i];
	return result;
}

unsignedBigInteger unsignedBigInteger::operator|(unsigned long long other)
{
	unsignedBigInteger result(*this);
	result[0] |= other;
	return result;
}

unsignedBigInteger unsignedBigInteger::operator&(unsigned long long other)
{
	unsignedBigInteger result(other);
	result[0] &= binaryContents[0];
	return result;
}

unsignedBigInteger unsignedBigInteger::operator^(unsigned long long other)
{
	unsignedBigInteger result(*this);
	result[0] ^= other;
	return result;
}

unsignedBigInteger& unsignedBigInteger::operator|=(unsignedBigInteger& other)
{
	// Extend for extra elements
	if (other.Size() > Size())
		Resize(other.Size());
	for (unsigned int i = 0; i < other.Size(); i++)
		binaryContents[i] |= other[i];
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator&=(unsignedBigInteger& other)
{
	// Remove any extra elements (same as AND with zeros)
	if (other.Size() < Size())
		Resize(other.Size());
	for (unsigned int i = 0; i < Size(); i++)
		binaryContents[i] &= other[i];
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator^=(unsignedBigInteger& other)
{
	// Extend for extra elements
	if (other.Size() > Size())
		Resize(other.Size());
	for (unsigned int i = 0; i < other.Size(); i++)
		binaryContents[i] ^= other[i];
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator|=(unsigned long long other)
{
	binaryContents[0] |= other;
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator&=(unsigned long long other)
{
	Resize(1); // Remove any extra bytes (same as AND with zeros)
	binaryContents[0] &= other;
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator^=(unsigned long long other)
{
	binaryContents[0] ^= other;
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator~()
{
	// Inverting is the same as bitwise xor with all ones (without changing the size)
	for (unsigned int i = 0; i < Size(); i++)
		binaryContents[i] ^= (LOW_DWORD | HIGH_DWORD);
	return *this;
}

//=========================================================================================================================
// Shift Operators (<<, >>, <<=, >>=) and Shifting Functions:
//=========================================================================================================================

unsignedBigInteger unsignedBigInteger::operator<<(unsigned long long other)
{
	unsigned int shiftElements = other >> 6; // equivalent to division by 64
	unsignedBigInteger result(*this);
	if (shiftElements + Size() > MAX_SIZE)
		return result;

	/*	The shift is done in 2 steps, 1 for 64-bit shifts (for elements in binaryContents) which is done in a separate function,
	 *	and the other for shifting less than 64 bits. Each element is divided into two parts as follows: (assuming 16-bit elements for simplicity)
	 *	0x1234ABCD is saved as {1234, ABCD} (2 16-bit elements)
	 *	(0x1234ABCD << 4) will results as 0x1234ABCD0 which will be saved as {0001, 234A, BCD0} (3 16-bit elements)
	 *	The operation will divide the elements as: {1, 2340}, {A, BCD0} by shifting 12 to the right for the higher part,
	 *	and shifting 4 to the left for the lower part (12+4=16),
	 *	then each higher part of a lower element will be bitwise-ORed with the lower part of the higher element {1, 234A, BCD0}
	 */

	unsigned int shiftBits = other & 63; // equivalent to modulo 64
	if (shiftBits > 0) {
		unsigned int complementShift = 64 - shiftBits;
		unsigned long long higherPart, previousHigherPart = 0;
		for (unsigned int i = 0; i < result.Size(); i++) {
			higherPart = (result[i] >> complementShift);	// current higher part in the new place
			result[i] <<= shiftBits;						// current lower part in the new place
			result[i] |= previousHigherPart;				// bitwise-or with the previous higher part
			previousHigherPart = higherPart;				// saving the higher part as the next element's previous one
		}
		// Add any extra higher part to a new element:
		if (previousHigherPart != 0)
			result.binaryContents.push_back(previousHigherPart);
	}
	if (shiftElements != 0)
		result.ShiftLeftBy(shiftElements);
	return result;
}

unsignedBigInteger unsignedBigInteger::operator>>(unsigned long long other)
{
	unsigned int shiftElements = other >> 6; // equivalent to division by 64
	unsignedBigInteger result(*this);
	if (shiftElements >= Size())
		return unsignedBigInteger(0);

	// This operation is the same as the (operator<<) in reverse, read description there

	unsigned int shiftBits = other & 63; // equivalent to modulo 64
	if (shiftBits > 0) {
		unsigned int complementShift = 64 - shiftBits;
		unsigned long long lowerPart, previousLowerPart = 0;
		for (unsigned int i = result.Size() - 1; ; i--) {
			lowerPart = (result[i] << complementShift);	// current lower part in the new place
			result[i] >>= shiftBits;					// current higher part in the new place
			result[i] |= previousLowerPart;				// bitwise-or with the previous lower part
			previousLowerPart = lowerPart;				// saving the lower part as the next element's previous one
			if (i == 0)
				break;
		}
		// Any extra lower part will be discarded.
	}
	if (shiftElements != 0)
		result.ShiftRightBy(shiftElements);
	result.ShrinkContents(); // To get rid of any leading zeros elements
	return result;
}

unsignedBigInteger unsignedBigInteger::operator<<(unsignedBigInteger& other)
{
	// Not allowing shifting by amounts that do not fit in 64-bit integer
	return (*this) << other.ToULongLong();
}

unsignedBigInteger unsignedBigInteger::operator>>(unsignedBigInteger& other)
{
	// Not allowing shifting by amounts that do not fit in 64-bit integer
	return (*this) >> other.ToULongLong();
}

unsignedBigInteger& unsignedBigInteger::operator<<=(unsigned long long other)
{
	unsigned int shiftElements = other >> 6; // equivalent to division by 64
	if (shiftElements + Size() > MAX_SIZE)
		return (*this);

	// Check (operator<<) for explanation

	unsigned int shiftBits = other & 63; // equivalent to modulo 64
	if (shiftBits > 0) {
		unsigned int complementShift = 64 - shiftBits;
		unsigned long long higherPart, previousHigherPart = 0;
		for (unsigned int i = 0; i < Size(); i++) {
			higherPart = (binaryContents[i] >> complementShift);	// current higher part in the new place
			binaryContents[i] <<= shiftBits;						// current lower part in the new place
			binaryContents[i] |= previousHigherPart;				// bitwise-or with the previous higher part
			previousHigherPart = higherPart;						// saving the higher part as the next element's previous one
		}
		// Any extra higher part will be added to a new element:
		if (previousHigherPart != 0)
			binaryContents.push_back(previousHigherPart);
	}
	if (shiftElements != 0)
		ShiftLeftBy(shiftElements);
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator>>=(unsigned long long other)
{
	unsigned int shiftElements = other >> 6; // equivalent to division by 64
	if (shiftElements >= Size())
		return (*this) = 0;

	// This operation is the same as the (operator<<) in reverse, read description there

	unsigned int shiftBits = other & 63; // equivalent to modulo 64
	if (shiftBits > 0) {
		unsigned int complementShift = 64 - shiftBits;
		unsigned long long lowerPart, previousLowerPart = 0;
		for (unsigned int i = Size() - 1; ; i--) {
			lowerPart = (binaryContents[i] << complementShift);	// current lower part in the new place
			binaryContents[i] >>= shiftBits;					// current higher part in the new place
			binaryContents[i] |= previousLowerPart;				// bitwise-or with the previous lower part
			previousLowerPart = lowerPart;						// saving the lower part as the next element's previous one
			if (i == 0)
				break;
		}
		// Any extra lower part will be discarded.
	}
	if (shiftElements != 0)
		ShiftRightBy(shiftElements);
	ShrinkContents(); // To get rid of any leading zeros elements
	return *this;
}

unsignedBigInteger& unsignedBigInteger::operator<<=(unsignedBigInteger& other)
{
	// Not allowing shifting by amounts that do not fit in 64-bit integer
	return (*this) <<= other.ToULongLong();
}

unsignedBigInteger& unsignedBigInteger::operator>>=(unsignedBigInteger& other)
{
	// Not allowing shifting by amounts that do not fit in 64-bit integer
	return (*this) >>= other.ToULongLong();
}

// These 2 functions shift by elements of 64-bit integers
unsignedBigInteger& unsignedBigInteger::ShiftRightBy(unsigned int shift)
{
	// Preventing shifting by more than the size
	if (Size() <= shift)
		(*this) = 0;

	// Cannot shift to the right if the value is already 0 
	if ((*this) == 0)
		return (*this);

	for (unsigned int i = 0; i + shift < Size(); i++)
		binaryContents[i] = binaryContents[i + shift];
	Resize(Size() - shift);
	return (*this);
}

unsignedBigInteger& unsignedBigInteger::ShiftLeftBy(unsigned int shift)
{
	// Cannot shift to the left if the value is already 0 
	if ((*this) == 0)
		return (*this);

	Resize(Size() + shift);
	for (unsigned int i = Size() - 1; i >= shift; i--)
		binaryContents[i] = binaryContents[i - shift];
	for (unsigned int i = 0; i < shift; i++)
		binaryContents[i] = 0;
	return (*this);
}

//=========================================================================================================================
// Printing Functions:
//=========================================================================================================================

bool unsignedBigInteger::PrintAsDecimal(char charAfter)
{
	if (!ConvertToDecimal())
		return false;
	for (unsigned int i = decimalContents.size() - 1; ; i--) {
		if (i < decimalContents.size() - 1)
			printf("%09d", decimalContents[i]);
		else
			printf("%d", decimalContents[i]);
		if (i == 0)
			break;
	}
	if (charAfter != '\0') printf("%c", charAfter);
	return false;
}

bool unsignedBigInteger::PrintAsHex(char charAfter)
{
	printf("0x");
	for (unsigned int i = Size() - 1; ; i--) {
		if (i == Size() - 1)
			printf("%llX", binaryContents[i]);
		else
			printf("%016llX", binaryContents[i]);
		if (i == 0)
			break;
	}
	if (charAfter != '\0') printf("%c", charAfter);
	return true;
}

// Un-implemented function [TODO]
bool unsignedBigInteger::PrintAsBinary(char charAfter)
{
	// Printing in binary to be added (or maybe not important?)
	// No specifier for binary
	printf("0b");
	printf("0");
	if (charAfter != '\0') printf("%c", charAfter);
	return true;
}

//=========================================================================================================================
// Sizing Functions:
//=========================================================================================================================

unsigned int unsignedBigInteger::NumberOfDigits()
{
	std::string str = ConvertToString(10);
	return str.length();
}

unsigned int unsignedBigInteger::NumberOfBits()
{
	ShrinkContents();
	unsigned int result = (Size() - 1) << 6; // ()<<6 is equivalent to ()*64
	unsigned long long temp = binaryContents.back();
	while (temp > 0) {
		result++;
		temp >>= 1;
	}
	return result;
}

inline unsigned long long unsignedBigInteger::Size()
{
	return binaryContents.size();
}

inline unsigned unsigned long long unsignedBigInteger::GetMaximumSize()
{
	return MAX_SIZE;
}

bool unsignedBigInteger::Resize(unsigned int newSize, bool extendMaxSize)
{
	// Preventing exceeding the absolute maximum size
	if (newSize > ABSOLUTE_MAX_SIZE)
		return false;

	// Preventing exceeding the maximum size if it is not extendable
	if (extendMaxSize)
		while (MAX_SIZE < newSize)
			MAX_SIZE <<= 1;
	else if (newSize > MAX_SIZE)
		return false;

	// Preventing deletion of the vector binaryContents:
	if (newSize == 0) {
		(*this) = 0;
		return true;
	}

	binaryContents.resize(newSize);
	return true;
}

bool unsignedBigInteger::ShrinkContents()
{
	// returns whether the object was shrunk
	bool result = false;
	while (Size() > 1 && binaryContents.back() == 0) {
		binaryContents.pop_back();
		result = true;
	}
	return result;
}

std::vector<unsigned long long> unsignedBigInteger::GetExpandedContents()
{
	std::vector<unsigned long long> result(Size() << 1);
	for (unsigned long long i = 0; i < Size(); i++) {
		unsigned long long currentElement = binaryContents[i];
		result[i << 1] = currentElement & LOW_DWORD;
		result[(i << 1) + 1] = (currentElement & HIGH_DWORD) >> 32;
	}

	// removing all leading zeros
	while (result.size() > 1 && result.back() == 0)
		result.pop_back();
	return result;
}

// This function will return the number of trailing zeros and will remove them, so be careful when calling it!!
// It will be helpful for muliplication and division.
// [TODO: Un-implemented function]
unsigned int unsignedBigInteger::RemoveTrailingZeros()
{
	return 0;
}

//=========================================================================================================================
// Converting Functions:
//=========================================================================================================================

inline unsigned int unsignedBigInteger::ToUInt()
{
	return binaryContents[0] & LOW_DWORD; // lowest 4 bytes
}

inline unsigned long long unsignedBigInteger::ToULongLong()
{
	return binaryContents[0]; // lowest 8 bytes
}

std::string unsignedBigInteger::ConvertToString(unsigned int base)
{
	if (base == 10) {
		if (!ConvertToDecimal())
			return "0";
		std::string result = "";
		result.reserve(9 * decimalContents.size());
		for (unsigned int i = 0; i < decimalContents.size(); i++) {
			// Zero padding for each packet:
			char packet[10];
			sprintf_s(packet, "%09d", decimalContents[i]); // Apparently, sprintf is not safe according to MS-VS
			result = packet + result;
		}
		// Remove any possible leading zeros
		unsigned int lastZero = 0;
		while (result[lastZero] == '0')
			lastZero++;
		return result.substr(lastZero);
	}
	else if (base == 16) {
		// [TODO: Un-implemented functionality]
		return "0";
	}
	else
		return "0";
}

bool unsignedBigInteger::ConvertToDecimal()
{
	// [TODO: Un-implemented functionality]
	// if (isConvertedToDecimal)
	//	return true;

	// conversion operation
	unsigned int packetSize = E9; // 10^9
	decimalContents.clear();
	
	unsignedBigInteger dividend(*this), quotient;
	unsigned int packetNumber;

	while (dividend > 0) {
		// Replaced by the below function (2 calculations at once)
		// packetNumber = (dividend % packetSize).ToULongLong();
		// dividend /= packetSize;

		if (Divide(dividend, packetSize, quotient, packetNumber))
			std::swap(dividend, quotient);
		else {
			printf("DEBUG: Error in conversion to decimal!\n");
			decimalContents.clear();
			break;
		}
		
		decimalContents.push_back(packetNumber);
	}
	
	if (decimalContents.empty())
		decimalContents.push_back(0);
	return true;
	// [TODO: Un-implemented functionality] 
	// update isConvertedToDecimal, and update it to false in all operations
}

bool unsignedBigInteger::ConvertFromStringDecimal(std::string str)
{
	for (char& ch : str)
		if (ch < '0' || ch>'9')
			return isConvertedToDecimal = false;

	unsigned int stringSize = str.length();
	const unsigned long long packetSize = E18; // 10^18
	const unsigned int packetLength = 18; // 18 digits at a time
	(*this) = 0;
	decimalContents.clear();
	decimalContents.reserve(1 + stringSize / packetLength);

	for (unsigned int i = 0; i < stringSize; i += packetLength) {
		std::string packetString = str.substr(i, packetLength);
		unsigned long long packetNumber = std::stoull(packetString);
		(*this) *= packetSize;
		(*this) += packetNumber;
		decimalContents.push_back(packetNumber);
	}

	return isConvertedToDecimal = true;
}

bool unsignedBigInteger::ConvertFromStringHex(std::string str)
{
	if (str.empty())
		return false;

	if (str[0] == '0' && ((str[1] == 'x') || (str[1] == 'X')))
		str = str.substr(2);

	for (char& ch : str) {
		bool ok = 0;
		if (ch >= '0' && ch <= '9') ok = 1;
		if (ch >= 'a' && ch <= 'f') ok = 1;
		if (ch >= 'A' && ch <= 'F') ok = 1;
		if (!ok) return false;
	}

	unsigned int stringSize = str.length();
	const unsigned int packetLength = 16; // 16 hexadecimal digits at a time

	(*this) = 0;
	binaryContents.reserve(1 + stringSize / packetLength);

	for (unsigned int i = 0; i < stringSize; i += packetLength) {
		std::string packetString = str.substr(i, packetLength);
		unsigned long long packetNumber = std::stoull(packetString, 0, 16);
		(*this).ShiftLeftBy(1);
		(*this) |= packetNumber;
	}

	return true;
}
