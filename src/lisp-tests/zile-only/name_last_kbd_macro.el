; start-kbd-macro capitalize-word end-kbd-macro M-x name-last-kbd-macro foo RET
; M-x foo RET M-x foo RET save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-x(\M-c\C-x)\M-xname-last-kbd-macro\rfoo\r\M-xfoo\r\M-xfoo\r\C-x\C-s\C-x\C-c")
