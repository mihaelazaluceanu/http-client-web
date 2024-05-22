CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c parson.c
	$(CC) -g -o client client.c requests.c helpers.c buffer.c parson.c

run: client
	./client

clean:
	rm -f *.o client

# Numele arhivei generate de comanda `pack`
ARCHIVE := 324CC_Zaluceanu_Mihaela_Tema4.zip

pack:
		@find $(SRC_DIR) \
		\( -path "./_utils/*" -prune \) -o \
		-regex ".*\.\(c\|h\)" -exec zip $(ARCHIVE) {} +
	@zip $(ARCHIVE) Makefile
	@[ -f README.md ] && zip $(ARCHIVE) README.md \
		|| echo "You should write README.md!"
	@echo "Created $(ARCHIVE)"
