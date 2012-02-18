; set-mark ESC 4 next-line shell-command-on-region "sort" RET
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\e4\C-n\C-u\M-|sort\r\C-x\C-s\C-x\C-c")
