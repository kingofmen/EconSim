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
