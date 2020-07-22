# Game of Life on a NxN grid
#
# Written by Stratton Sloane, June 2019

## Requires (from `boardX.s'):
# - N (word): board dimensions
# - board (byte[][]): initial board state
# - newBoard (byte[][]): next board state

## Provides:
	.globl	main
	.globl	decideCell
	.globl	neighbours
	.globl	copyBackAndShow


	.data
iter_query:		.asciiz "# Iterations: "
result_start:  	.asciiz "=== After iteration "
result_end:		.asciiz " ===\n"

########################################################################
# .TEXT <main>
	.text
main:

# Frame:	$fp, $ra, $s0, $s1, $s2, $s3, $s4, $s5, $s6
# Uses:		$s0, $s1, $s2, $s3, $s4, $s5, $s6
# Clobbers:	$a0, $a1

# Locals:	
#	- 'maxiters' in $s0
#	- 'i' in $s1
#	- 'n' in $s2
#	- 'x' in $s3
#	- 'y' in $s4
#	- 'board' in $s5
#	- 'newBoard' in $s6

# Structure:
#	main
#	-> [prologue]
#	-> iter_for
#		-> main_y_for
#			-> main_x_for
#			-> end_main_x_for
#		-> end_main_y_for
#	-> end_iter_for
#	-> [epilogue]

# Code:

	# Set up stack frame
	sw 	$fp, -4($sp)	# Preserve frame pointer
	la 	$fp, -4($sp)	# Set up frame pointer
	sw 	$ra, -4($fp)	# Preserve return address
	la	$sp, -4($fp)    # Set stack pointer to last pushed item

	# Push local variables
	addi 	$sp, -4
	sw	$s0, ($sp)		# Use s0 for: maxiters
	addi 	$sp, -4
	sw	$s1, ($sp)		# Use s1 for: i
	addi 	$sp, -4
	sw	$s2, ($sp)		# Use s2 for: n
	addi 	$sp, -4
	sw	$s3, ($sp)		# Use s3 for: x
	addi 	$sp, -4
	sw	$s4, ($sp)		# Use s4 for: y
	addi 	$sp, -4
	sw	$s5, ($sp)		# Use s5 for: board
	addi 	$sp, -4
	sw	$s6, ($sp)		# Use s6 for: newBoard

	# Ask for # of iterations
	la	$a0, iter_query
	li	$v0, 4
	syscall

	# Get input
	li	$v0, 5
	syscall

	move	$s0, $v0 	# set maxiters
	li 	$s1, 0			# i = 0
	lw	$s2, N 			# n = N

iter_for:
	bge $s1, $s0, end_iter_for	# Break if i >= maxiters

	li 	$s3, 0			# x = 0
	li 	$s4, 0			# y = 0
	la	$s5, board		# board = &board
	la	$s6, newBoard	# newBoard = &newBoard

main_y_for:
	bge $s4, $s2, end_main_y_for	# Break if y >= N

main_x_for:
	bge $s3, $s2, end_main_x_for	# Break if x >= N

	move $a0, $s3		# $a0 = x
	move $a1, $s4		# $a1 = y
	jal neighbours		# call neighbours(x, y)

	lb 	$a0, ($s5)		# $a0 = *board
	move 	$a1, $v0	# $a1 = result of neighbours()
	jal decideCell		# call decideCell(*board, neighbours)
	nop
	sb  $v0, ($s6)		# *newBoard = result of decideCell()

	addi    $s5, 1		# board++
	addi    $s6, 1		# newBoard++
	addi	$s3, 1		# x++
	j	main_x_for
end_main_x_for:

	li	$s3, 0			# x = 0
	addi 	$s4, 1		# y++
	j 	main_y_for
end_main_y_for:

	addi, $s1, 1		# i++

	# Print result
	la	$a0, result_start
	li	$v0, 4
	syscall
	move $a0, $s1		# print i
	li	$v0, 1
	syscall
	la	$a0, result_end
	li	$v0, 4
	syscall

	jal copyBackAndShow

	j 	iter_for
end_iter_for:

main__post:
	lw	$s6, ($sp)		# Restore s6
	addi 	$sp, 4
	lw	$s5, ($sp)		# Restore s5
	addi 	$sp, 4
	lw	$s4, ($sp)		# Restore s4
	addi 	$sp, 4
	lw	$s3, ($sp)		# Restore s3
	addi 	$sp, 4
	lw	$s2, ($sp)		# Restore s2
	addi 	$sp, 4
	lw	$s1, ($sp)		# Restore s1
	addi 	$sp, 4
	lw	$s0, ($sp)		# Restore s0
	addi 	$sp, 4

	la  $sp, 4($fp)
	lw	$ra, -4($fp)
	lw 	$fp, ($fp)

	li	$v0, 0
	jr	$ra

########################################################################
# .TEXT <decideCell>
	.text
decideCell:

# Frame:	$fp, $ra
# Uses:		$a0, $a1
# Clobbers:	$t0, $v0

# Locals:	N/A

