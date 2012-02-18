;; Cannot provide minibuffer input in Emacs batch mode.

; describe-key \C-f other-window set-mark forward-line copy-region-as-kill
; other-window yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-hk\C-f\C-xo\C-@\C-n\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
