# comment

first_label:
	mov Reg1, -1 #comment on line
	mov Reg0, Reg1HL
	add Reg0, 0xf3
	cmp Reg0, Reg1
	jl second_label
	jmp end

data:
.data 23 0x03 AZA

space:
.fill 0 256

second_label:
	mov Reg2, data
	xor Reg2L, 0xe3a9
	or Reg2, 0x606060
	shl Reg2, 2
	mov Reg3, space
	mov @Reg3, 0xffffffff
	nop

end:

