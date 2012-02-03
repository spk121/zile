; FIXME: Not sure why this doesn't work in Emacs
(end-of-buffer)
(scroll-down nil)
(insert "a")
(save-buffer)
(save-buffers-kill-emacs)
