// Package pop defines population dynamics.
package pop

import (
	"fmt"

	poppb "gogames/settlers/population/population_proto"
)

// Validate returns any errors in the households.
func Validate(templates []*poppb.Household) []error {
	errors := make([]error, 0, len(templates))
	seen := map[string]bool{}
	for idx, tmp := range templates {
		key := tmp.GetKey()
		if len(key) == 0 {
			errors = append(errors, fmt.Errorf("household with index %d has empty lookup key", idx))
			key = fmt.Sprintf("empty_key_%d", idx)
		} else {
			if seen[key] {
				errors = append(errors, fmt.Errorf("Duplicate lookup key %q", key))
			}
			seen[key] = true
		}
		sz, mm := tmp.GetSize(), tmp.GetMilitary()
		if sz < 1 {
			errors = append(errors, fmt.Errorf("%q has bad size %d, must be greater than 0", key, sz))
		}
		if sz < mm {
			errors = append(errors, fmt.Errorf("%q has more military (%d) than total (%d)", key, mm, sz))
		}
		if dt := tmp.GetDoubleTime(); dt < 1 {
			errors = append(errors, fmt.Errorf("%q has doubling time %d, must be at least 1", key, dt))
		}
	}
	return errors
}
