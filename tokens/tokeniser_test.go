package tokens

import (
	"reflect"
	"strings"
	"testing"
)

func TestTokeniser_Tokens(t *testing.T) {
	input := strings.NewReader(`package main
`)

	defer func() {
		err := recover()
		if err != nil {
			t.Errorf("unexpected panic: %+v", err)
		}
	}()

	m := New()

	out, err := m.Tokens("test.cb", input)
	if err != nil {
		t.Errorf("unexpected error: %+v", err)
	}

	if len(out) != 2 {
		t.Fatalf("expected output to include 2 Token, received %d", len(out))
	}

	tok0 := Token{Token: PACKAGE, String: "package", File: "test.cb", Line: 1, StartPos: 1, EndPos: 7}
	if !reflect.DeepEqual(out[0], tok0) {
		t.Errorf("expected %+v, received %+v", tok0, out[0])
	}

	tok1 := Token{Token: IDENT, String: "main", File: "test.cb", Line: 1, StartPos: 9, EndPos: 12}
	if !reflect.DeepEqual(out[1], tok1) {
		t.Errorf("expected %+v, received %+v", tok1, out[1])
	}
}
