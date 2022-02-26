#include "Decimal_32.h"

//private
void Decimal_32::pos_add(Decimal_32 a, Decimal_32 b){
	utiny carry = 0;

	for (int i = DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		wtf(i, c_currentDigit);
	}
	if (carry == 1) { //this means the number is too high for the current exponent, need to scootch everything over
		for (int i = DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			wtf(i, rff(i - 1));
		}
		wtf(0, 1);
		if (exponent_ < 255) exponent_++; //TO DO how should it handle if the exponent is already maxed
	}
}

void Decimal_32::pos_subtract(Decimal_32 a, Decimal_32 b) {
	utiny borrow = 0;
	if (b > a) { //wont infinite loop because these get swapped, but -- TO DO: AVOID COMPARISON TWICE (passed flag, seperate function, template, in this function?
		pos_subtract(b, a);
		negate();
		return;
	}
	for (int i = DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit - b_currentDigit - borrow;
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10;
		}
		else {
			borrow = 0; //clear carried number
		}
		wtf(i, c_currentDigit);
	}
}


//public
Decimal_32::~Decimal_32() {
	//does nothing
}

Decimal_32::Decimal_32() {
	for (int i = 0; i < 31; i++) {
		mantissa_[i] = B00000000;
	}
	exponent_ = 158;
}

Decimal_32::Decimal_32(float d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(double d, int digits) {
	std::cout << "I will implement this function later\n";
}

Decimal_32::Decimal_32(std::initializer_list<utiny> mantissa, utiny exponent, bool signd) {
	exponent_ = exponent;
	for (int i = 0; i < mantissa.size(); i++) {
		wtf(i + DIGITS_ - mantissa.size(), mantissa.begin()[i]);
	}
	if ((DIGITS_ - mantissa.size() - 1) % 2) {
		wtf(DIGITS_ - mantissa.size() - 1, 0);
	}
	for (int i = (DIGITS_ - mantissa.size() + 1/*Offset for sign*/ - 1/*it's an index*/ - 1/*We go one back*/) / 2; i >= 0; i--) {
		mantissa_[i] = B00000000;
	}
	mantissa_[0] = signd ? B00010000 : B00000000;
}

Decimal_32::Decimal_32(const Decimal_32& d) {
	for (int i = 0; i < DIGITBYTES_; i++) {
		mantissa_[i] = d.mantissa_[i]; //obv just copies bytes over
	}
	exponent_ = d.exponent_;
}

void Decimal_32::display(void) const {
	auto digitscache = digits();
	int dplocation = (int)exponent_ - 158 + digitscache; 
	int dpiterlocation = DIGITS_ + dplocation - digitscache; 
	if (isSigned()) {
		std::cout << '-';
	}
	if (dplocation <= 0) {
		std::cout << '.';
		for (int _ = dplocation; _ <= 0; _++) { //use an underscore if the variable is not needed within the loop but just to keep track of it
			std::cout << '0';
		}
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) {
			std::cout << (int)rff(i);
		}
	}
	else {
		for (int i = DIGITS_ - digitscache; i < DIGITS_; i++) { //more efficient alternative if there is an integrated decimal point
			if (dpiterlocation == i) {
				std::cout << '.';
			}
			std::cout << (int)rff(i);
		}
	}
}

Decimal_32 Decimal_32::abs(void) const {
	Decimal_32 ret = *this;
	ret.mantissa_[0] = B00001111 & ret.mantissa_[0]; //clear leading sign half-byte
	return ret;
}

