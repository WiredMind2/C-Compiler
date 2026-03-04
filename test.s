.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $5, %eax
    movl %eax, -4(%rbp)
    movl $5, %eax
    pushq %rax
    movl $3, %eax
    popq %rbx
    addl %ebx, %eax
    movl %eax, -8(%rbp)
    movl -4(%rbp), %eax
    pushq %rax
    movl -8(%rbp), %eax
    pushq %rax
    movl $2, %eax
    popq %rbx
    imull %ebx, %eax
    popq %rbx
    addl %ebx, %eax
    pushq %rax
    movl $5, %eax
    popq %rbx
    imull %ebx, %eax
    movl %eax, -12(%rbp)
    movl -12(%rbp), %eax
    popq %rbp
    ret
