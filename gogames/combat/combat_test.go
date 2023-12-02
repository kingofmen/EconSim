package combat

import (
	"testing"
)

type TestArmy struct {
	uuid ArmyId
}

func (a *TestArmy) GetArmyId() ArmyId {
	if a == nil {
		return 0
	}
	return a.uuid
}

func equalActions(one, two *Action) bool {
	if one == nil && two == nil {
		return true
	}
	if one == nil || two == nil {
		return false
	}
	if one.Kind != two.Kind {
		return false
	}
	if len(one.Combatants) != len(two.Combatants) {
		return false
	}
	for idx, aid := range one.Combatants {
		if aid != two.Combatants[idx] {
			return false
		}
	}
	return true
}

func TestGenerate(t *testing.T) {
	cases := []struct {
		desc   string
		armies []Combater
		want   []*Action
	}{
		{
			desc: "No armies",
		},
		/*
			{
				desc: "Attrition",
			},
		*/
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := Generate(cc.armies...)
			if len(got) != len(cc.want) {
				t.Errorf("%s: Generate() => %d actions, want %d; actions: %v", cc.desc, len(got), len(cc.want), got)
				return
			}
			for idx, act := range got {
				if !equalActions(act, cc.want[idx]) {
					t.Errorf("%s: Generate() => action %d differs, got %s, want %s", cc.desc, idx, act, cc.want[idx])
				}
			}
		})
	}
}
