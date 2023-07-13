;Binary Search program
ldc 0x1000                  ; MOVE SP far away/A=0x1000
a2sp                        ; transfer A to SP
call main                   ; call main function
HALT                        ; for stopping emulator

main:
    adj -1                  ; adjusting SP/SP=SP+(-1)
    stl 0                   ; mem[SP+0]=A,A=B

    adj -3                  ; adjusting SP for start, end and mid/SP=SP+(-3)
    ldc 0                   ; A=0
    stl 0                   ; storing start/mem[SP+0]=A,A=B
    ldc n                   ; A=n
    stl 1                   ; storing end/mem[SP+1]=A,A=B

    LOOP:                   ; check for end case
        ldl 1               ; end/B=A,A=mem[SP+1]
        ldl 0               ; start/B=A,A=mem[SP+0]
        sub                 ; A = end - start/A=B-A
        brlz EXIT           ; if (A<0) then EXIT
        brz EXIT            ; if (A==0) then EXIT

        ; calculating mid
        ldl 0               ; B=A,A=mem[SP+0]
        ldl 1               ; B=A,A=mem[SP+1]
        add                 ; A := start + end/A=B+A
        ldc 1               ; B := A, A := 1
        shr                 ; A := (start + end) >> 1
        stl 2               ; store mid/ mem[SP+2]=A,A=B

        ldl 2               ; load mid/B=A, A=mem[SP+2]
        ldc array           ; A = array + 0 and B = mid
        add                 ; A = array + mid/A=B+A
        ldnl 0              ; A = array[mid]/A=mem[A+0]

        ldc value           ; A = value and B = array[mid]
        sub                 ; A = array[mid] - value/A=B-A
        brz FOUND           ; if(A==0) then PC=PC+FOUND

        brlz INC_START      ; if(A<0) then PC=PC+INC_START
        ldl 2               ; B=A,A=mem[SP+2]/load mid
        stl 1               ; end := mid/mem[SP+1]=A
        br LOOP

        INC_START:
        ldl 2               ; B=A,A=mem[SP+2]/load mid
        ldc 1               ; B=mid, A=1
        add                 ; A := mid + 1/A=A+B
        stl 0               ; start := mid + 1/mem[SP+0]=A,A=B
        br LOOP             ; PC=PC+LOOP

    FOUND:
    ldl 2                   ; stored value of mid/B=A,A=mem[SP+2]/load mid
    ldc searched_index      ; get addr of result/B=mid,A=searched_index
    stnl 0                  ; store at result/mem[A+0]=mid

    EXIT:
    adj 3                   ; readjust
    ldl 0                   ; retrieve return address
    adj 1                   ; readjust
    return                  ; return

value: SET 100               ; Value to be searched
n: SET 11                   ; length of array
array: data 50              ; array
data 67
data 78
data 87
data 91
data 100
data 203
data 402
data 512
data 516
data 983
searched_index: data -1     ; INDEX