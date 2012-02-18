; set-variable "kill-whole-line" RET t RET
; kill-line save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\M-xset-variable\rkill-whole-line\rt\r\C-k\C-x\C-s\C-x\C-c")
