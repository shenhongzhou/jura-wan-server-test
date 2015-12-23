#/******************************************************************************
#Filename:     Makefile
#
#add by shenhz 20151210
#
#CP：CQJC Jura GateWay
#******************************************************************************/


TARGET = jura-wan-server


OBJS = tcpsocket.o \
	udpsocket.o \
	parson.o \
	base64.o \
	jura-wan-server.o \
	parse_pkt_fwd.o \


LIBS:= -L/usr/lib -ldl -lpthread

CFLAGS=-I/usr/include $(DEFS)

all: $(TARGET) 

$(TARGET) : $(OBJS)
	gcc -o $(TARGET) $^ $(LIBS)
	strip $(TARGET)
	

%.o : %.c
	gcc -o $@ $(CFLAGS) -c $< 

clean :
	rm -f *.o 
	rm -f $(TARGET)
	rm -f *.*~ *~
	
	


