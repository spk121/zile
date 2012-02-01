; switch-to-buffer "testbuf" ENTER "get this" ENTER switch-to-buffer ENTER
(execute-kbd-macro "\C-xbtestbuf\rget this\r\C-xb\r")

; switch-to-buffer "testb" TAB ENTER "leave this" ENTER goto-line 1 ENTER kill-line kill-line
; switch-to-buffer ENTER yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-xbtestb	\rleave this\r\M-gg1\r\C-k\C-k\C-xb\r\C-y\C-x\C-s\C-x\C-c")
