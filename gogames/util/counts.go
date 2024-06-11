// package counts provides utilities for manipulating
// string-count maps.
package counts

// Super returns true if p1 is a superset of p2.
func Super(avail, want map[string]int) bool {
	for key, count := range want {
		if avail[key] < count {
			return false
		}
	}
	return true
}

// SuperInt32 returns true if p1 is a superset of p2.
func SuperInt32(avail, want map[string]int32) bool {
	for key, count := range want {
		if avail[key] < count {
			return false
		}
	}
	return true
}

// Subtract subtracts the subtrahend from the minuend.
func Subtract(minuend, subtrahend map[string]int) {
	for k, v := range subtrahend {
		minuend[k] -= v
	}
}

// SubtractInt32 subtracts the subtrahend from the minuend.
func SubtractInt32(minuend, subtrahend map[string]int32) {
	for k, v := range subtrahend {
		minuend[k] -= v
	}
}

// Add adds the second argument to the first in-place.
func Add(first, addend map[string]int) {
	for k, v := range addend {
		first[k] += v
	}
}

// AddInt32 adds the second argument to the first in-place.
func AddInt32(first, addend map[string]int32) {
	for k, v := range addend {
		first[k] += v
	}
}
