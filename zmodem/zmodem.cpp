#include "zmodem.h"
#include "crctab.h"

int hex2int(char hex)
{
	if (hex >= '0' && hex <='9')
		return hex - '0';
	if (hex >= 'a' && hex <= 'f')
		return hex - 'a' + 10;
	return 0;
}

unsigned char decHex(const hex_str_t *h)
{
    return (hex2int(h->hex[0]) << 4) | hex2int(h->hex[1]);
}

void encHex(const unsigned char c, hex_str_t *h)
{
    h->hex[0] = HEX_ARRAY[(c&0xF0) >> 4];
    h->hex[1] = HEX_ARRAY[c&0x0F];
}

void convHex2Plain(const hex_t *hexframe, frame_t* frame)
{
    frame->type = decHex(&(hexframe->type));
    frame->flag[0] = decHex(&(hexframe->flag[0]));
    frame->flag[1] = decHex(&(hexframe->flag[1]));
    frame->flag[2] = decHex(&(hexframe->flag[2]));
    frame->flag[3] = decHex(&(hexframe->flag[3]));
    frame->crc = decHex(&(hexframe->crc[0]))<<8 | decHex(&(hexframe->crc[1]));
}

void convPlain2Hex(const frame_t* frame, hex_t *hexframe)
{
    encHex(frame->type, &(hexframe->type));
    encHex(frame->flag[0], &(hexframe->flag[0]));
    encHex(frame->flag[1], &(hexframe->flag[1]));
    encHex(frame->flag[2], &(hexframe->flag[2]));
    encHex(frame->flag[3], &(hexframe->flag[3]));
    encHex(frame->crc >> 8, &(hexframe->crc [0]));
    encHex(frame->crc & 0x00FF, &(hexframe->crc [1]));
}

uint16_t calcFrameCrc(const frame_t *frame)
{
    int i = 0;
    uint16_t crc;
    crc = updcrc((frame->type & 0x7f), 0);
    for (i = 0; i < 4; i++){
        crc = updcrc(frame->flag[i], crc);
    }
    crc = updcrc(0,updcrc(0,crc));
    return crc;
}

uint32_t calcFrameCrc32(const frame32_t *frame)
{
    int i = 0;
    uint32_t crc = 0xFFFFFFFFL;
    crc = UPDC32((frame->type & 0x7f), crc);
    for (i = 0; i < 4; i++){
        crc = UPDC32(frame->flag[i], crc);
    }
    crc = ~crc;;
    return crc;
}

uint32_t calcBufferCrc32(const char *buf, const unsigned len)
{
    uint32_t i = 0;
    uint32_t crc = 0xFFFFFFFFL;
    for (i = 0; i < len; i++){
        crc = UPDC32(buf[i], crc);
    }
    crc = ~crc;;
    return crc;
}

unsigned getPos(frame_t* frame)
{
	unsigned rxpos = frame->flag[ZP3] & 0377;
	rxpos = (rxpos<<8) + (frame->flag[ZP2] & 0377);
	rxpos = (rxpos<<8) + (frame->flag[ZP1] & 0377);
	rxpos = (rxpos<<8) + (frame->flag[ZP0] & 0377);
	return rxpos;
}

const char* getTypeStr(unsigned char type)
{
	switch (type){
    case ZRQINIT: return "ZRQINIT";
    case ZFILE: return "ZFILE";
    case ZDATA: return "ZDATA";
    case ZEOF: return "ZEOF";
    case ZFIN: return "ZFIN";
    case ZRINIT: return "ZRINIT";
	case ZRPOS: return "ZRPOS";
    case ZNAK: return "ZNAK";
    case ZSINIT: return "ZSINIT";
    case ZACK: return "ZACK";
    case ZSKIP: return "ZSKIP";
    case ZABORT: return "ZABORT";
    case ZFERR: return "ZFERR";
    case ZCRC: return "ZCRC";
    case ZCHALLENGE: return "ZCHALLENGE";
    case ZCOMPL: return "ZCOMPL";
    case ZCAN: return "ZCAN";
    case ZFREECNT: return "ZFREECNT";
    case ZCOMMAND: return "ZCOMMAND";
    case ZSTDERR: return "ZSTDERR";
    default: return "UNKNOWN";
    }

}

struct termios oldtty, tty;
void setTtyRawMode(int fd){
    tcgetattr(fd, &oldtty);
    tty = oldtty;

    tty.c_iflag = IGNBRK | IXOFF;

     /* No echo, crlf mapping, INTR, QUIT, delays, no erase/kill */
    tty.c_lflag &= ~(ECHO | ICANON | ISIG);
    tty.c_oflag = 0;    /* Transparent output */

    tty.c_cflag &= ~(PARENB);   /* Same baud rate, disable parity */
    /* Set character size = 8 */
    tty.c_cflag &= ~(CSIZE);
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 1; /* This many chars satisfies reads */
    tty.c_cc[VTIME] = 1;    /* or in this many tenths of seconds */
    tcsetattr(fd,TCSADRAIN,&tty);
}

void resetTty(int fd){
    tcdrain(fd); /* wait until everything is sent */
    tcflush(fd,TCIOFLUSH); /* flush input queue */
    tcsetattr(fd,TCSADRAIN,&oldtty);
    tcflow(fd,TCOON); /* restart output */
}

unsigned char zsendline_tab[256];
void initZmodemTab() {
	int i;
	for (i=0;i<256;i++) {	
		if (i & 0140){
			zsendline_tab[i]=0;
		}else {
			switch(i)
			{
			case ZDLE:
			case XOFF: /* ^Q */
			case XON: /* ^S */
			case (XOFF | 0200):
			case (XON | 0200):
				zsendline_tab[i]=1;
				break;
			case 020: /* ^P */
			case 0220:
				zsendline_tab[i]=1;
				break;
			case 015:
			case 0215:
				zsendline_tab[i]=0;
				break;
			default:
				zsendline_tab[i]=0;
			}
		}
	}
}

