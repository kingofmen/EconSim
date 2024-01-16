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
	alwaysTmpl := &Template{
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
	}

	cases := []struct {
		desc   string
		pos    triangles.TriPoint
		prep   []*Template
		tmpl   *Template
		errors []error
		want   []*Piece
	}{
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
			desc: "Always rule",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: alwaysTmpl,
		},
		{
			desc: "Nil vertex",
			pos:  triangles.TriPoint{0, 1, 1},
			tmpl: alwaysTmpl,
			errors: []error{
				fmt.Errorf("cannot apply vertex rules to nil vertex ((0, 1, 1) -> South    )"),
			},
		},
		{
			desc: "Nil face",
			pos:  triangles.TriPoint{0, 0, 100},
			tmpl: alwaysTmpl,
			errors: []error{
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
			errors: []error{
				fmt.Errorf("Never triangles!"),
				fmt.Errorf("Never face!"),
				fmt.Errorf("Never vertex!"),
				fmt.Errorf("Never vertices!"),
			},
		},
		{
			desc: "Empty tile rule (face fail)",
			pos:  triangles.TriPoint{0, 0, 1},
			prep: []*Template{alwaysTmpl},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{&EmptyRule{}},
						},
					},
				},
			},
			errors: []error{
				fmt.Errorf("tile is not empty"),
			},
		},
		{
			desc: "Empty tile rule (face succeed)",
			pos:  triangles.TriPoint{0, 0, 1},
			tmpl: &Template{
				shape: Shape{
					faces: []Face{
						Face{
							pos:   triangles.TriPoint{0, 0, 0},
							rules: []Rule{&EmptyRule{}},
						},
					},
				},
			},
		},
	}

	for _, cc := range cases {
		t.Run(cc.desc, func(t *testing.T) {
			board, err := NewHex(4)
			if err != nil {
				t.Fatalf("%s: Could not create board: %v", cc.desc, err)
			}
			for i, p := range cc.prep {
				if errs := board.Place(cc.pos, nil, p); len(errs) > 0 {
					t.Fatalf("%s: Could not place prep template %d: %v", cc.desc, i, errs)
				}
			}

			errs := board.Place(cc.pos, nil, cc.tmpl)
			if len(errs) != len(cc.errors) {
				t.Errorf("%s: Place() => %v, want %v", cc.desc, errs, cc.errors)
				return
			}
			for idx, err := range errs {
				if err.Error() != cc.errors[idx].Error() {
					t.Errorf("%s: Place() => error %d %v, want %v", cc.desc, idx, err, cc.errors[idx])
				}
			}
		})
	}
}
