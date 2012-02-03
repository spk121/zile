;;; FIXME: Although F1 usually outputs ^[OP, this test will fail
;;;        when the terminal outputs something else instead.

;; quoted-insert F1 RETURN
;; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-q\F1\RET\C-x\C-s\C-x\C-c")
