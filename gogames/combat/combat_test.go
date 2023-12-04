package combat

import (
	"testing"
)

var (
	testArmies map[ArmyId]*testArmy
	stances    map[[2]int]Attitude
)

type testArmy struct {
	uuid    ArmyId
	faction int
}

func (a *testArmy) GetArmyId() ArmyId {
	if a == nil {
		return 0
	}
	return a.uuid
}

func (a *testArmy) Stance(aid ArmyId) Attitude {
	if a == nil {
		return Hostile
	}
	other, ok := testArmies[aid]
	if !ok {
		return Hostile
	}

	if other.faction == a.faction {
		return Allied
	}

	if stances != nil {
		key := [2]int{a.faction, other.faction}
		if key[0] > key[1] {
			key[0], key[1] = key[1], key[0]
		}
		if att, ok := stances[key]; ok {
			return att
		}
		return Neutral
	}

	return Hostile
}

func mapArmies(armies ...*testArmy) {
	testArmies = make(map[ArmyId]*testArmy)
	for _, army := range armies {
		testArmies[army.uuid] = army
	}
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
	if len(one.Attackers) != len(two.Attackers) {
		return false
	}
	for idx, aid := range one.Attackers {
		if aid != two.Attackers[idx] {
			return false
		}
	}
	if len(one.Defenders) != len(two.Defenders) {
		return false
	}
	for idx, aid := range one.Defenders {
		if aid != two.Defenders[idx] {
			return false
		}
	}
	return true
}

func TestGenerate(t *testing.T) {
	cases := []struct {
		desc    string
		armies  []*testArmy
		stances map[[2]int]Attitude
		want    []*Action
	}{
		{
			desc: "No armies, no action",
		},
		{
			desc: "Single army, no action",
			armies: []*testArmy{
				&testArmy{uuid: 1, faction: 0},
			},
		},
		{
			desc: "Allied armies, no combat",
			armies: []*testArmy{
				&testArmy{uuid: 1, faction: 0},
				&testArmy{uuid: 2, faction: 0},
			},
		},
		{
			desc: "Two hostile armies, one battle",
			armies: []*testArmy{
				&testArmy{uuid: 1, faction: 0},
				&testArmy{uuid: 2, faction: 1},
			},
			want: []*Action{
				&Action{
					Attackers: []ArmyId{1},
					Defenders: []ArmyId{2},
				},
			},
		},
		{
			desc: "Two hostile factions, one battle",
			armies: []*testArmy{
				&testArmy{uuid: 1, faction: 0},
				&testArmy{uuid: 4, faction: 1},
				&testArmy{uuid: 2, faction: 1},
				&testArmy{uuid: 3, faction: 0},
			},
			want: []*Action{
				&Action{
					Attackers: []ArmyId{1, 3},
					Defenders: []ArmyId{4, 2},
				},
			},
		},
		{
			desc: "Switch order of battle",
			armies: []*testArmy{
				&testArmy{uuid: 4, faction: 1},
				&testArmy{uuid: 1, faction: 0},
				&testArmy{uuid: 2, faction: 1},
				&testArmy{uuid: 3, faction: 0},
			},
			want: []*Action{
				&Action{
					Attackers: []ArmyId{4, 2},
					Defenders: []ArmyId{1, 3},
				},
			},
		},
		{
			desc: "Three mutually hostile armies, three battles",
			armies: []*testArmy{
				&testArmy{uuid: 1, faction: 1},
				&testArmy{uuid: 2, faction: 2},
				&testArmy{uuid: 3, faction: 3},
			},
			want: []*Action{
				&Action{
					Attackers: []ArmyId{1},
					Defenders: []ArmyId{2},
				},
				&Action{
					Attackers: []ArmyId{1},
					Defenders: []ArmyId{3},
				},
				&Action{
					Attackers: []ArmyId{2},
					Defenders: []ArmyId{3},
				},
			},
		},
		{
			desc: "Massive clusterfuck",
			armies: []*testArmy{
				&testArmy{uuid: 0, faction: 1}, // Allied to 2, 3, 4; fighting 5.
				&testArmy{uuid: 1, faction: 1},
				&testArmy{uuid: 2, faction: 2}, // Fighting 6.
				&testArmy{uuid: 3, faction: 3}, // Hostile to 4 in spite of alliance; also to 5 and 6.
				&testArmy{uuid: 4, faction: 4}, // Separately hostile to 5.
				&testArmy{uuid: 5, faction: 5},
				&testArmy{uuid: 6, faction: 6},
				&testArmy{uuid: 7, faction: 7}, // Neutral all around.
				&testArmy{uuid: 8, faction: 8}, // Hostile to absolutely everyone (except 7 and 9).
				&testArmy{uuid: 9, faction: 9}, // Everyone's friend!
			},
			stances: map[[2]int]Attitude{
				{1, 2}: Allied,
				{1, 3}: Allied,
				{1, 4}: Allied,
				{1, 5}: Hostile,
				{1, 8}: Hostile,
				{1, 9}: Allied,
				{2, 6}: Hostile,
				{2, 8}: Hostile,
				{2, 9}: Allied,
				{3, 4}: Hostile,
				{3, 5}: Hostile,
				{3, 6}: Hostile,
				{3, 8}: Hostile,
				{3, 9}: Allied,
				{4, 5}: Hostile,
				{4, 6}: Hostile,
				{4, 8}: Hostile,
				{4, 9}: Allied,
				{5, 6}: Allied,
				{5, 8}: Hostile,
				{5, 9}: Allied,
				{6, 8}: Hostile,
				{6, 9}: Allied,
				{7, 9}: Allied,
				{8, 9}: Allied,
			},
			want: []*Action{
				// 1 and allies fight 5 and 6.
				&Action{
					Attackers: []ArmyId{0, 1, 3, 2},
					Defenders: []ArmyId{5, 6},
				},
				// 1 and allies separately fight 8.
				&Action{
					Attackers: []ArmyId{0, 1, 2, 3},
					Defenders: []ArmyId{8},
				},
				// 3 and 4 fight.
				&Action{
					Attackers: []ArmyId{3},
					Defenders: []ArmyId{4},
				},
				// 4 fights 5 and 6 separately.
				&Action{
					Attackers: []ArmyId{4},
					Defenders: []ArmyId{5, 6},
				},
				// 4 separately fights 8.
				&Action{
					Attackers: []ArmyId{4},
					Defenders: []ArmyId{8},
				},
				// 5 and 6 fight 8, as one does.
				&Action{
					Attackers: []ArmyId{5, 6},
					Defenders: []ArmyId{8},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			mapArmies(cc.armies...)
			stances = cc.stances
			combaters := make([]Combater, 0, len(cc.armies))
			for _, army := range cc.armies {
				combaters = append(combaters, army)
			}
			got := Generate(combaters...)
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
