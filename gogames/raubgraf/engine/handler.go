// Package handler implements an interface to client libraries.
package handler

import (
	"fmt"
	"sync"

	"github.com/golang/protobuf/proto"
	"gogames/raubgraf/engine/board"
	"gogames/raubgraf/engine/engine"

	spb "gogames/raubgraf/protos/state_proto"
)

var (
	// memGames caches games in memory. For simple debugging it can
	// be the only game storage.
	memGames = map[int]*engine.RaubgrafGame{}

	mu sync.Mutex
)

// toProto translates an in-memory board to its protobuf representation.
func toProto(bb *board.Board) (*spb.Board, error) {
	bw := uint32(len(bb.Vertices))
	if bw == 0 {
		return nil, fmt.Errorf("bad board state: No vertices")
	}
	bh := uint32(len(bb.Vertices[0]))
	if bh == 0 {
		return nil, fmt.Errorf("bad board state: Empty first column")
	}

	bp := &spb.Board{
		Width:  proto.Uint32(bw),
		Height: proto.Uint32(bh),
	}

	return bp, nil
}

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
	return toProto(bb)
}
