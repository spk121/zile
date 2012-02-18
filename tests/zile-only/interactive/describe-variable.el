;; Cannot provide minibuffer input in Emacs batch mode.

; describe-variable "tab-width" RET other-window set-mark forward-line
; universal-argument backward-word copy-region-as-kill other-window yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-hvtab-width\r\C-xo\C-@\C-n\C-u\M-b\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
