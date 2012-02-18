;; FIXME: Fails on Emacs unless run without --batch; why?

; kill-line kill-line next-line yank save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-k\C-k\C-n\C-y\C-x\C-s\C-x\C-c")
