package misprice

import (
	//"fmt"
	"testing"
	"time"

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
					expire:  time.Date(2024, time.January, 1, 0, 0, 0, 0, time.UTC),
					putAsk:  price{1.3, 4},
					putBid:  price{0.75, 33},
					callAsk: price{1, 2},
					callBid: price{0.5, 3},
				},
			},
		},
		{
			desc: "Matches by strike and expiry",
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
				&polygon.Ticker{
					ContractType: "call",
					ExpDate:      "2024-01-01",
					Price:        110,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{1},
					AskSize:  []float64{2},
					BidPrice: []float64{3},
					BidSize:  []float64{4},
				},
				&polygon.Ticker{
					ContractType: "put",
					ExpDate:      "2024-01-01",
					Price:        110,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{5},
					AskSize:  []float64{6},
					BidPrice: []float64{7},
					BidSize:  []float64{8},
				},
				&polygon.Ticker{
					ContractType: "call",
					ExpDate:      "2025-01-01",
					Price:        110,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{1},
					AskSize:  []float64{2},
					BidPrice: []float64{3},
					BidSize:  []float64{4},
				},
				&polygon.Ticker{
					ContractType: "put",
					ExpDate:      "2025-01-01",
					Price:        100,
				}: &marketdata.OptionQuote{
					AskPrice: []float64{5},
					AskSize:  []float64{6},
					BidPrice: []float64{7},
					BidSize:  []float64{8},
				},
			},
			want: []*pair{
				{
					strike:  100,
					expire:  time.Date(2024, time.January, 1, 0, 0, 0, 0, time.UTC),
					putAsk:  price{1.3, 4},
					putBid:  price{0.75, 33},
					callAsk: price{1, 2},
					callBid: price{0.5, 3},
				},
				{
					strike:  110,
					expire:  time.Date(2024, time.January, 1, 0, 0, 0, 0, time.UTC),
					putAsk:  price{5, 6},
					putBid:  price{7, 8},
					callAsk: price{1, 2},
					callBid: price{3, 4},
				},
				{
					strike: 100,
					expire: time.Date(2025, time.January, 1, 0, 0, 0, 0, time.UTC),
					putAsk: price{5, 6},
					putBid: price{7, 8},
				},
				{
					strike:  110,
					expire:  time.Date(2025, time.January, 1, 0, 0, 0, 0, time.UTC),
					callAsk: price{1, 2},
					callBid: price{3, 4},
				},
			},
		},
	}

	cmpOpts := []cmp.Option{
		cmp.AllowUnexported(pair{}, price{}),
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := createPairs(cc.input)
			if diff := cmp.Diff(cc.want, got, cmpOpts...); diff != "" {
				t.Errorf("%s: createPairs() => %+v,\nwant %+v,\ndiff %s", cc.desc, got, cc.want, diff)
			}
		})
	}
}
