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

func (e *Engine) Turn(oldState *gmpb.GameState) (*gmpb.GameState, error) {
	newState := proto.Clone(oldState).(*gmpb.GameState)
	board := newState.GetBoard()
	pops := make([]*poppb.Pop, 0, len(board.GetTiles())*10)
	for _, tile := range board.GetTiles() {
		for _, pop := range tile.GetPops() {
			pops = append(pops, pop)
		}
	}

	trades := make(map[*poppb.Pop][]*poppb.Pop)
	// TODO: Trade goes here.

	if err := e.popMgr.Produce(pops, trades); err != nil {
		return oldState, fmt.Errorf("unable to run production: %w", err)
	}

	// TODO: Raids and battles go here.

	if err := e.popMgr.Consume(pops); err != nil {
		return oldState, fmt.Errorf("unable to run consumption: %w", err)
	}
	if err := e.popMgr.Demographics(pops); err != nil {
		return oldState, fmt.Errorf("unable to run demographics: %w", err)
	}
	if err := e.popMgr.Evolve(pops); err != nil {
		return oldState, fmt.Errorf("unable to run updates: %w", err)
	}

	// TODO: Scoring goes here.

	return newState, nil
}
