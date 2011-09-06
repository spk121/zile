; start-kbd-macro foo RET end-kbd-macro call-last-kbd-macro undo
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-x(foo\r\C-x)\C-xe\C-_\C-x\C-s\C-x\C-c")
