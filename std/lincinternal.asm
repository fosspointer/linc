%define SYS_READ 0
%define SYS_WRITE 1
%define SYS_OPEN 2
%define SYS_CLOSE 3
%define SYS_MMAP 9
%define SYS_MUNMAP 11
%define SYS_EXIT 60
%define STR_BUFFER_SIZE 1024

%define STDOUT 1
%define STDIN 0

section .bss
    __std_string_buffer: resb STR_BUFFER_SIZE
section .data
    __std_true_literal db "true", 0
    __std_false_literal db "false", 0
section .text
    global __memory_copy:function
    global __memory_alloc:function
    global __memory_free:function

    global __mod_f32:function
    global __mod_f64:function

    global __string_length:function
    global __string_equals:function
    global __string_concat:function
    global __string_char_concat:function
    global __char_string_concat:function
    global __char_concat:function
    global __signed_to_string:function
    global __unsigned_to_string:function
    global __char_to_string:function
    global __bool_to_string:function
    global __count_digits_signed:function
    global __count_digits_unsigned:function
    
    global putc:function
    global puts:function
    global putln:function
    global readc:function
    global readraw:function

    global sys_read:function
    global sys_write:function
    global sys_open:function
    global sys_close:function
    global sys_exit:function

__memory_copy:
    test rdx, rdx
    jz .done
    mov rcx, rdx
    rep movsb
.done:
    ret

__memory_alloc:
    add rdi, 8
    mov rsi, rdi
    xor rdi, rdi
    mov rdx, 3
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    mov rax, SYS_MMAP
    syscall

    mov [rax], rsi
    add rax, 8
    ret

__memory_free:
    sub rdi, 8
    mov rsi, [rdi]
    mov rax, SYS_MUNMAP
    syscall
    ret

__mod_f32:
    movss xmm2, xmm0
    divss xmm2, xmm1
    roundss xmm2, xmm2, 3
    mulss xmm2, xmm1
    subss xmm0, xmm2
    ret

__mod_f64:
    movsd xmm2, xmm0
    divsd xmm2, xmm1
    roundsd xmm2, xmm2, 3
    mulsd xmm2, xmm1
    subsd xmm0, xmm2
    ret

__string_length:
    xor rax, rax
.loop:
    mov cl, byte [rdi + rax]
    inc rax
    test cl, cl
    jnz .loop
    dec rax
    ret

__string_equals:
    cmp rdi, rsi
    je .true
.loop:
    mov al, byte [rsi]
    test al, al
    jz .end
    cmp al, byte [rdi]
    jne .false
    inc rdi
    inc rsi
    jmp .loop
.end:
    cmp byte [rdi], 0
    jne .false
.true:
    mov rax, -1
    ret
.false:
    mov rax, 0
    ret

__string_concat:
    push rbx ; caller saved registers that are in use
    push r12
    push r13
    push r14
    call __string_length ; length of the first string (arg 0)
    mov rbx, rdi ; rbx = arg 0
    mov rdi, rsi ; dest = arg 1
    mov r13, rsi ; r13 = arg 1
    mov r14, rax ; r11 = strlen(arg 0)
    call __string_length ; length of the second string (arg 1)
    mov r12, rax ; r12 = strlen(arg 1)
    add rax, r14 ; rax = strlen(arg 1) + strlen(arg 2)
    inc rax ; rax = strlen(arg 1) + strlen(arg 2) + 1
    mov rdi, rax ; dest = rax (memory to allocate)
    call __memory_alloc ; allocate the memory
    push rax
    mov rdi, rax ; dest = the new address
    mov rsi, rbx ; source = arg 0
    mov rdx, r14 ; count = strlen(arg 0)
    call __memory_copy ; first copy
    mov byte [rdi], 'a'
    mov rsi, r13 ; source = arg 1
    mov rdx, r12 ; count = strlen(arg 1)
    call __memory_copy ; second copy
    pop rax
    pop r14
    pop r13 ; restore caller saved registers
    pop r12
    pop rbx
    ret

__string_char_concat:
    call __string_length
    push rax
    push rdi
    push rsi
    mov rdi, rax
    inc rdi
    call __memory_alloc
    pop r8
    pop rsi
    pop rdx
    mov rdi, rax
    call __memory_copy
    mov byte [rdi], r8b
    ret

__char_string_concat:
    mov rdx, rdi
    mov rdi, rsi
    call __string_length
    mov rsi, rdx
    push rax
    push rdi
    push rsi
    mov rdi, rax
    inc rdi
    call __memory_alloc
    pop rcx
    mov byte [rax], cl
    inc rax
    pop rsi
    pop rdx
    mov rdi, rax
    call __memory_copy
    dec rax
    ret

__char_concat:
    push rdi
    push rsi
    mov rdi, 3
    call __memory_alloc
    pop rsi
    pop rdi
    mov byte [rax], dil
    mov byte [rax + 1], sil
    ret 

