(macro defun (name args body) 
		`(define ,name (lambda ,args ,body)))

(macro cond (expr . rest)
	(if (not expr)
		nil
		(let ((test  (car expr)))
			`(if ,test
				(progn ,test ,@(cdr expr))
				(cond ,@rest)))))

(macro and (expr . rest)
	(if (not rest)
		expr
		(if (cond (not expr) nil)
			`(and ,@rest))))

(macro or (expr . rest)
	(if rest
		(cond (expr) (`(or ,@rest)))
		expr))

(macro when (test . rest)
	`(if ,test
		(progn ,@rest)))

(macro unless (test . rest)
	`(if (not ,test)
		(progn ,@rest)))

(defun list (x . y) (cons x y))

