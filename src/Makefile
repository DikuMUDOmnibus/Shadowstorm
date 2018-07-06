CC      = g++
PROF    = -ggdb
NOCRYPT =

C_FLAGS =  -Wall $(PROF) $(NOCRYPT) -Wno-write-strings
L_FLAGS =  $(PROF)
LIBS	= -lcrypt -lz
TARGET	= shadow

O_FILES = act_comm.o act_enter.o act_info.o act_mob.o act_move.o \
          act_obj.o act_wiz.o alias.o arena.o auction.o bank.o ban.o bard.o \
          bit.o board.o comm.o const.o cmdedit.o db.o db2.o effects.o fight.o flags.o \
	  gquest.o guild.o handler.o healer.o hedit.o instaroom.o interp.o lookup.o lycanth.o \
          magic.o magic2.o mem.o mine.o mob_cmds.o mob_prog.o object_cond.o \
          olc_mpcode.o map.o mount.o music.o multiclass.o nanny.o olc_act.o olc_save.o \
          olc_load_rooms.o olc_load_mobs.o olc_load_objs.o olc_save_new.o olc.o \
          religion.o recycle.o save.o scan.o skills.o skill_save.o social.o special.o string.o quest.o \
          tables.o timer.o unique.o update.o wizlist.o stat.o weather.o \
          trivia.o house.o buffer.o mail.o who.c client.o mccp.o marry.o

shadow: $(O_FILES)
	rm -f $(TARGET)
	$(CC) $(L_FLAGS) -o $(TARGET) $(O_FILES) $(LIBS)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<


clean:
	rm -f *.o
	rm -f *.~
#	rm -f "#*#"

bzip:
	rm -f *.o
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -jcvf ../../$(TARGET).tar.bz2 ../../shadow

gzip:	
	rm -f *.o
	rm -f *.~
#	rm -f "#*#"
	#rm -f $(TARGET)
	tar -zcvf ../../$(TARGET).tar.gz -"Xexclude.dat" ../


	
