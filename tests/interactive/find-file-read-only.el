; kill-buffer RET find-file-read-only "tests/interactive/find-file-read-only.input" save-buffers save-buffers-kill-emacs
(execute-kbd-macro "\C-xk\r\C-x\C-rtests/interactive/find-file-read-only.input\r\C-x\C-s\C-x\C-c")
