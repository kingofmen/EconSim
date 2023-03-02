package coords

import(
  "testing"
)

func TestBasics(t *testing.T) {
  points := []Point{
    New(1, 1),
    New(1, 2),
    New(-1, 1),
    Nil(),
  }

  for x, p1 := range points {
    for y, p2 := range points {
      if x == y {
        if p1 != p2 {
          t.Errorf("%s should equal %s", p1.String(), p2.String())
        }
      } else {
        if p1 == p2 {
          t.Errorf("%s should not equal %s", p1.String(), p2.String())
        }
      }
    }
  }
}

func TestNil(t *testing.T) {
  pnil := Nil()
  nnil := New(0, 0)

  if nnil.IsNil() {
    t.Errorf("%s should not be nil", nnil.String())
  }
  if !pnil.IsNil() {
    t.Errorf("%s should be nil", pnil.String())
  }
  if got := pnil.String(); got != "<nil>" {
    t.Errorf("nil.String() => %q, want <nil>", got)
  }
}
