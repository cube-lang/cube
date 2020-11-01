package parser

import (
	"fmt"

	"github.com/cube-lang/cube/tokens"
)

// Parser holds a set of rules, in order of precedence
type Parser struct {
	Rules []Rule
}

// New takes a set of rules and returns a Parser, thus allowing rules
// to be managed outside of this package.
func New(rules []Rule) Parser {
	return Parser{
		Rules: rules,
	}
}

// Parse returns a set of calls which will solve this set of tokens, as processed
// by github.com/cube-lang/cube/tokens
func (p Parser) Parse(toks []tokens.Token) (calls []InitialiserCall, err error) {
	calls = make([]InitialiserCall, 0)

	idx := 0
	for {
		startIdx := idx
		if len(toks) == 0 {
			break
		}

		for _, r := range p.Rules {
			ok, next := r.IsNext(toks)

			if ok {
				// Add correct args.
				//
				// The issue here is in handling args for repeatable/ variadic rules
				// Given VAR IDENT IDENT, repeating, args [1,2] and input
				// `string foo, int bar` we need to:
				//
				//   Split into sub tokens by comma
				//   Iterate through sub tokens for args 1, 2

				args := []tokens.Token{}
				for i := 0; i < next; i++ {
					if isin(r.Args, i, len(r.Pattern)+1) { // We add 1 for the presence of commas
						args = append(args, toks[i])
					}
				}

				calls = append(calls, InitialiserCall{
					F:    r.Initialiser,
					Args: args,
				})

				toks = toks[next:]
				idx += next
				break
			}
		}

		// Check we've moved past our start position
		if startIdx == idx {
			err = fmt.Errorf("syntax error at pos %d (%q)", startIdx, subset(toks))

			return
		}
	}

	return
}

// subset hjoins and returns a subset of tokens, suitable for error messages
func subset(t []tokens.Token) string {
	switch len(t) {
	case 1:
		return t[0].String
	}

	return fmt.Sprintf("%s %s ...", t[0].String, t[1].String)
}

// isin takes a set of rule Args, and the current index of the args iterator
// in Parser.Parse, returning true for when this index is in the rule Args.
//
// Why do this like so? Why not iterate through args and use those indices to select
// from tokens?
//
// Simply because that method wont work with variadic rules.
// Where a rule asks for args [0,1] and has repeated to true,
// and the tokens input are "string foo, int bar", we actually want
// args [0,1,3,4].
//
// Similarly, tokens "string foo, int bar, bool baz" we'd want args
// [0,1,3,4,6,7]
//
// Rather than writing a daft number of rules to handle this, and invariably
// erroring out the first time we have 1 Million and 1 args, we use this method.
//
// (Note: the above indices make little sense until you remember that a comma is
// also a token)
func isin(args []int, loc, max int) bool {
	idx := subs(loc, max)

	for _, arg := range args {
		if arg == idx {
			return true
		}
	}

	return false
}

// subs returns a normalised Arg index in combination with the
// subset function
func subs(loc, max int) int {
	if loc < max {
		return loc
	}

	loc -= max
	if loc >= max {
		return subs(loc, max)
	}

	return loc
}
