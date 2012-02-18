; kill-buffer RET find-file-read-only "tests/test.input" save-buffers save-buffers-kill-emacs
(execute-kbd-macro "\C-xk\r\C-x\C-rtests/test.input\r\C-x\C-s\C-x\C-c")
