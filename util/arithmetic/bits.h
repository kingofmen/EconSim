// Methods for bit-twiddling.
#ifndef UTIL_ARITHMETIC_BITS_H
#define UTIL_ARITHMETIC_BITS_H

#include <bitset>
#include <limits>
#include <vector>

namespace bits {

typedef std::bitset<32> Mask;

constexpr Mask kEmpty        = 0;

constexpr Mask kZero        = 1 << 0; 
constexpr Mask kOne         = 1 << 1; 
constexpr Mask kTwo         = 1 << 2; 
constexpr Mask kThree       = 1 << 3; 
constexpr Mask kFour        = 1 << 4; 
constexpr Mask kFive        = 1 << 5; 
constexpr Mask kSix         = 1 << 6; 
constexpr Mask kSeven       = 1 << 7; 
constexpr Mask kEight       = 1 << 8; 
constexpr Mask kNine        = 1 << 9; 
constexpr Mask kTen         = 1 << 10;
constexpr Mask kEleven      = 1 << 11;
constexpr Mask kTwelve      = 1 << 12;
constexpr Mask kThirteen    = 1 << 13;
constexpr Mask kFourteen    = 1 << 14;
constexpr Mask kFifteen     = 1 << 15;
constexpr Mask kSixteen     = 1 << 16;
constexpr Mask kSeventeen   = 1 << 17;
constexpr Mask kEighteen    = 1 << 18;
constexpr Mask kNineteen    = 1 << 19;
constexpr Mask kTwenty      = 1 << 20;
constexpr Mask kTwentyOne   = 1 << 21;
constexpr Mask kTwentyTwo   = 1 << 22;
constexpr Mask kTwentyThree = 1 << 23;
constexpr Mask kTwentyFour  = 1 << 24;
constexpr Mask kTwentyFive  = 1 << 25;
constexpr Mask kTwentySix   = 1 << 26;
constexpr Mask kTwentySeven = 1 << 27;
constexpr Mask kTwentyEight = 1 << 28;
constexpr Mask kTwentyNine  = 1 << 29;
constexpr Mask kThirty      = 1 << 30;
constexpr Mask kThirtyOne   = 1 << 31;
constexpr Mask kAll         = std::numeric_limits<unsigned long int>::max();

// MakeMask returns the OR of the provided 'count' of mask indices,
// where 1 corresponds to kOne and so on. Additional indices
// are ignored; it is an error to provide fewer indices than
// count.
Mask MakeMask(unsigned int count, ...);

// GetMask returns the OR of the given mask indices.
Mask GetMask(unsigned int m1);
Mask GetMask(unsigned int m1, unsigned int m2);
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3);
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3,
             unsigned int m4);
Mask GetMask(unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4,
             unsigned int m5);
Mask GetMask(std::vector<unsigned int> ms);


// Subset returns true if cand is a subset of super.
bool Subset(const Mask& cand, const Mask& super);

} // namespace bits

#endif
