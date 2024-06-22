package misprice

import (
	//"fmt"
	"testing"
	//"time"

	"github.com/google/go-cmp/cmp"
	"trading/marketdata"
	"trading/polygon"
)

func TestCreatePairs(t *testing.T) {
	cases := []struct {
		desc  string
		input map[*polygon.Ticker]*marketdata.OptionQuote
		want  []*pair
	}{
		{
			desc:  "No inputs",
			input: map[*polygon.Ticker]*marketdata.OptionQuote{},
			want:  []*pair{},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := createPairs(cc.input)
			if diff := cmp.Diff(cc.want, got); diff != "" {
				t.Errorf("%s: createPairs() => %+v, want %+v, diff %s", cc.desc, got, cc.want, diff)
			}
		})
	}
}
