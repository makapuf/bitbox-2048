NAME = 2048

GAME_C_FILES = $(NAME).c lib/blitter/blitter.c lib/blitter/blitter_tmap.c
GAME_BINARY_FILES = levels.tset levels.tmap pieces.tmap 
NO_AUDIO=1

include $(BITBOX)/kernel/bitbox.mk

$(NAME).c : levels.h pieces.h build/binaries.h

levels.tset levels.tmap levels.h: levels.tmx 
	python $(BITBOX)/lib/blitter/scripts/tmx.py levels.tmx > levels.h

pieces.tset pieces.tmap pieces.h: pieces.tmx 
	python $(BITBOX)/lib/blitter/scripts/tmx.py pieces.tmx > pieces.h

clean::
	rm -f levels.tset levels.tmap levels.h pieces.h pieces.tmap pieces.tset
