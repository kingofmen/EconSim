// Package flags provides tracking of effects that can be applied to objects.
package flags

type FlagHolder struct {
  flags map[string]int
}

// AddFlag increments the given flag, returning the new value.
func (fh *FlagHolder) AddFlag(f string, a int) int {
  if fh == nil {
    return 0
  }
  fh.flags[f] += a
  return fh.flags[f]
}

// Clear clears all flags.
func (fh *FlagHolder) Clear() {
  fh.flags = make(map[string]int)
}

// ClearFlag removes the provided flag.
func (fh *FlagHolder) ClearFlag(f string) {
  if fh == nil {
    return
  }
  delete(fh.flags, f)
}

// IncFlag increments the given flag by one, returning the new value.
func (fh *FlagHolder) IncFlag(f string) int {
  return fh.AddFlag(f, 1)
}

// New returns an empty FlagHolder.
func New() *FlagHolder {
  return &FlagHolder{
    flags: make(map[string]int),
  }
}

// Value returns the value of the given flag.
func (fh *FlagHolder) Value(f string) int {
  if fh == nil {
    return 0
  }
  return fh.flags[f]
}
