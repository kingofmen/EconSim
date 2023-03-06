// Package flags provides tracking of effects that can be applied to objects.
package flags

type Holder struct {
	values map[string]int
	floors map[string]int
	roofs  map[string]int
}

// New returns an empty Holder.
func New() *Holder {
	return &Holder{
		values: make(map[string]int),
		floors: make(map[string]int),
		roofs:  make(map[string]int),
	}
}

// AddFlag increments the given flag, returning the new value.
func (fh *Holder) AddFlag(f string, a int) int {
	if fh == nil {
		return 0
	}
	nval := fh.values[f] + a
	if lim, ok := fh.roofs[f]; ok && nval > lim {
		nval = lim
	}
	if lim, ok := fh.floors[f]; ok && nval < lim {
		nval = lim
	}
	fh.values[f] = nval
	return nval
}

// Add increases the value of all flags by those in the provided holder.
func (fh *Holder) Add(o *Holder) {
	if fh == nil || o == nil {
		return
	}
	for k, v := range o.values {
		fh.AddFlag(k, v)
	}
}

// Clear clears all flags.
func (fh *Holder) Clear() {
	fh.values = make(map[string]int)
}

// ClearFlag removes the provided flag.
func (fh *Holder) ClearFlag(f string) {
	if fh == nil {
		return
	}
	delete(fh.values, f)
}

// IncFlag increments the given flag by one, returning the new value.
func (fh *Holder) IncFlag(f string) int {
	return fh.AddFlag(f, 1)
}

// SetFloor sets a minimum for the provided flag. If the value is higher
// than an existing ceiling it will be set to equal the ceiling. An existing
// value will be capped.
func (fh *Holder) SetFloor(f string, val int) {
	if fh == nil {
		return
	}
	if max, ok := fh.roofs[f]; ok && val > max {
		val = max
	}
	if exist := fh.values[f]; exist < val {
		fh.values[f] = val
	}
	fh.floors[f] = val
}

// SetCeiling sets a maximum for the provided flag. If the value is lower
// than an existing floor it will be set to equal the floor. An existing
// value will be capped.
func (fh *Holder) SetCeiling(f string, val int) {
	if fh == nil {
		return
	}
	if min, ok := fh.floors[f]; ok && val < min {
		val = min
	}
	if exist := fh.values[f]; exist > val {
		fh.values[f] = val
	}
	fh.roofs[f] = val
}

// Value returns the value of the given flag.
func (fh *Holder) Value(f string) int {
	if fh == nil {
		return 0
	}
	return fh.values[f]
}
