package misprice

import (
	//"fmt"
	"testing"
	"time"

	"github.com/google/go-cmp/cmp"
	"github.com/google/go-cmp/cmp/cmpopts"
	"trading/marketdata"
	"trading/polygon"
)

func TestCreatePairs(t *testing.T) {
	utc := time.FixedZone("UTC", 0)
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
		{
			desc: "Creates one pair",
			input: map[*polygon.Ticker]*marketdata.OptionQuote{
				&polygon.Ticker{
					ContractType: "call",
					ExpDate:      "2024-01-01",
					Price:        100,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{1},
					AskSize:  []float64{2},
					BidPrice: []float64{0.5},
					BidSize:  []float64{3},
				},
				&polygon.Ticker{
					ContractType: "put",
					ExpDate:      "2024-01-01",
					Price:        100,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{1.3},
					AskSize:  []float64{4},
					BidPrice: []float64{0.75},
					BidSize:  []float64{33},
				},
			},
			want: []*pair{
				{
					strike:  100,
					expire:  time.Date(2024, time.January, 1, 0, 0, 0, 0, utc),
					putAsk:  price{1.3, 4},
					putBid:  price{0.75, 33},
					callAsk: price{1, 2},
					callBid: price{0.5, 3},
				},
			},
		},
	}

	cmpOpts := []cmp.Option{
		cmp.AllowUnexported(pair{}, price{}),
		cmpopts.SortSlices(func(one, two *pair) bool {
			if one.expire.Before(two.expire) {
				return true
			}
			if one.strike < two.strike {
				return true
			}
			if one.putAsk.money < two.putAsk.money {
				return true
			}
			if one.callAsk.money < two.callAsk.money {
				return true
			}
			return false
		}),
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := createPairs(cc.input)
			if diff := cmp.Diff(cc.want, got, cmpOpts...); diff != "" {
				t.Errorf("%s: createPairs() => %+v, want %+v, diff %s", cc.desc, got, cc.want, diff)
			}
		})
	}
}
