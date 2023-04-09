package faction

import (
	"testing"

	"gogames/raubgraf/engine/dna"
)

func TestCommand(t *testing.T) {
	fdna := "aaaa"
	other := "bbbb"
	cases := []struct {
		desc string
		pdna string
		exp  bool
	}{
		{
			desc: "Full faction DNA",
			pdna: dna.Make(fdna, fdna),
			exp:  true,
		},
		{
			desc: "Paternal only",
			pdna: dna.Make(fdna, other),
			exp:  true,
		},
		{
			desc: "Maternal only",
			pdna: dna.Make(other, fdna),
		},
		{
			desc: "Neither",
			pdna: dna.Make(other, other),
		},
	}

	for _, cc := range cases {
		f := New(fdna, fdna)
		if got := f.Command(cc.pdna); got != cc.exp {
			t.Errorf("%s: (%s).Command(%s) => %t, want %t", cc.desc, f.GetDNA(), cc.pdna, got, cc.exp)
		}
	}
}

func TestScore(t *testing.T) {
	fdna := "aaaa"
	other := "bbbb"
	cases := []struct {
		desc string
		pdna string
		exp  int
	}{
		{
			desc: "Full faction DNA",
			pdna: dna.Make(fdna, fdna),
			exp:  2,
		},
		{
			desc: "Paternal only",
			pdna: dna.Make(fdna, other),
			exp:  1,
		},
		{
			desc: "Maternal only",
			pdna: dna.Make(other, fdna),
			exp:  1,
		},
		{
			desc: "Neither",
			pdna: dna.Make(other, other),
			exp:  0,
		},
	}

	for _, cc := range cases {
		f := New(fdna, fdna)
		if got := f.Score(cc.pdna); got != cc.exp {
			t.Errorf("%s: (%s).Score(%s) => %d, want %d", cc.desc, f.GetDNA(), cc.pdna, got, cc.exp)
		}
	}
}