# Structure:
#	decideCell
#	-> [prologue]
#	-> decide_cell_alive
#		-> decide_underpop
#		-> decide_stillalive
#		-> decide_overpop
#	-> decide_cell_dead
#		-> decide_dead_growth
#	-> [epilogue]

# Code:

	# Set up stack frame
	sw 	$fp, -4($sp)	# Preserve frame pointer
	la 	$fp, -4($sp)	# Set up frame pointer
	sw 	$ra, -4($fp)	# Preserve return address
	la	$sp, -4($fp)    # Set stack pointer to last pushed item

	li	$t0, 1
	beq $a0, $t0, decide_cell_alive	# if old == 1
	j	decide_cell_dead			# else
	
	decide_cell_alive:
	li	$t0, 2
	blt $a1, $t0, decide_underpop	# if nn < 2
	li 	$t0, 3
	bgt $a1, $t0, decide_overpop 	# else if nn > 3
	j 	decide_stillalive			# else
	decide_underpop:
	li	$v0, 0			# ret = 0
	j	end_decide
	decide_stillalive:
	li 	$v0, 1			# ret = 1
	j 	end_decide
	decide_overpop:
	li 	$v0, 0			# ret = 0
	j 	end_decide

	decide_cell_dead:
	li 	$t0, 3
	beq $a1, $t0, decide_dead_growth	# if nn == 3
	li  $v0, 0							# else
	j 	end_decide
	decide_dead_growth:
	li 	$v0, 1			# ret = 1
	j 	end_decide

	end_decide:

	la  $sp, 4($fp)		# Reset stack pointer
	lw	$ra, -4($fp)	# Restore return address
	lw 	$fp, ($fp)		# Restore frame pointer

	jr 	$ra;

########################################################################
# .TEXT <neighbours>
	.text
neighbours:

# Frame:	$fp, $ra, $s0, $s1, $s2, $s3, $s4, $s5
# Uses:		$a0, $a1, $s0, $s1, $s2, $s3, $s4, $s5
# Clobbers:	$t0, $t1, $t2, $t3

# Locals:	
#	- 'x' in $s0
#	- 'y' in $s1
#	- 'cell_x' in $s2 (from $a0)
#	- 'cell_y' in $s3 (from $a1)
#	- 'board' in $s4
#	- 'nn' in $s5

# Structure:
#	neighbours
#	-> [prologue]
#	-> n_y_for
#		-> n_x_for
#			-> neighbour_exists
#			-> cont_n_x_for
#		-> end_n_x_for
#	-> end_n_y_for
#	-> [epilogue]

# Code:

	# Set up stack frame
	sw 	$fp, -4($sp)	# Preserve frame pointer
	la 	$fp, -4($sp)	# Set up frame pointer
	sw 	$ra, -4($fp)	# Preserve return address
	la	$sp, -4($fp)    # Set stack pointer to last pushed item

	# Push local variables
	addi 	$sp, -4
	sw	$s0, ($sp)		# Use s0 for: x
	addi 	$sp, -4
	sw	$s1, ($sp)		# Use s1 for: y
	addi 	$sp, -4
	sw	$s2, ($sp)		# Use s2 for: cell_x
	addi 	$sp, -4
	sw	$s3, ($sp)		# Use s3 for: cell_y
	addi 	$sp, -4
	sw	$s4, ($sp)		# Use s4 for: board
	addi 	$sp, -4
	sw	$s5, ($sp)		# Use s5 for: nn

	# Initialise variables
	li 	$s0, -1			# x = -1
	li 	$s1, -1			# y = -1
	li 	$s5, 0			# nn = 0
	move 	$s2, $a0 	# cell_x = $a0
	move 	$s3, $a1 	# cell_y = $a1
	la	$s4, board 		# board = &board

	# Function code
n_y_for:
	li 	$t0, 1
	bgt $s1, $t0, end_n_y_for 	# Break if y > 1

n_x_for:
	li 	$t0, 1
	bgt $s0, $t0, end_n_x_for 	# Break if x > 1

	add 	$t0, $s2, $s0		# $t0 = cell_x + x
	add 	$t1, $s3, $s1 		# $t1 = cell_y + y
	lw 	$t2, N 			# $t2 = *N
	addi 	$t2, -1		# $t2 = N - 1 (board limits)

	blt	$t0, $0, cont_n_x_for	# Continue if (cell_x + x < 0) or
	blt	$t1, $0, cont_n_x_for	# 			  (cell_y + y < 0) or
	bgt $t0, $t2, cont_n_x_for  # 			  (cell_x + x > N - 1) or
	bgt $t1, $t2, cont_n_x_for  # 			  (cell_y + y > N - 1) or
	or	$t3, $s0, $s1	# $t3 = x | y
	beq $t3, $0,  cont_n_x_for  #			  x == 0 && y == 0

	# Calculate array location
	lw 	$t2, N 			# loc = *N
	mul $t2, $t2, $t1 	# loc = (cell_y + y) * loc 
	add $t2, $t2, $t0	# loc += (cell_x + x)
	add $t2, $t2, $s4	# loc += &board

	lb 	$t0, ($t2)		# cell = *location
	li 	$t1, 1			
	beq $t0, $t1, neighbour_exists 		# if cell == 1
	j cont_n_x_for
