; start-kbd-macro open-line foo LEFT LEFT LEFT end-kbd-macro
; prefix-cmd 3 call-last-kbd-macro save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-x(foo\r\UP\C-x)\e3\C-xe\C-x\C-s\C-x\C-c")
