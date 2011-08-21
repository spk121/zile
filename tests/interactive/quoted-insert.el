; quoted-insert C-a quoted-insert C-h quoted-insert C-z quoted-insert C-q
; quoted-insert ESC O P quoted-insert TAB save-buffer save-buffers-kill-emacs
;
; FIXME: don't allow silly things like "\C-q\F1"
(execute-kbd-macro "\C-q\C-a\C-q\C-h\C-q\C-z\C-q\C-q\C-q\eOP\C-q\t\C-x\C-s\C-x\C-c")
