package settlers

import (
	"gogames/tiles/triangles"
)

type Activity int
type Rotate int

const (
	CONSUME = iota // Eat, drink, and be merry; all that feeds the body.
	MEANING        // Ritual, sport, family, worship; all that makes life worth living.
	PRODUCE        // Investment, building, the creation of tools.
	MILITIA        // Drill, weapons, training, patrol; the defense of the other three.

	None Rotate = iota
	Once
	Twice
)

// Turns converts a signed number of turns into the equivalent
// number of clockwise ticks; triangles have only three possible
// orientations.
func Turns(r int) Rotate {
	r = r % 3
	if r < 0 {
		r += 3
	}
	switch r {
	case 0:
		return None
	case 1:
		return Once
	case 2:
		return Twice
	}
	return None
}

// String returns a human-readable debug string.
func (r Rotate) String() string {
	switch r {
	case None:
		return "None"
	case Once:
		return "Once"
	case Twice:
		return "Twice"
	}
	return "Impossible"
}

// Edge contains occupation information for a triangle edge.
type Edge struct {
	dir      triangles.Direction
	rules    []Rule
	occupied bool
}

// Vert contains occupation information for a triangle vertex.
type Vert struct {
	// Tri-coordinates relative to base face.
	pos triangles.TriPoint
	// Vertex-specific rules.
	rules []Rule
	// Whether the piece covers the vertex.
	occupied bool
}

// From returns the coordinates of the vertex relative to the given
// origin, possibly flipped and rotated.
func (v Vert) From(org triangles.TriPoint, turns Rotate) triangles.TriPoint {
	if org.Points(triangles.North) {
		return triangles.Sum(org, v.pos.Flip())
	}
	return triangles.Sum(org, v.pos)
}

// Face contains requirement and occupation information for one triangle tile.
type Face struct {
	// Tri-coordinates relative to base face.
	pos triangles.TriPoint
	// Rules for the face.
	rules []Rule
	// Whether the piece covers the face.
	occupied bool
}

// From returns the coordinates of the face relative to the given
// origin, possibly flipped and rotated.
func (f Face) From(org triangles.TriPoint, turns Rotate) triangles.TriPoint {
	if org.Points(triangles.North) {
		// Flip by reversing all directions.
		return triangles.Sum(org, triangles.Diff(triangles.Zero(), f.pos))
	}
	return triangles.Sum(org, f.pos)
}

// Shape is a collection of Faces.
type Shape struct {
	faces []Face
	verts []Vert
	// Rules for all occupied faces.
	faceRules []Rule
	// Rules for all edges.
	edgeRules []Rule
	// Rules for all vertices.
	vertRules []Rule
}

// Template contains information shared across a class of game pieces,
// such as shape and production capability.
type Template struct {
	key    string
	people int64
	shape  Shape
}

// Key returns the template's lookup key.
func (tp *Template) Key() string {
	if tp == nil {
		return "nil"
	}
	return tp.key
}

// Occupies returns the triangle and vertex coordinates the template
// occupies.
func (tp *Template) Occupies() (tris, verts []triangles.TriPoint) {
	for _, f := range tp.shape.faces {
		if f.occupied {
			tris = append(tris, f.pos.Copy())
		}
	}
	for _, v := range tp.shape.verts {
		if v.occupied {
			verts = append(verts, v.pos.Copy())
		}
	}
	return
}

// Piece is an instantiation of a Template, placed on the board.
type Piece struct {
	kind       *Template
	faction    *Faction
	priorities [4]int16
	people     int64
}

func NewPiece(tmp *Template, fac *Faction) *Piece {
	return &Piece{
		kind:       tmp,
		faction:    fac,
		priorities: [4]int16{250, 250, 250, 250},
		people:     0,
	}
}

// GetKey returns the lookup key for the piece's template.
func (p *Piece) GetKey() string {
	if p == nil {
		return "nil"
	}
	return p.kind.key
}

func (s *Piece) Populate() {
	if s == nil {
		return
	}
	if s.faction == nil {
		return
	}

	want := s.kind.people - s.people
	if s.faction.Freemen < want {
		want = s.faction.Freemen
	}
	s.people += want
	s.faction.Freemen -= want
}

// Prioritize calculates the production fractions.
func (s *Piece) Prioritize() {
	if s == nil {
		return
	}
	// TODO: Replace this placeholder stub with an actual calculation!
	s.priorities[CONSUME] = 250
	s.priorities[MEANING] = 250
	s.priorities[PRODUCE] = 250
	s.priorities[MILITIA] = 250
}

func (s *Piece) Produce() {
	if s == nil {
		return
	}
}

// DevTemplates returns some trivial templates for dev work.
func DevTemplates() []*Template {
	return []*Template{
		&Template{
			key: "village",
			shape: Shape{
				faces: []Face{
					Face{
						pos:      triangles.TriPoint{0, 0, 0},
						occupied: true,
					},
				},
				faceRules: []Rule{&EmptyRule{}},
			},
		},
		&Template{
			key: "tower",
			shape: Shape{
				faces: []Face{
					Face{
						pos:   triangles.TriPoint{0, 0, 0},
						rules: []Rule{HasKeys("village")},
					},
				},
				verts: []Vert{
					Vert{
						pos:      triangles.TriPoint{0, -1, 0},
						occupied: true,
						rules:    []Rule{&EmptyRule{}},
					},
				},
			},
		},
		&Template{
			key: "temple",
			shape: Shape{
				faces: []Face{
					Face{
						pos:   triangles.TriPoint{0, 0, 0},
						rules: []Rule{HasKeys("village")},
					},
				},
				verts: []Vert{
					Vert{
						pos:      triangles.TriPoint{-1, 0, 0},
						occupied: true,
						rules:    []Rule{&EmptyRule{}},
					},
				},
			},
		},
	}
}