void Decimal_32::operator|(Decimal_32& b) {
	//Count avaiable shifting slots for both numbers
	int spacea = 0;
	int spaceb = 0;
	for (int i = 0; i < /*a.*/DIGITS_; i++) {
		if (!rff(i)) {
			spacea++;
		}
		else {
			break;
		}
	}
	for (int i = 0; i < b.DIGITS_/*Using B here because b is used in the rest of the function*/; i++) {
		if (!rff(i)) {
			spaceb++;
		}
		else {
			break;
		}
	}
	bool adderbool = ((int)/*a.*/exponent_ - spacea < (int)b.exponent_ - spaceb); //inverted because lesser should be the adder; then it will offset both and add
	Decimal_32& adder = adderbool ? *this : b;
	Decimal_32& addee = adderbool ? b : *this;
	int exponentDifference = diff(/*a.*/exponent_, b.exponent_);
	//offset the adder
	int addeeOffset = min(exponentDifference, (!adderbool) ? spacea : spaceb);
	for (int i = 0; i < addee.DIGITS_ - addeeOffset; i++) { //goes through and shifts
		addee.wtf(i, addee.rff(i + addeeOffset));
	}
	for (int i = addee.DIGITS_ - addeeOffset; i < addee.DIGITS_; i++) { // flushes after data
		addee.wtf(i, 0); 
	}
	addee.exponent_ -= addeeOffset; //TO DO consider max/min exponents
	int adderOffset = exponentDifference - addeeOffset;
	if (adderOffset) {
		for (int i = adder.DIGITS_ - 1; i >= adderOffset; i--) { //goes through and shifts
			adder.wtf(i, adder.rff(i - adderOffset));
		}
		for (int i = adderOffset - 1; i >= 0; i--) { // flushes after data
			adder.wtf(i, 0); 
		}
	}
	adder.exponent_ += adderOffset; //TO DO consider max/min exponents
}

void Decimal_32::lshift() {
	int space = lspace();
	if (!space) return;
	for (int i = space; i < DIGITS_; i++) {
		wtf(i - space, rff(i));
	}
	exponent_ -= space; // TO DO consider what to do when mantissa goes out of range, also rshift
}

void Decimal_32::rshift() {
	int space = rspace();
	if (!space) return;
	for (int i = DIGITS_ - 1; i >= space; i--) {
		wtf(i, rff(i) - space);
	}
	exponent_ += space;
}

void Decimal_32::lshift(utiny shift) {
	if (!shift) return;
	for (int i = shift; i < DIGITS_; i++) {
		wtf(i - shift, rff(i));
	}
	exponent_ -= shift; // TO DO consider what to do when mantissa goes out of range, also rshift
}

void Decimal_32::rshift(utiny shift) {
	if (!shift) return;
	for (int i = DIGITS_ - 1; i >= shift; i--) {
		wtf(i, rff(i) - shift);
	}
	exponent_ += shift;
}

utiny Decimal_32::lspace() {
	int space = 0;
	for (int i = 0; i < /*a.*/DIGITS_; i++) {
		if (!rff(i)) {
			space++;
		}
		else {
			break;
		}
	}
	return space;
}

utiny Decimal_32::rspace() {
	int space = 0;
	for (int i = DIGITS_ - 1; i >= 0; i--) {
		if (!rff(i)) {
			space++;
		}
		else {
			break;
		}
	}
	return space;
}

Decimal_32 operator+(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.exponent_ = a.exponent_;

	bool signa = a.mantissa_[0] >> 4;
	bool signb = b.mantissa_[0] >> 4;
	if (!signa && signb) {
		c.pos_subtract(a, b);
		return c;
	}
	if (signa && !signb) {
		c.pos_subtract(b, a);
		return c;
	}
	if (signa && signb) {
		c.pos_add(a, b);
		c.negate();
		return c;
	}

	utiny carry = 0;
	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit + b_currentDigit + carry;
		if (c_currentDigit > 9/*Max number in decimal*/) {
			carry = 1;
			c_currentDigit -= 10;
		}
		else {
			carry = 0; //clear carried number
		}
		c.wtf(i, c_currentDigit); 
	}
	if (carry == 1) { //this means the number is too high for the current exponent, need to scootch everything over
		for (int i = c.DIGITS_ - 1; i >= 0 + 1/*because it is scootching it shouldn't go past 1*/; i--) {
			c.wtf(i, c.rff(i - 1));
		}
		c.wtf(0, 1);
		if (c.exponent_ < 255) c.exponent_++; //TO DO how should it handle if the exponent is already maxed
	}
	return c;
}

