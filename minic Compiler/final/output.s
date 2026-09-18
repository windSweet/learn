.section .rdata
.LC0:
    .asciz "%d\n"
.text
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
    movl $10, %eax
    movl %eax, -4(%rbp)
    movl $0, %eax
    movl %eax, -8(%rbp)
    movl $1, %eax
    movl %eax, -12(%rbp)
    movl $1, %eax
    movl %eax, -16(%rbp)
    movl $2, %eax
    movl %eax, -20(%rbp)
.LL0:
    movl -8(%rbp), %eax
    pushq %rax
    movl -4(%rbp), %eax
    movl %eax, %ecx
    popq %rax
    cmpl %ecx, %eax
    setl %al
    movzbl %al, %eax
    movl %eax, -24(%rbp)
    movl -24(%rbp), %eax
    cmpl $0, %eax
    je .LL1
    movl $1, %eax
    movl %eax, -16(%rbp)
    movl -12(%rbp), %eax
    pushq %rax
    movl $1, %eax
    movl %eax, %ecx
    popq %rax
    addl %ecx, %eax
    movl %eax, -28(%rbp)
    movl -28(%rbp), %eax
    movl %eax, -12(%rbp)
    movl $2, %eax
    movl %eax, -20(%rbp)
.LL2:
    movl -20(%rbp), %eax
    pushq %rax
    movl -20(%rbp), %eax
    movl %eax, %ecx
    popq %rax
    imull %ecx, %eax
    movl %eax, -32(%rbp)
    movl -32(%rbp), %eax
    pushq %rax
    movl -12(%rbp), %eax
    movl %eax, %ecx
    popq %rax
    cmpl %ecx, %eax
    setle %al
    movzbl %al, %eax
    movl %eax, -36(%rbp)
    movl -36(%rbp), %eax
    cmpl $0, %eax
    je .LL3
    movl -12(%rbp), %eax
    pushq %rax
    movl -20(%rbp), %eax
    movl %eax, %ecx
    popq %rax
    cdq
    idivl %ecx
    movl %edx, %eax
    movl %eax, -40(%rbp)
    movl -40(%rbp), %eax
    pushq %rax
    movl $0, %eax
    movl %eax, %ecx
    popq %rax
    cmpl %ecx, %eax
    sete %al
    movzbl %al, %eax
    movl %eax, -44(%rbp)
    movl -44(%rbp), %eax
    cmpl $0, %eax
    je .LL4
    movl $0, %eax
    movl %eax, -16(%rbp)
.LL4:
    movl -20(%rbp), %eax
    pushq %rax
    movl $1, %eax
    movl %eax, %ecx
    popq %rax
    addl %ecx, %eax
    movl %eax, -48(%rbp)
    movl -48(%rbp), %eax
    movl %eax, -20(%rbp)
    jmp .LL2
.LL3:
    movl -16(%rbp), %eax
    cmpl $0, %eax
    je .LL6
    movl -8(%rbp), %eax
    pushq %rax
    movl $1, %eax
    movl %eax, %ecx
    popq %rax
    addl %ecx, %eax
    movl %eax, -52(%rbp)
    movl -52(%rbp), %eax
    movl %eax, -8(%rbp)
.LL6:
    jmp .LL0
.LL1:
    movl -12(%rbp), %eax
    movl %eax, %edx
    leaq .LC0(%rip), %rcx
    subq $40, %rsp
    call printf
    addq $40, %rsp
    movl $1, %eax
    movl %eax, %edx
    leaq .LC0(%rip), %rcx
    subq $40, %rsp
    call printf
    addq $40, %rsp
    movl $0, %eax
    leave
    ret
    xorl %eax, %eax
    leave
    ret
