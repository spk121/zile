; FIXME: Next line should not be needed
(setq sentence-end-double-space nil)

; end-of-line
; " I am now going to make the line sufficiently long that I can wrap it."
; fill-paragraph save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-e I am now going to make the line sufficiently long that I can wrap it.\M-q\C-x\C-s\C-x\C-c")
