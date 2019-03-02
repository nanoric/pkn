.code
     

get_fs proc
    mov rax, fs
    ret
get_gs endp
     

get_gs proc
    mov rax, gs
    ret
get_gs endp


set_fs proc
    mov fs, rcx
    ret
get_gdtr endp


get_gdtr proc
    sgdt [rcx]
    ret
get_gdtr endp


end