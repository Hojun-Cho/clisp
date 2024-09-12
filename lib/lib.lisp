(defmacro defun (name args body) 
		`(define ,name (lambda ,args (block ,name ,body))))

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

(defmacro return (res)
	(return-from nil `,res))

(defun list (x . y) (cons x y))


