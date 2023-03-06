package war

import (
	"testing"
)

func lightCav() *FieldUnit {
	return &FieldUnit{
		drill: 4,
		equip: 2,
		mount: 3,
		state: Fresh,
	}
}

func heavyCav() *FieldUnit {
	return &FieldUnit{
		drill: 4,
		equip: 8,
		mount: 6,
		state: Fresh,
	}
}

func lightInf() *FieldUnit {
	return &FieldUnit{
		drill: 4,
		equip: 2,
		mount: 0,
		state: Fresh,
	}
}

func heavyInf() *FieldUnit {
	return &FieldUnit{
		drill: 4,
		equip: 8,
		mount: 0,
		state: Fresh,
	}
}

func levies() *FieldUnit {
	return &FieldUnit{
		drill: 0,
		equip: 0,
		mount: 0,
		state: Fresh,
	}
}

func TestBattle(t *testing.T) {
	cases := []struct {
		desc      string
		attackers []*FieldUnit
		defenders []*FieldUnit
		aState    UnitState
		result    BattleResult
	}{
		{
			desc: "Equal units are a draw",
			attackers: []*FieldUnit{
				lightInf(), heavyInf(), lightCav(), heavyCav(),
			},
			defenders: []*FieldUnit{
				lightInf(), heavyInf(), lightCav(), heavyCav(),
			},
			result: Draw,
		},
		{
			desc: "Unit state matters",
			attackers: []*FieldUnit{
				lightInf(), heavyInf(), lightCav(), heavyCav(),
			},
			defenders: []*FieldUnit{
				lightInf(), heavyInf(), lightCav(), heavyCav(),
			},
			aState: Tired,
			result: MinorLoss,
		},
		{
			desc: "Heavy infantry beats light",
			attackers: []*FieldUnit{
				heavyInf(),
			},
			defenders: []*FieldUnit{
				lightInf(),
			},
			result: Victory,
		},
		{
			desc: "Levies are useless",
			attackers: []*FieldUnit{
				heavyInf(),
			},
			defenders: []*FieldUnit{
				levies(),
			},
			result: Overrun,
		},
		{
			desc: "Levies are also useless offensively",
			attackers: []*FieldUnit{
				levies(),
			},
			defenders: []*FieldUnit{
				heavyInf(),
			},
			result: Disaster,
		},
		{
			desc: "Heavy infantry cannot catch light cavalry",
			attackers: []*FieldUnit{
				heavyInf(),
			},
			defenders: []*FieldUnit{
				lightCav(),
			},
			result: Draw,
		},
		{
			desc: "Light cav runs away from heavy",
			attackers: []*FieldUnit{
				heavyCav(),
			},
			defenders: []*FieldUnit{
				lightCav(),
			},
			result: BreakContact,
		},
		{
			desc: "Cavalry beats infantry",
			attackers: []*FieldUnit{
				heavyCav(), heavyCav(), lightCav(),
			},
			defenders: []*FieldUnit{
				heavyInf(), heavyInf(), lightInf(),
			},
			result: Victory,
		},
		{
			desc: "Combined arms more powerful",
			attackers: []*FieldUnit{
				heavyCav(), heavyCav(), lightCav(),
			},
			defenders: []*FieldUnit{
				heavyInf(), heavyInf(), lightCav(),
			},
			result: MinorVictory,
		},
		{
			desc: "Cavalry doesn't beat lots of infantry",
			attackers: []*FieldUnit{
				heavyCav(), heavyCav(), lightCav(),
			},
			defenders: []*FieldUnit{
				heavyInf(), heavyInf(), heavyInf(), lightInf(), lightInf(), lightInf(),
			},
			result: Draw,
		},
		{
			desc: "Combined arms will help",
			attackers: []*FieldUnit{
				heavyCav(), heavyCav(), lightInf(),
			},
			defenders: []*FieldUnit{
				heavyInf(), heavyInf(), lightInf(), lightInf(), lightInf(), lightInf(),
			},
			result: MinorVictory,
		},
		{
			desc: "Quantity has a quality all its own",
			attackers: []*FieldUnit{
				heavyInf(),
			},
			defenders: []*FieldUnit{
				lightInf(), lightInf(), lightInf(),
				levies(), levies(), levies(), levies(), levies(), levies(), levies(), levies(),
				levies(), levies(), levies(), levies(), levies(), levies(), levies(), levies(),
				levies(), levies(), levies(), levies(), levies(), levies(), levies(), levies(),
				levies(), levies(), levies(), levies(), levies(), levies(), levies(), levies(),
			},
			result: Loss,
		},
		{
			desc: "Good scouts will not attack superior force",
			attackers: []*FieldUnit{
				lightCav(), lightCav(), lightCav(),
			},
			defenders: []*FieldUnit{
				heavyInf(), heavyInf(), heavyInf(), heavyInf(), heavyInf(),
			},
			result: Standoff,
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			for _, a := range cc.attackers {
				a.state = cc.aState
			}
			if got := Battle(cc.attackers, cc.defenders); got != cc.result {
				attC, attS := power(cc.attackers)
				defC, defS := power(cc.defenders)
				t.Errorf("%s: Battle() => %s, expect %s Power (%.2f, %.2f) vs (%.2f, %.2f)", cc.desc, got, cc.result, attC, attS, defC, defS)
			}
		})
	}
}
