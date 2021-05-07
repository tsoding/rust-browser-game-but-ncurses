rust-browser-game-but-ncurses: main.o game.o
	cc -Wall -Wextra -o rust-browser-game-but-ncurses main.o game.o

main.o: main.c game.h
	cc -Wall -Wextra -std=c11 -c main.c

game.o: game.rs
	rustc -C opt-level=s -C panic=abort game.rs --emit=obj
