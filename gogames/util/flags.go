// Package flags provides tracking of effects that can be applied to objects.
package flags

type Holder struct {
  flags map[string]int
}

// AddFlag increments the given flag, returning the new value.
func (fh *Holder) AddFlag(f string, a int) int {
  if fh == nil {
    return 0
  }
  fh.flags[f] += a
  return fh.flags[f]
}

// Clear clears all flags.
func (fh *Holder) Clear() {
  fh.flags = make(map[string]int)
}

// ClearFlag removes the provided flag.
func (fh *Holder) ClearFlag(f string) {
  if fh == nil {
    return
  }
  delete(fh.flags, f)
}

// IncFlag increments the given flag by one, returning the new value.
func (fh *Holder) IncFlag(f string) int {
  return fh.AddFlag(f, 1)
}

// New returns an empty Holder.
func New() *Holder {
  return &Holder{
    flags: make(map[string]int),
  }
}

// Value returns the value of the given flag.
func (fh *Holder) Value(f string) int {
  if fh == nil {
    return 0
  }
  return fh.flags[f]
}
