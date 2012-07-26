all: generate-config

generate-config: generate-config.c ../src/keys.c ../src/utils.c \
	../src/settings.c ../src/colors.c
	cc generate-config.c -o generate-config -I ../src/ ../src/keys.c \
		../src/rules.c ../src/utils.c ../src/settings.c ../src/colors.c -lX11 -lconfig
