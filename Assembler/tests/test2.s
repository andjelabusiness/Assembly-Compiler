.global g1, g2
.extern e1 , e2

.section prva
int *(%r3)
add  s3(%r6) , s1(%pc)
.equ a, g1
s1:  jne *g2
.equ c, 0x03 - 1 + b -g2 + s3

.section druga
mulb %r1h, g2
g1: shl $3, %r4
.equ b , 11- 0b001 + s3- g2
.byte 0x0000A , 9, 0b0101
call *s3(%r7)

.section treca
cmpw $s1 , s3
g2:  jgt *g1(%pc)
push $19
jmp *g2(%r2)
s3: .word   g2, s3

.end