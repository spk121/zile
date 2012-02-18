; undo in Emacs undoes everything from the start of the script (cf. fill-paragraph_2.el)

; delete-char delete-char delete-char delete-char undo undo undo
;  save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-d\C-d\C-d\C-d\C-_\C-_\C-_\C-x\C-s\C-x\C-c")
