;;; Regression:
;;; crash when creating suffixed buffers for files with the same name

; find-file a/f RET find-file b/f RET kill-buffer RET kill-buffer RET
; save-buffer save-buffers-kill-emacs
(execute-kbd-macro "\C-x\C-fa/f\r\C-x\C-fb/f\r\C-xk\r\C-xk\r\C-x\C-s\C-x\C-c")
