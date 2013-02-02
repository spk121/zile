; FIXME: Next line should not be needed
(setq sentence-end-double-space nil)

; set-mark goto-line 3 RET kill-region yank yank open-line
; goto-line 6 RET yank goto-line 9 RET
(execute-kbd-macro "\C-@\M-gg3\r\C-w\C-y\C-y\C-o\M-gg6\r\C-y\M-gg9\r")

; ESC 3 set-fill-column fill-paragraph ESC -3 goto-line 6 RET
; ESC 12 set-fill-column fill-paragraph ESC -3 goto-line 2 RET backward-char
; ESC 33 set-fill-column fill-paragraph save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\e3\C-xf\M-q\M-gg6\r\e12\C-xf\M-q\M-gg2\r\C-b\e33\C-xf\M-q\C-x\C-s\C-x\C-c")
