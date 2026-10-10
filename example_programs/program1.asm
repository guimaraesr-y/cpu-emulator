; Each instruction occupies 3 bytes. Labels become byte addresses.
mov R0, 10
mov R1, 20
add R0, R1
sub R1, R0
jmp end
mov R2, 99 ; skipped
end:
hlt
