(toggle-read-only)
; FIXME: The next line causes an error in Emacs so the test fails
;(insert "aaa")
(toggle-read-only)
(insert "a")
(save-buffer)
(save-buffers-kill-emacs)
