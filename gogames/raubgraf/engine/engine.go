package engine

type Direction int

const (
  North Direction = iota
  NorthEast
  SouthEast
  South
  SouthWest
  NorthWest
)

func Clockwise(d Direction) Direction {
  switch d{
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
  panic()
}
func CounterClockwise(d Direction) Direction {
  switch d{
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
  panic()
}

type triangle struct {

}

