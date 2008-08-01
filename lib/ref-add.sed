/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ zile / zile /
  tb
  s/ $/ zile /
  :b
  s/^/# Packages using this file:/
}
