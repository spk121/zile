; set-mark ESC 2 forward-line kill-region ESC 3 forward-line yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\e2\C-n\C-w\e3\C-n\C-y\C-x\C-s\C-x\C-c")
