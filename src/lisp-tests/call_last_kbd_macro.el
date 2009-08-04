(start-kbd-macro nil)
; forward-word a
(execute-kbd-macro "\M-fa")
(end-kbd-macro)
(call-last-kbd-macro)
(save-buffer)
(save-buffers-kill-emacs)