neighbour_exists:
	addi $s5, 1			# nn++
cont_n_x_for:
	addi 	$s0, 1		# x++
	j 	n_x_for
end_n_x_for:

	li 	$s0, -1			# x = -1
	addi 	$s1, 1		# y++
	j 	n_y_for
end_n_y_for:

	move 	$v0, $s5 	# return nn

	# Tear down stack frame
	lw	$s5, ($sp)
	addi 	$sp, 4		# Restore s5
	lw	$s4, ($sp)
	addi 	$sp, 4		# Restore s4
	lw	$s3, ($sp)
	addi 	$sp, 4		# Restore s3
	lw	$s2, ($sp)
	addi 	$sp, 4		# Restore s2
	lw	$s1, ($sp)
	addi 	$sp, 4		# Restore s1
	lw	$s0, ($sp)
	addi 	$sp, 4		# Restore s0

	la  $sp, 4($fp)		# Reset stack pointer
	lw	$ra, -4($fp)	# Restore return address
	lw 	$fp, ($fp)		# Restore frame pointer

	jr 	$ra;

########################################################################
# .TEXT <copyBackAndShow>
	.text
copyBackAndShow:

# Frame:	$fp, $ra, $s0, $s1, $s2, $s3, $s4
# Uses:		$s0, $s1, $s2, $s3, $s4
# Clobbers:	$t0

# Locals:	
#	- 'board' in $s0
#	- 'n' in $s1
#	- 'y' in $s2
#	- 'x' in $s3
#	- 'newBoard' in $s4

# Structure:
#	copyBackAndShow
#	-> [prologue]
#	-> y_for
#		-> x_for
#			-> cell_alive
#			-> cell_dead
#			-> end_cell_check
#		-> end_x_for
#	-> end_y_for
#	-> [epilogue]

# Code:

	# Set up stack frame
	sw 	$fp, -4($sp)		# Preserve frame pointer
	la 	$fp, -4($sp)		# Set up frame pointer
	sw 	$ra, -4($fp)		# Preserve return address
	sw 	$s0, -8($fp)		# Use s0 for: board
	sw 	$s1, -12($fp)		# Use s1 for: n
	sw 	$s2, -16($fp)		# Use s2 for: y
	sw 	$s3, -20($fp)		# Use s3 for: x
	sw 	$s4, -24($fp)		# Use s4 for: newBoard
	la	$sp, -24($fp)		# Set stack pointer to last pushed item

	la	$s0, board			# board = &board
	lw	$s1, N				# n = N 
	li	$s2, 0				# y = 0
	li	$s3, 0				# x = 0
	la	$s4, newBoard		# newBoard = &newBoard

y_for:
	bge	$s2, $s1, end_y_for	# if (y >= n)	

x_for:
	bge	$s3, $s1, end_x_for	# if (x >= n)	

	# Copy new cell value
	lb  $t0, ($s4)			# $t0 = *newBoard
	sb 	$t0, ($s0)			# *board = $t0

	# Print # or -, depending on whether cell is alive
	lb	$t0, ($s0)			# $t0 = *board (current cell)	
	beq $t0, $0, cell_dead	# if cell == 0
	j cell_alive			# else
cell_alive:
	li	$a0, 35				# $a0 = '#'
	j end_cell_check
cell_dead:
	li	$a0, 46				# $a0 = '.'
	j end_cell_check
end_cell_check:
	li	$v0, 11				# Print character
	syscall

	# Go to next cell in row
	addi	$s0, 1			# board++
	addi	$s4, 1			# newBoard++
	addi	$s3, 1			# x++
	j 	x_for
end_x_for:

	# Go to next row
	li	$a0, 10				# $a0 = '\n'
	li  $v0, 11				# call putchar()
	syscall

	li	$s3, 0				# x = 0	
	addi	$s2, 1			# y++
	j 	y_for
end_y_for:

	# Restore stack frame
	la  $sp, 4($fp)
	lw	$ra, -4($fp)
	lw 	$s0, -8($fp)		# Restore s0
	lw 	$s1, -12($fp)		# Restore s1
	lw 	$s2, -16($fp)		# Restore s2
	lw 	$s3, -20($fp)		# Restore s3
	lw 	$s3, -24($fp)		# Restore s4
	lw 	$fp, ($fp)

	jr	$ra
# board2.s ... Game of Life on a 15x15 grid

	.data

N:	.word 15  # gives board dimensions

board:
	.byte 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
	.byte 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0
	.byte 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0
	.byte 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0
	.byte 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1
	.byte 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0
	.byte 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0
	.byte 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0
	.byte 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0
	.byte 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0
	.byte 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1
	.byte 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0
	.byte 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0
	.byte 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0
	.byte 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0

newBoard: .space 225
