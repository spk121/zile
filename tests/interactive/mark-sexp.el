; forward-word ( ESC 3 forward-word ) backward-word backward-word ESC 5
; backward-char mark-sexp kill-region save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-f(\e3\M-f)\M-b\M-b\e5\C-b\C-\M-@\C-w\C-x\C-s\C-x\C-c")
