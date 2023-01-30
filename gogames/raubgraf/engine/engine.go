package engine

import (
  "fmt"
)

type Direction int

const (
  North Direction = iota
  NorthEast
  SouthEast
  South
  SouthWest
  NorthWest
)

func (d Direction) Clockwise() Direction {
  switch d {
    case North:
    return NorthEast
    case NorthEast:
    return SouthEast
    case SouthEast:
    return South
    case South:
    return SouthWest
    case SouthWest:
    return NorthWest
    case NorthWest:
    return North
  }
  panic(fmt.Errorf("Clockwise received unknown direction %v", d))
}
func (d Direction) CounterClockwise() Direction {
  switch d {
    case North:
    return NorthWest
    case NorthWest:
    return SouthWest
    case SouthWest:
    return South
    case South:
    return SouthEast
    case SouthEast:
    return NorthEast
    case NorthEast:
    return North
  }
  panic(fmt.Errorf("CounterClockwise received unknown direction %v", d))
}

type triangle struct {

}

