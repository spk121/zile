;; By default interactive downcase-region is disabled in GNU Emacs.

; set-mark goto-line 3 RET downcase-region
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-@\M-gg3\r\C-x\C-l\C-x\C-s\C-x\C-c")
