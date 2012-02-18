;; FIXME: Fails on Emacs unless run without --batch; why?

; ESC 3 next-line a save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\e3\C-na\C-x\C-s\C-x\C-c")
