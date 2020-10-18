(require 'generic-x) ;; we need this

(define-generic-mode
    'cube-mode                         ;; name of the mode to create
  '("//")                           ;; comments start with '!!'
  '("var" "f"
    "return" "package")                     ;; some keywords
  '(("=" . 'font-lock-operator)     ;; '=' is an operator
    ("[0-9]+" . 'font-lock-variable-name-face)
    ("->" . 'font-lock-builtin))
  '("\\.cb$")                      ;; files for which to activate this mode
  nil                              ;; other functions to call
  "A mode for cube files"            ;; doc string for this mode
  )
