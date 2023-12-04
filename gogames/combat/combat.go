// Package combat defines an interface for armies and resolves combats between them.
package combat

import (
	"fmt"
)

type ActionType int
type Attitude int
type ArmyId int

const (
	Skirmish ActionType = iota
	Ambush
	Recon
	Meeting
	SetPiece

	Hostile Attitude = iota
	Neutral
	Allied
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
	Stance(aid ArmyId) Attitude
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

// canJoinAgainst returns true if army is allied to at least one
// army ID in join, and hostile to none; and is hostile to at least
// one army in against, and allied to none.
func canJoinAgainst(army Combater, join, against []ArmyId) bool {
	count := 0
	for _, jid := range join {
		switch army.Stance(jid) {
		case Allied:
			count++
		case Hostile:
			return false
		default:
			continue
		}
	}
	if count == 0 {
		return false
	}

	count = 0
	for _, aid := range against {
		switch army.Stance(aid) {
		case Allied:
			return false
		case Hostile:
			count++
		default:
			continue
		}
	}
	return count > 0
}

// Generate creates battles between each pair of hostile armies.
func Generate(armies ...Combater) []*Action {
	numArmies := len(armies)
	if numArmies < 2 {
		return nil
	}
	fighting := make(map[ArmyId]map[ArmyId]bool)
	for _, army := range armies {
		fighting[army.GetArmyId()] = make(map[ArmyId]bool)
	}
	// Battles are created for each pair of hostile armies
	// which don't already have a battle between them. Armies
	// join battles if they are allied to at least one army
	// on one side, and hostile to nobody on that side; and
	// are hostile to at least one and allied to none on the
	// other.
	// In the case where an army would join a battle but is
	// prevented by hostility on one side, or alliance on the
	// other, joining is in order of appearance in the input.
	// Example: France is allied to Britain and Italy against
	// Germany; but Britain is hostile to Italy. Whichever of
	// the three allies is first in the input creates a battle
	// against Germany. The ally first in the list and able to
	// join, does; the other creates a separate battle. This
	// means France will be joined by either Britain or Italy,
	// depending on the order; and the other will fight Germany
	// separately.
	actions := make([]*Action, 0, numArmies*2)
	for idx, curr := range armies {
		currid := curr.GetArmyId()
		for _, cand := range armies[idx+1:] {
			candid := cand.GetArmyId()
			if curr.Stance(candid) != Hostile {
				continue
			}
			if fighting[currid][candid] {
				continue
			}
			action := &Action{
				Attackers: []ArmyId{currid},
				Defenders: []ArmyId{candid},
			}
			actions = append(actions, action)
			fighting[currid][candid] = true
			fighting[candid][currid] = true
		allyLoop:
			for _, ally := range armies[idx+1:] {
				allyid := ally.GetArmyId()
				if allyid == currid {
					continue
				}
				if allyid == candid {
					continue
				}
				if fighting[allyid][currid] {
					continue
				}
				if fighting[allyid][candid] {
					continue
				}
				if canJoinAgainst(ally, action.Attackers, action.Defenders) {
					action.Attackers = append(action.Attackers, allyid)
					for _, eid := range action.Defenders {
						fighting[allyid][eid] = true
						fighting[eid][allyid] = true
					}
					goto allyLoop
				}
				if canJoinAgainst(ally, action.Defenders, action.Attackers) {
					action.Defenders = append(action.Defenders, allyid)
					for _, eid := range action.Attackers {
						fighting[allyid][eid] = true
						fighting[eid][allyid] = true
					}
					goto allyLoop
				}
			}
		}
	}
	return actions
}
