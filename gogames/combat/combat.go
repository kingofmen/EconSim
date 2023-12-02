// Package combat defines an interface for armies and resolves combats between them.
package combat

import (
	"fmt"
)

type ActionType int
type ArmyId int

const (
	Attrition ActionType = iota
	Scouting
	Raiding
	Battle
)

// String returns a human-readable string suitable for debugging;
// it is not meant to be user-visible.
func (a ActionType) String() string {
	switch a {
	case Attrition:
		return "attrition"
	case Scouting:
		return "scouting"
	case Raiding:
		return "raiding"
	case Battle:
		return "battle"
	default:
		return "unknown action type"
	}
}

type Combater interface {
	GetArmyId() ArmyId
}

type Action struct {
	Kind       ActionType
	Combatants []ArmyId
}

// String returns a human-readable string suitable for debugging;
// it is not meant to be user-visible.
func (a *Action) String() string {
	if a == nil {
		return "<nil>"
	}
	return fmt.Sprintf("%s: %v", a.Kind, a.Combatants)
}

func Generate(armies ...Combater) []*Action {
	numArmies := len(armies)
	if numArmies == 0 {
		return nil
	}
	actions := make([]*Action, 0, numArmies*2)
	// Every army takes attrition.
	for _, army := range armies {
		actions = append(actions, &Action{
			Kind:       Attrition,
			Combatants: []ArmyId{army.GetArmyId()},
		})
	}

	return actions
}
