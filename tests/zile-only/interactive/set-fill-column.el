;; FIXME: Fails on Emacs unless run without --batch; why?

; FIXME: Next line should not be needed
(setq sentence-end-double-space nil)

; set-mark next-line next-line kill-region yank yank open-line
; next-line yank next-line
(execute-kbd-macro "\C-@\C-n\C-n\C-w\C-y\C-y\C-o\C-n\C-y\C-n")

; ESC 3 set-fill-column fill-paragraph ESC -3 next-line
; ESC 12 set-fill-column fill-paragraph ESC -3 next-line end-of-line
; ESC 33 set-fill-column fill-paragraph save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\e3\C-xf\M-q\e-3\C-n\e12\C-xf\M-q\e-3\C-n\C-e\e33\C-xf\M-q\C-x\C-s\C-x\C-c")
