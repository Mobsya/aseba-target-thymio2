#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef uint32_t uint32;           //!< 32 bits unsigned integer
typedef int32_t sint16;            //!< 16 bits signed integer
typedef uint16_t uint16;          //!< 16 bits unsigned integer
typedef signed char sint8;                      //!< 8 bits signed integer
typedef unsigned char uint8;            //!< 8 bits unsigned integer


struct __attribute((packed)) _record  {
        uint16 poweron;
        uint16 studio;
        uint16 usb;
        uint8 flags[3];
        uint8 switchon;
        uint8 reprogram;
        uint8 mmenu;
        uint8 mfollow;
        uint8 mexplorer;
        uint8 macc;
        uint8 mline;
        uint8 mrc5;
        uint8 msound;
        uint8 mvm;
        uint8 poweroff;
        uint8 _;
}; // Sizeof(_record) == 21 == 7 instructions


struct __attribute((packed)) _header  {
        uint16 version;     // binary format version.
        uint16 switchon; // number of time it has been switched on
        uint32 poweron;  // Poweron time in minutes
        uint32 studio; // Studio use time in minutes
        uint32 usb; // usb-non-studio time in minutes
        uint16 reprogram; // Number of time it has been reprogramed
        uint16 mmenu;       // mode time, in minutes
        uint16 mfollow; // ''
        uint16 mexplorer;// ''
        uint16 macc;        // ''
        uint16 mline;       // ''
        uint16 mrc5;        // ''
        uint16 msound;      // ''
        uint16 mvm;         // ''
        uint16 poweroff;// poweroff time in days.
        uint8 flags[3]; // flags, or-ed
        uint8 _[6];      // padding, can be used for something else.
}; // sizeof(_header) == 45 == 15 instruction

#define RECORD_PER_PAGE 71
struct  __attribute((packed)) _page  {
	struct _header header;
	struct _record record[RECORD_PER_PAGE];
}; // sizeof(_page) ==45 + 71*21 = 1536

struct __attribute((packed)) _dump {
	struct _page page[2];
	struct _record current_day;
}; // sizeof(_dump) = 1536*2+21 = 3093


#define LOG_FLAG_INTERNAL		(0) // always not set
#define LOG_FLAG_BATTERY 		(1)
#define LOG_FLAG_ASEBABUG 		(2)
#define LOG_FLAG_VMCODESD		(3)
#define LOG_FLAG_PLAYBACKSD		(4)
#define LOG_FLAG_FLASHVMCODE	(5)
#define LOG_FLAG_RECORDSD		(6)
#define LOG_FLAG_MOTORUSED		(7)
#define LOG_FLAG_IRUSED			(8)
#define LOG_FLAG_NTCUSED		(9)
#define LOG_FLAG_SOUND			(10)
#define LOG_FLAG_LEDIR			(11)
#define LOG_FLAG_LEDRGB			(12)
#define LOG_FLAG_LEDCIRCLE		(13)
#define LOG_FLAG_ACCUSED		(14)
#define LOG_FLAG_BUTTONUSED		(15)
#define LOG_FLAG_SOUNDTRESH		(16)
#define LOG_FLAG_RC5USED		(17)
#define LOG_FLAG_LEDBUTTON		(18)
#define LOG_FLAG_LEDOTHER		(19)


static int flag_set(uint8 flags[3], int f) {
	unsigned int p = f / 8;
	return flags[p] & (1 << (f - 8*p));
}

