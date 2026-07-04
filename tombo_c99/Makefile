CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -DTOMBO -I../contrib/TomboCrypt
LDFLAGS = -lgdi32 -lcomctl32 -lcomdlg32 -lshell32

TC = ../contrib/TomboCrypt

SRCS = main.c config.c ini.c $(TC)/bf01_file.c $(TC)/blowfish.c $(TC)/md5.c

TARGET = tombo.exe

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	del $(TARGET) 2>nul
