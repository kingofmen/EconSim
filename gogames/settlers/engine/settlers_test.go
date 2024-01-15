package settlers

import (
	"fmt"
	"testing"

	"gogames/tiles/triangles"
)

type testRule struct {
	triangle func(tri *triangles.Surface, fac *Faction) error
	vertex   func(vtx *triangles.Vertex, fac *Faction) error
}

func (tr *testRule) Triangle(tri *triangles.Surface, fac *Faction) error {
	if tr.triangle == nil {
		return nil
	}
	return tr.triangle(tri, fac)
}
func (tr *testRule) Vertex(vtx *triangles.Vertex, fac *Faction) error {
	if tr.vertex == nil {
		return nil
	}
	return tr.vertex(vtx, fac)
}

func neverTriangleRule(s string) *testRule {
	return &testRule{
		triangle: func(tri *triangles.Surface, fac *Faction) error {
			return fmt.Errorf("Triangle: Never %s!", s)
		},
	}
}
func neverVertexRule(s string) *testRule {
	return &testRule{
		vertex: func(tri *triangles.Vertex, fac *Faction) error {
			return fmt.Errorf("Vertex: Never %s!", s)
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
							rules: []Rule{neverTriangleRule("face")},
							verts: []Vert{
								Vert{
									dir:   triangles.South,
									rules: []Rule{neverVertexRule("vertex")},
								},
							},
						},
					},
					faceRules: []Rule{neverTriangleRule("triangles")},
					vertRules: []Rule{neverVertexRule("vertices")},
				},
			},
			want: []error{
				fmt.Errorf("Triangle: Never triangles!"),
				fmt.Errorf("Triangle: Never face!"),
				fmt.Errorf("Vertex: Never vertices!"),
				fmt.Errorf("Vertex: Never vertex!"),
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
