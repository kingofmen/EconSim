// Package logic provides boolean logic on predicates defined by protobufs.
package logic

import (
	"fmt"
	"sort"
	"strconv"
	"strings"

	lpb "gogames/util/logic_go_proto"
)

const (
	kScopeSeparator = "."
)

type Lookup interface {
	GetInt(key string) (int32, error)
	GetStr(key string) (string, error)
	GetStrArr(key string) ([]string, error)
	GetScope(key string) Lookup
	SetScope(key string, scope Lookup)
	ListScopes() []string
}

// Scoper provides a default in-memory scope-management object
// which implementors of the Lookup interface can embed if they
// don't want to roll their own.
type Scoper struct {
	scopes map[string]Lookup
}

// GetScope returns the given lookup scope.
func (sc *Scoper) GetScope(key string) Lookup {
	if sc == nil {
		return nil
	}
	return sc.scopes[key]
}

// SetScope sets the key to point to the scope.
func (sc *Scoper) SetScope(key string, scope Lookup) {
	if sc == nil {
		return
	}
	if sc.scopes == nil {
		sc.scopes = make(map[string]Lookup)
	}
	sc.scopes[key] = scope
}

// ListScopes returns the keys of any existing scopes.
func (sc *Scoper) ListScopes() []string {
	if sc == nil {
		return nil
	}
	ret := make([]string, 0, len(sc.scopes))
	for key := range sc.scopes {
		ret = append(ret, key)
	}
	sort.Strings(ret)
	return ret
}

// evalCombination returns true if the logical expression is true.
func evalCombination(comb *lpb.Combine, lookup Lookup) (bool, error) {
	switch comb.GetOperation() {
	case lpb.Combine_IF_ALL:
		for _, p := range comb.GetOperands() {
			v, err := Eval(p, lookup)
			if err != nil {
				return false, err
			}
			if !v {
				return false, nil
			}
		}
		return true, nil
	case lpb.Combine_IF_ANY:
		for _, p := range comb.GetOperands() {
			v, err := Eval(p, lookup)
			if err != nil {
				return false, err
			}
			if v {
				return true, nil
			}
		}
		return false, nil
	case lpb.Combine_IF_NONE:
		for _, p := range comb.GetOperands() {
			v, err := Eval(p, lookup)
			if err != nil {
				return false, err
			}
			if v {
				return false, nil
			}
		}
		return true, nil
	}
	return false, nil
}

// getInt returns an integer either because key is a literal,
// or from the lookup table.
func getInt(key string, lookup Lookup) (int32, error) {
	if val, err := strconv.Atoi(key); err == nil {
		return int32(val), nil
	}
	scope, skey, has := strings.Cut(key, kScopeSeparator)
	if has {
		slookup := lookup.GetScope(scope)
		if slookup == nil {
			return 0, fmt.Errorf("invalid scope lookup %q from integer key %q", scope, key)
		}
		return getInt(skey, slookup)
	}

	return lookup.GetInt(key)
}

// getStr returns a string either because key is a literal,
// or from the lookup table.
func getStr(key string, lookup Lookup) (string, error) {
	if len(key) > 0 && key[0] == byte('\'') {
		return key[1:], nil
	}
	scope, skey, has := strings.Cut(key, kScopeSeparator)
	if has {
		slookup := lookup.GetScope(scope)
		if slookup == nil {
			return "", fmt.Errorf("invalid scope lookup %q from string key %q", scope, key)
		}
		return getStr(skey, slookup)
	}
	return lookup.GetStr(key)
}

// getStrArr returns a string array, either from the lookup table
// or from parsing an array literal.
func getStrArr(key string, lookup Lookup) ([]string, error) {
	if l, ok := strings.CutSuffix(key, "]"); ok {
		if literal, ok := strings.CutPrefix(l, "["); ok {
			entries := strings.Split(literal, ",")
			for idx, entry := range entries {
				val, err := getStr(strings.Trim(entry, " "), lookup)
				if err != nil {
					return nil, fmt.Errorf("error constructing array entry %q: %w", entry, err)
				}
				entries[idx] = val
			}
			return entries, nil
		}
	}
	scope, skey, has := strings.Cut(key, kScopeSeparator)
	if has {
		slookup := lookup.GetScope(scope)
		if slookup == nil {
			return nil, fmt.Errorf("invalid scope lookup %q from string array key %q", scope, key)
		}
		return getStrArr(skey, slookup)
	}
	return lookup.GetStrArr(key)
}

// evalIntComparison returns the truth-value of the integer predicate.
func evalIntComparison(comp *lpb.Compare, lookup Lookup) (bool, error) {
	one, err := getInt(comp.GetKeyOne(), lookup)
	if err != nil {
		return false, err
	}
	two, err := getInt(comp.GetKeyTwo(), lookup)
	if err != nil {
		return false, err
	}
	switch comp.GetOperation() {
	case lpb.Compare_CMP_GT:
		return one > two, nil
	case lpb.Compare_CMP_LT:
		return one < two, nil
	case lpb.Compare_CMP_EQ:
		return one == two, nil
	case lpb.Compare_CMP_GTE:
		return one >= two, nil
	case lpb.Compare_CMP_LTE:
		return one <= two, nil
	case lpb.Compare_CMP_NEQ:
		return one != two, nil
	}
	return false, fmt.Errorf("cannot evaluate unknown (int) operator %d %v %d", one, comp.GetOperation(), two)
}

// evalStrComparison returns the truth-value of the string predicate.
func evalStrComparison(comp *lpb.Compare, lookup Lookup) (bool, error) {
	one, err := getStr(comp.GetKeyOne(), lookup)
	if err != nil {
		return false, err
	}
	op := comp.GetOperation()
	key := comp.GetKeyTwo()
	if op == lpb.Compare_CMP_STRIN {
		return evalStrIn(one, key, lookup)
	}
	two, err := getStr(key, lookup)
	if err != nil {
		return false, err
	}
	switch op {
	case lpb.Compare_CMP_STREQ:
		return one == two, nil
	}
	return false, fmt.Errorf("cannot evaluate unknown (string) operator %q %v %q", one, op, two)
}

// evalStrIn returns whether key is in the array.
func evalStrIn(key, arrKey string, lookup Lookup) (bool, error) {
	arr, err := getStrArr(arrKey, lookup)
	if err != nil {
		return false, err
	}
	for _, val := range arr {
		if key == val {
			return true, nil
		}
	}
	return false, nil
}

func evalComparison(comp *lpb.Compare, lookup Lookup) (bool, error) {
	op := comp.GetOperation()
	// Check for string operations.
	if op == lpb.Compare_CMP_STREQ || op == lpb.Compare_CMP_STRIN {
		return evalStrComparison(comp, lookup)
	}

	return evalIntComparison(comp, lookup)
}

// Eval returns the truth value of the predicate.
func Eval(pred *lpb.Predicate, lookup Lookup) (bool, error) {
	if comb := pred.GetComb(); comb != nil {
		return evalCombination(comb, lookup)
	}
	if comp := pred.GetComp(); comp != nil {
		return evalComparison(comp, lookup)
	}
	return false, nil
}

// TODO: Compile function.