Decimal_32 operator-(Decimal_32 a, Decimal_32 b) {
	Decimal_32 c;
	a | b; // TO DO: use the output of the space search to quit adding to save time
	c.exponent_ = a.exponent_;
	
	bool signa = a.mantissa_[0] >> 4;
	bool signb = b.mantissa_[0] >> 4;
	if (!signa && signb) {
		c.pos_add(a, b);
		return c;
	}
	if (signa && !signb) { 
		c.pos_add(a, b);
		c.negate();
		return c;
	}
	if (signa && signb) { 
		c.pos_subtract(a, b);
		c.negate();
		return c; 
	}

	utiny borrow = 0;

	for (int i = c.DIGITS_ - 1; i >= 0; i--) {
		utiny a_currentDigit = a.rff(i);
		utiny b_currentDigit = b.rff(i);
		utiny c_currentDigit = a_currentDigit - b_currentDigit - borrow;
		if (c_currentDigit < 0/*underflow for subtracting, have to borrow*/) {
			borrow = 1;
			c_currentDigit += 10 ;
		}
		else {
			borrow = 0; //clear carried number
		}
		c.wtf(i, c_currentDigit);
	}
	//cannot be left with leftover borrow, or it would mean the b was bigger than be, contradicting the original statement
	return c;
}

Decimal_32 operator*(Decimal_32 a, Decimal_32 b)
{
	Decimal_32 c;
	a.rshift(); //TO DO figure out more optimal solution
	b.rshift();
	utiny alspace = a.lspace();
	utiny blspace = b.lspace();
	bool aisbigger = a.exponent_ > b.exponent_;
	Decimal_32& biggerexp = aisbigger ? a : b;
	Decimal_32& smallerexp = aisbigger ? b : a;
	utiny bilspace = aisbigger ? alspace : blspace; // bi l space (bigger left space)
	utiny smlspace = aisbigger ? blspace : alspace;
	utiny spacediff = diff(bilspace,smlspace); //we cant use biggerexp and smallerexp to decide diff because this is based on space not exponent
	utiny expdiff = biggerexp.exponent_ - smallerexp.exponent_; //dont use diff macro because we have saved comparison
	//c.exponent_ = a.exponent_ + b.exponent_;
	int outputdigcount = ((biggerexp.exponent_ + (biggerexp.DIGITS_ - bilspace)) - (smallerexp.exponent_)); //gotta account for possible different exponents
	/*for (int i = alspace; i < a.DIGITS_; i++) {
		utiny arffi = a.rff(i);
		if (arffi != a.SQRT10_[i]) {
			if (arffi > a.SQRT10_[i]) {
				outputdigcount += 1;
			}
			break;
		}
	}
	for (int i = blspace; i < b.DIGITS_; i++) {
		utiny brffi = b.rff(i);
		if (brffi != b.SQRT10_[i]) {
			if (brffi > b.SQRT10_[i]) {
				outputdigcount += 1;
			}
			break;
		}
	}*/ //this doesn't work at all it just checks if the numbers are both less than the sqrt of 10 and I don't think THAT even works
	for (int i = bilspace; i < biggerexp.DIGITS_; i++) {
		utiny currentbiggerexpdig = biggerexp.rff(i); //TO DO profile here also  
		utiny currentsmallerexpdig = smallerexp.rff(i - spacediff);
		if (currentbiggerexpdig * currentsmallerexpdig != 9) {
			if (currentbiggerexpdig > currentsmallerexpdig) {
				outputdigcount++;
			}
			break;
		}
	}
	std::cout << outputdigcount << '\n';
	return c;
	if (outputdigcount > c.DIGITS_) {
	//wait look up, still have it wrong
	}
}


bool operator<(Decimal_32 a, Decimal_32 b) { //>, >=, <= are missing comments but they are just variations of these comments
	//two odd lines of logic here, which are IFF'ed.
	//we have smaller and bigger exponents, and if they are in order (smaller, bigger) to (a,b), then this yields true, and other way is false.
	//if the smaller exponent is the smaller number, then this yields true, and the other way is false.
	Decimal_32& biggerexp = a.exponent_ < b.exponent_ ? b : a;
	Decimal_32& smallerexp = a.exponent_ < b.exponent_ ? a : b; //inverted logic not sign bc of edge cases like =
	bool correct = a.exponent_ < b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	//the iterators do NOT LINE UP in the following loops; they couldn't without weirdness. The first is j for biggerexp, then the next two are i's for smallerexp.
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct; //biggerexp is bigger, has leading numbers that smallerexp doesn't have
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) { //check along the aligned digits
		int smallerexpdig = smallerexp.rff(i); //TO DO consider profiling just making it inline?, also profiling ? < : > method, also use result on *
		int biggerexpdig = biggerexp.rff(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (smallerexpdig < biggerexpdig) ==/*IFF operator*/ correct; //an aligned digit is different. figure out which number is bigger.
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) { //was biggerexp, which was a bug? //TO DO consider making function that indexes faster by taking whole byte
			return !correct; //smallerexp is bigger, has trailing numbers that biggerexp doesn't have
		}
	}
	return false; //isn't related to which is which, they're equal so neither. Could also write return correct because in case it gets here it's equal
}

