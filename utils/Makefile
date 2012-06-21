all: generate-menu

generate-menu: generate-menu.c ../src/keys.c ../src/utils.c \
	../src/config.c ../src/colors.c
	cc generate-menu.c -o generate-menu -I ../src/ ../src/keys.c \
		../src/utils.c ../src/config.c ../src/colors.c -lX11
