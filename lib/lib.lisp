(macro defun (name args body) 
		`(define ,name (lambda ,args ,body)))

(defun list (x . y) (cons x y))

(macro and (expr . rest)
	(if rest (list 'if expr (cons 'and rest)) expr))

(macro cond (expr. rest)
	(if rest (list 'if (car expr) (car (cdr expr)) (cons 'cond rest)) 
	expr))

;exampe (cond ((== 1 0) 0) ((== 1 1) -1) (+ 100000000))
;exampe (cond ((== 1 0) 0) ((== 1 0) -1) (+ 100000000))
