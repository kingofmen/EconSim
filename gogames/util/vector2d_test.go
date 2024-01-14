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
		{
			desc: "Sanity",
			one:  UnitX(),
			two:  New(0.5, 0.5),
			want: 0.5,
		},
		{
			desc: "Numbers at random",
			one:  New(13, 79),
			two:  New(-2, 32),
			want: 2502,
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

func TestInPlace(t *testing.T) {
	one := Zero()
	two := Zero()
	three := Zero()
	cases := []struct {
		desc string
		ops  []func()
		want Vector
	}{
		{
			desc: "Add",
			ops: []func(){
				func() {
					one.Add(1.3, 0.1)
				},
			},
			want: New(1.3, 0.1),
		},
		{
			desc: "AddInt",
			ops: []func(){
				func() {
					one.AddInt(1, 11)
				},
			},
			want: New(1.0, 11.0),
		},
		{
			desc: "MultiAdd",
			ops: []func(){
				func() {
					one.Add(1.0, 11.0)
					one.Add(2.0, -3.5)
				},
			},
			want: New(3.0, 7.5),
		},
		{
			desc: "Sum",
			ops: []func(){
				func() {
					three.Add(1.0, 11.0)
					two.Add(2.0, -3.5)
					one = Sum(two, three)
				},
			},
			want: New(3.0, 7.5),
		},
		{
			desc: "Diff",
			ops: []func(){
				func() {
					three.Add(1.0, 11.0)
					two.Add(2.0, -3.5)
					one = Diff(two, three)
				},
			},
			want: New(1.0, -14.5),
		},
		{
			desc: "Sub",
			ops: []func(){
				func() {
					one.Set(1.0, 2.0)
					one.Sub(0.5, 0.5)
				},
			},
			want: New(0.5, 1.5),
		},
		{
			desc: "SubInt",
			ops: []func(){
				func() {
					one.Set(1.5, 2.55)
					one.SubInt(1, 1)
				},
			},
			want: New(0.5, 1.55),
		},
		{
			desc: "SetInt",
			ops: []func(){
				func() {
					one.Set(4, 2)
				},
			},
			want: New(4.0, 2.0),
		},
	}

	tolerance := 0.001
	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			one.Set(0, 0)
			two.Set(0, 0)
			three.Set(0, 0)
			for _, op := range cc.ops {
				op()
			}
			xdiff, ydiff := one.X()-cc.want.X(), one.Y()-cc.want.Y()
			if math.Abs(xdiff) > tolerance || math.Abs(ydiff) > tolerance {
				t.Errorf("%s: Result is %s, want %s", cc.desc, one, cc.want)
			}
		})
	}
}
