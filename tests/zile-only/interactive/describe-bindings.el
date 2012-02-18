;; Cannot provide minibuffer input in Emacs batch mode.

; describe-bindings other-window set-mark forward-line copy-region-as-kill
; other-window yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-hb\C-xo\C-@\C-n\ew\C-xo\C-y\C-x\C-s\C-x\C-c")
