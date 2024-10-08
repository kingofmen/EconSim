package logic

import (
	"fmt"
)

// TestLookup implements Lookup in memory, for easy testing.
type TestLookup struct {
	Scoper
	ints    map[string]int32
	strs    map[string]string
	strarrs map[string][]string
}

// NewTestLookup returns an empty TestLookup.
func NewTestLookup() *TestLookup {
	return &TestLookup{
		ints:    make(map[string]int32),
		strs:    make(map[string]string),
		strarrs: make(map[string][]string),
	}
}

func (tl *TestLookup) GetInt(key string) (int32, error) {
	if tl == nil {
		return 0, fmt.Errorf("nil lookup object")
	}
	val, ok := tl.ints[key]
	if !ok {
		return 0, fmt.Errorf("unknown key %q", key)
	}
	return val, nil
}

func (tl *TestLookup) GetStr(key string) (string, error) {
	if tl == nil {
		return "", fmt.Errorf("nil lookup object")
	}
	val, ok := tl.strs[key]
	if !ok {
		return "", fmt.Errorf("unknown key %q", key)
	}
	return val, nil
}

func (tl *TestLookup) GetStrArr(key string) ([]string, error) {
	if tl == nil {
		return nil, fmt.Errorf("nil lookup object")
	}
	val, ok := tl.strarrs[key]
	if !ok {
		return nil, fmt.Errorf("unknown key %q", key)
	}
	return val, nil
}

func (tl *TestLookup) WithInt(key string, val int32) *TestLookup {
	if tl == nil {
		tl = NewTestLookup()
	}
	tl.ints[key] = val
	return tl
}

func (tl *TestLookup) WithStr(key string, val string) *TestLookup {
	if tl == nil {
		tl = NewTestLookup()
	}
	tl.strs[key] = val
	return tl
}

func (tl *TestLookup) WithStrArr(key string, val []string) *TestLookup {
	if tl == nil {
		tl = NewTestLookup()
	}
	tl.strarrs[key] = val
	return tl
}
