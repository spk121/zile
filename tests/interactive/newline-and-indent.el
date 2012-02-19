; newline-and-indent goto-line 3 RET
; newline-and-indent goto-line 5 RET
; newline-and-indent newline-and-indent save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-j\M-gg3\r\C-j\M-gg5\r\C-j\C-j\C-x\C-s\C-x\C-c")
