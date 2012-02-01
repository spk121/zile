; search-forward "ra" isearch-forward ENTER 2 isearch-forward isearch-forward ENTER 3
; save-buffer save-buffers-kill-emacs
(set-variable 'isearch-nonincremental-instead nil)
(execute-kbd-macro "\C-sra\C-s\r2\C-s\C-s\r3\C-x\C-s\C-x\C-c")
