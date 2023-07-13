; A program to calculate and store the sum of two given numbers

ldc a       ; get address of a
ldnl 0      ; store a in reg A

ldc b       ; get address of b; A's shifted to B
ldnl 0      ; store b in reg b

add         ;  A = B+A

ldc result  ; get result address; A's shifted to B
stnl 0      ; store B's

ldc result  ; get address of result
ldnl 0      

ldc c       ; get address of c
ldnl 0      

add         ;  A = C+A

ldc result  ; get result address; A's shifted to B
stnl 0      ; store B's

HALT        ; end the program



a:  data 4
b:  data 5
c:  data 7

result:    data 0   ; resut will be stored here