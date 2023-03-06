package building

import (
	"gogames/raubgraf/engine/pop"
)

// Building models a settlement, e.g. a castle or city.
type Building struct {
	*pop.Dwelling
	food int
}

func New() *Building {
	return &Building{
		Dwelling: pop.NewDwelling(),
		food:     0,
	}
}
