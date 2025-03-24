.data
arr1: .word 3,4,5,6
n: .word 45
.text
# --- Compute fib(0) ---
addi x5, x0, 0       # x5 = 0 (input to fib)
jal x1, fib
add x11, x10, x0     # x11 = fib(0)
# --- Compute fib(1) ---
addi x5, x0, 1       # x5 = 1
jal x1, fib
add x12, x10, x0     # x12 = fib(1)
# --- Compute fib(2) ---
addi x5, x0, 2       # x5 = 2
jal x1, fib
add x13, x10, x0     # x13 = fib(2)
jal x0, end          # halt
# --- Fibonacci function ---
fib:
addi x6, x0, 1
bge x5, x6, recurse  # if x5 >= 1, recurse
add x10, x5, x0      # base case: return x5
jalr x0, x1, 0       # return
recurse:
addi x5, x5, -1
jal x2, fib
add x6, x10, x0      # save fib(n-1)
addi x5, x5, -1
jal x2, fib
add x7, x10, x0      # save fib(n-2)
add x10, x6, x7      # x10 = fib(n-1) + fib(n-2)
jalr x0, x1, 0       # return
end:
addi x10, x10, 0       # dummy halt