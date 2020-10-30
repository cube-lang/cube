package tokens

import (
	"fmt"
	"io"
	"regexp"
	"text/scanner"
)

type tokInp struct {
	re  string
	tok TokenID
}

type Token struct {
	Token    TokenID
	String   string
	File     string
	Line     int
	StartPos int
	EndPos   int
}

type tokenRegexp struct {
	r *regexp.Regexp
	t TokenID
}

func (tr tokenRegexp) isToken(s string) bool {
	return tr.r.Match([]byte(s))
}

type Tokeniser []tokenRegexp

func New() (m Tokeniser) {
	m = make(Tokeniser, 0)

	for _, t := range tokens {
		m.Add(t.re, t.tok)
	}

	return
}

func (m *Tokeniser) Add(re string, tok TokenID) {
	*m = append(*m, tokenRegexp{r: regexp.MustCompile(re), t: tok})
}

func (m Tokeniser) TokenID(s string) (t TokenID, ok bool) {
	for _, tr := range m {
		ok = tr.isToken(s)
		if ok {
			t = tr.t

			return
		}
	}

	return
}

func (m Tokeniser) Tokens(fn string, f io.Reader) (t []Token, err error) {
	var s scanner.Scanner

	s.Init(f)
	s.Filename = fn
	s.Mode &= scanner.SkipComments
	s.Whitespace = 0

	t = make([]Token, 0)

	var thing string
	for tok := s.Scan(); tok != scanner.EOF; tok = s.Scan() {
		thing = ""
		startPos := -1
		endPos := -1

		for {
			if startPos == -1 {
				startPos = s.Position.Column
			}

			if tok == '\t' || tok == ' ' || tok == '\n' || tok == '\r' {
				break
			}

			endPos = s.Position.Column
			txt := s.TokenText()

			thing = fmt.Sprintf("%s%s", thing, txt)

			tok = s.Scan()

			if tok == scanner.EOF {
				break
			}
		}

		if thing == "" {
			continue
		}

		matched, ok := m.TokenID(thing)
		if !ok {
			err = fmt.Errorf("%s: %q at line %d, columns %d to %d is invalid", s.Position.Filename, thing, s.Position.Line, startPos, endPos)

			return
		}

		t = append(t, Token{Token: matched, String: thing, File: s.Filename, Line: s.Position.Line, StartPos: startPos, EndPos: endPos})
	}

	return
}
