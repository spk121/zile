; describe-bindings other-window set-mark goto-line 2 RET copy-region-as-kill
; other-window yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-xdescribe-bindings\r\C-xo\C-@\M-gg2\r\ew\C-xo\C-y\C-x\C-s\C-x\C-c")
