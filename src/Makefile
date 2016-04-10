SRC=onid-api.c 
SERSRC=simple-server.c
CLISRC=simple-client.c
HDR=*.h
SERBIN=onid_ser
CLIBIN=onid_cli
CC=gcc

all: $(SERBIN) $(CLIBIN)

$(SERBIN): $(SERSRC) $(SRC) $(HDR)
	$(CC) -Wall -Wunused-parameter -Wextra -Wpedantic -g $(SERSRC) $(SRC) -o $(SERBIN)
$(CLIBIN): $(CLISRC) $(SRC) $(HDR)
	$(CC) -Wall -Wunused-parameter -Wextra -Wpedantic -g $(CLISRC) $(SRC) -o $(CLIBIN)

.PHONY=clean
clean:
	rm -f $(SERBIN) $(CLIBIN)
