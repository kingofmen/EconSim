// Package logic provides boolean logic on predicates defined by protobufs.
package logic

import (
	"fmt"

	lpb "gogames/util/logic_go_proto"
)

type Lookup interface {
	GetInt(key string) (int32, error)
	GetStr(key string) (string, error)
}

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

func evalComparison(comp *lpb.Compare, lookup Lookup) (bool, error) {
	one, err := lookup.GetInt(comp.GetKeyOne())
	if err != nil {
		return false, err
	}
	two, err := lookup.GetInt(comp.GetKeyTwo())
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
	return false, fmt.Errorf("cannot evaluate unknown operator %d %v %d", one, comp.GetOperation(), two)
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
