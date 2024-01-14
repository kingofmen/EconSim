package vector2d

import (
	"math"
	"testing"
)

func TestDotProduct(t *testing.T) {
	cases := []struct {
		desc string
		one  Vector
		two  Vector
		want float64
	}{
		{
			desc: "Unit vectors",
			one:  UnitX(),
			two:  UnitY(),
			want: 0,
		},
	}

	tolerance := 0.001
	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			got := cc.one.Dot(cc.two)
			if math.Abs(got-cc.want) > tolerance {
				t.Errorf("%s: %s * %s => %v, want %v", cc.desc, cc.one, cc.two, got, cc.want)
			}
		})
	}
}
