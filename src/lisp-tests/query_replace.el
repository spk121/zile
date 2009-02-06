; query-replace l e RET b d RET ! save-buffer save-buffers-kill-zile
(execute-kbd-macro "\M-%le\RETbd\RET!\C-x\C-s\C-x\C-c")
