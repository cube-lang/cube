package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"

	"golang.org/x/exp/ebnf"
)

func grammars() (out map[string]io.Reader, err error) {
	out = make(map[string]io.Reader)

	err = filepath.Walk(".", func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if !info.IsDir() && filepath.Ext(path) == ".ebnf" {
			out[path], err = os.Open(path)

			return err
		}

		return nil
	})

	return
}

func main() {
	s, err := grammars()
	if err != nil {
		log.Panic(err)
	}

	success := 0
	errors := 0
	for fn, data := range s {
		log.Printf("=======>\t Testing %q", fn)

		grammar, err := ebnf.Parse(fn, data)
		if err != nil {
			panic(err)
		}

		err = ebnf.Verify(grammar, "Program")
		if err != nil {
			errors++
			log.Print(err)
		} else {
			success++
		}

		fmt.Println()
	}

	log.Print("Complete")
	log.Printf("Cases: %d, Successful: %d, Failed: %d", success+errors, success, errors)

	// Let the exit status come from errors. This allows 0 errors to be a successful exit status
	// and allows CI/ builds be clever about counting errors for soft failures/ reoporting
	os.Exit(errors)
}
