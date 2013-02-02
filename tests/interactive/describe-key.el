; describe-key \C-f other-window set-mark goto-line 2 RET copy-region-as-kill
; other-window yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-xdescribe-key\r\C-f\C-xo\C-@\M-gg2\r\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
