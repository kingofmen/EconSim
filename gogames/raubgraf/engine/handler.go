// Package handler implements an interface to client libraries.
package handler

import (
	"fmt"
	"sync"

	"gogames/raubgraf/engine/board"
	"gogames/raubgraf/engine/engine"
	"google.golang.org/protobuf/encoding/prototext"

	spb "gogames/raubgraf/protos/state_proto"
)

var (
	// memGames caches games in memory. For simple debugging it can
	// be the only game storage.
	memGames = map[int]*engine.RaubgrafGame{}

	mu sync.Mutex
)

// getGame returns the game with ID gid, if it exists.
func getGame(gid int) *engine.RaubgrafGame {
	return memGames[gid]
}

// CreateGame creates a new game and returns its ID.
func CreateGame(width, height int) (int, error) {
	game, err := engine.NewGame(width, height)
	if err != nil {
		return -1, err
	}
	mu.Lock()
	defer mu.Unlock()
	idx := len(memGames)
	memGames[idx] = game
	return idx, nil
}

// LoadGame loads a game from protobuf and returns its ID.
func LoadGame(bp *spb.Board) (int, error) {
	bb, err := board.FromProto(bp)
	if err != nil {
		return -1, fmt.Errorf("error loading from proto: %w", err)
	}
	game, err := engine.FromBoard(bb)
	if err != nil {
		return -1, fmt.Errorf("error creating game from proto: %w", err)
	}
	mu.Lock()
	defer mu.Unlock()
	idx := len(memGames)
	memGames[idx] = game
	return idx, nil
}

// GetGameState returns the protobuf encoding of the given game.
func GetGameState(gid int) (*spb.Board, error) {
	game := getGame(gid)
	if game == nil {
		return nil, fmt.Errorf("cannot return game state: Game %d is not in memory", gid)
	}
	bb := game.GetBoard()
	if bb == nil {
		return nil, fmt.Errorf("bad state: Game %d has no board", gid)
	}
	return bb.ToProto()
}

// PlayerActions accepts game actions from a player.
func PlayerActions(gid, fnum int, orders []*spb.Action) error {
	game := getGame(gid)
	if game == nil {
		return fmt.Errorf("cannot receive player actions: Game %d is not in memory", gid)
	}
	for _, order := range orders {
		pid := order.GetPopId()
		target := game.PopByID(pid)
		if target == nil {
			return fmt.Errorf("cannot execute order %s: No Pop with ID %d found", prototext.Format(order), pid)
		}
	}
	return nil
}
