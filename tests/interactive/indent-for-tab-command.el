; goto-line 1 RET indent-for-tab-command
; goto-line 2 RET indent-for-tab-command
; goto-line 4 RET indent-for-tab-command
; indent-for-tab-command save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-gg1\r\t\M-gg2\r\t\M-gg4\r\t\C-x\C-s\C-x\C-c")
