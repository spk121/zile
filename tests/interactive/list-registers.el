; set-mark copy-to-register 1 list-registers other-window set-mark
; goto-line 2 RET copy-region-as-kill other-window yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\C-xrx1\M-xlist-registers\r\C-xo\C-@\M-gg2\r\M-w\C-xo\C-y\C-x\C-s\C-x\C-c")
