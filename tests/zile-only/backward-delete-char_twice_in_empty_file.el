;; This test passes if it doesn't crash; it doesn't change the input
(find-file "foo")
(insert "abc")
(backward-delete-char 3)
(undo)
(undo)
(save-buffers-kill-emacs)
