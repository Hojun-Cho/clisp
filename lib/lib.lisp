(macro defun (name args body) 
		`(define ,name (lambda ,args ,body)))

(defun list (x . y) (cons x y))

(macro cond (expr . rest)
	(if (not expr) nil
		`(if ,(car expr) (progn ,@expr) (cond ,@rest))))
