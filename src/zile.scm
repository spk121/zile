 ;;   Zile scheme library

 ;;   Copyright (c) 2011 Michael L Gran

 ;;   This file is part of Michael Gran's fork of GNU Zile.

 ;;   This is free software; you can redistribute it and/or modify it
 ;;   under the terms of the GNU General Public License as published by
 ;;   the Free Software Foundation; either version 3, or (at your option)
 ;;   any later version.

 ;;   This is distributed in the hope that it will be useful, but
 ;;   WITHOUT ANY WARRANTY; without even the implied warranty of
 ;;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ;;   General Public License for more details.

 ;;   You should have received a copy of the GNU General Public License
 ;;   along with GNU Zile; see the file COPYING.  If not, write to the
 ;;   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
 ;;   MA 02111-1301, USA.


(define-module (zile)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-69)
  #:use-module (ice-9 pretty-print)
  #:export (
            get-variable-completions
	    %procedure-completions
	    zile-procedures
	    zile-intro
	    set-key
	    global-set-key
	    setq
            ))

(defmacro setq (var val)
  `(define ',var ,val))

(define (dump-module-vars)
  "Make a list of all the variables in this module."
  (hash-map->list cons (module-obarray (current-module))))

(define (current-module-is-zile?)
  "Return #t if the current module is (zile)"
  (let* ((top-module (resolve-module '(zile)))
         (current-module (current-module)))
    (eq? top-module current-module)))

(define (buffer-local-variables)
  "Return a list of the buffer-local variables in the current buffer"
  (if (current-module-is-zile?)
      ;; FIXME: signal an error?
      '()
      ;; else if current module is not ZILE
      (hash-map->list cons (module-obarray (current-module)))))

(define (get-variable-completions)
  "Return a list of the variables in the current buffer for use
as a completion list.  Zile variables can only be boolean, string,
or integer, so other types of variables are not included."
  (define (var? sym var)
    "Return SYM if VAR is a variable that contains a boolean, string,
or integer"
    (if (and (variable? var)
             (or (eq? #t (variable-ref var))
                 (eq? #f (variable-ref var))
                 (integer? (variable-ref var))
                 (string? (variable-ref var))))
        (list sym)
        '()))

  (let ((local-list
         (if (current-module-is-zile?)
             '()			; an empty list
	     ;; else
             (apply append
                    (hash-map->list var? (module-obarray (current-module))))))
        (zile-list
         (apply append
          (hash-map->list
           var?
           (module-obarray (module-public-interface (resolve-module '(zile))))))))
    (append local-list zile-list)))

(define (procedure-minimum-arity proc)
  (let ((arity (procedure-property proc 'arity)))
    (car arity)))

(define (%procedure-completions)
  "Return a list of the procedures the current buffer that
can be called from the minibuffer.  The procedures must have zero
required arguments."
  (define (proc? sym var)
    "Return SYM if VAR is a variable that contains a procedure
that requires no arguments"
    (if (and (variable? var)
	     (procedure? (variable-ref var))
	     (eq? 0 (procedure-minimum-arity (variable-ref var))))
        (list sym)
        '()))

  (let ((local-list '())
        (zile-list
         (apply append
		(hash-map->list
		 proc?
		 (module-obarray (module-public-interface (current-module)))))))
    (append local-list zile-list)))

(define (zile-procedures)
  "Return a list of the procedures in the Zile module."
  (define (proc? sym var)
    "Return SYM if VAR is a procedure"
    (if (and (variable? var)
	     (procedure? (variable-ref var)))
        (list sym)
        '()))
  (let ((zile-list
         (apply append
          (hash-map->list
           proc?
           (module-obarray (module-public-interface (current-module)))))))
    zile-list))

(define (global-set-key key sym)
  "Bind the key sequence KEY to the procedure SYM, where SYM is the
a symbol of the procedure's name.
For example, (global-set-key \"\\C-f\" 'forward-char)"
  (set-key key sym))

(define (zile-intro)
  (let ((txt (list
  ""
  ""
  "You can use this prompt to make temporary macros for Zile."
  ""
  "First, write a procedure with zero arguments: like so"
  "   (define (blammo) (insert \"blammo\"))"
  "Make sure to export it:"
  "   (export blammo)"
  ""
  "Then use 'set-key' set a key sequence to call that procedure."
  "This procedure is also known as 'global-set-key'."
  ""
  "(set-key <key> <command>)"
  ""
  "<command> is a symbol of the name of your procedure."
  "<key> is a string that contains a key sequence."
  ""
  "Sample key sequences"
  "  \"x\"     is the 'x' key"
  "  \"\\\\C-x\" is Control+x"
  "  \"\\\\M-x\" is Alt+x"
  "  \"\\\\F3\"  is the function key F3"
  ""
  "For example (set-key \"\\\\F9\" 'blammo)"
  ""
  "The, ',q' will take you back to your buffer, so you can try it out."
  "")))
    (for-each (lambda (x)
		(display x)
		(newline))
	      txt)))
