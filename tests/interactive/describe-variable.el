; describe-variable "tab-width" RET other-window set-mark goto-line 2 RET
; universal-argument backward-word copy-region-as-kill other-window yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-xdescribe-variable\rtab-width\r\C-xo\C-@\M-gg2\r\C-u\M-b\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
