// Package game contains the core game-loop logic for landnam.
package game

import (
	"google.golang.org/protobuf/proto"

	gmpb "gogames/landnam/core/game_go_proto"
)

func Turn(oldState *gmpb.GameState) *gmpb.GameState {
	newState := proto.Clone(oldState).(*gmpb.GameState)
	return newState
}
