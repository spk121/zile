; this is a bunch of supported stuff.  it's good regression testing material...
; uncomment the part you want to try...
; (+ 4 5)

; some simple math functions
(+ 4 3 2)
(- 10 4)
(- 3)
(- (- 1 10) )
(* 4 9)
(/ 100 20)

(% 2001 4)
(% 4 2001)

(4)
(-4)
4

(and (< 100 (setq a 1)) (> 300 (setq a 2)) (setq a 3) )
(a)
(and (> 100 (setq a 1)) (> 300 (setq a 2)) (setq a 3) )
(a)
(and (> 100 (setq a 1)) (> 300 (setq a 2)) (setq a 400) )
(a)
(and (> 100 (setq a 1)) (> 300 (setq a 2)) )
(a)

(or (< 100 (setq a 1)) (< 300 (setq a 2)) NIL)
(a)

(< 100 5)
(< 5 100)
(< 5 5)

(<= 100 5)
(<= 5 100)
(<= 5 5)

(> 100 5)
(> 5 100)
(> 5 5)

(>= 100 5)
(>= 5 100)
(>= 5 5)

(= 100 5)
(= 5 100)
(= 5 5)

(not (= 100 5))
(not (= 5 100))
(not (= 5 5))

(setq hello '(0 1 1 3 4))
(hello)
(atom hello)
;(atom (setq foo  (+ 2 3)))
(atom (setq bar '(+ 2 3)))
(atom 'a)
(atom 8)
(atom '(a b c))

(setq hi quote (H E L L O))
(setq hi2 '(H E L L O))

(setq floop goo)
(setq cheese "cheese is quite yummy")
(setq f 34)
(setq g '(+ 4 3))
(setq q 3)
(setq w '3)
;(setq e '(3))    ; these don't currently get handled properly in eval
;(setq r '((3)))  ; these don't currently get handled properly in eval
;(setq t '(()))   ; these don't currently get handled properly in eval
(setq g (+ 9 2))
(setq foo '(+ (- 3 4 5) (* 2 3 4)))
(setq bar (+ (- 3 4 5) (* 2 3 4)))
(+ 2 (setq p (+ 5 3)))

(+ f g p)

(+ 2 (setq x (* 3 4)))
(+ 0 x)

(setq x 5)
(setq y (+ 1 (+ 0 x)))
(x) (y)

(+ 4 '(+ 3 4 '5 6 7))
(+ 4 quote (+ 3 4 '5 6 7))

(setq mud "dirt" smog "smoke")

(car '(a b c))
(cdr '(a b c))
(setq x '(a b c))
(car x)
(cdr x)

(cdr '())
(car '())
(cdr '(a))
(car '(a))

(car '((a b)))
(car (car '((a b))))
(cdr '((a b) (c d))))

(cdr '(a b))
(car (cdr '(a b c d e)))
(car '( (a b c) (d e f) (g h i) ) )
(cdr '( (a b c) (d e f) (g h i) ) )

(car (cdr '((a b) (c d)) ))
(cdr (car '((a b) (c d)) ))

(atom (cdr '((a b) (c d)) ))

(cdr (car '((a b) (c d))))

(cdr (car '((a b c) (d e f))))
(car (car '((a b c) (d e f))))
(car (cdr '((a b c) (d e f))))
(car (car (cdr '((a b c) (d e f)))))
(cdr (car (cdr '((a b c) (d e f)))))
(car (cdr (car (cdr '((a b c) (d e f))))))

(cons 'a '(b c))
(setq x (cons 'a '(b c)))
(car x)
(cdr x)
(cons 'a '(b))
(cons '(a b) '(c d))
(cons 'a (cons 'b '(c d)))

(setq x '(a b))
(cons (car x) (cons (car (cdr x)) '(c d)) )

(setq x 'a)
(setq y '(b c))
(cons x y)
(x)
(y)
(car (setq x '(a b c)))
(car '(setq x '(a b c)))

(list 'a 'b 'c)
(list 'a '(b c) 'd)
(list 'a 'b 'c 'd)
(list 'a)
(list)

(setq result (if (< 3 4) (setq a '(t r o o)) (setq b '(f a l e s))) )
(a)(b)(result)
(setq result (if (> 3 4) (setq c '(t r o o)) (setq d '(f a l e s))) )
(c)(d)(result)

(setq result (if (< 3 4) (setq a '(t r o o)) ) )
(a)(b)(result)
(setq result (if (> 3 4) (setq c '(t r o o)) ) )
(c)(d)(result)

(unless (< 3 4) (setq x (+ 3 4)) (setq y '(+ 9 8)) )
(unless (> 3 4) (setq w (+ 3 4)) (setq z '(+ 9 8)) )

(when (< 3 4) (setq a (+ 3 4)) (setq d '(+ 9 8)) )
(when (> 3 4) (setq s (+ 3 4)) (setq f '(+ 9 8)) )

(cond ( (> 3 4) (setq a 'one1) (setq b 'one2) )
      ( (= 3 4) (setq c 'two1) (setq d 'two2) )
      ( (< 3 4) (setq e 'three1) (setq f 'three2) )
)

(cond ( (> 3 4) (setq g 'one1) (setq h 'one2) )
      ( (= 3 4) (setq i 'two1) (setq j 'two2) )
      ( (< 3 4) )
)

(princ '"hello world")
(princ "this is also a test");

(setq a 'b)
(setq b 'c)
(a)
(b)
(eval a)

(eval (cons '+ '(2 3)))

(prog1 (car '(a b c)) (cdr '(a b c)) (cdr '(d e f)) (cdr '(g h i)))
(prog2 (car '(a b c)) (cdr '(a b c)) (cdr '(d e f)) (cdr '(g h i)))
(progn (car '(a b c)) (cdr '(a b c)) (cdr '(d e f)) (cdr '(g h i)))

(setq hello '(0 1 1 3 4))
(hello)
(atom hello)

(setq a '(f 0 o))
(setq b (+ 3 4))
(setq c (+ 3 4) d (+ 1 b) e '(4 5 6))
(setq f (+ 3 4) g (+ 1 b) h)

(setq g '(a b c d))
(set 'b '(a b c d))

(set (car g) (cdr g))

(setq x '(a b c))
(setq y (cdr x))
(setq z '(b c))

(equal (cdr x) y)
(equal y z)
(equal z z)
(equal x z)
(equal x '(a b c))

(defun addthree (x) (+ x 3))

(defun addtwoto3 (x y) (+ x y 3))

(addtwoto3 2 3)
(addtwoto3 2 3 4)

addthree 3)
(addthree 3 5 7)

(addthree (* 4 (1- 7)))

(defun average (x y) (/ (+ x y) 2))
(average 7 (car '(21 3 4 5)))
(average 9 31)

(setq foo  '(7 21 34 22 99))
(car foo) (car (cdr foo))
(/ (+ (car foo) (car (cdr foo))) 2)

(defun averagel (x) (/ (+ (car x) (car (cdr x))) 2))
(averagel '(7 21 34 22 99))
(averagel '(9 31 34 22 99))

(defun three (x) (+ 2 1))
(three 4)
(setq x (+ 4 5))
(setq x (+ 1 x))
(defun addthree (x) (+ x 3))

(addthree 4)
(addthree (5))

(addthree 4)
(addthree 5 2)
(defun seven () '(7))
(defun seven2 () (7))

(seven)
(seven2 ())
