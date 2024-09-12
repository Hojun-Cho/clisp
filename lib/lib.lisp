(defmacro defun (name args body) 
		`(define ,name (block ,name (lambda ,args ,body))))

(defmacro cond (expr . rest)
	(if (not expr)
		nil
		(let ((test  (car expr)))
			`(if ,test
				(progn ,test ,@(cdr expr))
				(cond ,@rest)))))

(defmacro and (expr . rest)
	(if (not rest)
		expr
		(if (cond (not expr) nil)
			`(and ,@rest))))

(defmacro or (expr . rest)
	(if rest
		(cond (expr) (`(or ,@rest)))
		expr))

(defmacro when (test . rest)
	`(if ,test
		(progn ,@rest)))

(defmacro unless (test . rest)
	`(if (not ,test)
		(progn ,@rest)))

(defun list (x . y) (cons x y))

