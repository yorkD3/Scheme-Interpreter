# Scheme-Interpreter
This project can tokenize, parse and interpret a wide array of scheme code and supports many scheme functions including:
- Define
- Cons,Cdr,Car,Null?, Set, Begin, Quote, Let, Let*, Letrec, Lambda
- Arithmetic, Comparisons, Modulo
- Conditionals
- Currying
- and more


How to run Scheme Interpreter:
Test cases m and e
- ./test-m
- ./test-e

As an interactive interpreter:
- ./scheme < [file.scm] (Interpret a scheme file found in project folder)
- ./scheme (Interactive use of interpreter)

Example Code
```
./scheme
scheme@(guile-user)> (+ 1 2)
$1 = 3
scheme@(guile-user)> (define less-than-or-equal
  (lambda (x y)
    (if (> x y) #f #t)))
scheme@(guile-user)> (less-than-or-equal 200 100)
$2 = #f
scheme@(guile-user)> (cons 1 (cons 1 3))
$3 = (1 1 . 3)
```

* If permissions are denied, enter the command 'chmod a+x test-m test-e'
* Code cannot be run on mac terminal, attempt importing to repl.it and running it there.