static int dump_record(struct _record * r, int c) {
	int i;
	if(flag_set(r->flags, LOG_FLAG_INTERNAL))
		return 0;

	if(c != -1)
		printf("Record: #%d\n",c);
	
	printf("\tPoweron time: \t\t%d \tminutes\n",r->poweron);
	printf("\tAsebastudio time: \t%d \tminutes\n",r->studio);
	printf("\tUsb-only time: \t\t%d \tminutes\n",r->usb);
	printf("\tPoweroff time: \t\t%d \tdays\n",r->poweroff);
	printf("\tSwitch-on count:\t%d\n",r->switchon);
	printf("\tReprogram count:\t%d\n",r->reprogram);
	printf("\tMode time:\n");
	printf("\t\tMenu\t\t%d\tminutes\n",r->mmenu);
	printf("\t\tExplorer\t%d\tminutes\n",r->mexplorer);
	printf("\t\tFollow\t\t%d\tminutes\n",r->mfollow);
	printf("\t\tAcc\t\t%d\tminutes\n",r->macc);
	printf("\t\tLine\t\t%d\tminutes\n",r->mline);
	printf("\t\tRc5\t\t%d\tminutes\n",r->mrc5);
	printf("\t\tSound\t\t%d\tminutes\n",r->msound);
	printf("\t\tVirtual Machine\t%d\tminutes\n",r->mvm);
	
	printf("Flags: 0x%02x%02x%02x (", r->flags[0], r->flags[1], r->flags[2]);
	for(i = 0; i < 24; i++) 
		if(flag_set(r->flags, i))
			printf("%d ",i);
	if(!r->flags[0] && !r->flags[1] && !r->flags[2])
		printf("none");
 	printf(")\n");
	
	return 1;
}

static void dump_header(struct _header * h) {
	int i;
	if(flag_set(h->flags, LOG_FLAG_INTERNAL)) {
		printf("* Header absent\n");
		return;
	}
		
	
	if(h->version != 2)
		printf("/!\\ Warning: header version: %d while expecting version 2 /!\\ \n",h->version);
	
	printf("Poweron time: \t\t%d \tminutes\n",h->poweron);
	printf("Asebastudio time: \t%d \tminutes\n",h->studio);
	printf("Usb-only time: \t\t%d \tminutes\n",h->usb);
	printf("Poweroff time: \t\t%d \tdays\n",h->poweroff);
	printf("Switch-on count:\t%d\n",h->switchon);
	printf("Reprogram count:\t%d\n",h->reprogram);
	printf("Mode time:\n");
	printf("\tMenu\t\t%d\tminutes\n",h->mmenu);
	printf("\tExplorer\t%d\tminutes\n",h->mexplorer);
	printf("\tFollow\t\t%d\tminutes\n",h->mfollow);
	printf("\tAcc\t\t%d\tminutes\n",h->macc);
	printf("\tLine\t\t%d\tminutes\n",h->mline);
	printf("\tRc5\t\t%d\tminutes\n",h->mrc5);
	printf("\tSound\t\t%d\tminutes\n",h->msound);
	printf("\tVirtual Machine\t%d\tminutes\n",h->mvm);
	
	printf("Flags: 0x%02x%02x%02x (", h->flags[0], h->flags[1], h->flags[2]);
	for(i = 0; i < 24; i++) 
		if(flag_set(h->flags, i))
			printf("%d ",i);
	if(!h->flags[0] && !h->flags[1] && !h->flags[2])
		printf("none");
 	printf(")\n");
}

static void dump_page(struct _page * p) {
	int i;
	int valid_record = 0;
	
	printf("---- Page dump\n");
	
	dump_header(&p->header);
	
	for(i = 0; i < RECORD_PER_PAGE; i++)
		valid_record += dump_record(&p->record[i], valid_record);
	
	printf("Total: %d valid record\n", valid_record);
}

static void dump_robot(struct _dump * d) {
	printf("####################### START DUMP ##########################\n");
	
	dump_page(&d->page[0]);
	dump_page(&d->page[1]);
	
	
	printf("Current active record:\n");
	dump_record(&d->current_day, -1);
	
	
	printf("####################### END DUMP  ##########################\n");
}

static void usage(char * bin) {
	printf("%s file\n",bin);
}

int main(int argc, char ** argv) {
	FILE * f;
	struct _dump d;

	if(argc != 2) {
		usage(argv[0]);
		exit(-1);
	}

	f = fopen(argv[1],"rb");
	if(!f) {
		perror("Unable to open log file");
		exit(-2);
	}
	
	while(fread(&d, sizeof(d), 1, f) == 1)
		dump_robot(&d);

	return 0;
	
}

