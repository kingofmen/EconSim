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
			s1:   "a",
			s2:   "b",
			dist: 1,
		},
		{
			s1:   "a0000000",
			s2:   "a",
			dist: 0,
		},
		{
			s1:   "11111111",
			s2:   "",
			dist: 8,
		},
		{
			s1:   "a",
			s2:   "z",
			dist: 25,
		},
		{
			s1:   "abababab",
			s2:   "babababa",
			dist: 8,
		},
		{
			s1:   "ababababthisdoesn'tmatter",
			s2:   "babababawhateverdude",
			dist: 8,
		},
	}

	for _, cc := range cases {
		t.Run(fmt.Sprintf("%s_%s", cc.s1, cc.s2), func(t *testing.T) {
			s1 := FromString(cc.s1)
			s2 := FromString(cc.s2)
			got1 := s1.Distance(s2)
			got2 := s2.Distance(s1)
			if got1 != got2 {
				t.Errorf("%s/%s: Distance should be symmetric, got %d vs %d", cc.s1, cc.s2, got1, got2)
			}
			if got1 != cc.dist {
				t.Errorf("%s/%s: Got distance %d, expect %d", cc.s1, cc.s2, got1, cc.dist)
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
			want: color.NRGBA64{12336, 12336, 12336, 12336},
		},
		{
			seq:  "aabbccdd",
			want: color.NRGBA64{24929, 25186, 25443, 25700},
		},
		{
			seq:  "\x00\x00\x00\x00\xFF\xFF\x00\x00",
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
