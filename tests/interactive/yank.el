; set-mark goto-line 3 RET kill-region end-of-buffer yank
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\M-gg3\r\C-w\M->\C-y\C-x\C-s\C-x\C-c")
