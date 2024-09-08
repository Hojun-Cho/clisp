(defn list (x . y) (cons x y))

(macro cond (expr. rest)
	(if rest (list 'if (car expr) (car (cdr expr)) (cons 'cond rest)) 
	expr))

(macro and (expr . rest)
	(if rest (list 'if expr (cons 'and rest)) expr))

;(cond ((== 1 0) 0) ((== 1 1) -1) (+ 100000000))
