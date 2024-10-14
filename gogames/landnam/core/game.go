// Package game contains the core game-loop logic for landnam.
package game

import (
	"fmt"

	"gogames/combat/tables"
	"gogames/landnam/population/pop"
	"gogames/util/logic"
	"google.golang.org/protobuf/proto"

	gmpb "gogames/landnam/core/game_go_proto"
	poppb "gogames/landnam/population/pop_go_proto"
)

// CampaignLookup handles calculations for combat. It satisfies
// logic.Lookup.
type CampaignLookup struct {
	logic.Scoper
	attackers []*poppb.Pop
	defenders []*poppb.Pop
}

type Engine struct {
	popMgr *pop.Manager
	cmpMgr *CampaignLookup
	combat *tables.Lookup
	dice   tables.DieRoller
}

// GetInt returns an integer value for the campaign.
func (cmp *CampaignLookup) GetInt(key string) (int32, error) {
	return 0, fmt.Errorf("unknown key %q for GetInt", key)
}

func (cmp *CampaignLookup) GetStr(key string) (string, error) {
	return "", fmt.Errorf("unknown string key %q", key)
}

func (cmp *CampaignLookup) GetStrArr(key string) ([]string, error) {
	return nil, fmt.Errorf("unknown string array key %q", key)
}

func (e *Engine) handleCampaign(cmp *gmpb.Campaign, popMap map[string]*poppb.Pop) error {
	if e.cmpMgr == nil {
		e.cmpMgr = &CampaignLookup{}
	}
	e.cmpMgr.attackers = make([]*poppb.Pop, 0, len(cmp.GetAttackerIds()))
	e.cmpMgr.defenders = make([]*poppb.Pop, 0, len(cmp.GetTargetIds()))

	for _, attid := range cmp.GetAttackerIds() {
		if pop, ok := popMap[attid]; ok {
			e.cmpMgr.attackers = append(e.cmpMgr.attackers, pop)
		}
	}
	for _, trgid := range cmp.GetTargetIds() {
		if pop, ok := popMap[trgid]; ok {
			e.cmpMgr.defenders = append(e.cmpMgr.defenders, pop)
		}
	}

	_, err := e.combat.Resolve(cmp.GetStartKey(), e.dice, e.cmpMgr)
	if err != nil {
		return err
	}

	return nil
}

func (e *Engine) Turn(oldState *gmpb.GameState) (*gmpb.GameState, error) {
	newState := proto.Clone(oldState).(*gmpb.GameState)
	board := newState.GetBoard()
	popList := make([]*poppb.Pop, 0, len(board.GetTiles())*10)
	popMap := make(map[string]*poppb.Pop)
	for _, tile := range board.GetTiles() {
		for _, pop := range tile.GetPops() {
			popList = append(popList, pop)
			popMap[pop.GetKey()] = pop
		}
	}

	trades := make(map[*poppb.Pop][]*poppb.Pop)
	// TODO: Trade goes here.

	if err := e.popMgr.Produce(popList, trades); err != nil {
		return oldState, fmt.Errorf("unable to run production: %w", err)
	}

	for _, cmp := range newState.GetCampaigns() {
		if err := e.handleCampaign(cmp, popMap); err != nil {
			return oldState, fmt.Errorf("error handling campaign: %w", err)
		}
	}

	if err := e.popMgr.Consume(popList); err != nil {
		return oldState, fmt.Errorf("unable to run consumption: %w", err)
	}
	if err := e.popMgr.Demographics(popList); err != nil {
		return oldState, fmt.Errorf("unable to run demographics: %w", err)
	}
	if err := e.popMgr.Evolve(popList); err != nil {
		return oldState, fmt.Errorf("unable to run updates: %w", err)
	}

	// TODO: Scoring goes here.

	return newState, nil
}
