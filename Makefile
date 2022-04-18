all: favicon.ico retro_computers

.PHONY: clean
clean:
	rm favicon.ico
	cd retro_computers && make clean

.PHONY: retro_computers
retro_computers:
	cd retro_computers && make

favicon.ico: favicon.svg
	convert -density 384 favicon.svg -define icon:auto-resize favicon.ico