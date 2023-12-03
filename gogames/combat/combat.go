// Package combat defines an interface for armies and resolves combats between them.
package combat

import (
	"fmt"
)

type ActionType int
type ArmyId int

const (
	Skirmish ActionType = iota
	Ambush
	Recon
	Meeting
	SetPiece
)

// String returns a human-readable string suitable for debugging;
// it is not meant to be user-visible.
func (a ActionType) String() string {
	switch a {
	case Skirmish:
		return "skirmish"
	case Ambush:
		return "ambush"
	case Recon:
		return "recon"
	case Meeting:
		return "meeting"
	case SetPiece:
		return "setpiece"
	default:
		return "unknown action type"
	}
}

type Combater interface {
	GetArmyId() ArmyId
}

type Action struct {
	Kind      ActionType
	Attackers []ArmyId
	Defenders []ArmyId
}

// String returns a human-readable string suitable for debugging;
// it is not meant to be user-visible.
func (a *Action) String() string {
	if a == nil {
		return "<nil>"
	}
	return fmt.Sprintf("%s: %v attacks %v", a.Kind, a.Attackers, a.Defenders)
}

func Generate(armies ...Combater) []*Action {
	numArmies := len(armies)
	if numArmies < 2 {
		return nil
	}
	actions := make([]*Action, 0, numArmies*2)
	return actions
}
