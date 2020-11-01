package parser

import (
	"reflect"

	"github.com/cube-lang/cube/tokens"
)

// Initialiser is a function which takes the tokens matched for a rule
type Initialiser func([]tokens.Token)

// InitialiserCall holds an Initialiser and the args which need to be passed to
// it.
//
// This is returned from the parser, allowing the calling context to do whatever
// it needs to do.
type InitialiserCall struct {
	F    Initialiser
	Args []tokens.Token
}

type Rule struct {
	// Pattern of TokenIDs this rule relates to. PACKAGE IDENT, for instance,
	// will match `package main`, `package foo`.
	Pattern []tokens.TokenID

	// Whether or not the patter repeats; useful for arguments in functions
	// `IDENT IDENT`, with repeats, will match
	// string foo
	// string foo, string bar
	// string foo, int bar, bool baz
	// Note that at the end of each repeat, a COMMA token is implicitly added
	Repeats bool

	// An Initialiser is a function which takes a set of Tokens. These tokens are
	// a subset of the tokens in a stream of tokens as chosen from Args
	Initialiser Initialiser

	// Args is a slice of indices from tokens that are passed to an Initialiser.
	// When Repeats is true, args 'loops'. For instance:
	//
	// IDENT IDENT, repeats = true, args = [0,1]
	// string foo, bool bar
	// args = [string, foo, bool, bar]
	//  - args is implicitly [0,1,3,4] -> we add len(tokens)+len(COMMA) for each
	// repetition of Pattern we find until we hit the end of the rule
	Args []int
}

// IsNext returns true if the first n tokens of t matches the tokens in this call.
//
// It also returns the index of the first call which does _not_ match this rule,
// allowing the token spool to advance by thgis much.
func (r Rule) IsNext(t []tokens.Token) (ok bool, next int) {
	if len(r.Pattern) > len(t) {
		return
	}

	ids := make([]tokens.TokenID, len(r.Pattern))
	for idx := range ids {
		ids[idx] = t[idx].Token
	}

	if reflect.DeepEqual(r.Pattern, ids) {
		ok = true
		next = len(r.Pattern)

		if next >= len(t) {
			return
		}

		if r.Repeats && t[next].Token == tokens.COMMA {
			tmptok := t[next+1:]
			nok, nnext := r.IsNext(tmptok)
			if nok {
				next += nnext + 1
			}
		}
	}

	return
}
