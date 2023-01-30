package engine

type PopType int

const (
  Peasant PopType = iota
  Bandit
  Burgher
  Merchant
  Knight
  Noble
)

type Pop struct {
  kind PopType
}

