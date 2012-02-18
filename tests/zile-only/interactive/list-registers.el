;; Cannot provide minibuffer input in Emacs batch mode.

; set-mark copy-to-register 49 list-registers other-window set-mark
; forward-line copy-region-as-kill other-window yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\C-xrx1\M-xlist-registers\r\C-xo\C-@\C-n\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
