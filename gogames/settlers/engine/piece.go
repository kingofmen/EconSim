package settlers

import (
	"gogames/tiles/triangles"
)

type Activity int

const (
	CONSUME = iota // Eat, drink, and be merry; all that feeds the body.
	MEANING        // Ritual, sport, family, worship; all that makes life worth living.
	PRODUCE        // Investment, building, the creation of tools.
	MILITIA        // Drill, weapons, training, patrol; the defense of the other three.
)

// Edge contains occupation information for a triangle edge.
type Edge struct {
	dir   triangles.Direction
	rules []Rule
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
func (v Vert) From(org triangles.TriPoint) triangles.TriPoint {
	if org.Points(triangles.North) {
		triangles.Sum(org, v.pos.Flip())
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
	// Occupied edges.
	edges []Edge
}

// From returns the coordinates of the face relative to the given
// origin, possibly flipped and rotated.
func (f Face) From(org triangles.TriPoint) triangles.TriPoint {
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
