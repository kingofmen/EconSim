// Package handler implements an interface to client libraries.
package handler

import (
	"fmt"
	"sync"

	"gogames/raubgraf/engine/engine"

	spb "gogames/raubgraf/protos/state_proto"
)

var (
	// memGames caches games in memory. For simple debugging it can
	// be the only game storage.
	memGames = map[int]*engine.RaubgrafGame{}

	mu sync.Mutex
)

// CreateGame creates a new game and returns its ID.
func CreateGame(width, height int) (int, error) {
	game, err := engine.NewGame(width, height)
	if err != nil {
		return 0, err
	}
	mu.Lock()
	defer mu.Unlock()
	idx := len(memGames)
	memGames[idx] = game
	return idx, nil
}

// GetGameState returns the protobuf encoding of the given game.
func GetGameState(gid int) (*spb.Board, error) {
	game, ok := memGames[gid]
	if !ok {
		return nil, fmt.Errorf("not found: Game %d is not in memory", gid)
	}
	bb := game.GetBoard()
	if bb == nil {
		return nil, fmt.Errorf("bad state: Game %d has no board", gid)
	}
	return bb.ToProto()
}
