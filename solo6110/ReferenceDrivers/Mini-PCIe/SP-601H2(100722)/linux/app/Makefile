
CFLAGS	+= -I../module/include

all: display enc_config stream_rec stream_play stream_loopback audio_rec audio_play fb_print fbctrl jpeg video_input

solo_disp.o: solo_disp.c
	$(CC) $(CFLAGS) -o solo_disp.o -c solo_disp.c

display.o: display.c
	$(CC) $(CFLAGS) -o display.o -c display.c

display: display.o solo_disp.o
	$(CC) -o display display.o solo_disp.o

fbctrl.o: fbctrl.c
	$(CC) $(CFLAGS) -o fbctrl.o -c fbctrl.c

fbctrl: fbctrl.o
	$(CC) -o fbctrl fbctrl.o

fb_print.o: fb_print.c
	$(CC) $(CFLAGS) -o fb_print.o -c fb_print.c

fb_print: fb_print.o
	$(CC) -o fb_print fb_print.o

enc_config.o: enc_config.c
	$(CC) $(CFLAGS) -o enc_config.o -c enc_config.c

enc_config: enc_config.o
	$(CC) -lcurses -o enc_config enc_config.o

stream_rec.o: stream_rec.c
	$(CC) $(CFLAGS) -o stream_rec.o -c stream_rec.c

stream_rec: stream_rec.o
	$(CC) -o stream_rec stream_rec.o

stream_play.o: stream_play.c
	$(CC) $(CFLAGS) -o stream_play.o -c stream_play.c

stream_play: stream_play.o solo_disp.o
	$(CC) -o stream_play stream_play.o solo_disp.o

stream_loopback.o :stream_loopback.c
	$(CC) $(CFLAGS) -o stream_loopback.o -c stream_loopback.c

stream_loopback: stream_loopback.o solo_disp.o
	$(CC) -o stream_loopback -lpthread -lcurses stream_loopback.o solo_disp.o

audio_rec.o: audio_rec.c
	$(CC) $(CFLAGS) -o audio_rec.o -c audio_rec.c

audio_rec: audio_rec.o
	$(CC) -o audio_rec audio_rec.o

audio_play.o: audio_play.c
	$(CC) $(CFLAGS) -o audio_play.o -c audio_play.c

audio_play: audio_play.o
	$(CC) -o audio_play audio_play.o

jpeg.o: jpeg.c
	$(CC) $(CFLAGS) -o jpeg.o -c jpeg.c

jpeg: jpeg.o
	$(CC) -o jpeg jpeg.o

video_input.o: video_input.c
	$(CC) $(CFLAGS) -o video_input.o -c video_input.c

video_input: video_input.o
	$(CC) -o video_input video_input.o

clean:
	rm -f *.o display enc_config stream_rec stream_play stream_loopback audio_rec audio_play fb_print fbctrl jpeg video_input
