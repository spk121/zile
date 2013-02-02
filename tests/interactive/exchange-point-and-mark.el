; ESC 4 forward-char set-mark forward-line forward-line
; exchange-point-and-mark f save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\e4\C-f\C-@f\M->\C-x\C-x\C-x\C-s\C-x\C-c")

;; With a direct translation, Emacs exits 255 with 'end of buffer' error
;; for some reason?!
;(execute-kbd-macro "\e4\C-f\C-@\C-n\C-n\C-x\C-xf\C-x\C-s\C-x\C-c")
