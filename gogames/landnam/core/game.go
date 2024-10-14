// Package game contains the core game-loop logic for landnam.
package game

import (
	"fmt"

	"gogames/landnam/population/pop"
	"google.golang.org/protobuf/proto"

	gmpb "gogames/landnam/core/game_go_proto"
	poppb "gogames/landnam/population/pop_go_proto"
)

type Engine struct {
	popMgr *pop.Manager
}

func (e *Engine) handleCampaign(cmp *gmpb.Campaign, popMap map[string]*poppb.Pop) error {
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
