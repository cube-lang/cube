package parser

import (
	"fmt"
	"reflect"
	"testing"

	"github.com/cube-lang/cube/tokens"
)

var pkg = ""

func testInitialiser(args []tokens.Token) {
	pkg = args[0].String
}

func TestParser_Parse(t *testing.T) {
	rules := []Rule{
		Rule{Pattern: []tokens.TokenID{tokens.PACKAGE, tokens.IDENT}, Initialiser: testInitialiser, Args: []int{1}},
	}

	toks := []tokens.Token{
		tokens.Token{Token: tokens.PACKAGE, String: "package"},
		tokens.Token{Token: tokens.IDENT, String: "main"},
	}

	parser := New(rules)
	calls, err := parser.Parse(toks)
	if err != nil {
		t.Fatalf("unexpected error: %+v", err)
	}

	if len(calls) != 1 {
		t.Errorf("expected one call, received %d", len(calls))
	}

	calls[0].F(calls[0].Args)

	expectPkg := "main"
	if pkg != expectPkg {
		t.Errorf("expected package %q, received %q", expectPkg, pkg)
	}
}

type args []string

func (a *args) init(args []tokens.Token) {
	for _, arg := range args {
		*a = append(*a, arg.String)
	}
}

func (a *args) arry() []string {
	out := make([]string, len(*a))
	for i := range *a {
		out[i] = (*a)[i]
	}
	return out
}

func TestParser_Parse_Variadic(t *testing.T) {
	argList := make(args, 0)

	rules := []Rule{
		Rule{Pattern: []tokens.TokenID{tokens.IDENT, tokens.IDENT}, Initialiser: argList.init, Args: []int{0, 1}, Repeats: true},
	}

	toks := []tokens.Token{
		tokens.Token{Token: tokens.IDENT, String: "string"},
		tokens.Token{Token: tokens.IDENT, String: "foo"},
		tokens.Token{Token: tokens.COMMA, String: ","},
		tokens.Token{Token: tokens.IDENT, String: "int"},
		tokens.Token{Token: tokens.IDENT, String: "bar"},
		tokens.Token{Token: tokens.COMMA, String: ","},
		tokens.Token{Token: tokens.IDENT, String: "bool"},
		tokens.Token{Token: tokens.IDENT, String: "baz"},
	}

	expect := []string{"string", "foo", "int", "bar", "bool", "baz"}

	parser := New(rules)
	calls, err := parser.Parse(toks)
	if err != nil {
		t.Fatalf("unexpected error: %+v", err)
	}

	if len(calls) != 1 {
		t.Errorf("expected one call, received %d", len(calls))
	}

	calls[0].F(calls[0].Args)

	received := argList.arry()

	if !reflect.DeepEqual(expect, received) {
		t.Errorf("expected %+v (len: %d), received %+v (len: %d)", expect, len(expect), received, len(received))
	}
}

func TestSubs(t *testing.T) {
	for _, test := range []struct {
		loc    int
		max    int
		expect int
	}{
		{0, 3, 0},
		{1, 3, 1},
		{2, 3, 2},
		{3, 3, 0},
		{4, 3, 1},
		{5, 3, 2},
		{6, 3, 0},
		{7, 3, 1},
		{8, 3, 2},
		{9, 3, 0},
	} {
		t.Run(fmt.Sprintf("subs(%d,%d)", test.loc, test.max), func(t *testing.T) {
			received := subs(test.loc, test.max)
			if test.expect != received {
				t.Errorf("expected %d, received %d", test.expect, received)
			}
		})
	}
}
