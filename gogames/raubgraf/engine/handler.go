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

// PollForUpdate runs a turn of the game if it is ready, and returns
// the resulting board state; otherwise it returns an error.
func PollForUpdate(gid int) (*spb.Board, error) {
	game := getGame(gid)
	if game == nil {
		return nil, fmt.Errorf("cannot run turn: Game %d is not in memory", gid)
	}

	if waiting := game.TurnReady(); len(waiting) > 0 {
		return nil, fmt.Errorf("cannot run turn: Waiting for players %v", waiting)
	}

	if err := game.ResolveTurn(); err != nil {
		return nil, fmt.Errorf("error resolving turn in game %d: %w", gid, err)
	}

	return GetGameState(gid)
}

func handlePopOrder(game *engine.RaubgrafGame, fdna string, order *spb.Action) error {
	pid := order.GetPopId()
	target := game.PopByID(pid)
	if target == nil {
		return fmt.Errorf("cannot execute order %s: No Pop with ID %d found", prototext.Format(order), pid)
	}
	return nil
}

// PlayerActions accepts game actions from a player.
func PlayerActions(gid int, fdna string, orders []*spb.Action) error {
	game := getGame(gid)
	if game == nil {
		return fmt.Errorf("cannot receive player actions: Game %d is not in memory", gid)
	}
	endTurn := false
	for _, order := range orders {
		switch order.GetKind() {
		case spb.Action_AK_UNKNOWN:
			return fmt.Errorf("cannot execute order with UNKNOWN action")
		case spb.Action_AK_POP_ORDER:
			if err := handlePopOrder(game, fdna, order); err != nil {
				return err
			}
		case spb.Action_AK_PLAYER_DONE:
			// Do this after all other orders are processed.
			endTurn = true
		}
	}
	if endTurn {
		if err := game.EndPlayerTurn(fdna); err != nil {
			return fmt.Errorf("cannot end turn of faction %q: %w", fdna, err)
		}
	}
	return nil
}
