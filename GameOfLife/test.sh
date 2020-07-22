#/bin/sh
cat prog.s board2.s > life.s
echo $1 | spim -file life.s | tail -n +6 > asm.out
gcc -o life life.c
echo $1 | ./life > c.out
diff asm.out c.out