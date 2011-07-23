; FIXME: Next line should not be needed
(setq sentence-end-double-space nil)
(end-of-line)
(insert " I am now going to make the line sufficiently long that I can wrap it.")
(fill-paragraph)
(undo)
(insert "a")
(save-buffer)
(save-buffers-kill-emacs)
