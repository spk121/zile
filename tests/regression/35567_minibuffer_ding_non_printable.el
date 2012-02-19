; ESC 1 ESC ! "echo foo" \M-x RET
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\e1\e!echo foo\M-x\r\C-x\C-s\C-x\C-c")
