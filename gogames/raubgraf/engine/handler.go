// Package handler implements an interface to client libraries.
package handler

import (
	"fmt"
	"sync"

	"gogames/raubgraf/engine/board"
	"gogames/raubgraf/engine/dna"
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
func LoadGame(gs *spb.GameState) (int, error) {
	bb, err := board.FromProto(gs.GetBoard())
	if err != nil {
		return -1, fmt.Errorf("error loading board from proto: %w", err)
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
func GetGameState(gid int) (*spb.GameState, error) {
	game := getGame(gid)
	if game == nil {
		return nil, fmt.Errorf("cannot return game state: Game %d is not in memory", gid)
	}
	bb := game.GetBoard()
	if bb == nil {
		return nil, fmt.Errorf("bad state: Game %d has no board", gid)
	}
	bp, err := bb.ToProto()
	if err != nil {
		return nil, err
	}
	return &spb.GameState{
		Board: bp,
	}, nil
}

// PollForUpdate runs a turn of the game if it is ready, and returns
// the resulting board state; otherwise it returns an error.
func PollForUpdate(gid int) (*spb.GameState, error) {
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

func convertDNA(seq *spb.Dna) *dna.Sequence {
	return dna.New(seq.GetPaternal(), seq.GetMaternal())
}

func handlePopOrder(game *engine.RaubgrafGame, seq *dna.Sequence, order *spb.PopOrder) error {
	pid := order.GetPopId()
	target := game.PopByID(pid)
	if target == nil {
		return fmt.Errorf("no Pop with ID %d found", pid)
	}
	fcn := game.FactionBySequence(seq)
	if fcn == nil {
		return fmt.Errorf("no faction with sequence %q found", seq.String())
	}
	if !fcn.Command(target.Sequence) {
		return fmt.Errorf("does not match DNA of Pop %d", pid)
	}
	return nil
}

// PlayerActions accepts game actions from a player.
func PlayerActions(gid int, actions []*spb.Action) error {
	game := getGame(gid)
	if game == nil {
		return fmt.Errorf("cannot receive player actions: Game %d is not in memory", gid)
	}
	var endTurn *dna.Sequence
	for _, action := range actions {
		seq := convertDNA(action.GetSource())
		order := action.GetOrder()
		if order == nil {
			return fmt.Errorf("cannot execute action with nil order")
		}
		switch data := order.(type) {
		case *spb.Action_PopAction:
			if err := handlePopOrder(game, seq, data.PopAction); err != nil {
				return fmt.Errorf("cannot execute action %s: %w", prototext.Format(action), err)
			}
		case *spb.Action_EndTurn:
			// Do this after all other actions are processed.
			endTurn = seq
		}
	}
	if endTurn != nil {
		if err := game.EndPlayerTurn(endTurn); err != nil {
			return fmt.Errorf("cannot end turn of faction %q: %w", endTurn.String(), err)
		}
	}
	return nil
}
