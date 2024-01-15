package settlers

import (
	"fmt"
	"testing"

	"gogames/tiles/triangles"
)

type testRule struct {
	allow func(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error
}

func (tr *testRule) Allow(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error {
	if tr == nil || tr.allow == nil {
		return nil
	}
	return tr.allow(tile, fac, vertices...)
}

func neverRule(s string) *testRule {
	return &testRule{
		allow: func(tile *Tile, fac *Faction, vertices ...*triangles.Vertex) error {
			return fmt.Errorf("Never %s!", s)
		},
	}
}

func TestPlace(t *testing.T) {
	cases := []struct {
		desc string
		pos  triangles.TriPoint
		tmpl *Template
		want []error
	}{
		{
			desc: "Always rule",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{&testRule{}},
							verts: []Vert{
								Vert{
									dir:   triangles.South,
									rules: []Rule{&testRule{}},
								},
							},
						},
					},
					faceRules: []Rule{&testRule{}},
					vertRules: []Rule{&testRule{}},
				},
			},
		},
		{
			desc: "No rules",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos: triangles.TriPoint{0, 0, 0},
							verts: []Vert{
								Vert{
									dir: triangles.South,
								},
							},
						},
					},
				},
			},
		},
		{
			desc: "Nil vertex",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{&testRule{}},
							verts: []Vert{
								Vert{
									dir:   triangles.North,
									rules: []Rule{&testRule{}},
								},
							},
						},
					},
					faceRules: []Rule{&testRule{}},
					vertRules: []Rule{&testRule{}},
				},
			},
			want: []error{
				fmt.Errorf("cannot apply vertex rules to nil vertex ((0, 0, 1) -> North    )"),
			},
		},
		{
			desc: "Nil face",
			pos:  triangles.TriPoint{0, 0, 100},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{&testRule{}},
							verts: []Vert{
								Vert{
									dir:   triangles.South,
									rules: []Rule{&testRule{}},
								},
							},
						},
					},
					faceRules: []Rule{&testRule{}},
					vertRules: []Rule{&testRule{}},
				},
			},
			want: []error{
				fmt.Errorf("required position (0, 0, 100) not on the board"),
			},
		},
		{
			desc: "Never rule",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{neverRule("face")},
							verts: []Vert{
								Vert{
									dir:   triangles.South,
									rules: []Rule{neverRule("vertex")},
								},
							},
						},
					},
					faceRules: []Rule{neverRule("triangles")},
					vertRules: []Rule{neverRule("vertices")},
				},
			},
			want: []error{
				fmt.Errorf("Never triangles!"),
				fmt.Errorf("Never face!"),
				fmt.Errorf("Never vertex!"),
				fmt.Errorf("Never vertices!"),
			},
		},
	}

	board, err := NewHex(2)
	if err != nil {
		t.Fatalf("Could not create board: %v", err)
	}
	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			errs := board.Place(cc.pos, nil, cc.tmpl)
			if len(errs) != len(cc.want) {
				t.Errorf("%s: Place() => %v, want %v", cc.desc, errs, cc.want)
				return
			}
			for idx, err := range errs {
				if err.Error() != cc.want[idx].Error() {
					t.Errorf("%s: Place() => error %d %v, want %v", cc.desc, idx, err, cc.want[idx])
				}
			}
		})
	}
}
