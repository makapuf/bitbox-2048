NAME = 2048

GAME_C_FILES = $(NAME).c 
GAME_BINARY_FILES = levels.tset levels.tmap pieces.tmap
USE_ENGINE = 1

include lib/bitbox.mk

build/$(NAME).o : levels.h pieces.h

levels.tset levels.tmap levels.h: levels.tmx 
	python ../bitbox-github/bitbox/scripts/tmx.py levels.tmx > levels.h

pieces.tset pieces.tmap pieces.h: pieces.tmx 
	python ../bitbox-github/bitbox/scripts/tmx.py pieces.tmx > pieces.h

clean::
	rm -f levels.tset levels.tmap levels.h pieces.h pieces.tmap pieces.test