__signed_to_string:
    push rdi; save rdi - number
    call __count_digits_signed; count the digits of the number
    push rax; save count
    lea rdi, [rax + 1]; allocation = digit count + 1 (for nul termination)
    call __memory_alloc; __memory_alloc(__count_digits_signed(rdi) + 1)
    xor r8b, r8b; clear negative flag
    pop rcx; restore count
    pop rdi; restore rdi - number
    cmp rdi, 0; number test cases
    je .zero; zero case
    lea rsi, [rax + rcx]; buffer += count
    mov rax, rdi; number is in rax now
    mov rdi, 10; rdi = base/divisor
    jg .loop; negative case below:
    mov r8b, -1; set negative counter
    neg rax; negate number 
.loop:
    test rax, rax; check number
    jz .end; if it's zero end the loop
    xor rdx, rdx
    div rdi; divide by 10
    dec rsi
    add dl, '0'; turn the remainder into an ascii digit
    mov byte [rsi], dl; move it to the current byte
    jmp .loop; continue loop
.end:
    test r8b, r8b; test negative flag
    jz .skip; skip negation if positive
    dec rsi; get to last byte
    mov byte [rsi], '-'; set the negative sign
.skip:
    mov rax, rsi; move the buffer to the accumulator
    ret
.zero:
    mov byte [rax], '0'; set the only byte to '0' (not counting NUL)
    ret

__unsigned_to_string:
    push rdi; save rdi - number
    call __count_digits_unsigned; count the digits of the number
    push rax; save count
    lea rdi, [rax + 1]; vvv
    call __memory_alloc; __memory_alloc(__count_digits_unsigned(rdi) + 1)
    pop rcx; restore count
    pop rdi; restore rdi - number
    test rdi, rdi; number test cases
    jz .zero; zero case
    lea rsi, [rax + rcx]; buffer += count
    mov rax, rdi; number is in rax now
    mov rdi, 10; rdi = base/divisor
.loop:
    test rax, rax; check number
    jz .end; if it's zero end the loop
    xor rdx, rdx
    div rdi; divide by 10
    dec rsi
    add dl, '0'; turn the remainder into an ascii digit
    mov byte [rsi], dl; move it to the current byte
    jmp .loop; continue loop
.end:
    mov rax, rsi; move the buffer to the accumulator
    ret
.zero:
    mov byte [rax], '0'; set the only byte to '0' (not counting NUL)
    ret

__char_to_string:
    push rdi
    mov rdi, 2
    call __memory_alloc
    pop rdi
    mov qword [rax], rdi
    ret

__bool_to_string:
    lea rax, [rel __std_true_literal]
    lea rdx, [rel __std_false_literal]
    test di, di
    cmovz rax, rdx
    ret

; putc(char): void
putc:
    push rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rsp] 
    mov rdx, 1
    syscall
    pop rdi
    ret

; puts(string): void
puts:
    xor rdx, rdx
.loop:
    inc rdx
    mov cl, [rdi + rdx]
    cmp cl, 0
    jnz .loop
    mov rsi, rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    syscall
    ret

; putln(string): void
putln:
    call puts
    mov dil, 0x0a
    call putc
    ret

; __count_digits_signed(i64): u64
__count_digits_signed:
    mov rax, rdi
    mov rdi, 10
    xor rcx, rcx
    cmp rax, 0
    jg .loop
    je .zero
    neg rax
    inc rcx
.loop:
    test rax, rax
    jz .end
    xor rdx, rdx
    div rdi
    inc rcx
    jmp .loop
.end:
    mov rax, rcx
    ret
.zero:
    mov rax, 1
    ret

; __count_digits_unsigned(u64): u64
__count_digits_unsigned:
    mov rax, rdi
    mov rdi, 10
    xor rcx, rcx
    test rax, rax
    jnz .loop
    mov rax, 1
    ret
.loop:
    test rax, rax
    jz .end
    xor rdx, rdx
    div rdi
    inc rcx
    jmp .loop
.end:
    mov rax, rcx
    ret

; readc(): char
readc:
    mov rdx, 1
    lea rsi, [rel __std_string_buffer]
    mov rax, SYS_READ
    mov rdi, STDIN
    mov rax, [rel __std_string_buffer]
    syscall
    ret

; readraw(): string
readraw:
    push r11
    mov rdx, STR_BUFFER_SIZE
    lea rsi, [rel __std_string_buffer]
    mov rax, SYS_READ
    mov rdi, STDIN
    syscall
    lea rax, [rel __std_string_buffer]
    lea rsi, [rel __std_string_buffer]
.find_newline:
    cmp byte [rsi], 0xa
    je .found
    inc rsi
    jmp .find_newline
.found:
    mov byte [rsi], 0
    sub rsi, rax
    mov rdi, rsi
    mov r11, rsi
    call __memory_alloc
    mov rdx, r11
    mov rdi, rax
    lea rsi, [rel __std_string_buffer]
    call __memory_copy
    pop r11
    ret

; sys_read(u32, string, u64): void
sys_read:
    mov rax, SYS_READ
    syscall
    ret

; sys_write(u32, string, u64): void
sys_write:
    mov rax, SYS_WRITE
    syscall
    ret

; sys_open(string, i32, i32): u32
sys_open:
    mov rax, SYS_OPEN
    syscall
    ret

; sys_close(u32): void
sys_close:
    mov rax, SYS_CLOSE
    syscall
    ret

; exit(i32): void
sys_exit:
    mov rax, SYS_EXIT
    syscall