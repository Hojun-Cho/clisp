* see lib/lib.lisp

* (define fac (lambda (n) (if (== n 0) 1 (* n (fac (+ n -1))))))

* macro
```c
    (macro and (expr . rest)
        (if rest (list 'if expr (cons 'and rest)) expr))

    (and (== 1 1) (== 0 0) (if nil nil 1) (+ 100 100))
    (and ())
```



