.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
BB0:
    movl $5, -8(%rbp)
    movl -8(%rbp), %eax
    movl %eax, -4(%rbp)
    movl $5, -16(%rbp)
    movl $3, -20(%rbp)
    movl -16(%rbp), %eax
    addl -20(%rbp), %eax
    movl %eax, -24(%rbp)
    movl -24(%rbp), %eax
    movl %eax, -12(%rbp)
    movl $2, -32(%rbp)
    movl -12(%rbp), %eax
    imull -32(%rbp), %eax
    movl %eax, -36(%rbp)
    movl -4(%rbp), %eax
    addl -36(%rbp), %eax
    movl %eax, -40(%rbp)
    movl $5, -44(%rbp)
    movl -40(%rbp), %eax
    imull -44(%rbp), %eax
    movl %eax, -48(%rbp)
    movl -48(%rbp), %eax
    movl %eax, -28(%rbp)
    movl -28(%rbp), %eax
    movl %eax, %eax
    leave
    ret