bool operator>(Decimal_32 a, Decimal_32 b) { //see comments on operator<.
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; //now doing everything in >. Note that edge cases will now swap whether being biggerexp or smaller, but that's intended.
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;
	bool correct = a.exponent_ > b.exponent_; //SWAPPED THIS, THE NEW CORRECT ORDER. EVERYTHING ELSE CHECKS THE SAME; SMALLER SHOULD BE SMALLER, AND VICE VERSA, EXCEPT THE EDGE CASES ARE FLIPPED (see above).

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rff(i);
		int biggerexpdig = biggerexp.rff(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (biggerexpdig > smallerexpdig) ==/*IFF operator*/ correct; //this is still the same but I just swapped around the expression because we are using the > operator in this function and this emphasizes it.
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) {
			return !correct;
		}
	}
	return false;
}

bool operator==(Decimal_32 a, Decimal_32 b) {
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; 
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return false; //exclusive leading digits should not be anything besides 0
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		if (smallerexp.rff(i) != biggerexp.rff(i + exponentdiff)) {
			return false; //aligned digits have to be equal
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) {
			return false; // exclusive trailing digits should not be anything besides 0
		}
	}
	return true;
}

bool operator!=(Decimal_32 a, Decimal_32 b) {
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b; //see ==, returns opposite
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return true;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		if (smallerexp.rff(i) != biggerexp.rff(i + exponentdiff)) {
			return true;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) {
			return true;
		}
	}
	return false;
}

bool operator<=(Decimal_32 a, Decimal_32 b) { //see comments on operator<.
	Decimal_32& biggerexp = a.exponent_ < b.exponent_ ? b : a;
	Decimal_32& smallerexp = a.exponent_ < b.exponent_ ? a : b;
	bool correct = a.exponent_ < b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rff(i);
		int biggerexpdig = biggerexp.rff(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (smallerexpdig < biggerexpdig) ==/*IFF operator*/ correct;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) {
			return !correct;
		}
	}
	return true; //instead, we return true here, because <= should output true on equal.
}

bool operator>=(Decimal_32 a, Decimal_32 b) { //see comments on operator<=.
	Decimal_32& biggerexp = a.exponent_ > b.exponent_ ? a : b;
	Decimal_32& smallerexp = a.exponent_ > b.exponent_ ? b : a;
	bool correct = a.exponent_ > b.exponent_;

	utiny exponentdiff = biggerexp.exponent_ - smallerexp.exponent_;
	for (int j = 0; j < exponentdiff; j++) {
		if (biggerexp.rff(j) != 0) {
			return correct;
		}
	}
	for (int i = 0; i < smallerexp.DIGITS_ - exponentdiff; i++) {
		int smallerexpdig = smallerexp.rff(i);
		int biggerexpdig = biggerexp.rff(i + exponentdiff);
		if (smallerexpdig != biggerexpdig) {
			return (biggerexpdig > smallerexpdig) == correct;
		}
	}
	for (int i = smallerexp.DIGITS_ - exponentdiff; i < smallerexp.DIGITS_; i++) {
		if (smallerexp.rff(i) != 0) {
			return !correct;
		}
	}
	return true;
}

Decimal_32 Decimal_32::/*Is not a friend function, stays in scope*/operator-() const {
	Decimal_32 ret = *this;
	ret.mantissa_[0] = ((ret.mantissa_[0] ^ B11111111) & B11110000) | ret.mantissa_[0]; //clear leading sign half-byte
	return ret;
}