package dna

import (
	"fmt"
	"image/color"
	"testing"
)

func TestStrings(t *testing.T) {
	cases := []struct {
		s1, s2 string
		dist   int
	}{
		{
			// Unintuitive because of the stretching, unfortunately.
			s1:   "a",
			s2:   "b",
			dist: 4,
		},
		{
			s1:   "a       ",
			s2:   "a",
			dist: 0,
		},
		{
			s1:   "        ",
			s2:   "!!!!!!!!",
			dist: 16,
		},
		{
			s1:   " ! ! ! !",
			s2:   "! ! ! ! ",
			dist: 16,
		},
		{
			s1:   " ! ! ! !thisdoesn'tmatter",
			s2:   "! ! ! ! whateverdude",
			dist: 16,
		},
	}

	for _, cc := range cases {
		t.Run(fmt.Sprintf("%s_%s", cc.s1, cc.s2), func(t *testing.T) {
			s1 := FromString(cc.s1)
			s2 := FromString(cc.s2)
			got1 := s1.Distance(s2)
			got2 := s2.Distance(s1)
			if got1 != got2 {
				t.Errorf("%q/%q: Distance should be symmetric, got %d vs %d", cc.s1, cc.s2, got1, got2)
			}
			if got1 != cc.dist {
				t.Errorf("%q/%q: Got distance %d, expect %d", cc.s1, cc.s2, got1, cc.dist)
			}
		})
	}
}

func TestRGBA(t *testing.T) {
	cases := []struct {
		seq  string
		want color.NRGBA64
	}{
		{
			seq:  "",
			want: color.NRGBA64{0, 0, 0, 0},
		},
		{
			seq:  "!#09RS{}",
			want: color.NRGBA64{520, 10818, 34442, 63485},
		},
		{
			seq:  "    ~~  ",
			want: color.NRGBA64{0, 0, 65535, 0},
		},
	}

	for _, cc := range cases {
		got := FromString(cc.seq).RGBA()
		if got != cc.want {
			t.Errorf("Sequence %q => %+v, want %+v", cc.seq, got, cc.want)
		}
	}
}
