#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include "build.h"
#include "names.h"
#include "pragmas.h"

#define TICSPERFRAME 3
#define MOVEFIFOSIZ 256

/***************************************************************************
	KEN'S TAG DEFINITIONS:      (Please define your own tags for your games)

 sector[?].lotag = 0   Normal sector
 sector[?].lotag = 1   If you are on a sector with this tag, then all sectors
								  with same hi tag as this are operated.  Once.
 sector[?].lotag = 2   Same as sector[?].tag = 1 but this is retriggable.
 sector[?].lotag = 3   A really stupid sector that really does nothing now.
 sector[?].lotag = 4   A sector where you are put closer to the floor
								  (such as the slime in DOOM1.DAT)
 sector[?].lotag = 5   A really stupid sector that really does nothing now.
 sector[?].lotag = 6   A normal door - instead of pressing D, you tag the
								  sector with a 6.  The reason I make you edit doors
								  this way is so that can program the doors
								  yourself.
 sector[?].lotag = 7   A door the goes down to open.
 sector[?].lotag = 8   A door that opens horizontally in the middle.
 sector[?].lotag = 9   A sliding door that opens vertically in the middle.
								  -Example of the advantages of not using BSP tree.
 sector[?].lotag = 10  A warping sector with floor and walls that shade.
 sector[?].lotag = 11  A sector with all walls that do X-panning.
 sector[?].lotag = 12  A sector with walls using the dragging function.
 sector[?].lotag = 13  A sector with some swinging doors in it.
 sector[?].lotag = 14  A revolving door sector.
 sector[?].lotag = 15  A subway track.
 sector[?].lotag = 16  A true double-sliding door.

	wall[?].lotag = 0   Normal wall
	wall[?].lotag = 1   Y-panning wall
	wall[?].lotag = 2   Switch - If you flip it, then all sectors with same hi
								  tag as this are operated.
	wall[?].lotag = 3   Marked wall to detemine starting dir. (sector tag 12)
	wall[?].lotag = 4   Mark on the shorter wall closest to the pivot point
								  of a swinging door. (sector tag 13)
	wall[?].lotag = 5   Mark where a subway should stop. (sector tag 15)
	wall[?].lotag = 6   Mark for true double-sliding doors (sector tag 16)
	wall[?].lotag = 7   Water fountain
	wall[?].lotag = 8   Bouncy wall!

 sprite[?].lotag = 0   Normal sprite
 sprite[?].lotag = 1   If you press space bar on an AL, and the AL is tagged
								  with a 1, he will turn evil.
 sprite[?].lotag = 2   When this sprite is operated, a bomb is shot at its
								  position.
 sprite[?].lotag = 3   Rotating sprite.
 sprite[?].lotag = 4   Sprite switch.
 sprite[?].lotag = 5   Basketball hoop score.

KEN'S STATUS DEFINITIONS:  (Please define your own statuses for your games)
 status = 0            Inactive sprite
 status = 1            Active monster sprite
 status = 2            Monster that becomes active only when it sees you
 status = 3            Smoke on the wall for chainguns
 status = 4            Splashing sprites (When you shoot slime)
 status = 5            Explosion!
 status = 6            Travelling bullet
 status = 7            Bomb sprial-out explosion
 status = 8            Player!
 status = 9            EVILALGRAVE shrinking list
 status = 10           EVILAL list
 status = 11           Sprite respawning list
 status = MAXSTATUS    Non-existent sprite (this will be true for your
														  code also)
**************************************************************************/

typedef struct
{
	long x, y, z;
} point3d;

typedef struct
{
	signed char fvel, svel, avel;
	short bits;
} input;

static long screentilt = 0;

void (__interrupt __far *oldtimerhandler)();
void __interrupt __far timerhandler(void);

#define KEYFIFOSIZ 64
void (__interrupt __far *oldkeyhandler)();
void __interrupt __far keyhandler(void);
volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
volatile char readch, oldreadch, extended, keytemp;

static long fvel, svel, avel;
static long fvel2, svel2, avel2;

extern volatile long recsnddone, recsndoffs;
static long recording = -2;

#define NUMOPTIONS 8
#define NUMKEYS 19
static long chainxres[4] = {256,320,360,400};
static long chainyres[11] = {200,240,256,270,300,350,360,400,480,512,540};
static long vesares[13][2] = {320,200,360,200,320,240,360,240,320,400,
									360,400,640,350,640,400,640,480,800,600,
									1024,768,1280,1024,1600,1200};
static char option[NUMOPTIONS] = {0,0,0,0,0,0,1,0};
static char keys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};

static long digihz[7] = {6000,8000,11025,16000,22050,32000,44100};

static char frame2draw[MAXPLAYERS];
static long frameskipcnt[MAXPLAYERS];

static char gundmost[320];

#define LAVASIZ 128
#define LAVALOGSIZ 7
#define LAVAMAXDROPS 32
static char lavabakpic[(LAVASIZ+4)*(LAVASIZ+4)], lavainc[LAVASIZ];
static long lavanumdrops, lavanumframes;
static long lavadropx[LAVAMAXDROPS], lavadropy[LAVAMAXDROPS];
static long lavadropsiz[LAVAMAXDROPS], lavadropsizlookup[LAVAMAXDROPS];
static long lavaradx[32][128], lavarady[32][128], lavaradcnt[32];

	//Shared player variables
static long posx[MAXPLAYERS], posy[MAXPLAYERS], posz[MAXPLAYERS];
static long horiz[MAXPLAYERS], zoom[MAXPLAYERS], hvel[MAXPLAYERS];
static short ang[MAXPLAYERS], cursectnum[MAXPLAYERS], ocursectnum[MAXPLAYERS];
static short playersprite[MAXPLAYERS], deaths[MAXPLAYERS];
static long lastchaingun[MAXPLAYERS];
static long health[MAXPLAYERS], flytime[MAXPLAYERS];
static short numbombs[MAXPLAYERS], oflags[MAXPLAYERS];
static char dimensionmode[MAXPLAYERS];
static char revolvedoorstat[MAXPLAYERS];
static short revolvedoorang[MAXPLAYERS], revolvedoorrotang[MAXPLAYERS];
static long revolvedoorx[MAXPLAYERS], revolvedoory[MAXPLAYERS];

	//ENGINE CONTROLLED MULTIPLAYER VARIABLES:
extern short numplayers, myconnectindex;
extern short connecthead, connectpoint2[MAXPLAYERS];   //Player linked list variables (indeces, not connection numbers)

static long nummoves;
#define NUMSTATS 11
static signed char statrate[NUMSTATS] = {-1,0,-1,0,0,0,1,3,0,3,15};

	//Input structures
static long locselectedgun, locselectedgun2;
static input loc, oloc, loc2;
static input fsync[MAXPLAYERS], osync[MAXPLAYERS], sync[MAXPLAYERS];
	//Input faketimerhandler -> movethings fifo
static long movefifoplc, movefifoend;
static input baksync[MOVEFIFOSIZ][MAXPLAYERS];
	//Game recording variables
static long reccnt, recstat = 1;
static input recsync[16384][2];

	//MULTI.OBJ sync state variables
extern char syncstate;
	//GAME.C sync state variables
static short syncstat = 0;
static long syncvalhead = 0, othersyncvalhead = 0, syncvaltail = 0;
static short syncval[MOVEFIFOSIZ], othersyncval[MOVEFIFOSIZ];

extern long crctable[256];
#define updatecrc16(crc,dat) (crc = (crc<<8)^crctable[(crc>>8)^(dat&255)])

static char detailmode = 0, ready2send = 0;
static long ototalclock = 0, gotlastpacketclock = 0, smoothratio;
static long oposx[MAXPLAYERS], oposy[MAXPLAYERS], oposz[MAXPLAYERS];
static long ohoriz[MAXPLAYERS], ozoom[MAXPLAYERS];
static short oang[MAXPLAYERS];

static point3d osprite[MAXSPRITES];

#define MAXINTERPOLATIONS 1024
static long numinterpolations = 0, startofdynamicinterpolations = 0;
static long oldipos[MAXINTERPOLATIONS];
static long bakipos[MAXINTERPOLATIONS];
static long *curipos[MAXINTERPOLATIONS];

extern long cachecount, transarea;

static char playerreadyflag[MAXPLAYERS];

	//Miscellaneous variables
static char packbuf[MAXXDIM];
static char tempbuf[MAXXDIM], boardfilename[80];
static short tempshort[MAXSECTORS];
static short screenpeek = 0, oldmousebstatus = 0, brightness = 0;
static short screensize, screensizeflag = 0;
static short neartagsector, neartagwall, neartagsprite;
static long lockclock, neartagdist, neartaghitdist;
extern char chainstat;
extern long frameplace, pageoffset, ydim16;
static long globhiz, globloz, globhihit, globlohit;
extern long stereofps, stereowidth, stereopixelwidth;

	//Board animation variables
#define MAXMIRRORS 64
static short mirrorwall[MAXMIRRORS], mirrorsector[MAXMIRRORS], mirrorcnt;
static short floormirrorsector[64], floormirrorcnt;
static short turnspritelist[16], turnspritecnt;
static short warpsectorlist[64], warpsectorcnt;
static short xpanningsectorlist[16], xpanningsectorcnt;
static short ypanningwalllist[64], ypanningwallcnt;
static short floorpanninglist[64], floorpanningcnt;
static short dragsectorlist[16], dragxdir[16], dragydir[16], dragsectorcnt;
static long dragx1[16], dragy1[16], dragx2[16], dragy2[16], dragfloorz[16];
static short swingcnt, swingwall[32][5], swingsector[32];
static short swingangopen[32], swingangclosed[32], swingangopendir[32];
static short swingang[32], swinganginc[32];
static long swingx[32][8], swingy[32][8];
static short revolvesector[4], revolveang[4], revolvecnt;
static long revolvex[4][16], revolvey[4][16];
static long revolvepivotx[4], revolvepivoty[4];
static short subwaytracksector[4][128], subwaynumsectors[4], subwaytrackcnt;
static long subwaystop[4][8], subwaystopcnt[4];
static long subwaytrackx1[4], subwaytracky1[4];
static long subwaytrackx2[4], subwaytracky2[4];
static long subwayx[4], subwaygoalstop[4], subwayvel[4], subwaypausetime[4];
static short waterfountainwall[MAXPLAYERS], waterfountaincnt[MAXPLAYERS];
static short slimesoundcnt[MAXPLAYERS];

	//Variables that let you type messages to other player
static char getmessage[162], getmessageleng;
static long getmessagetimeoff;
static char typemessage[162], typemessageleng = 0, typemode = 0;
static char scantoasc[128] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
static char scantoascwithshift[128] =
{
	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

	//These variables are for animating x, y, or z-coordinates of sectors,
	//walls, or sprites (They are NOT to be used for changing the [].picnum's)
	//See the setanimation(), and getanimategoal() functions for more details.
#define MAXANIMATES 512
static long *animateptr[MAXANIMATES], animategoal[MAXANIMATES];
static long animatevel[MAXANIMATES], animateacc[MAXANIMATES], animatecnt = 0;

	//These parameters are in exact order of sprite structure in BUILD.H
#define spawnsprite(newspriteindex2,x2,y2,z2,cstat2,shade2,pal2,       \
		clipdist2,xrepeat2,yrepeat2,xoffset2,yoffset2,picnum2,ang2,      \
		xvel2,yvel2,zvel2,owner2,sectnum2,statnum2,lotag2,hitag2,extra2) \
{                                                                      \
	spritetype *spr2;                                                   \
	newspriteindex2 = insertsprite(sectnum2,statnum2);                  \
	spr2 = &sprite[newspriteindex2];                                    \
	spr2->x = x2; spr2->y = y2; spr2->z = z2;                           \
	spr2->cstat = cstat2; spr2->shade = shade2;                         \
	spr2->pal = pal2; spr2->clipdist = clipdist2;                       \
	spr2->xrepeat = xrepeat2; spr2->yrepeat = yrepeat2;                 \
	spr2->xoffset = xoffset2; spr2->yoffset = yoffset2;                 \
	spr2->picnum = picnum2; spr2->ang = ang2;                           \
	spr2->xvel = xvel2; spr2->yvel = yvel2; spr2->zvel = zvel2;         \
	spr2->owner = owner2;                                               \
	spr2->lotag = lotag2; spr2->hitag = hitag2; spr2->extra = extra2;   \
	copybuf(&spr2->x,&osprite[newspriteindex2].x,3);                    \
	show2dsprite[newspriteindex2>>3] &= ~(1<<(newspriteindex2&7));      \
	if (show2dsector[sectnum2>>3]&(1<<(sectnum2&7)))                    \
		show2dsprite[newspriteindex2>>3] |= (1<<(newspriteindex2&7));    \
}                                                                      \

main(short int argc,char **argv)
{
	long i, j, k, l, fil, waitplayers, x1, y1, x2, y2;
	short other, packleng;
	char *ptr;

	initgroupfile("stuff.dat");

	if ((argc >= 2) && (stricmp("-net",argv[1]) != 0) && (stricmp("/net",argv[1]) != 0))
	{
		strcpy(&boardfilename,argv[1]);
		if(strchr(boardfilename,'.') == 0)
			strcat(boardfilename,".map");
	}
	else
		strcpy(&boardfilename,"nukeland.map");

	if ((fil = open("setup.dat",O_BINARY|O_RDWR,S_IREAD)) != -1)
	{
		read(fil,&option[0],NUMOPTIONS);
		read(fil,&keys[0],NUMKEYS);
		close(fil);
	}
	if (option[3] != 0) initmouse();

	switch(option[0])
	{
		case 0: initengine(0,chainxres[option[6]&15],chainyres[option[6]>>4]); break;
		case 1: initengine(1,vesares[option[6]&15][0],vesares[option[6]&15][1]); break;
		default: initengine(option[0],320L,200L); break;
	}
	initkeys();
	inittimer();
	initmultiplayers(option[4],option[5],0);

	pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;

	initsb(option[1],option[2],digihz[option[7]>>4],((option[7]&4)>0)+1,((option[7]&2)>0)+1,60,option[7]&1);
	if (strcmp(boardfilename,"klab.map") == 0)
		loadsong("klabsong.kdm");
	else
		loadsong("neatsong.kdm");
	musicon();

	loadpics("tiles000.art");                      //Load artwork

		//Here's an example of TRUE ornamented walls
		//The allocatepermanenttile should be called right after loadpics
		//Since it resets the tile cache for each call.
	if (allocatepermanenttile(SLIME,128,128) == 0)    //If enough memory
	{
		printf("Not enough memory for slime!\n");
		exit(0);
	}
	if (allocatepermanenttile(4095,64,64) != 0)    //If enough memory
	{
			//My face with an explosion written over it
		copytilepiece(KENPICTURE,0,0,64,64,4095,0,0);
		copytilepiece(EXPLOSION,0,0,64,64,4095,0,0);
	}

	initlava();

		//Get dmost table for canon
	if (waloff[GUNONBOTTOM] == 0) loadtile(GUNONBOTTOM);
	x1 = tilesizx[GUNONBOTTOM]; y1 = tilesizy[GUNONBOTTOM];
	ptr = (char *)(waloff[GUNONBOTTOM]);
	for(i=0;i<x1;i++)
	{
		y2 = y1-1; while ((ptr[y2] != 255) && (y2 >= 0)) y2--;
		gundmost[i] = y2+1;
		ptr += y1;
	}

	for(j=0;j<256;j++)
		tempbuf[j] = ((j+32)&255);  //remap colors for screwy palette sectors
	makepalookup(16,tempbuf,0,0,0,1);

	for(j=0;j<256;j++) tempbuf[j] = j;
	makepalookup(17,tempbuf,24,24,24,1);

	for(j=0;j<256;j++) tempbuf[j] = j; //(j&31)+32;
	makepalookup(18,tempbuf,8,8,48,1);

	prepareboard(boardfilename);                   //Load board

	if (option[4] > 0)
	{
		x1 = ((xdim-screensize)>>1);
		x2 = x1+screensize-1;
		y1 = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
		y2 = y1 + scale(screensize,ydim-32,xdim)-1;
		drawtilebackground(0L,0L,BACKGROUND,8,x1,y1,x2,y2,0);

		sendlogon();
		if (option[4] < 5) waitplayers = 2; else waitplayers = option[4]-3;
		while (numplayers < waitplayers)
		{
			sprintf(tempbuf,"%ld of %ld players in...",numplayers,waitplayers);
			printext256(68L,84L,31,0,tempbuf,0);
			nextpage();

			if (getpacket(&other,packbuf) > 0)
				if (packbuf[0] == 255)
					keystatus[1] = 1;

			if (keystatus[1] > 0)
			{
				sendlogoff();         //Signing off
				musicoff();
				uninitmultiplayers();
				uninittimer();
				uninitkeys();
				uninitengine();
				uninitsb();
				uninitgroupfile();
				setvmode(0x3);        //Set back to text mode
				exit(0);
			}
		}
		screenpeek = myconnectindex;

		j = 1;
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			if (myconnectindex == i) break;
			j++;
		}
		if (j == 1)
			strcpy(getmessage,"Player 1 (Master)");
		else
			sprintf(getmessage,"Player %ld (Slave)",j);
		getmessageleng = strlen(getmessage);
		getmessagetimeoff = totalclock+120;
	}

	reccnt = 0;
	for(i=connecthead;i>=0;i=connectpoint2[i]) initplayersprite((short)i);

	//waitforeverybody();
	resettiming(); ototalclock = 0; gotlastpacketclock = 0; nummoves = 0;

	ready2send = 1;
	while (keystatus[1] == 0)       //Main loop starts here
	{
			// backslash (useful only with KDM)
		if (keystatus[0x2b] > 0) { keystatus[0x2b] = 0; preparesndbuf(); }

		getpackets();
		while (movefifoplc != movefifoend) domovethings();
		drawscreen(screenpeek,(totalclock-gotlastpacketclock)*(65536/TICSPERFRAME));

	}
	ready2send = 0;

	sendlogoff();         //Signing off
	musicoff();
	uninitmultiplayers();
	uninittimer();
	uninitkeys();
	uninitengine();
	uninitsb();
	uninitgroupfile();
	setvmode(0x3);        //Set back to text mode
	showengineinfo();     //Show speed statistics

	return(0);
}

operatesector(short dasector)
{     //Door code
	long i, j, k, s, nexti, good, cnt, datag;
	long dax, day, daz, dax2, day2, daz2, centx, centy;
	short startwall, endwall, wallfind[2];

	datag = sector[dasector].lotag;

	startwall = sector[dasector].wallptr;
	endwall = startwall + sector[dasector].wallnum;
	centx = 0L, centy = 0L;
	for(i=startwall;i<endwall;i++)
	{
		centx += wall[i].x;
		centy += wall[i].y;
	}
	centx /= (endwall-startwall);
	centy /= (endwall-startwall);

		//Simple door that moves up  (tag 8 is a combination of tags 6 & 7)
	if ((datag == 6) || (datag == 8))    //If the sector in front is a door
	{
		i = getanimationgoal((long)&sector[dasector].ceilingz);
		if (i >= 0)      //If door already moving, reverse its direction
		{
			if (datag == 8)
				daz = ((sector[dasector].ceilingz+sector[dasector].floorz)>>1);
			else
				daz = sector[dasector].floorz;

			if (animategoal[i] == daz)
				animategoal[i] = sector[nextsectorneighborz(dasector,sector[dasector].floorz,-1,-1)].ceilingz;
			else
				animategoal[i] = daz;
			animatevel[i] = 0;
		}
		else      //else insert the door's ceiling on the animation list
		{
			if (sector[dasector].ceilingz == sector[dasector].floorz)
				daz = sector[nextsectorneighborz(dasector,sector[dasector].floorz,-1,-1)].ceilingz;
			else
			{
				if (datag == 8)
					daz = ((sector[dasector].ceilingz+sector[dasector].floorz)>>1);
				else
					daz = sector[dasector].floorz;
			}
			if ((j = setanimation(&sector[dasector].ceilingz,daz,6L,6L)) >= 0)
				wsayfollow("updowndr.wav",4096L+(krand()&255)-128,256L,&centx,&centy,0);
		}
	}
		//Simple door that moves down
	if ((datag == 7) || (datag == 8)) //If the sector in front's elevator
	{
		i = getanimationgoal((long)&sector[dasector].floorz);
		if (i >= 0)      //If elevator already moving, reverse its direction
		{
			if (datag == 8)
				daz = ((sector[dasector].ceilingz+sector[dasector].floorz)>>1);
			else
				daz = sector[dasector].ceilingz;

			if (animategoal[i] == daz)
				animategoal[i] = sector[nextsectorneighborz(dasector,sector[dasector].ceilingz,1,1)].floorz;
			else
				animategoal[i] = daz;
			animatevel[i] = 0;
		}
		else      //else insert the elevator's ceiling on the animation list
		{
			if (sector[dasector].floorz == sector[dasector].ceilingz)
				daz = sector[nextsectorneighborz(dasector,sector[dasector].ceilingz,1,1)].floorz;
			else
			{
				if (datag == 8)
					daz = ((sector[dasector].ceilingz+sector[dasector].floorz)>>1);
				else
					daz = sector[dasector].ceilingz;
			}
			if ((j = setanimation(&sector[dasector].floorz,daz,6L,6L)) >= 0)
				wsayfollow("updowndr.wav",4096L+(krand()&255)-128,256L,&centx,&centy,0);
		}
	}

	if (datag == 9)   //Smooshy-wall sideways double-door
	{
		//find any points with either same x or same y coordinate
		//  as center (centx, centy) - should be 2 points found.
		wallfind[0] = -1;
		wallfind[1] = -1;
		for(i=startwall;i<endwall;i++)
			if ((wall[i].x == centx) || (wall[i].y == centy))
			{
				if (wallfind[0] == -1)
					wallfind[0] = i;
				else
					wallfind[1] = i;
			}

		for(j=0;j<2;j++)
		{
			if ((wall[wallfind[j]].x == centx) && (wall[wallfind[j]].y == centy))
			{
				//find what direction door should open by averaging the
				//  2 neighboring points of wallfind[0] & wallfind[1].
				i = wallfind[j]-1; if (i < startwall) i = endwall-1;
				dax2 = ((wall[i].x+wall[wall[wallfind[j]].point2].x)>>1)-wall[wallfind[j]].x;
				day2 = ((wall[i].y+wall[wall[wallfind[j]].point2].y)>>1)-wall[wallfind[j]].y;
				if (dax2 != 0)
				{
					dax2 = wall[wall[wall[wallfind[j]].point2].point2].x;
					dax2 -= wall[wall[wallfind[j]].point2].x;
					setanimation(&wall[wallfind[j]].x,wall[wallfind[j]].x+dax2,4L,0L);
					setanimation(&wall[i].x,wall[i].x+dax2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].x,wall[wall[wallfind[j]].point2].x+dax2,4L,0L);
				}
				else if (day2 != 0)
				{
					day2 = wall[wall[wall[wallfind[j]].point2].point2].y;
					day2 -= wall[wall[wallfind[j]].point2].y;
					setanimation(&wall[wallfind[j]].y,wall[wallfind[j]].y+day2,4L,0L);
					setanimation(&wall[i].y,wall[i].y+day2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].y,wall[wall[wallfind[j]].point2].y+day2,4L,0L);
				}
			}
			else
			{
				i = wallfind[j]-1; if (i < startwall) i = endwall-1;
				dax2 = ((wall[i].x+wall[wall[wallfind[j]].point2].x)>>1)-wall[wallfind[j]].x;
				day2 = ((wall[i].y+wall[wall[wallfind[j]].point2].y)>>1)-wall[wallfind[j]].y;
				if (dax2 != 0)
				{
					setanimation(&wall[wallfind[j]].x,centx,4L,0L);
					setanimation(&wall[i].x,centx+dax2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].x,centx+dax2,4L,0L);
				}
				else if (day2 != 0)
				{
					setanimation(&wall[wallfind[j]].y,centy,4L,0L);
					setanimation(&wall[i].y,centy+day2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].y,centy+day2,4L,0L);
				}
			}
		}
		wsayfollow("updowndr.wav",4096L-256L,256L,&centx,&centy,0);
		wsayfollow("updowndr.wav",4096L+256L,256L,&centx,&centy,0);
	}

	if (datag == 13)  //Swinging door
	{
		for(i=0;i<swingcnt;i++)
		{
			if (swingsector[i] == dasector)
			{
				if (swinganginc[i] == 0)
				{
					if (swingang[i] == swingangclosed[i])
					{
						swinganginc[i] = swingangopendir[i];
						wsayfollow("opendoor.wav",4096L+(krand()&511)-256,256L,&centx,&centy,0);
					}
					else
						swinganginc[i] = -swingangopendir[i];
				}
				else
					swinganginc[i] = -swinganginc[i];

				for(j=1;j<=3;j++)
				{
					setinterpolation(&wall[swingwall[i][j]].x);
					setinterpolation(&wall[swingwall[i][j]].y);
				}
			}
		}
	}

	if (datag == 16)  //True sideways double-sliding door
	{
		 //get 2 closest line segments to center (dax, day)
		wallfind[0] = -1;
		wallfind[1] = -1;
		for(i=startwall;i<endwall;i++)
			if (wall[i].lotag == 6)
			{
				if (wallfind[0] == -1)
					wallfind[0] = i;
				else
					wallfind[1] = i;
			}

		for(j=0;j<2;j++)
		{
			if ((((wall[wallfind[j]].x+wall[wall[wallfind[j]].point2].x)>>1) == centx) && (((wall[wallfind[j]].y+wall[wall[wallfind[j]].point2].y)>>1) == centy))
			{     //door was closed
					//find what direction door should open
				i = wallfind[j]-1; if (i < startwall) i = endwall-1;
				dax2 = wall[i].x-wall[wallfind[j]].x;
				day2 = wall[i].y-wall[wallfind[j]].y;
				if (dax2 != 0)
				{
					dax2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].x;
					dax2 -= wall[wall[wall[wallfind[j]].point2].point2].x;
					setanimation(&wall[wallfind[j]].x,wall[wallfind[j]].x+dax2,4L,0L);
					setanimation(&wall[i].x,wall[i].x+dax2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].x,wall[wall[wallfind[j]].point2].x+dax2,4L,0L);
					setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x,wall[wall[wall[wallfind[j]].point2].point2].x+dax2,4L,0L);
				}
				else if (day2 != 0)
				{
					day2 = wall[wall[wall[wall[wallfind[j]].point2].point2].point2].y;
					day2 -= wall[wall[wall[wallfind[j]].point2].point2].y;
					setanimation(&wall[wallfind[j]].y,wall[wallfind[j]].y+day2,4L,0L);
					setanimation(&wall[i].y,wall[i].y+day2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].y,wall[wall[wallfind[j]].point2].y+day2,4L,0L);
					setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y,wall[wall[wall[wallfind[j]].point2].point2].y+day2,4L,0L);
				}
			}
			else
			{    //door was not closed
				i = wallfind[j]-1; if (i < startwall) i = endwall-1;
				dax2 = wall[i].x-wall[wallfind[j]].x;
				day2 = wall[i].y-wall[wallfind[j]].y;
				if (dax2 != 0)
				{
					setanimation(&wall[wallfind[j]].x,centx,4L,0L);
					setanimation(&wall[i].x,centx+dax2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].x,centx,4L,0L);
					setanimation(&wall[wall[wall[wallfind[j]].point2].point2].x,centx+dax2,4L,0L);
				}
				else if (day2 != 0)
				{
					setanimation(&wall[wallfind[j]].y,centy,4L,0L);
					setanimation(&wall[i].y,centy+day2,4L,0L);
					setanimation(&wall[wall[wallfind[j]].point2].y,centy,4L,0L);
					setanimation(&wall[wall[wall[wallfind[j]].point2].point2].y,centy+day2,4L,0L);
				}
			}
		}
		wsayfollow("updowndr.wav",4096L-64L,256L,&centx,&centy,0);
		wsayfollow("updowndr.wav",4096L+64L,256L,&centx,&centy,0);
	}
}

operatesprite(short dasprite)
{
	long datag;

	datag = sprite[dasprite].lotag;

	if (datag == 2)    //A sprite that shoots a bomb
	{
		shootgun(dasprite,
					sprite[dasprite].x,sprite[dasprite].y,sprite[dasprite].z,
					sprite[dasprite].ang,100L,sprite[dasprite].sectnum,2);
	}
}

changehealth(short snum, short deltahealth)
{
	long dax, day;
	short good, k, startwall, endwall, s;

	if (health[snum] > 0)
	{
		health[snum] += deltahealth;
		if (health[snum] > 999) health[snum] = 999;

		if (health[snum] <= 0)
		{
			health[snum] = -1;
			wsayfollow("death.wav",4096L+(krand()&127)-64,256L,&posx[snum],&posy[snum],1);
			sprite[playersprite[snum]].picnum = SKELETON;
		}

		if ((snum == screenpeek) && (screensize <= xdim))
		{
			if (health[snum] > 0)
				sprintf(&tempbuf,"Health: %3d",health[snum]);
			else
				sprintf(&tempbuf,"YOU STINK!!");

			printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-24,tempbuf,ALPHABET,80);
		}
	}
	return(health[snum] <= 0);      //You were just injured
}

prepareboard(char *daboardfilename)
{
	short startwall, endwall, dasector;
	long i, j, k, s, dax, day, daz, dax2, day2;

	getmessageleng = 0;
	typemessageleng = 0;

	randomseed = 17L;

		//Clear (do)animation's list
	animatecnt = 0;
	typemode = 0;
	locselectedgun = 0;
	locselectedgun2 = 0;

	if (loadboard(daboardfilename,&posx[0],&posy[0],&posz[0],&ang[0],&cursectnum[0]) == -1)
	{
		musicoff();
		uninitmultiplayers();
		uninittimer();
		uninitkeys();
		uninitengine();
		uninitsb();
		uninitgroupfile();
		setvmode(0x3);        //Set back to text mode
		printf("Board not found\n");
		exit(0);
	}

	for(i=0;i<MAXPLAYERS;i++)
	{
		posx[i] = posx[0];
		posy[i] = posy[0];
		posz[i] = posz[0];
		ang[i] = ang[0];
		cursectnum[i] = cursectnum[0];
		ocursectnum[i] = cursectnum[0];
		horiz[i] = 100;
		lastchaingun[i] = 0;
		health[i] = 100;
		dimensionmode[i] = 3;
		numbombs[i] = 0;
		zoom[i] = 768L;
		deaths[i] = 0L;
		playersprite[i] = -1;
		screensize = xdim;
		flytime[i] = 0L;

		oposx[i] = posx[0];
		oposy[i] = posy[0];
		oposz[i] = posz[0];
		ohoriz[i] = horiz[0];
		ozoom[i] = zoom[0];
		oang[i] = ang[0];
	}

	movefifoplc = 0; movefifoend = 0;
	syncvalhead = 0L; othersyncvalhead = 0L; syncvaltail = 0L;
	numinterpolations = 0;

	setup3dscreen();

	oloc.fvel = oloc.svel = oloc.avel = oloc.bits = 0;
	for(i=0;i<MAXPLAYERS;i++)
	{
		fsync[i].fvel = sync[i].fvel = osync[i].fvel = 0;
		fsync[i].svel = sync[i].svel = osync[i].svel = 0;
		fsync[i].avel = sync[i].avel = osync[i].avel = 0;
		fsync[i].bits = sync[i].bits = osync[i].bits = 0;
	}

		//Scan sector tags

	for(i=0;i<MAXPLAYERS;i++)
	{
		waterfountainwall[i] = -1;
		waterfountaincnt[i] = 0;
	}
	slimesoundcnt[i] = 0;
	warpsectorcnt = 0;      //Make a list of warping sectors
	xpanningsectorcnt = 0;  //Make a list of wall x-panning sectors
	floorpanningcnt = 0;    //Make a list of slime sectors
	dragsectorcnt = 0;      //Make a list of moving platforms
	swingcnt = 0;           //Make a list of swinging doors
	revolvecnt = 0;         //Make a list of revolving doors
	subwaytrackcnt = 0;     //Make a list of subways

	floormirrorcnt = 0;
	tilesizx[FLOORMIRROR] = 0;
	tilesizy[FLOORMIRROR] = 0;

	for(i=0;i<numsectors;i++)
	{
		switch(sector[i].lotag)
		{
			case 4:
				floorpanninglist[floorpanningcnt++] = i;
				break;
			case 10:
				warpsectorlist[warpsectorcnt++] = i;
				break;
			case 11:
				xpanningsectorlist[xpanningsectorcnt++] = i;
				break;
			case 12:
				dasector = i;
				dax = 0x7fffffff;
				day = 0x7fffffff;
				dax2 = 0x80000000;
				day2 = 0x80000000;
				startwall = sector[i].wallptr;
				endwall = startwall+sector[i].wallnum;
				for(j=startwall;j<endwall;j++)
				{
					if (wall[j].x < dax) dax = wall[j].x;
					if (wall[j].y < day) day = wall[j].y;
					if (wall[j].x > dax2) dax2 = wall[j].x;
					if (wall[j].y > day2) day2 = wall[j].y;
					if (wall[j].lotag == 3) k = j;
				}
				if (wall[k].x == dax) dragxdir[dragsectorcnt] = -16;
				if (wall[k].y == day) dragydir[dragsectorcnt] = -16;
				if (wall[k].x == dax2) dragxdir[dragsectorcnt] = 16;
				if (wall[k].y == day2) dragydir[dragsectorcnt] = 16;

				dasector = wall[startwall].nextsector;
				dragx1[dragsectorcnt] = 0x7fffffff;
				dragy1[dragsectorcnt] = 0x7fffffff;
				dragx2[dragsectorcnt] = 0x80000000;
				dragy2[dragsectorcnt] = 0x80000000;
				startwall = sector[dasector].wallptr;
				endwall = startwall+sector[dasector].wallnum;
				for(j=startwall;j<endwall;j++)
				{
					if (wall[j].x < dragx1[dragsectorcnt]) dragx1[dragsectorcnt] = wall[j].x;
					if (wall[j].y < dragy1[dragsectorcnt]) dragy1[dragsectorcnt] = wall[j].y;
					if (wall[j].x > dragx2[dragsectorcnt]) dragx2[dragsectorcnt] = wall[j].x;
					if (wall[j].y > dragy2[dragsectorcnt]) dragy2[dragsectorcnt] = wall[j].y;

					setinterpolation(&sector[dasector].floorz);
					setinterpolation(&wall[j].x);
					setinterpolation(&wall[j].y);
					setinterpolation(&wall[wall[j].nextwall].x);
					setinterpolation(&wall[wall[j].nextwall].y);
				}

				dragx1[dragsectorcnt] += (wall[sector[i].wallptr].x-dax);
				dragy1[dragsectorcnt] += (wall[sector[i].wallptr].y-day);
				dragx2[dragsectorcnt] -= (dax2-wall[sector[i].wallptr].x);
				dragy2[dragsectorcnt] -= (day2-wall[sector[i].wallptr].y);

				dragfloorz[dragsectorcnt] = sector[i].floorz;

				dragsectorlist[dragsectorcnt++] = i;
				break;
			case 13:
				startwall = sector[i].wallptr;
				endwall = startwall+sector[i].wallnum;
				for(j=startwall;j<endwall;j++)
				{
					if (wall[j].lotag == 4)
					{
						k = wall[wall[wall[wall[j].point2].point2].point2].point2;
						if ((wall[j].x == wall[k].x) && (wall[j].y == wall[k].y))
						{     //Door opens counterclockwise
							swingwall[swingcnt][0] = j;
							swingwall[swingcnt][1] = wall[j].point2;
							swingwall[swingcnt][2] = wall[wall[j].point2].point2;
							swingwall[swingcnt][3] = wall[wall[wall[j].point2].point2].point2;
							swingangopen[swingcnt] = 1536;
							swingangclosed[swingcnt] = 0;
							swingangopendir[swingcnt] = -1;
						}
						else
						{     //Door opens clockwise
							swingwall[swingcnt][0] = wall[j].point2;
							swingwall[swingcnt][1] = j;
							swingwall[swingcnt][2] = lastwall(j);
							swingwall[swingcnt][3] = lastwall(swingwall[swingcnt][2]);
							swingwall[swingcnt][4] = lastwall(swingwall[swingcnt][3]);
							swingangopen[swingcnt] = 512;
							swingangclosed[swingcnt] = 0;
							swingangopendir[swingcnt] = 1;
						}
						for(k=0;k<4;k++)
						{
							swingx[swingcnt][k] = wall[swingwall[swingcnt][k]].x;
							swingy[swingcnt][k] = wall[swingwall[swingcnt][k]].y;
						}

						swingsector[swingcnt] = i;
						swingang[swingcnt] = swingangclosed[swingcnt];
						swinganginc[swingcnt] = 0;
						swingcnt++;
					}
				}
				break;
			case 14:
				startwall = sector[i].wallptr;
				endwall = startwall+sector[i].wallnum;
				dax = 0L;
				day = 0L;
				for(j=startwall;j<endwall;j++)
				{
					dax += wall[j].x;
					day += wall[j].y;
				}
				revolvepivotx[revolvecnt] = dax / (endwall-startwall);
				revolvepivoty[revolvecnt] = day / (endwall-startwall);

				k = 0;
				for(j=startwall;j<endwall;j++)
				{
					revolvex[revolvecnt][k] = wall[j].x;
					revolvey[revolvecnt][k] = wall[j].y;

					setinterpolation(&wall[j].x);
					setinterpolation(&wall[j].y);
					setinterpolation(&wall[wall[j].nextwall].x);
					setinterpolation(&wall[wall[j].nextwall].y);

					k++;
				}
				revolvesector[revolvecnt] = i;
				revolveang[revolvecnt] = 0;

				revolvecnt++;
				break;
			case 15:
				subwaytracksector[subwaytrackcnt][0] = i;

				subwaystopcnt[subwaytrackcnt] = 0;
				dax = 0x7fffffff;
				day = 0x7fffffff;
				dax2 = 0x80000000;
				day2 = 0x80000000;
				startwall = sector[i].wallptr;
				endwall = startwall+sector[i].wallnum;
				for(j=startwall;j<endwall;j++)
				{
					if (wall[j].x < dax) dax = wall[j].x;
					if (wall[j].y < day) day = wall[j].y;
					if (wall[j].x > dax2) dax2 = wall[j].x;
					if (wall[j].y > day2) day2 = wall[j].y;
				}
				for(j=startwall;j<endwall;j++)
				{
					if (wall[j].lotag == 5)
					{
						if ((wall[j].x > dax) && (wall[j].y > day) && (wall[j].x < dax2) && (wall[j].y < day2))
						{
							subwayx[subwaytrackcnt] = wall[j].x;
						}
						else
						{
							subwaystop[subwaytrackcnt][subwaystopcnt[subwaytrackcnt]] = wall[j].x;
							subwaystopcnt[subwaytrackcnt]++;
						}
					}
				}

				for(j=1;j<subwaystopcnt[subwaytrackcnt];j++)
					for(k=0;k<j;k++)
						if (subwaystop[subwaytrackcnt][j] < subwaystop[subwaytrackcnt][k])
						{
							s = subwaystop[subwaytrackcnt][j];
							subwaystop[subwaytrackcnt][j] = subwaystop[subwaytrackcnt][k];
							subwaystop[subwaytrackcnt][k] = s;
						}

				subwaygoalstop[subwaytrackcnt] = 0;
				for(j=0;j<subwaystopcnt[subwaytrackcnt];j++)
					if (klabs(subwaystop[subwaytrackcnt][j]-subwayx[subwaytrackcnt]) < klabs(subwaystop[subwaytrackcnt][subwaygoalstop[subwaytrackcnt]]-subwayx[subwaytrackcnt]))
						subwaygoalstop[subwaytrackcnt] = j;

				subwaytrackx1[subwaytrackcnt] = dax;
				subwaytracky1[subwaytrackcnt] = day;
				subwaytrackx2[subwaytrackcnt] = dax2;
				subwaytracky2[subwaytrackcnt] = day2;

				subwaynumsectors[subwaytrackcnt] = 1;
				for(j=0;j<numsectors;j++)
					if (j != i)
					{
						startwall = sector[j].wallptr;
						if (wall[startwall].x > subwaytrackx1[subwaytrackcnt])
							if (wall[startwall].y > subwaytracky1[subwaytrackcnt])
								if (wall[startwall].x < subwaytrackx2[subwaytrackcnt])
									if (wall[startwall].y < subwaytracky2[subwaytrackcnt])
									{
										if (sector[j].floorz != sector[i].floorz)
										{
											sector[j].ceilingstat |= 64;
											sector[j].floorstat |= 64;
										}
										subwaytracksector[subwaytrackcnt][subwaynumsectors[subwaytrackcnt]] = j;
										subwaynumsectors[subwaytrackcnt]++;
									}
					}

				subwayvel[subwaytrackcnt] = 64;
				subwaypausetime[subwaytrackcnt] = 720;

				startwall = sector[i].wallptr;
				endwall = startwall+sector[i].wallnum;
				for(k=startwall;k<endwall;k++)
					if (wall[k].x > subwaytrackx1[subwaytrackcnt])
						if (wall[k].y > subwaytracky1[subwaytrackcnt])
							if (wall[k].x < subwaytrackx2[subwaytrackcnt])
								if (wall[k].y < subwaytracky2[subwaytrackcnt])
									setinterpolation(&wall[k].x);

				for(j=1;j<subwaynumsectors[subwaytrackcnt];j++)
				{
					dasector = subwaytracksector[subwaytrackcnt][j];

					startwall = sector[dasector].wallptr;
					endwall = startwall+sector[dasector].wallnum;
					for(k=startwall;k<endwall;k++)
						setinterpolation(&wall[k].x);

					for(k=headspritesect[dasector];k>=0;k=nextspritesect[k])
						if (statrate[sprite[k].statnum] < 0)
							setinterpolation(&sprite[k].x);
				}


				subwaytrackcnt++;
				break;
		}
		if (sector[i].floorpicnum == FLOORMIRROR)
			floormirrorsector[mirrorcnt++] = i;
	}

		//Scan wall tags

	mirrorcnt = 0;
	tilesizx[MIRROR] = 0;
	tilesizy[MIRROR] = 0;
	for(i=0;i<MAXMIRRORS;i++)
	{
		tilesizx[i+MIRRORLABEL] = 0;
		tilesizy[i+MIRRORLABEL] = 0;
	}

	ypanningwallcnt = 0;
	for(i=0;i<numwalls;i++)
	{
		if (wall[i].lotag == 1) ypanningwalllist[ypanningwallcnt++] = i;
		s = wall[i].nextsector;
		if ((s >= 0) && (wall[i].overpicnum == MIRROR) && (wall[i].cstat&32))
		{
			if ((sector[s].floorstat&1) == 0)
			{
				wall[i].overpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].ceilingpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].floorpicnum = MIRRORLABEL+mirrorcnt;
				sector[s].floorstat |= 1;
				mirrorwall[mirrorcnt] = i;
				mirrorsector[mirrorcnt] = s;
				mirrorcnt++;
			}
			else
				wall[i].overpicnum = sector[s].ceilingpicnum;
		}
	}

		//Invalidate textures in sector behind mirror
	for(i=0;i<mirrorcnt;i++)
	{
		k = mirrorsector[i];
		startwall = sector[k].wallptr;
		endwall = startwall + sector[k].wallnum;
		for(j=startwall;j<endwall;j++)
		{
			wall[j].picnum = MIRROR;
			wall[j].overpicnum = MIRROR;
		}
	}

		//Scan sprite tags&picnum's

	turnspritecnt = 0;
	for(i=0;i<MAXSPRITES;i++)
	{
		if (sprite[i].lotag == 3) turnspritelist[turnspritecnt++] = i;

		if (sprite[i].statnum < MAXSTATUS)    //That is, if sprite exists
			switch(sprite[i].picnum)
			{
				case BROWNMONSTER:              //All cases here put the sprite
					if ((sprite[i].cstat&128) == 0)
					{
						sprite[i].z -= ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						sprite[i].cstat |= 128;
					}
					sprite[i].extra = 0;
					sprite[i].clipdist = mulscale7(sprite[i].xrepeat,tilesizx[sprite[i].picnum]);
					if (sprite[i].statnum != 1) changespritestat(i,2);   //on waiting for you (list 2)
					sprite[i].lotag = mulscale5(sprite[i].xrepeat,sprite[i].yrepeat);
					sprite[i].cstat |= 0x101;    //Set the hitscan sensitivity bit
					break;
				case AL:
					sprite[i].cstat |= 0x101;    //Set the hitscan sensitivity bit
					changespritestat(i,0);
					break;
				case EVILAL:
					sprite[i].cstat |= 0x101;    //Set the hitscan sensitivity bit
					changespritestat(i,10);
					break;
			}
	}

	for(i=MAXSPRITES-1;i>=0;i--) copybuf(&sprite[i].x,&osprite[i].x,3);

	searchmap(cursectnum[connecthead]);

	lockclock = 0;
	ototalclock = 0;
	gotlastpacketclock = 0;

	screensize = xdim;
	dax = ((xdim-screensize)>>1);
	dax2 = dax+screensize-1;
	day = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
	day2 = day + scale(screensize,ydim-32,xdim)-1;
	setview(dax,day,dax2,day2);

	startofdynamicinterpolations = numinterpolations;
}

checktouchsprite(short snum, short sectnum)
{
	long i, nexti;

	if ((sectnum < 0) || (sectnum >= numsectors)) return;

	for(i=headspritesect[sectnum];i>=0;i=nexti)
	{
		nexti = nextspritesect[i];
		if (sprite[i].cstat&0x8000) continue;
		if ((klabs(posx[snum]-sprite[i].x)+klabs(posy[snum]-sprite[i].y) < 512) && (klabs((posz[snum]>>8)-((sprite[i].z>>8)-(tilesizy[sprite[i].picnum]>>1))) <= 40))
		{
			switch(sprite[i].picnum)
			{
				case COINSTACK:
					wsayfollow("getstuff.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
					changehealth(snum,25);
					sprite[i].cstat |= 0x8000;
					sprite[i].extra = 120*180;
					changespritestat((short)i,11);
					break;
				case GIFTBOX:
					wsayfollow("getstuff.wav",4096L+(krand()&127)+256-mulscale4(sprite[i].xrepeat,sprite[i].yrepeat),208L,&sprite[i].x,&sprite[i].y,0);
					changehealth(snum,max(mulscale8(sprite[i].xrepeat,sprite[i].yrepeat),1));
					deletesprite((short)i);
					break;
				case COIN:
					wsayfollow("getstuff.wav",4096L+(krand()&127)-64,192L,&sprite[i].x,&sprite[i].y,0);
					changehealth(snum,5);
					sprite[i].cstat |= 0x8000;
					sprite[i].extra = 120*60;
					changespritestat((short)i,11);
					break;
				case DIAMONDS:
					wsayfollow("getstuff.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
					changehealth(snum,15);
					sprite[i].cstat |= 0x8000;
					sprite[i].extra = 120*120;
					changespritestat((short)i,11);
					break;
				case CANON:
					wsayfollow("getstuff.wav",3584L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
					if (snum == myconnectindex) keystatus[4] = 1;
					numbombs[snum] += ((sprite[i].xrepeat+sprite[i].yrepeat)>>1);
					sprite[i].cstat |= 0x8000;
					sprite[i].extra = 60*(sprite[i].xrepeat+sprite[i].yrepeat);
					changespritestat((short)i,11);
					break;
				case AIRPLANE:
					wsayfollow("getstuff.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
					if (flytime[snum] < lockclock) flytime[snum] = lockclock;
					flytime[snum] += 60*(sprite[i].xrepeat+sprite[i].yrepeat);
					sprite[i].cstat |= 0x8000;
					sprite[i].extra = 120*(sprite[i].xrepeat+sprite[i].yrepeat);
					changespritestat((short)i,11);
					break;
			}
		}
	}
}

shootgun(short snum, long x, long y, long z,
			short daang, long dahoriz, short dasectnum, char guntype)
{
	short hitsect, hitwall, hitsprite, daang2;
	long i, j, daz2, hitx, hity, hitz;

	switch(guntype)
	{
		case 0:    //Shoot silver sphere bullet
			spawnsprite(j,x,y,z,1+128,0,0,16,64,64,0,0,BULLET,daang,
						  sintable[(daang+512)&2047]>>5,sintable[daang&2047]>>5,
						  (100-dahoriz)<<6,snum+4096,dasectnum,6,0,0,0);
			wsayfollow("shoot2.wav",4096L+(krand()&127)-64,184L,&sprite[j].x,&sprite[j].y,1);
			break;
		case 1:    //Shoot chain gun
			daang2 = ((daang + (krand()&31)-16)&2047);
			daz2 = ((100-dahoriz)*2000) + ((krand()-32768)>>1);

			hitscan(x,y,z,dasectnum,                   //Start position
				sintable[(daang2+512)&2047],            //X vector of 3D ang
				sintable[daang2&2047],                  //Y vector of 3D ang
				daz2,                                   //Z vector of 3D ang
				&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz);

			if (wall[hitwall].picnum == KENPICTURE)
			{
				if (waloff[4095] != 0) wall[hitwall].picnum = 4095;
				wsayfollow("hello.wav",4096L+(krand()&127)-64,256L,&wall[hitwall].x,&wall[hitwall].y,0);
			}
			else if (((hitwall < 0) && (hitsprite < 0) && (hitz >= z) && ((sector[hitsect].floorpicnum == SLIME) || (sector[hitsect].floorpicnum == FLOORMIRROR))) || ((hitwall >= 0) && (wall[hitwall].picnum == SLIME)))
			{    //If you shoot slime, make a splash
				wsayfollow("splash.wav",4096L+(krand()&511)-256,256L,&hitx,&hity,0);
				spawnsprite(j,hitx,hity,hitz,2,0,0,32,64,64,0,0,SPLASH,daang,
					0,0,0,snum+4096,hitsect,4,63,0,0); //63=time left for splash
			}
			else
			{
				wsayfollow("shoot.wav",4096L+(krand()&127)-64,256L,&hitx,&hity,0);

				if ((hitsprite >= 0) && (sprite[hitsprite].statnum < MAXSTATUS))
					switch(sprite[hitsprite].picnum)
					{
						case BROWNMONSTER:
							if (sprite[hitsprite].lotag > 0) sprite[hitsprite].lotag -= 10;
							if (sprite[hitsprite].lotag > 0)
							{
								wsayfollow("hurt.wav",4096L+(krand()&511)-256,256L,&hitx,&hity,0);
								if (sprite[hitsprite].lotag <= 25)
									sprite[hitsprite].cstat |= 2;
							}
							else
							{
								wsayfollow("blowup.wav",4096L+(krand()&127)-64,256L,&hitx,&hity,0);
								sprite[hitsprite].z += ((tilesizy[sprite[hitsprite].picnum]*sprite[hitsprite].yrepeat)<<1);
								sprite[hitsprite].picnum = GIFTBOX;
								sprite[hitsprite].cstat &= ~0x83;    //Should not clip, foot-z
								changespritestat(hitsprite,0);

								spawnsprite(j,hitx,hity,hitz+(32<<8),0,-4,0,32,64,64,
									0,0,EXPLOSION,daang,0,0,0,snum+4096,
									hitsect,5,31,0,0);
							}
							break;
						case EVILAL:
							wsayfollow("blowup.wav",4096L+(krand()&127)-64,256L,&hitx,&hity,0);
							sprite[hitsprite].picnum = EVILALGRAVE;
							sprite[hitsprite].cstat = 0;
							sprite[hitsprite].lotag = 255;
							sprite[hitsprite].xvel = (krand()&255)-128;
							sprite[hitsprite].yvel = (krand()&255)-128;
							sprite[hitsprite].zvel = (krand()&4095)-3072;
							changespritestat(hitsprite,9);

							spawnsprite(j,hitx,hity,hitz+(32<<8),0,-4,0,32,64,64,0,
								0,EXPLOSION,daang,0,0,0,snum+4096,hitsect,5,31,0,0);
								 //31=time left for explosion

							break;
						case DOOMGUY:
							for(j=connecthead;j>=0;j=connectpoint2[j])
								if (playersprite[j] == hitsprite)
								{
									wsayfollow("ouch.wav",4096L+(krand()&127)-64,256L,&hitx,&hity,0);
									changehealth(j,-10);
									break;
								}
							break;
					}

				spawnsprite(j,hitx,hity,hitz+(8<<8),2,-4,0,32,16,16,0,0,
					EXPLOSION,daang,0,0,0,snum+4096,hitsect,3,63,0,0);

					//Sprite starts out with center exactly on wall.
					//This moves it back enough to see it at all angles.
				movesprite((short)j,-(((long)sintable[(512+daang)&2047]*TICSPERFRAME)<<4),-(((long)sintable[daang]*TICSPERFRAME)<<4),0L,4L<<8,4L<<8,1);
			}
			break;
		case 2:    //Shoot bomb
		  spawnsprite(j,x,y,z,128,0,0,12,16,16,0,0,BOMB,daang,
			  sintable[(daang+512)&2047]*5>>8,sintable[daang&2047]*5>>8,
			  (80-dahoriz)<<6,snum+4096,dasectnum,6,0,0,0);

			wsayfollow("shoot3.wav",4096L+(krand()&127)-64,256L,&sprite[j].x,&sprite[j].y,1);
			break;
	}
}

analyzesprites(long dax, long day)
{
	long i, j, k;
	point3d *ospr;
	spritetype *tspr;

		//This function is called between drawrooms() and drawmasks()
		//It has a list of possible sprites that may be drawn on this frame

	for(i=0,tspr=&tsprite[0];i<spritesortcnt;i++,tspr++)
	{
		switch(tspr->picnum)
		{
			case DOOMGUY:
					//Get which of the 8 angles of the sprite to draw (0-7)
					//k ranges from 0-7
				k = getangle(tspr->x-dax,tspr->y-day);
				k = (((tspr->ang+3072+128-k)&2047)>>8)&7;
					//This guy has only 5 pictures for 8 angles (3 are x-flipped)
				if (k <= 4)
				{
					tspr->picnum += (k<<2);
					tspr->cstat &= ~4;   //clear x-flipping bit
				}
				else
				{
					tspr->picnum += ((8-k)<<2);
					tspr->cstat |= 4;    //set x-flipping bit
				}
				break;
		}

		k = statrate[tspr->statnum];
		if (k >= 0)  //Interpolate moving sprite
		{
			ospr = &osprite[tspr->owner];
			switch(k)
			{
				case 0: j = smoothratio; break;
				case 1: j = (smoothratio>>1)+(((nummoves-tspr->owner)&1)<<15); break;
				case 3: j = (smoothratio>>2)+(((nummoves-tspr->owner)&3)<<14); break;
				case 7: j = (smoothratio>>3)+(((nummoves-tspr->owner)&7)<<13); break;
				case 15: j = (smoothratio>>4)+(((nummoves-tspr->owner)&15)<<12); break;
			}
			k = tspr->x-ospr->x; tspr->x = ospr->x;
			if (k != 0) tspr->x += mulscale16(k,j);
			k = tspr->y-ospr->y; tspr->y = ospr->y;
			if (k != 0) tspr->y += mulscale16(k,j);
			k = tspr->z-ospr->z; tspr->z = ospr->z;
			if (k != 0) tspr->z += mulscale16(k,j);
		}

			//Don't allow close explosion sprites to be transluscent
		k = tspr->statnum;
		if ((k == 3) || (k == 4) || (k == 5) || (k == 7))
			if (klabs(dax-tspr->x) < 256)
				if (klabs(day-tspr->y) < 256)
					tspr->cstat &= ~2;

		tspr->shade += 6;
		if (sector[tspr->sectnum].ceilingstat&1)
			tspr->shade += sector[tspr->sectnum].ceilingshade;
		else
			tspr->shade += sector[tspr->sectnum].floorshade;
	}
}

tagcode()
{
	long i, nexti, j, k, l, s, dax, day, daz, dax2, day2, cnt, good;
	short startwall, endwall, dasector, p, oldang;

	for(p=connecthead;p>=0;p=connectpoint2[p])
	{
		if (sector[cursectnum[p]].lotag == 1)
		{
			activatehitag(sector[cursectnum[p]].hitag);
			sector[cursectnum[p]].lotag = 0;
			sector[cursectnum[p]].hitag = 0;
		}
		if ((sector[cursectnum[p]].lotag == 2) && (cursectnum[p] != ocursectnum[p]))
			activatehitag(sector[cursectnum[p]].hitag);
	}

	for(i=0;i<warpsectorcnt;i++)
	{
		dasector = warpsectorlist[i];
		j = ((lockclock&127)>>2);
		if (j >= 16) j = 31-j;
		{
			sector[dasector].ceilingshade = j;
			sector[dasector].floorshade = j;
			startwall = sector[dasector].wallptr;
			endwall = startwall+sector[dasector].wallnum;
			for(s=startwall;s<endwall;s++)
				wall[s].shade = j;
		}
	}

	for(p=connecthead;p>=0;p=connectpoint2[p])
		if (sector[cursectnum[p]].lotag == 10)  //warp sector
		{
			if (cursectnum[p] != ocursectnum[p])
			{
				warpsprite(playersprite[p]);
				posx[p] = sprite[playersprite[p]].x;
				posy[p] = sprite[playersprite[p]].y;
				posz[p] = sprite[playersprite[p]].z;
				ang[p] = sprite[playersprite[p]].ang;
				cursectnum[p] = sprite[playersprite[p]].sectnum;

				sprite[playersprite[p]].z += (32<<8);

				//warp(&posx[p],&posy[p],&posz[p],&ang[p],&cursectnum[p]);
					//Update sprite representation of player
				//setsprite(playersprite[p],posx[p],posy[p],posz[p]+(32<<8));
				//sprite[playersprite[p]].ang = ang[p];
			}
		}

	for(i=0;i<xpanningsectorcnt;i++)   //animate wall x-panning sectors
	{
		dasector = xpanningsectorlist[i];

		startwall = sector[dasector].wallptr;
		endwall = startwall+sector[dasector].wallnum;
		for(s=startwall;s<endwall;s++)
			wall[s].xpanning = ((lockclock>>2)&255);
	}

	for(i=0;i<ypanningwallcnt;i++)
		wall[ypanningwalllist[i]].ypanning = ~(lockclock&255);

	for(i=0;i<turnspritecnt;i++)
	{
		sprite[turnspritelist[i]].ang += (TICSPERFRAME<<2);
		sprite[turnspritelist[i]].ang &= 2047;
	}

	for(i=0;i<floorpanningcnt;i++)   //animate floor of slime sectors
	{
		sector[floorpanninglist[i]].floorxpanning = ((lockclock>>2)&255);
		sector[floorpanninglist[i]].floorypanning = ((lockclock>>2)&255);
	}

	for(i=0;i<dragsectorcnt;i++)
	{
		dasector = dragsectorlist[i];

		startwall = sector[dasector].wallptr;
		endwall = startwall+sector[dasector].wallnum;

		if (wall[startwall].x+dragxdir[i] < dragx1[i]) dragxdir[i] = 16;
		if (wall[startwall].y+dragydir[i] < dragy1[i]) dragydir[i] = 16;
		if (wall[startwall].x+dragxdir[i] > dragx2[i]) dragxdir[i] = -16;
		if (wall[startwall].y+dragydir[i] > dragy2[i]) dragydir[i] = -16;

		for(j=startwall;j<endwall;j++)
			dragpoint(j,wall[j].x+dragxdir[i],wall[j].y+dragydir[i]);
		j = sector[dasector].floorz;
		sector[dasector].floorz = dragfloorz[i]+(sintable[(lockclock<<4)&2047]>>3);

		for(p=connecthead;p>=0;p=connectpoint2[p])
			if (cursectnum[p] == dasector)
			{
				posx[p] += dragxdir[i];
				posy[p] += dragydir[i];
				//posz[p] += (sector[dasector].floorz-j);

					//Update sprite representation of player
				setsprite(playersprite[p],posx[p],posy[p],posz[p]+(32<<8));
				sprite[playersprite[p]].ang = ang[p];
			}
	}

	for(i=0;i<swingcnt;i++)
	{
		if (swinganginc[i] != 0)
		{
			oldang = swingang[i];
			for(j=0;j<(TICSPERFRAME<<2);j++)
			{
				swingang[i] = ((swingang[i]+swinganginc[i])&2047);
				if (swingang[i] == swingangclosed[i])
				{
					wsayfollow("closdoor.wav",4096L+(krand()&511)-256,256L,&swingx[i][0],&swingy[i][0],0);
					swinganginc[i] = 0;
				}
				if (swingang[i] == swingangopen[i]) swinganginc[i] = 0;
			}
			for(k=1;k<=3;k++)
				rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);

			if (swinganginc[i] != 0)
			{
				for(p=connecthead;p>=0;p=connectpoint2[p])
					if ((cursectnum[p] == swingsector[i]) || (testneighborsectors(cursectnum[p],swingsector[i]) == 1))
					{
						cnt = 256;
						do
						{
							good = 1;

								//swingangopendir is -1 if forwards, 1 is backwards
							l = (swingangopendir[i] > 0);
							for(k=l+3;k>=l;k--)
								if (clipinsidebox(posx[p],posy[p],swingwall[i][k],128L) != 0)
								{
									good = 0;
									break;
								}
							if (good == 0)
							{
								if (cnt == 256)
								{
									swinganginc[i] = -swinganginc[i];
									swingang[i] = oldang;
								}
								else
								{
									swingang[i] = ((swingang[i]-swinganginc[i])&2047);
								}
								for(k=1;k<=3;k++)
									rotatepoint(swingx[i][0],swingy[i][0],swingx[i][k],swingy[i][k],swingang[i],&wall[swingwall[i][k]].x,&wall[swingwall[i][k]].y);
								if (swingang[i] == swingangclosed[i])
								{
									wsayfollow("closdoor.wav",4096L+(krand()&511)-256,256L,&swingx[i][0],&swingy[i][0],0);
									swinganginc[i] = 0;
									break;
								}
								if (swingang[i] == swingangopen[i])
								{
									swinganginc[i] = 0;
									break;
								}
								cnt--;
							}
						} while ((good == 0) && (cnt > 0));
					}
			}
		}
		if (swinganginc[i] == 0)
			for(j=1;j<=3;j++)
			{
				stopinterpolation(&wall[swingwall[i][j]].x);
				stopinterpolation(&wall[swingwall[i][j]].y);
			}
	}

	for(i=0;i<revolvecnt;i++)
	{
		startwall = sector[revolvesector[i]].wallptr;
		endwall = startwall + sector[revolvesector[i]].wallnum;

		revolveang[i] = ((revolveang[i]-(TICSPERFRAME<<2))&2047);
		for(k=startwall;k<endwall;k++)
		{
			rotatepoint(revolvepivotx[i],revolvepivoty[i],revolvex[i][k-startwall],revolvey[i][k-startwall],revolveang[i],&dax,&day);
			dragpoint(k,dax,day);
		}
	}

	for(i=0;i<subwaytrackcnt;i++)
	{
		if ((subwayvel[i] < -2) || (subwayvel[i] > 2))
		{
			dasector = subwaytracksector[i][0];
			startwall = sector[dasector].wallptr;
			endwall = startwall+sector[dasector].wallnum;
			for(k=startwall;k<endwall;k++)
				if (wall[k].x > subwaytrackx1[i])
					if (wall[k].y > subwaytracky1[i])
						if (wall[k].x < subwaytrackx2[i])
							if (wall[k].y < subwaytracky2[i])
								wall[k].x += subwayvel[i];

			for(j=1;j<subwaynumsectors[i];j++)
			{
				dasector = subwaytracksector[i][j];

				startwall = sector[dasector].wallptr;
				endwall = startwall+sector[dasector].wallnum;
				for(k=startwall;k<endwall;k++)
					wall[k].x += subwayvel[i];

				for(s=headspritesect[dasector];s>=0;s=nextspritesect[s])
					sprite[s].x += subwayvel[i];
			}

			for(p=connecthead;p>=0;p=connectpoint2[p])
				if (cursectnum[p] != subwaytracksector[i][0])
					if (sector[cursectnum[p]].floorz != sector[subwaytracksector[i][0]].floorz)
						if (posx[p] > subwaytrackx1[i])
							if (posy[p] > subwaytracky1[i])
								if (posx[p] < subwaytrackx2[i])
									if (posy[p] < subwaytracky2[i])
									{
										posx[p] += subwayvel[i];

											//Update sprite representation of player
										setsprite(playersprite[p],posx[p],posy[p],posz[p]+(32<<8));
										sprite[playersprite[p]].ang = ang[p];
									}

			subwayx[i] += subwayvel[i];
		}

		j = subwayvel[i];
		k = subwaystop[i][subwaygoalstop[i]] - subwayx[i];
		if (k > 0)
		{
			if (k > 4096)
			{
				if (subwayvel[i] < 256) subwayvel[i]++;
			}
			else
				subwayvel[i] = (k>>4)+1;
		}
		else if (k < 0)
		{
			if (k < -4096)
			{
				if (subwayvel[i] > -256) subwayvel[i]--;
			}
			else
				subwayvel[i] = (k>>4)-1;
		}
		if ((j < 0) && (subwayvel[i] >= 0)) subwayvel[i] = -1;
		if ((j > 0) && (subwayvel[i] <= 0)) subwayvel[i] = 1;

		if ((subwayvel[i] <= 2) && (subwayvel[i] >= -2) && (klabs(k) < 2048))
		{
			  //Open / close doors
			if ((subwaypausetime[i] == 720) || ((subwaypausetime[i] >= 120) && (subwaypausetime[i]-TICSPERFRAME < 120)))
				activatehitag(sector[subwaytracksector[i][0]].hitag);

			subwaypausetime[i] -= TICSPERFRAME;
			if (subwaypausetime[i] < 0)
			{
				subwaypausetime[i] = 720;
				if (subwayvel[i] < 0)
				{
					subwaygoalstop[i]--;
					if (subwaygoalstop[i] < 0)
					{
						subwaygoalstop[i] = 1;
						subwayvel[i] = 1;
					}
				}
				else if (subwayvel[i] > 0)
				{
					subwaygoalstop[i]++;
					if (subwaygoalstop[i] >= subwaystopcnt[i])
					{
						subwaygoalstop[i] = subwaystopcnt[i]-2;
						subwayvel[i] = -1;
					}
				}
			}
		}
	}
}

statuslistcode()
{
	short p, target, hitobject, daang, osectnum, movestat;
	long i, nexti, j, nextj, k, l, dax, day, daz, dist, ox, oy, mindist;
	long doubvel, xvect, yvect;

		//Go through active BROWNMONSTER list
	for(i=headspritestat[1];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		k = (krand()&63);

			//Choose a target player
		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = klabs(sprite[i].x-posx[p])+klabs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}

			//brown monster decides to shoot bullet
		if ((sprite[i].lotag > 25) && (k == 23))
		{
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[target],posy[target],posz[target],cursectnum[target]) == 0)
			{
				changespritestat(i,2);
			}
			else
			{
				wsayfollow("zipguns.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,1);

				doubvel = (TICSPERFRAME<<((sync[target].bits&256)>0));
				xvect = 0, yvect = 0;
				if (sync[target].fvel != 0)
				{
					xvect += ((((long)sync[target].fvel)*doubvel*(long)sintable[(ang[target]+512)&2047])>>3);
					yvect += ((((long)sync[target].fvel)*doubvel*(long)sintable[ang[target]&2047])>>3);
				}
				if (sync[target].svel != 0)
				{
					xvect += ((((long)sync[target].svel)*doubvel*(long)sintable[ang[target]&2047])>>3);
					yvect += ((((long)sync[target].svel)*doubvel*(long)sintable[(ang[target]+1536)&2047])>>3);
				}

				ox = posx[target]; oy = posy[target];

					//distance is k
				k = ksqrt((ox-sprite[i].x)*(ox-sprite[i].x)+(oy-sprite[i].y)*(oy-sprite[i].y));

				switch(sprite[i].extra&3)
				{
					case 1: k = -(k>>1); break;
					case 3: k = 0; break;
					case 0: case 2: break;
				}
				sprite[i].extra++;

					//rate is (TICSPERFRAME<<19)
				xvect = scale(xvect,k,TICSPERFRAME<<19);
				yvect = scale(yvect,k,TICSPERFRAME<<19);
				clipmove(&ox,&oy,&posz[target],&cursectnum[target],xvect<<14,yvect<<14,128L,4<<8,4<<8,0);
				ox -= sprite[i].x;
				oy -= sprite[i].y;

				daang = ((getangle(ox,oy)+(krand()&7)-4)&2047);

				dax = (sintable[(daang+512)&2047]>>6);
				day = (sintable[daang&2047]>>6);
				daz = 0;
				if (ox != 0)
					daz = scale(dax,posz[target]+(8<<8)-sprite[i].z,ox);
				else if (oy != 0)
					daz = scale(day,posz[target]+(8<<8)-sprite[i].z,oy);

				spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,128,0,0,
					16,sprite[i].xrepeat,sprite[i].yrepeat,0,0,BULLET,daang,dax,day,daz,i,sprite[i].sectnum,6,0,0,0);

				sprite[i].extra = 0;
			}
		}

			//Move brown monster
		dax = sprite[i].x;   //Back up old x&y if stepping off cliff
		day = sprite[i].y;

		doubvel = max(mulscale7(sprite[i].xrepeat,sprite[i].yrepeat),4);

		osectnum = sprite[i].sectnum;
		movestat = movesprite((short)i,(long)sintable[(sprite[i].ang+512)&2047]*doubvel,(long)sintable[sprite[i].ang]*doubvel,0L,4L<<8,4L<<8,0);
		if (globloz > sprite[i].z+(48<<8))
			{ sprite[i].x = dax; sprite[i].y = day; movestat = 1; }
		else
			sprite[i].z = globloz-((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);

		if ((sprite[i].sectnum != osectnum) && (sector[sprite[i].sectnum].lotag == 10))
			{ warpsprite((short)i); movestat = 0; }

		if ((movestat != 0) || (k == 1))
		{
			daang = (getangle(posx[target]-sprite[i].x,posy[target]-sprite[i].y)&2047);
			sprite[i].ang = ((daang+(krand()&1023)-512)&2047);
		}
	}

	for(i=headspritestat[10];i>=0;i=nexti)  //EVILAL list
	{
		nexti = nextspritestat[i];

		if (sprite[i].yrepeat < 38) continue;
		if (sprite[i].yrepeat < 64)
		{
			sprite[i].xrepeat++;
			sprite[i].yrepeat++;
			continue;
		}

		if ((nummoves-i)&statrate[10]) continue;

			//Choose a target player
		mindist = 0x7fffffff; target = connecthead;
		for(p=connecthead;p>=0;p=connectpoint2[p])
		{
			dist = klabs(sprite[i].x-posx[p])+klabs(sprite[i].y-posy[p]);
			if (dist < mindist) mindist = dist, target = p;
		}

		k = (krand()&255);

		if (k < 48)  //Al decides to reproduce
		{
			if (k < 2)  //Give him a chance to reproduce without seeing you
			{
				spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,1+2+256,0,0,
					32,38,38,0,0,EVILAL,krand()&2047,0,0,0,i,
					sprite[i].sectnum,10,0,0,0);
			}
			else if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[target],posy[target],posz[target],cursectnum[target]) == 1)
			{
				spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,1+2+256,0,0,
					32,38,38,0,0,EVILAL,krand()&2047,0,0,0,i,
					sprite[i].sectnum,10,0,0,0);
			}
		}
		if (k >= 208)     //Al decides to shoot bullet
		{
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[target],posy[target],posz[target],cursectnum[target]) == 1)
			{
				wsayfollow("zipguns.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,1);

				spawnsprite(j,sprite[i].x,sprite[i].y,
					sector[sprite[i].sectnum].floorz-(24<<8),
					0,0,0,16,32,32,0,0,BULLET,
					(getangle(posx[target]-sprite[j].x,
						posy[target]-sprite[j].y)+(krand()&15)-8)&2047,
					sintable[(sprite[j].ang+512)&2047]>>6,
					sintable[sprite[j].ang&2047]>>6,
					((posz[target]+(8<<8)-sprite[j].z)<<8) /
					  (ksqrt((posx[target]-sprite[j].x) *
								(posx[target]-sprite[j].x) +
								(posy[target]-sprite[j].y) *
								(posy[target]-sprite[j].y))+1),
								i,sprite[i].sectnum,6,0,0,0);
			}
		}

			//Move Al
		dax = ((sintable[(sprite[i].ang+512)&2047]*TICSPERFRAME)<<8);
		day = ((sintable[sprite[i].ang]*TICSPERFRAME)<<8);

		osectnum = sprite[i].sectnum;
		movestat = movesprite((short)i,dax,day,0L,-8L<<8,-8L<<8,0);
		sprite[i].z = globloz;
		if ((sprite[i].sectnum != osectnum) && (sector[sprite[i].sectnum].lotag == 10))
		{
			warpsprite((short)i);
			movestat = 0;
		}

		if (movestat != 0)
		{
			if ((k&2) && (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[target],posy[target],posz[target],cursectnum[target]) == 1))
				sprite[i].ang = getangle(posx[target]-sprite[i].x,posy[target]-sprite[i].y);
			else
				sprite[i].ang = (krand()&2047);

			if ((movestat&49152) == 49152)
				if (sprite[movestat&16383].picnum == EVILAL)
					if ((k&31) >= 30)
					{
						wsayfollow("blowup.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
						sprite[i].picnum = EVILALGRAVE;
						sprite[i].cstat = 0;
						sprite[i].lotag = 255;
						sprite[i].xvel = (krand()&255)-128;
						sprite[i].yvel = (krand()&255)-128;
						sprite[i].zvel = (krand()&4095)-3072;
						changespritestat(i,9);
					}
			if (movestat == -1)
			{
				wsayfollow("blowup.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
				sprite[i].picnum = EVILALGRAVE;
				sprite[i].cstat = 0;
				sprite[i].lotag = 255;
				sprite[i].xvel = (krand()&255)-128;
				sprite[i].yvel = (krand()&255)-128;
				sprite[i].zvel = (krand()&4095)-3072;
				changespritestat(i,9);
			}
		}
	}

		//Go through travelling bullet sprites
	for(i=headspritestat[6];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		if ((nummoves-i)&statrate[6]) continue;

			 //If the sprite is a bullet then...
		if ((sprite[i].picnum == BULLET) || (sprite[i].picnum == BOMB))
		{
			dax = ((((long)sprite[i].xvel)*TICSPERFRAME)<<12);
			day = ((((long)sprite[i].yvel)*TICSPERFRAME)<<12);
			daz = ((((long)sprite[i].zvel)*TICSPERFRAME)>>2);
			if (sprite[i].picnum == BOMB) daz = 0;

			osectnum = sprite[i].sectnum;
			hitobject = movesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);
			if ((sprite[i].sectnum != osectnum) && (sector[sprite[i].sectnum].lotag == 10))
			{
				warpsprite((short)i);
				hitobject = 0;
			}

			if (sprite[i].picnum == BOMB)
			{
				j = sprite[i].sectnum;
				if ((sector[j].floorstat&2) && (sprite[i].z > globloz-(8<<8)))
				{
					k = sector[j].wallptr;
					daang = getangle(wall[wall[k].point2].x-wall[k].x,wall[wall[k].point2].y-wall[k].y);
					sprite[i].xvel += mulscale22(sintable[(daang+1024)&2047],sector[j].floorheinum);
					sprite[i].yvel += mulscale22(sintable[(daang+512)&2047],sector[j].floorheinum);
				}
			}

			if (sprite[i].picnum == BOMB)
			{
				sprite[i].z += sprite[i].zvel;
				sprite[i].zvel += (TICSPERFRAME<<7);
				if (sprite[i].z < globhiz+(tilesizy[BOMB]<<6))
				{
					sprite[i].z = globhiz+(tilesizy[BOMB]<<6);
					sprite[i].zvel = -(sprite[i].zvel>>1);
				}
				if (sprite[i].z > globloz-(tilesizy[BOMB]<<6))
				{
					sprite[i].z = globloz-(tilesizy[BOMB]<<6);
					sprite[i].zvel = -(sprite[i].zvel>>1);
				}
				dax = sprite[i].xvel; day = sprite[i].yvel;
				dist = dax*dax+day*day;
				if (dist < 512)
				{
					bombexplode(i);
					goto bulletisdeletedskip;
				}
				if (dist < 4096)
				{
					sprite[i].xrepeat = ((4096+2048)*16) / (dist+2048);
					sprite[i].yrepeat = sprite[i].xrepeat;
					sprite[i].xoffset = (krand()&15)-8;
					sprite[i].yoffset = (krand()&15)-8;
				}
				if (mulscale30(krand(),dist) == 0)
				{
					sprite[i].xvel -= ksgn(sprite[i].xvel);
					sprite[i].yvel -= ksgn(sprite[i].yvel);
					sprite[i].zvel -= ksgn(sprite[i].zvel);
				}
			}

				//Check for bouncy objects before killing bullet
			if ((hitobject&0xc000) == 32768)  //Bullet hit a wall
			{
				if (wall[hitobject&4095].lotag == 8)
				{
					dax = sprite[i].xvel; day = sprite[i].yvel;
					if ((sprite[i].picnum != BOMB) || (dax*dax+day*day >= 512))
					{
						k = (hitobject&4095); l = wall[k].point2;
						j = getangle(wall[l].x-wall[k].x,wall[l].y-wall[k].y)+512;

							//k = cos(ang) * sin(ang) * 2
						k = mulscale13(sintable[(j+512)&2047],sintable[j&2047]);
							//l = cos(ang * 2)
						l = sintable[((j<<1)+512)&2047];

						ox = sprite[i].xvel; oy = sprite[i].yvel;
						dax = -ox; day = -oy;
						sprite[i].xvel = dmulscale14(day,k,dax,l);
						sprite[i].yvel = dmulscale14(dax,k,-day,l);

						if (sprite[i].picnum == BOMB)
						{
							sprite[i].xvel -= (sprite[i].xvel>>3);
							sprite[i].yvel -= (sprite[i].yvel>>3);
							sprite[i].zvel -= (sprite[i].zvel>>3);
						}
						ox -= sprite[i].xvel; oy -= sprite[i].yvel;
						dist = ((ox*ox+oy*oy)>>8);
						wsayfollow("bouncy.wav",4096L+(krand()&127)-64,min(dist,256),&sprite[i].x,&sprite[i].y,1);
						hitobject = 0;
						sprite[i].owner = -1;   //Bullet turns evil!
					}
				}
			}
			else if ((hitobject&0xc000) == 49152)  //Bullet hit a sprite
			{
				if (sprite[hitobject&4095].picnum == BOUNCYMAT)
				{
					if ((sprite[hitobject&4095].cstat&48) == 0)
					{
						sprite[i].xvel = -sprite[i].xvel;
						sprite[i].yvel = -sprite[i].yvel;
						sprite[i].zvel = -sprite[i].zvel;
						dist = 255;
					}
					else if ((sprite[hitobject&4095].cstat&48) == 16)
					{
						j = sprite[hitobject&4095].ang;

							//k = cos(ang) * sin(ang) * 2
						k = mulscale13(sintable[(j+512)&2047],sintable[j&2047]);
							//l = cos(ang * 2)
						l = sintable[((j<<1)+512)&2047];

						ox = sprite[i].xvel; oy = sprite[i].yvel;
						dax = -ox; day = -oy;
						sprite[i].xvel = dmulscale14(day,k,dax,l);
						sprite[i].yvel = dmulscale14(dax,k,-day,l);

						ox -= sprite[i].xvel; oy -= sprite[i].yvel;
						dist = ((ox*ox+oy*oy)>>8);
					}
					sprite[i].owner = -1;   //Bullet turns evil!
					wsayfollow("bouncy.wav",4096L+(krand()&127)-64,min(dist,256),&sprite[i].x,&sprite[i].y,1);
					hitobject = 0;
				}
			}

			if (hitobject != 0)
			{
				if (sprite[i].picnum == BOMB)
				{
					if ((hitobject&0xc000) == 49152)
						if (sprite[hitobject&4095].lotag == 5)  //Basketball hoop
						{
							wsayfollow("niceshot.wav",3840L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
							deletesprite((short)i);
							goto bulletisdeletedskip;
						}

					bombexplode(i);
					goto bulletisdeletedskip;
				}

				if ((hitobject&0xc000) == 16384)  //Hits a ceiling / floor
				{
					wsayfollow("bullseye.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
					deletesprite((short)i);
					goto bulletisdeletedskip;
				}
				else if ((hitobject&0xc000) == 32768)  //Bullet hit a wall
				{
					if (wall[hitobject&4095].picnum == KENPICTURE)
					{
						if (waloff[4095] != 0)
							wall[hitobject&4095].picnum = 4095;
						wsayfollow("hello.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);   //Ken says, "Hello... how are you today!"
					}
					else
						wsayfollow("bullseye.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);

					deletesprite((short)i);
					goto bulletisdeletedskip;
				}
				else if ((hitobject&0xc000) == 49152)  //Bullet hit a sprite
				{
						//Check if bullet hit a player & find which player it was...
					if (sprite[hitobject&4095].picnum == DOOMGUY)
						for(j=connecthead;j>=0;j=connectpoint2[j])
							if (sprite[i].owner != j+4096)
								if (playersprite[j] == (hitobject&4095))
								{
									wsayfollow("ouch.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
									changehealth(j,-mulscale8(sprite[i].xrepeat,sprite[i].yrepeat));
									deletesprite((short)i);
									goto bulletisdeletedskip;
								}

						//Check if bullet hit any monsters...
					j = (hitobject&4095);     //j is the spritenum that the bullet (spritenum i) hit
					if (sprite[i].owner != j)
					{
						switch(sprite[j].picnum)
						{
							case BROWNMONSTER:
								if (sprite[j].lotag > 0) sprite[j].lotag -= mulscale8(sprite[i].xrepeat,sprite[i].yrepeat);
								if (sprite[j].lotag > 0)
								{
									if (sprite[j].lotag <= 25) sprite[j].cstat |= 2;
									wsayfollow("hurt.wav",4096L+(krand()&511)-256,256L,&sprite[i].x,&sprite[i].y,1);
								}
								else
								{
									wsayfollow("blowup.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
									sprite[j].z += ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<1);
									sprite[j].picnum = GIFTBOX;
									sprite[j].cstat &= ~0x83;    //Should not clip, foot-z

									spawnsprite(k,sprite[j].x,sprite[j].y,sprite[j].z,
										0,-4,0,32,64,64,0,0,EXPLOSION,sprite[j].ang,
										0,0,0,j,sprite[j].sectnum,5,31,0,0);
											//31=Time left for explosion to stay

									changespritestat(j,0);
								}
								deletesprite((short)i);
								goto bulletisdeletedskip;
							case EVILAL:
								wsayfollow("blowup.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
								sprite[j].picnum = EVILALGRAVE;
								sprite[j].cstat = 0;
								sprite[j].lotag = 255;
								sprite[j].xvel = (krand()&255)-128;
								sprite[j].yvel = (krand()&255)-128;
								sprite[j].zvel = (krand()&4095)-3072;
								changespritestat(j,9);

								deletesprite((short)i);
								goto bulletisdeletedskip;
							case AL:
								wsayfollow("blowup.wav",5144L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
								sprite[j].xrepeat += 2;
								sprite[j].yrepeat += 2;
								if (sprite[j].yrepeat >= 38)
								{
									sprite[j].picnum = EVILAL;
									sprite[j].cstat |= 2;      //Make him transluscent
									changespritestat(j,10);
								}
								deletesprite((short)i);
								goto bulletisdeletedskip;
							default:
								wsayfollow("bullseye.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
								deletesprite((short)i);
								goto bulletisdeletedskip;
						}
					}
				}
			}
		}
bulletisdeletedskip: continue;
	}

		//Go through monster waiting for you list
	for(i=headspritestat[2];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		if ((nummoves-i)&15) continue;

			//Use dot product to see if monster's angle is towards a player
		for(p=connecthead;p>=0;p=connectpoint2[p])
			if (sintable[(sprite[i].ang+512)&2047]*(posx[p]-sprite[i].x) + sintable[sprite[i].ang&2047]*(posy[p]-sprite[i].y) >= 0)
				if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[p],posy[p],posz[p],cursectnum[p]) == 1)
				{
					changespritestat(i,1);
					if (sprite[i].lotag == 100)
					{
						wsayfollow("iseeyou.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,1);
						sprite[i].lotag = 99;
					}
				}
	}

		//Go through smoke sprites
	for(i=headspritestat[3];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].z -= (TICSPERFRAME<<6);
		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0) deletesprite(i);
	}

		//Go through splash sprites
	for(i=headspritestat[4];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].lotag -= TICSPERFRAME;
		sprite[i].picnum = SPLASH + ((63-sprite[i].lotag)>>4);
		if (sprite[i].lotag < 0) deletesprite(i);
	}

		//Go through explosion sprites
	for(i=headspritestat[5];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0) deletesprite(i);
	}

		//Go through bomb spriral-explosion sprites
	for(i=headspritestat[7];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].xrepeat = (sprite[i].lotag>>2);
		sprite[i].yrepeat = (sprite[i].lotag>>2);
		sprite[i].lotag -= (TICSPERFRAME<<2);
		if (sprite[i].lotag < 0) { deletesprite(i); continue; }

		if ((nummoves-i)&statrate[7]) continue;

		sprite[i].x += ((sprite[i].xvel*TICSPERFRAME)>>2);
		sprite[i].y += ((sprite[i].yvel*TICSPERFRAME)>>2);
		sprite[i].z += ((sprite[i].zvel*TICSPERFRAME)>>2);

		sprite[i].zvel += (TICSPERFRAME<<9);
		if (sprite[i].z < sector[sprite[i].sectnum].ceilingz+(4<<8))
		{
			sprite[i].z = sector[sprite[i].sectnum].ceilingz+(4<<8);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
		if (sprite[i].z > sector[sprite[i].sectnum].floorz-(4<<8))
		{
			sprite[i].z = sector[sprite[i].sectnum].floorz-(4<<8);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
	}

		//EVILALGRAVE shrinking list
	for(i=headspritestat[9];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].xrepeat = (sprite[i].lotag>>2);
		sprite[i].yrepeat = (sprite[i].lotag>>2);
		sprite[i].lotag -= TICSPERFRAME;
		if (sprite[i].lotag < 0) { deletesprite(i); continue; }

		if ((nummoves-i)&statrate[9]) continue;

		sprite[i].x += (sprite[i].xvel*TICSPERFRAME);
		sprite[i].y += (sprite[i].yvel*TICSPERFRAME);
		sprite[i].z += (sprite[i].zvel*TICSPERFRAME);

		sprite[i].zvel += (TICSPERFRAME<<8);
		if (sprite[i].z < sector[sprite[i].sectnum].ceilingz)
		{
			sprite[i].z = sector[sprite[i].sectnum].ceilingz;
			sprite[i].xvel -= (sprite[i].xvel>>2);
			sprite[i].yvel -= (sprite[i].yvel>>2);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
		if (sprite[i].z > sector[sprite[i].sectnum].floorz)
		{
			sprite[i].z = sector[sprite[i].sectnum].floorz;
			sprite[i].xvel -= (sprite[i].xvel>>2);
			sprite[i].yvel -= (sprite[i].yvel>>2);
			sprite[i].zvel = -(sprite[i].zvel>>1);
		}
	}

		//Re-spawning sprite list
	for(i=headspritestat[11];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];

		sprite[i].extra -= TICSPERFRAME;
		if (sprite[i].extra < 0)
		{
			wsayfollow("warp.wav",6144L+(krand()&127)-64,128L,&sprite[i].x,&sprite[i].y,0);
			sprite[i].cstat &= ~0x8000;
			sprite[i].extra = -1;
			changespritestat((short)i,0);
		}
	}
}

activatehitag(short dahitag)
{
	long i, nexti;

	for(i=0;i<numsectors;i++)
		if (sector[i].hitag == dahitag) operatesector(i);

	for(i=headspritestat[0];i>=0;i=nexti)
	{
		nexti = nextspritestat[i];
		if (sprite[i].hitag == dahitag) operatesprite(i);
	}
}

bombexplode(long i)
{
	long j, nextj, k, daang, dax, day, dist;

	spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z,0,-4,0,
		32,64,64,0,0,EXPLOSION,sprite[i].ang,
		0,0,0,sprite[i].owner,sprite[i].sectnum,5,31,0,0);
		  //31=Time left for explosion to stay

	for(k=0;k<12;k++)
	{
		spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z+(8<<8),2,-4,0,
			32,24,24,0,0,EXPLOSION,sprite[i].ang,
			(krand()>>7)-256,(krand()>>7)-256,(krand()>>2)-8192,
			sprite[i].owner,sprite[i].sectnum,7,96,0,0);
				//96=Time left for smoke to be alive
	}

	for(j=connecthead;j>=0;j=connectpoint2[j])
	{
		dist = (posx[j]-sprite[i].x)*(posx[j]-sprite[i].x);
		dist += (posy[j]-sprite[i].y)*(posy[j]-sprite[i].y);
		dist += ((posz[j]-sprite[i].z)>>4)*((posz[j]-sprite[i].z)>>4);
		if (dist < 4194304)
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,posx[j],posy[j],posz[j],cursectnum[j]) == 1)
			{
				k = ((32768/((dist>>16)+4))>>5);
				if (j == myconnectindex)
				{
					daang = getangle(posx[j]-sprite[i].x,posy[j]-sprite[i].y);
					dax = ((k*sintable[(daang+512)&2047])>>14);
					day = ((k*sintable[daang&2047])>>14);
					fvel += ((dax*sintable[(ang[j]+512)&2047]+day*sintable[ang[j]&2047])>>14);
					svel += ((day*sintable[(ang[j]+512)&2047]-dax*sintable[ang[j]&2047])>>14);
				}
				changehealth(j,-k);    //if changehealth returns 1, you're dead
			}
	}

	for(k=1;k<=2;k++)         //Check for hurting monsters
	{
		for(j=headspritestat[k];j>=0;j=nextj)
		{
			nextj = nextspritestat[j];

			dist = (sprite[j].x-sprite[i].x)*(sprite[j].x-sprite[i].x);
			dist += (sprite[j].y-sprite[i].y)*(sprite[j].y-sprite[i].y);
			dist += ((sprite[j].z-sprite[i].z)>>4)*((sprite[j].z-sprite[i].z)>>4);
			if (dist >= 4194304) continue;
			if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,sprite[j].x,sprite[j].y,sprite[j].z-(tilesizy[sprite[j].picnum]<<7),sprite[j].sectnum) == 0)
				continue;
			if (sprite[j].picnum == BROWNMONSTER)
			{
				sprite[j].z += ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<1);
				sprite[j].picnum = GIFTBOX;
				sprite[j].cstat &= ~0x83;    //Should not clip, foot-z
				changespritestat(j,0);
			}
		}
	}

	for(j=headspritestat[10];j>=0;j=nextj)   //Check for EVILAL's
	{
		nextj = nextspritestat[j];

		dist = (sprite[j].x-sprite[i].x)*(sprite[j].x-sprite[i].x);
		dist += (sprite[j].y-sprite[i].y)*(sprite[j].y-sprite[i].y);
		dist += ((sprite[j].z-sprite[i].z)>>4)*((sprite[j].z-sprite[i].z)>>4);
		if (dist >= 4194304) continue;
		if (cansee(sprite[i].x,sprite[i].y,sprite[i].z-(tilesizy[sprite[i].picnum]<<7),sprite[i].sectnum,sprite[j].x,sprite[j].y,sprite[j].z-(tilesizy[sprite[j].picnum]<<7),sprite[j].sectnum) == 0)
			continue;

		sprite[j].picnum = EVILALGRAVE;
		sprite[j].cstat = 0;
		sprite[j].lotag = 255;
		sprite[j].xvel = (krand()&255)-128;
		sprite[j].yvel = (krand()&255)-128;
		sprite[j].zvel = (krand()&4095)-3072;
		changespritestat(j,9);
	}

	wsayfollow("blowup.wav",3840L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,0);
	deletesprite((short)i);
}

processinput(short snum)
{
	long oldposx, oldposy, nexti;
	long i, j, k, doubvel, xvect, yvect, goalz;
	long dax, day, dax2, day2, odax, oday, odax2, oday2;
	short startwall, endwall;
	char *ptr;

		//SHARED KEYS:
		//Movement code
	if ((sync[snum].fvel|sync[snum].svel) != 0)
	{
		doubvel = (TICSPERFRAME<<((sync[snum].bits&256)>0));

		xvect = 0, yvect = 0;
		if (sync[snum].fvel != 0)
		{
			xvect += ((((long)sync[snum].fvel)*doubvel*(long)sintable[(ang[snum]+512)&2047])>>3);
			yvect += ((((long)sync[snum].fvel)*doubvel*(long)sintable[ang[snum]&2047])>>3);
		}
		if (sync[snum].svel != 0)
		{
			xvect += ((((long)sync[snum].svel)*doubvel*(long)sintable[ang[snum]&2047])>>3);
			yvect += ((((long)sync[snum].svel)*doubvel*(long)sintable[(ang[snum]+1536)&2047])>>3);
		}
		clipmove(&posx[snum],&posy[snum],&posz[snum],&cursectnum[snum],xvect,yvect,128L,4<<8,4<<8,0);
		revolvedoorstat[snum] = 1;
	}
	else
	{
		revolvedoorstat[snum] = 0;
	}

	sprite[playersprite[snum]].cstat &= ~1;
		//Push player away from walls if clipmove doesn't work
	if (pushmove(&posx[snum],&posy[snum],&posz[snum],&cursectnum[snum],128L,4<<8,4<<8,0) < 0)
		changehealth(snum,-1000);  //If this screws up, then instant death!!!

			// Getzrange returns the highest and lowest z's for an entire box,
			// NOT just a point.  This prevents you from falling off cliffs
			// when you step only slightly over the cliff.
	getzrange(posx[snum],posy[snum],posz[snum],cursectnum[snum],&globhiz,&globhihit,&globloz,&globlohit,128L,0);
	sprite[playersprite[snum]].cstat |= 1;

	if (sync[snum].avel != 0)          //ang += avel * constant
	{                         //ENGINE calculates avel for you
		doubvel = TICSPERFRAME;
		if ((sync[snum].bits&256) > 0)  //Lt. shift makes turn velocity 50% faster
			doubvel += (TICSPERFRAME>>1);
		ang[snum] += ((((long)sync[snum].avel)*doubvel)>>4);
		ang[snum] &= 2047;
	}

	if (health[snum] < 0)
	{
		health[snum] -= TICSPERFRAME;
		if (health[snum] <= -160)
		{
			hvel[snum] = 0;
			if (snum == myconnectindex)
				fvel = 0, svel = 0, avel = 0, keystatus[3] = 1;

			deaths[snum]++;
			health[snum] = 100;
			numbombs[snum] = 0;
			flytime[snum] = 0;

			findrandomspot(&posx[snum],&posy[snum],&cursectnum[snum]);
			posz[snum] = getflorzofslope(cursectnum[snum],posx[snum],posy[snum])-(1<<8);
			horiz[snum] = 100;
			ang[snum] = (krand()&2047);

			setsprite(playersprite[snum],posx[snum],posy[snum],posz[snum]+(32<<8));
			sprite[playersprite[snum]].picnum = DOOMGUY;
			sprite[playersprite[snum]].ang = ang[snum];
			sprite[playersprite[snum]].xrepeat = 64;
			sprite[playersprite[snum]].yrepeat = 64;

			if ((snum == screenpeek) && (screensize <= xdim))
			{
				sprintf(&tempbuf,"Deaths: %d",deaths[snum]);
				printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-16,tempbuf,ALPHABET,80);
				sprintf(&tempbuf,"Health: %3d",health[snum]);
				printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-24,tempbuf,ALPHABET,80);
			}

			i = playersprite[snum];
			wsayfollow("zipguns.wav",4096L+(krand()&127)-64,256L,&sprite[i].x,&sprite[i].y,1);
			for(k=0;k<16;k++)
			{
				spawnsprite(j,sprite[i].x,sprite[i].y,sprite[i].z+(8<<8),2,-4,0,
					32,24,24,0,0,EXPLOSION,sprite[i].ang,
					(krand()&511)-256,(krand()&511)-256,(krand()&16384)-8192,
					sprite[i].owner,sprite[i].sectnum,7,96,0,0);
						//96=Time left for smoke to be alive
			}
		}
		else
		{
			sprite[playersprite[snum]].xrepeat = max(((128+health[snum])>>1),0);
			sprite[playersprite[snum]].yrepeat = max(((128+health[snum])>>1),0);

			hvel[snum] += (TICSPERFRAME<<2);
			horiz[snum] = max(horiz[snum]-4,0);
			posz[snum] += hvel[snum];
			if (posz[snum] > globloz-(4<<8))
			{
				posz[snum] = globloz-(4<<8);
				horiz[snum] = min(horiz[snum]+5,200);
				hvel[snum] = 0;
			}
		}
	}

	if (((sync[snum].bits&8) > 0) && (horiz[snum] > 100-(200>>1))) horiz[snum] -= 4;     //-
	if (((sync[snum].bits&4) > 0) && (horiz[snum] < 100+(200>>1))) horiz[snum] += 4;   //+

	goalz = globloz-(32<<8);         //32 pixels above floor
	if (sector[cursectnum[snum]].lotag == 4)   //slime sector
		if ((globlohit&0xc000) != 49152)            //You're not on a sprite
		{
			goalz = globloz-(8<<8);
			if (posz[snum] >= goalz-(2<<8))
			{
				clipmove(&posx[snum],&posy[snum],&posz[snum],&cursectnum[snum],-TICSPERFRAME<<14,-TICSPERFRAME<<14,128L,4<<8,4<<8,0);

				if (slimesoundcnt[snum] >= 0)
				{
					slimesoundcnt[snum] -= TICSPERFRAME;
					while (slimesoundcnt[snum] < 0)
					{
						slimesoundcnt[snum] += 120;
						wsayfollow("slime.wav",4096L+(krand()&127)-64,256L,&posx[snum],&posy[snum],1);
					}
				}
			}
		}
	if (goalz < globhiz+(16<<8))   //ceiling&floor too close
		goalz = ((globloz+globhiz)>>1);
	//goalz += mousz;
	if (health[snum] >= 0)
	{
		if ((sync[snum].bits&1) > 0)                         //A (stand high)
		{
			if (flytime[snum] <= lockclock)
			{
				if (posz[snum] >= globloz-(32<<8))
				{
					goalz -= (16<<8);
					if (sync[snum].bits&256) goalz -= (24<<8);
				}
			}
			else
			{
				hvel[snum] -= 192;
				if (sync[snum].bits&256) hvel[snum] -= 192;
			}
		}
		if ((sync[snum].bits&2) > 0)                         //Z (stand low)
		{
			if (flytime[snum] <= lockclock)
			{
				goalz += (12<<8);
				if (sync[snum].bits&256) goalz += (12<<8);
			}
			else
			{
				hvel[snum] += 192;
				if (sync[snum].bits&256) hvel[snum] += 192;
			}
		}
	}

	if (flytime[snum] <= lockclock)
	{
		if (posz[snum] < goalz)
			hvel[snum] += (TICSPERFRAME<<4);
		else
			hvel[snum] = (((goalz-posz[snum])*TICSPERFRAME)>>5);
	}
	else
	{
		hvel[snum] -= (hvel[snum]>>2);
		hvel[snum] -= ksgn(hvel[snum]);
	}

	posz[snum] += hvel[snum];
	if (posz[snum] > globloz-(4<<8)) posz[snum] = globloz-(4<<8), hvel[snum] = 0;
	if (posz[snum] < globhiz+(4<<8)) posz[snum] = globhiz+(4<<8), hvel[snum] = 0;

	if (dimensionmode[snum] != 3)
	{
		if (((sync[snum].bits&32) > 0) && (zoom[snum] > 48)) zoom[snum] -= (zoom[snum]>>4);
		if (((sync[snum].bits&16) > 0) && (zoom[snum] < 4096)) zoom[snum] += (zoom[snum]>>4);
	}

		//Update sprite representation of player
		//   -should be after movement, but before shooting code
	setsprite(playersprite[snum],posx[snum],posy[snum],posz[snum]+(32<<8));
	sprite[playersprite[snum]].ang = ang[snum];

	if (health[snum] >= 0)
	{
		if ((cursectnum[snum] < 0) || (cursectnum[snum] >= numsectors))
		{       //How did you get in the wrong sector?
			wsayfollow("ouch.wav",4096L+(krand()&127)-64,64L,&posx[snum],&posy[snum],1);
			changehealth(snum,-TICSPERFRAME);
		}
		else if (globhiz+(8<<8) > globloz)
		{       //Ceiling and floor are smooshing you!
			wsayfollow("ouch.wav",4096L+(krand()&127)-64,64L,&posx[snum],&posy[snum],1);
			changehealth(snum,-TICSPERFRAME);
		}
	}

	if ((waterfountainwall[snum] >= 0) && (health[snum] >= 0))
		if ((wall[neartagwall].lotag != 7) || ((sync[snum].bits&1024) == 0))
		{
			i = waterfountainwall[snum];
			if (wall[i].overpicnum == USEWATERFOUNTAIN)
				wall[i].overpicnum = WATERFOUNTAIN;
			else if (wall[i].picnum == USEWATERFOUNTAIN)
				wall[i].picnum = WATERFOUNTAIN;

			waterfountainwall[snum] = -1;
		}

	if ((sync[snum].bits&1024) > 0)  //Space bar
	{
			//Continuous triggers...

		neartag(posx[snum],posy[snum],posz[snum],cursectnum[snum],ang[snum],&neartagsector,&neartagwall,&neartagsprite,&neartaghitdist,1024L,3);
		if (neartagsector == -1)
		{
			i = cursectnum[snum];
			if ((sector[i].lotag|sector[i].hitag) != 0)
				neartagsector = i;
		}

		if (wall[neartagwall].lotag == 7)  //Water fountain
		{
			if (wall[neartagwall].overpicnum == WATERFOUNTAIN)
			{
				wsayfollow("water.wav",4096L+(krand()&127)-64,256L,&posx[snum],&posy[snum],1);
				wall[neartagwall].overpicnum = USEWATERFOUNTAIN;
				waterfountainwall[snum] = neartagwall;
			}
			else if (wall[neartagwall].picnum == WATERFOUNTAIN)
			{
				wsayfollow("water.wav",4096L+(krand()&127)-64,256L,&posx[snum],&posy[snum],1);
				wall[neartagwall].picnum = USEWATERFOUNTAIN;
				waterfountainwall[snum] = neartagwall;
			}

			if (waterfountainwall[snum] >= 0)
			{
				waterfountaincnt[snum] -= TICSPERFRAME;
				while (waterfountaincnt[snum] < 0)
				{
					waterfountaincnt[snum] += 120;
					wsayfollow("water.wav",4096L+(krand()&127)-64,256L,&posx[snum],&posy[snum],1);
					changehealth(snum,2);
				}
			}
		}

			//1-time triggers...
		if ((oflags[snum]&1024) == 0)
		{
			if (neartagsector >= 0)
				if (sector[neartagsector].hitag == 0)
					operatesector(neartagsector);

			if (neartagwall >= 0)
				if (wall[neartagwall].lotag == 2)  //Switch
				{
					activatehitag(wall[neartagwall].hitag);

					j = wall[neartagwall].overpicnum;
					if (j == SWITCH1ON)                     //1-time switch
					{
						wall[neartagwall].overpicnum = GIFTBOX;
						wall[neartagwall].lotag = 0;
						wall[neartagwall].hitag = 0;
					}
					if (j == GIFTBOX)                       //1-time switch
					{
						wall[neartagwall].overpicnum = SWITCH1ON;
						wall[neartagwall].lotag = 0;
						wall[neartagwall].hitag = 0;
					}
					if (j == SWITCH2ON) wall[neartagwall].overpicnum = SWITCH2OFF;
					if (j == SWITCH2OFF) wall[neartagwall].overpicnum = SWITCH2ON;
					if (j == SWITCH3ON) wall[neartagwall].overpicnum = SWITCH3OFF;
					if (j == SWITCH3OFF) wall[neartagwall].overpicnum = SWITCH3ON;

					i = wall[neartagwall].point2;
					dax = ((wall[neartagwall].x+wall[i].x)>>1);
					day = ((wall[neartagwall].y+wall[i].y)>>1);
					wsayfollow("switch.wav",4096L+(krand()&255)-128,256L,&dax,&day,0);
				}

			if (neartagsprite >= 0)
			{
				if (sprite[neartagsprite].lotag == 1)
				{  //if you're shoving innocent little AL around, he gets mad!
					if (sprite[neartagsprite].picnum == AL)
					{
						sprite[neartagsprite].picnum = EVILAL;
						sprite[neartagsprite].cstat |= 2;   //Make him transluscent
						sprite[neartagsprite].xrepeat = 38;
						sprite[neartagsprite].yrepeat = 38;
						changespritestat(neartagsprite,10);
					}
				}
				if (sprite[neartagsprite].lotag == 4)
				{
					activatehitag(sprite[neartagsprite].hitag);

					j = sprite[neartagsprite].picnum;
					if (j == SWITCH1ON)                     //1-time switch
					{
						sprite[neartagsprite].picnum = GIFTBOX;
						sprite[neartagsprite].lotag = 0;
						sprite[neartagsprite].hitag = 0;
					}
					if (j == GIFTBOX)                       //1-time switch
					{
						sprite[neartagsprite].picnum = SWITCH1ON;
						sprite[neartagsprite].lotag = 0;
						sprite[neartagsprite].hitag = 0;
					}
					if (j == SWITCH2ON) sprite[neartagsprite].picnum = SWITCH2OFF;
					if (j == SWITCH2OFF) sprite[neartagsprite].picnum = SWITCH2ON;
					if (j == SWITCH3ON) sprite[neartagsprite].picnum = SWITCH3OFF;
					if (j == SWITCH3OFF) sprite[neartagsprite].picnum = SWITCH3ON;

					dax = sprite[neartagsprite].x;
					day = sprite[neartagsprite].y;
					wsayfollow("switch.wav",4096L+(krand()&255)-128,256L,&dax,&day,0);
				}
			}
		}
	}

	if ((sync[snum].bits&2048) > 0)      //Shoot a bullet
	{
		if ((numbombs[snum] == 0) && (((sync[snum].bits>>13)&7) == 2) && (myconnectindex == snum))
			locselectedgun = 1;

		if ((health[snum] >= 0) || ((krand()&127) > -health[snum]))
			switch((sync[snum].bits>>13)&7)
			{
				case 0:
					if ((oflags[snum]&2048) == 0)
						shootgun(snum,posx[snum],posy[snum],posz[snum],ang[snum],horiz[snum],cursectnum[snum],0);
					break;
				case 1:
					if (lockclock > lastchaingun[snum]+8)
					{
						lastchaingun[snum] = lockclock;
						shootgun(snum,posx[snum],posy[snum],posz[snum],ang[snum],horiz[snum],cursectnum[snum],1);
					}
					break;
				case 2:
					if ((oflags[snum]&2048) == 0)
						if (numbombs[snum] > 0)
						{
							shootgun(snum,posx[snum],posy[snum],posz[snum],ang[snum],horiz[snum],cursectnum[snum],2);
							numbombs[snum]--;
						}
					break;
			}
	}

	if ((sync[snum].bits&4096) > (oflags[snum]&4096))  //Keypad enter
	{
		dimensionmode[snum]++;
		if (dimensionmode[snum] > 3) dimensionmode[snum] = 1;
	}

	oflags[snum] = sync[snum].bits;
}

drawscreen(short snum, long dasmoothratio)
{
	long i, j, k, l, charsperline, templong;
	long x1, y1, x2, y2, ox1, oy1, ox2, oy2, dist, maxdist;
	long cposx, cposy, cposz, choriz, czoom, tposx, tposy;
	long tiltlock, *longptr, ovisibility, oparallaxvisibility;
	short cang, tang;
	char ch, *ptr, *ptr2, *ptr3, *ptr4;
	spritetype *tspr;

	smoothratio = max(min(dasmoothratio,65536),0);

	dointerpolations();

	cposx = oposx[snum]+mulscale16(posx[snum]-oposx[snum],smoothratio);
	cposy = oposy[snum]+mulscale16(posy[snum]-oposy[snum],smoothratio);
	cposz = oposz[snum]+mulscale16(posz[snum]-oposz[snum],smoothratio);
	choriz = ohoriz[snum]+mulscale16(horiz[snum]-ohoriz[snum],smoothratio);
	czoom = ozoom[snum]+mulscale16(zoom[snum]-ozoom[snum],smoothratio);
	cang = oang[snum]+mulscale16(((ang[snum]+1024-oang[snum])&2047)-1024,smoothratio);

	setears(cposx,cposy,(long)sintable[(cang+512)&2047]<<14,(long)sintable[cang&2047]<<14);

	if (typemode != 0)
	{
		charsperline = 40;
		//if (dimensionmode[snum] == 2) charsperline = 80;

		for(i=0;i<=typemessageleng;i+=charsperline)
		{
			for(j=0;j<charsperline;j++)
				tempbuf[j] = typemessage[i+j];
			if (typemessageleng < i+charsperline)
			{
				tempbuf[(typemessageleng-i)] = '_';
				tempbuf[(typemessageleng-i)+1] = 0;
			}
			else
				tempbuf[charsperline] = 0;
			//if (dimensionmode[snum] == 3)
				printext256(0L,(i/charsperline)<<3,183,-1,tempbuf,0);
			//else
			//   printext16(0L,((i/charsperline)<<3)+(pageoffset/640),10,-1,tempbuf,0);
		}
	}
	else
	{
		if (dimensionmode[myconnectindex] == 3)
		{
			templong = screensize;

			if (((loc.bits&32) > (screensizeflag&32)) && (screensize > 64))
			{
				ox1 = ((xdim-screensize)>>1);
				ox2 = ox1+screensize-1;
				oy1 = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
				oy2 = oy1 + scale(screensize,ydim-32,xdim)-1;
				screensize -= (screensize>>3);

				if (templong > xdim)
				{
					screensize = xdim;

					rotatesprite((xdim-320)<<15,(ydim-32)<<16,65536L,0,STATUSBAR,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
					i = ((xdim-320)>>1);
					while (i >= 8) i -= 8, rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL8,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
					if (i >= 4) i -= 4, rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL4,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
					i = ((xdim-320)>>1)+320;
					while (i <= xdim-8) rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL8,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L), i += 8;
					if (i <= xdim-4) rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL4,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L), i += 4;

					sprintf(&tempbuf,"Deaths: %d",deaths[screenpeek]);
					printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-16,tempbuf,ALPHABET,80);

					sprintf(&tempbuf,"Health: %3d",health[screenpeek]);
					printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-24,tempbuf,ALPHABET,80);
				}

				x1 = ((xdim-screensize)>>1);
				x2 = x1+screensize-1;
				y1 = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
				y2 = y1 + scale(screensize,ydim-32,xdim)-1;
				setview(x1,y1,x2,y2);

				// (ox1,oy1)ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
				//          ³  (x1,y1)        ³
				//          ³     ÚÄÄÄÄÄ¿     ³
				//          ³     ³     ³     ³
				//          ³     ÀÄÄÄÄÄÙ     ³
				//          ³        (x2,y2)  ³
				//          ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ(ox2,oy2)

				drawtilebackground(0L,0L,BACKGROUND,8,ox1,oy1,x1-1,oy2,0);
				drawtilebackground(0L,0L,BACKGROUND,8,x2+1,oy1,ox2,oy2,0);
				drawtilebackground(0L,0L,BACKGROUND,8,x1,oy1,x2,y1-1,0);
				drawtilebackground(0L,0L,BACKGROUND,8,x1,y2+1,x2,oy2,0);
			}
			if (((loc.bits&16) > (screensizeflag&16)) && (screensize <= xdim))
			{
				screensize += (screensize>>3);
				if ((screensize > xdim) && (templong == xdim))
				{
					screensize = xdim+1;
					x1 = 0; y1 = 0;
					x2 = xdim-1; y2 = ydim-1;
				}
				else
				{
					if (screensize > xdim) screensize = xdim;
					x1 = ((xdim-screensize)>>1);
					x2 = x1+screensize-1;
					y1 = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
					y2 = y1 + scale(screensize,ydim-32,xdim)-1;
				}
				setview(x1,y1,x2,y2);
			}
			screensizeflag = loc.bits;
		}
	}

	if (dimensionmode[snum] != 2)
	{
		if ((numplayers > 1) && (option[4] == 0))
		{
				//Do not draw other views constantly if they're staying still
				//It's a shame this trick will only work in screen-buffer mode
				//At least screen-buffer mode covers all the HI hi-res modes
			if (vidoption == 2)
			{
				for(i=connecthead;i>=0;i=connectpoint2[i]) frame2draw[i] = 0;
				frame2draw[snum] = 1;

					//2-1,3-1,4-2
					//5-2,6-2,7-2,8-3,9-3,10-3,11-3,12-4,13-4,14-4,15-4,16-5
				x1 = posx[snum]; y1 = posy[snum];
				for(j=(numplayers>>2)+1;j>0;j--)
				{
					maxdist = 0x80000000;
					for(i=connecthead;i>=0;i=connectpoint2[i])
						if (frame2draw[i] == 0)
						{
							x2 = posx[i]-x1; y2 = posy[i]-y1;
							dist = dmulscale12(x2,x2,y2,y2);

							if (dist < 64) dist = 16384;
							else if (dist > 16384) dist = 64;
							else dist = 1048576 / dist;

							dist *= frameskipcnt[i];

								//Increase frame rate if screen is moving
							if ((posx[i] != oposx[i]) || (posy[i] != oposy[i]) ||
								 (posz[i] != oposz[i]) || (ang[i] != oang[i]) ||
								 (horiz[i] != ohoriz[i])) dist += dist;

							if (dist > maxdist) maxdist = dist, k = i;
						}

					for(i=connecthead;i>=0;i=connectpoint2[i])
						frameskipcnt[i] += (frameskipcnt[i]>>3)+1;
					frameskipcnt[k] = 0;

					frame2draw[k] = 1;
				}
			}
			else
			{
				for(i=connecthead;i>=0;i=connectpoint2[i]) frame2draw[i] = 1;
			}

			for(i=connecthead,j=0;i>=0;i=connectpoint2[i],j++)
				if (frame2draw[i] != 0)
				{
					if (numplayers <= 4)
					{
						switch(j)
						{
							case 0: setview(0,0,(xdim>>1)-1,(ydim>>1)-1); break;
							case 1: setview((xdim>>1),0,xdim-1,(ydim>>1)-1); break;
							case 2: setview(0,(ydim>>1),(xdim>>1)-1,ydim-1); break;
							case 3: setview((xdim>>1),(ydim>>1),xdim-1,ydim-1); break;
						}
					}
					else
					{
						switch(j)
						{
							case 0: setview(0,0,(xdim>>2)-1,(ydim>>2)-1); break;
							case 1: setview(xdim>>2,0,(xdim>>1)-1,(ydim>>2)-1); break;
							case 2: setview(xdim>>1,0,xdim-(xdim>>2)-1,(ydim>>2)-1); break;
							case 3: setview(xdim-(xdim>>2),0,xdim-1,(ydim>>2)-1); break;
							case 4: setview(0,ydim>>2,(xdim>>2)-1,(ydim>>1)-1); break;
							case 5: setview(xdim>>2,ydim>>2,(xdim>>1)-1,(ydim>>1)-1); break;
							case 6: setview(xdim>>1,ydim>>2,xdim-(xdim>>2)-1,(ydim>>1)-1); break;
							case 7: setview(xdim-(xdim>>2),ydim>>2,xdim-1,(ydim>>1)-1); break;
							case 8: setview(0,ydim>>1,(xdim>>2)-1,ydim-(ydim>>2)-1); break;
							case 9: setview(xdim>>2,ydim>>1,(xdim>>1)-1,ydim-(ydim>>2)-1); break;
							case 10: setview(xdim>>1,ydim>>1,xdim-(xdim>>2)-1,ydim-(ydim>>2)-1); break;
							case 11: setview(xdim-(xdim>>2),ydim>>1,xdim-1,ydim-(ydim>>2)-1); break;
							case 12: setview(0,ydim-(ydim>>2),(xdim>>2)-1,ydim-1); break;
							case 13: setview(xdim>>2,ydim-(ydim>>2),(xdim>>1)-1,ydim-1); break;
							case 14: setview(xdim>>1,ydim-(ydim>>2),xdim-(xdim>>2)-1,ydim-1); break;
							case 15: setview(xdim-(xdim>>2),ydim-(ydim>>2),xdim-1,ydim-1); break;
						}
					}

					if (i == snum)
					{
						drawrooms(cposx,cposy,cposz,cang,choriz,cursectnum[i]);
						analyzesprites(cposx,cposy);
					}
					else
					{
						drawrooms(posx[i],posy[i],posz[i],ang[i],horiz[i],cursectnum[i]);
						analyzesprites(posx[i],posy[i]);
					}
					drawmasks();
					if (numbombs[i] > 0)
						rotatesprite(160<<16,184L<<16,65536,0,GUNONBOTTOM,sector[cursectnum[i]].floorshade,0,2,windowx1,windowy1,windowx2,windowy2);

					if (lockclock < 384)
					{
						if (lockclock < 128)
							rotatesprite(320<<15,200<<15,lockclock<<9,lockclock<<4,DEMOSIGN,(128-lockclock)>>2,0,1+2,windowx1,windowy1,windowx2,windowy2);
						else if (lockclock < 256)
							rotatesprite(320<<15,200<<15,65536,0,DEMOSIGN,0,0,2,windowx1,windowy1,windowx2,windowy2);
						else
							rotatesprite(320<<15,200<<15,(384-lockclock)<<9,lockclock<<4,DEMOSIGN,(lockclock-256)>>2,0,1+2,windowx1,windowy1,windowx2,windowy2);
					}

					if (health[i] <= 0)
						rotatesprite(320<<15,200<<15,(-health[i])<<11,(-health[i])<<5,NO,0,0,2,windowx1,windowy1,windowx2,windowy2);
				}
		}
		else
		{
				//Init for screen rotation
			tiltlock = screentilt;
			if ((tiltlock) || (detailmode))
			{
				walock[4094] = 255;
				if (waloff[4094] == 0)
					allocache(&waloff[4094],320L*320L,&walock[4094]);
				if ((tiltlock&1023) == 0)
					setviewtotile(4094,200L>>detailmode,320L>>detailmode);
				else
					setviewtotile(4094,320L>>detailmode,320L>>detailmode);
				if ((tiltlock&1023) == 512)
				{     //Block off unscreen section of 90ø tilted screen
					j = ((320-60)>>detailmode);
					for(i=(60>>detailmode)-1;i>=0;i--)
					{
						startumost[i] = 1; startumost[i+j] = 1;
						startdmost[i] = 0; startdmost[i+j] = 0;
					}
				}
			}

			if ((gotpic[FLOORMIRROR>>3]&(1<<(FLOORMIRROR&7))) > 0)
			{
				if (!chainstat)
				{
					dist = 0x7fffffff; i = 0;
					for(k=floormirrorcnt-1;k>=0;k--)
					{
						j = klabs(wall[sector[floormirrorsector[k]].wallptr].x-cposx);
						j += klabs(wall[sector[floormirrorsector[k]].wallptr].y-cposy);
						if (j < dist) dist = j, i = k;
					}

					j = floormirrorsector[i];

					drawrooms(cposx,cposy,(sector[j].floorz<<1)-cposz,cang,201-choriz,j);
					analyzesprites(cposx,cposy);
					drawmasks();

						//Temp horizon
					l = scale(choriz-100,windowx2-windowx1,320)+((windowy1+windowy2)>>1);
					for(y1=windowy1,y2=windowy2;y1<y2;y1++,y2--)
					{
						ptr = (char *)(frameplace+ylookup[y1]);
						ptr2 = (char *)(frameplace+ylookup[y2]);
						ptr3 = palookup[18];
						ptr3 += (min(klabs(y1-l)>>2,31)<<8);
						ptr4 = palookup[18];
						ptr4 += (min(klabs(y2-l)>>2,31)<<8);

						j = sintable[((y2+numframes)<<7)&2047];
						j += sintable[((y2-numframes)<<8)&2047];
						j >>= 14;

						ptr2 += j;

						for(x1=windowx1;x1<=windowx2;x1++)
							{ ch = ptr[x1]; ptr[x1] = ptr3[ptr2[x1]]; ptr2[x1] = ptr4[ch]; }
					}
					gotpic[FLOORMIRROR>>3] &= ~(1<<(FLOORMIRROR&7));
				}
			}

				//Startdmost optimization for weapons
			if ((numbombs[screenpeek] > 0) && (windowx1 == 0) && (windowx2 == 319) && (yxaspect == 65536) && (tiltlock == 0))
			{
				x1 = 160L-(tilesizx[GUNONBOTTOM]>>1);
				y1 = 184L-(tilesizy[GUNONBOTTOM]>>1);
				for(i=0;i<tilesizx[GUNONBOTTOM];i++)
					startdmost[i+x1] = min(windowy2+1,gundmost[i]+y1);
			}

				//WARNING!  Assuming (MIRRORLABEL&31) = 0 and MAXMIRRORS = 64
			longptr = (long *)FP_OFF(gotpic[MIRRORLABEL>>3]);
			if (longptr[0]|longptr[1])
				for(i=MAXMIRRORS-1;i>=0;i--)
					if (gotpic[(i+MIRRORLABEL)>>3]&(1<<(i&7)))
					{
						gotpic[(i+MIRRORLABEL)>>3] &= ~(1<<(i&7));

							//Prepare drawrooms for drawing mirror and calculate reflected
							//position into tposx, tposy, and tang (tposz == cposz)
							//Must call preparemirror before drawrooms and
							//          completemirror after drawrooms
						preparemirror(cposx,cposy,cposz,cang,choriz,
									  mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);

						ovisibility = visibility;
						oparallaxvisibility = parallaxvisibility;
						visibility <<= 1;
						parallaxvisibility <<= 1;
						ptr = palookup[0]; palookup[0] = palookup[17]; palookup[17] = ptr;

						drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]);
						for(j=0,tspr=&tsprite[0];j<spritesortcnt;j++,tspr++)
							if ((tspr->cstat&48) == 0) tspr->cstat |= 4;
						analyzesprites(tposx,tposy);
						drawmasks();

						ptr = palookup[0]; palookup[0] = palookup[17]; palookup[17] = ptr;
						visibility = ovisibility;
						parallaxvisibility = oparallaxvisibility;

						completemirror();   //Reverse screen x-wise in this function

						break;
					}

				//Over the shoulder mode
			//cposx -= (sintable[(cang+512)&2047]>>4);
			//cposy -= (sintable[cang&2047]>>4);
			drawrooms(cposx,cposy,cposz,cang,choriz,cursectnum[snum]);
			analyzesprites(cposx,cposy);
			drawmasks();

			if ((gotpic[FLOORMIRROR>>3]&(1<<(FLOORMIRROR&7))) > 0)
				transarea += (windowx2-windowx1)*(windowy2-windowy1);

				//Finish for screen rotation
			if ((tiltlock) || (detailmode))
			{
				setviewback();
				rotatesprite(320<<15,200<<15,65536+(detailmode<<16),tiltlock+512,4094,0,0,2+4+64,windowx1,windowy1,windowx2,windowy2);
				walock[4094] = 1;
			}

			if (numbombs[screenpeek] > 0)
			{
					//Reset startdmost to bottom of screen
				if ((windowx1 == 0) && (windowx2 == 319) && (yxaspect == 65536) && (tiltlock == 0))
				{
					x1 = 160L-(tilesizx[GUNONBOTTOM]>>1); y1 = windowy2+1;
					for(i=0;i<tilesizx[GUNONBOTTOM];i++)
						startdmost[i+x1] = y1;
				}
				rotatesprite(160<<16,184L<<16,65536,0,GUNONBOTTOM,sector[cursectnum[screenpeek]].floorshade,0,2,windowx1,windowy1,windowx2,windowy2);
			}

			if (cachecount != 0)
			{
				rotatesprite((320-16)<<16,16<<16,32768,0,BUILDDISK,0,0,2+64,windowx1,windowy1,windowx2,windowy2);
				cachecount = 0;
			}

			if (lockclock < 384)
			{
				if (lockclock < 128)
					rotatesprite(320<<15,200<<15,lockclock<<9,lockclock<<4,DEMOSIGN,(128-lockclock)>>2,0,1+2,windowx1,windowy1,windowx2,windowy2);
				else if (lockclock < 256)
					rotatesprite(320<<15,200<<15,65536,0,DEMOSIGN,0,0,2,windowx1,windowy1,windowx2,windowy2);
				else
					rotatesprite(320<<15,200<<15,(384-lockclock)<<9,lockclock<<4,DEMOSIGN,(lockclock-256)>>2,0,1+2,windowx1,windowy1,windowx2,windowy2);
			}

			if (health[screenpeek] <= 0)
				rotatesprite(320<<15,200<<15,(-health[screenpeek])<<11,(-health[screenpeek])<<5,NO,0,0,2,windowx1,windowy1,windowx2,windowy2);
		}
	}

		//Only animate lava if its picnum is on screen
		//gotpic is a bit array where the tile number's bit is set
		//whenever it is drawn (ceilings, walls, sprites, etc.)
	if ((gotpic[SLIME>>3]&(1<<(SLIME&7))) > 0)
	{
		gotpic[SLIME>>3] &= ~(1<<(SLIME&7));
		if (waloff[SLIME] != 0) movelava((char *)waloff[SLIME]);
	}

	if ((show2dsector[cursectnum[snum]>>3]&(1<<(cursectnum[snum]&7))) == 0)
		searchmap(cursectnum[snum]);

	if (dimensionmode[snum] != 3)
	{
			//Move back pivot point
		i = scale(czoom,screensize,320);
		if (dimensionmode[snum] == 2)
		{
			clearview(0L);  //Clear screen to specified color
			drawmapview(cposx,cposy,i,cang);
		}
		drawoverheadmap(cposx,cposy,i,cang);
	}

	if (getmessageleng > 0)
	{
		charsperline = 40;
		//if (dimensionmode[snum] == 2) charsperline = 80;

		for(i=0;i<=getmessageleng;i+=charsperline)
		{
			for(j=0;j<charsperline;j++)
				tempbuf[j] = getmessage[i+j];
			if (getmessageleng < i+charsperline)
				tempbuf[(getmessageleng-i)] = 0;
			else
				tempbuf[charsperline] = 0;

			printext256(0L,((i/charsperline)<<3)+(200-32-8)-(((getmessageleng-1)/charsperline)<<3),151,-1,tempbuf,0);
		}
		if (totalclock > getmessagetimeoff)
			getmessageleng = 0;
	}
	if ((numplayers >= 2) && (screenpeek != myconnectindex))
	{
		j = 1;
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			if (i == screenpeek) break;
			j++;
		}
		sprintf(tempbuf,"(Player %ld's view)",j);
		printext256((xdim>>1)-(strlen(tempbuf)<<2),0,24,-1,tempbuf,0);
	}

	if (syncstat != 0) printext256(68L,84L,31,0,"OUT OF SYNC!",0);
	if (syncstate != 0) printext256(68L,92L,31,0,"Missed Network packet!",0);

	nextpage();

	if (keystatus[0x3f] > 0)   //F5
	{
		keystatus[0x3f] = 0;
		detailmode ^= 1;
	}
	if (keystatus[0x58] > 0)   //F12
	{
		keystatus[0x58] = 0;
		screencapture("captxxxx.pcx",keystatus[0x2a]|keystatus[0x36]);
	}
	if (stereofps != 0)  //Adjustments for red-blue / 120 crystal eyes modes
	{
		if ((keystatus[0x2a]|keystatus[0x36]) > 0)
		{
			if (keystatus[0x1a] > 0) stereopixelwidth--;   //Shift [
			if (keystatus[0x1b] > 0) stereopixelwidth++;   //Shift ]
		}
		else
		{
			if (keystatus[0x1a] > 0) stereowidth -= 512;   //[
			if (keystatus[0x1b] > 0) stereowidth += 512;   //]
		}
	}

	if (option[4] == 0)           //Single player only keys
	{
		if (keystatus[0xd2] > 0)   //Insert - Insert player
		{
			keystatus[0xd2] = 0;
			if (numplayers < MAXPLAYERS)
			{
				connectpoint2[numplayers-1] = numplayers;
				connectpoint2[numplayers] = -1;

				initplayersprite(numplayers);

				clearallviews(0L);  //Clear screen to specified color

				numplayers++;
			}
		}
		if (keystatus[0xd3] > 0)   //Delete - Delete player
		{
			keystatus[0xd3] = 0;
			if (numplayers > 1)
			{
				numplayers--;
				connectpoint2[numplayers-1] = -1;

				deletesprite(playersprite[numplayers]);
				playersprite[numplayers] = -1;

				if (myconnectindex >= numplayers) myconnectindex = 0;
				if (screenpeek >= numplayers) screenpeek = 0;

				if (numplayers < 2)
					setup3dscreen();
				else
					clearallviews(0L);  //Clear screen to specified color
			}
		}
		if (keystatus[0x46] > 0)   //Scroll Lock
		{
			keystatus[0x46] = 0;

			myconnectindex = connectpoint2[myconnectindex];
			if (myconnectindex < 0) myconnectindex = connecthead;
			screenpeek = myconnectindex;
		}
	}

	restoreinterpolations();
}

movethings()
{
	long i;

	gotlastpacketclock = totalclock;
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		baksync[movefifoend][i].fvel = fsync[i].fvel;
		baksync[movefifoend][i].svel = fsync[i].svel;
		baksync[movefifoend][i].avel = fsync[i].avel;
		baksync[movefifoend][i].bits = fsync[i].bits;
	}
	movefifoend = ((movefifoend+1)&(MOVEFIFOSIZ-1));
}

domovethings()
{
	short i, j, startwall, endwall;
	spritetype *spr;
	walltype *wal;
	point3d *ospr;

	nummoves++;

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		sync[i].fvel = baksync[movefifoplc][i].fvel;
		sync[i].svel = baksync[movefifoplc][i].svel;
		sync[i].avel = baksync[movefifoplc][i].avel;
		sync[i].bits = baksync[movefifoplc][i].bits;
	}
	movefifoplc = ((movefifoplc+1)&(MOVEFIFOSIZ-1));

	if (option[4] != 0)
	{
		syncval[syncvalhead] = getsyncstat();
		syncvalhead = ((syncvalhead+1)&(MOVEFIFOSIZ-1));
	}

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		oposx[i] = posx[i];
		oposy[i] = posy[i];
		oposz[i] = posz[i];
		ohoriz[i] = horiz[i];
		ozoom[i] = zoom[i];
		oang[i] = ang[i];
	}

	for(i=NUMSTATS-1;i>=0;i--)
		if (statrate[i] >= 0)
			for(j=headspritestat[i];j>=0;j=nextspritestat[j])
				if (((nummoves-j)&statrate[i]) == 0)
					copybuf(&sprite[j].x,&osprite[j].x,3);

	for(i=connecthead;i>=0;i=connectpoint2[i])
		ocursectnum[i] = cursectnum[i];

	updateinterpolations();

	if ((numplayers <= 2) && (recstat == 1))
	{
		j = 0;
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			recsync[reccnt][j].fvel = sync[i].fvel;
			recsync[reccnt][j].svel = sync[i].svel;
			recsync[reccnt][j].avel = sync[i].avel;
			recsync[reccnt][j].bits = sync[i].bits;
			j++;
		}
		reccnt++; if (reccnt > 16383) reccnt = 16383;
	}

	lockclock += TICSPERFRAME;

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		processinput(i);                        //Move player

		checktouchsprite(i,cursectnum[i]);      //Pick up coins
		startwall = sector[cursectnum[i]].wallptr;
		endwall = startwall + sector[cursectnum[i]].wallnum;
		for(j=startwall,wal=&wall[j];j<endwall;j++,wal++)
			if (wal->nextsector >= 0) checktouchsprite(i,wal->nextsector);
	}

	doanimations();
	tagcode();            //Door code, moving sector code, other stuff
	statuslistcode();     //Monster / bullet code / explosions

	checkmasterslaveswitch();
}

getinput()
{
	char ch, keystate, *ptr;
	long i, j, k;
	short mousx, mousy, bstatus;

	if (typemode == 0)           //if normal game keys active
	{
		if ((keystatus[0x2a]&keystatus[0x36]&keystatus[0x13]) > 0)   //Sh.Sh.R (replay)
		{
			keystatus[0x13] = 0;
			playback();
		}

		if ((keystatus[0x26]&(keystatus[0x1d]|keystatus[0x9d])) > 0) //Load game
		{
			keystatus[0x26] = 0;
			loadgame();
		}

		if ((keystatus[0x1f]&(keystatus[0x1d]|keystatus[0x9d])) > 0) //Save game
		{
			keystatus[0x1f] = 0;
			savegame();
		}

		if (keystatus[keys[15]] > 0)
		{
			keystatus[keys[15]] = 0;

			screenpeek = connectpoint2[screenpeek];
			if (screenpeek < 0) screenpeek = connecthead;
		}

		for(i=7;i>=0;i--)
			if (keystatus[i+2] > 0)
				{ keystatus[i+2] = 0; locselectedgun = i; break; }
	}


		//KEYTIMERSTUFF
	if (keystatus[keys[5]] == 0)
	{
		if (keystatus[keys[2]] > 0) avel = max(avel-16*TICSPERFRAME,-128);
		if (keystatus[keys[3]] > 0) avel = min(avel+16*TICSPERFRAME,127);
	}
	else
	{
		if (keystatus[keys[2]] > 0) svel = min(svel+8*TICSPERFRAME,127);
		if (keystatus[keys[3]] > 0) svel = max(svel-8*TICSPERFRAME,-128);
	}
	if (keystatus[keys[0]] > 0) fvel = min(fvel+8*TICSPERFRAME,127);
	if (keystatus[keys[1]] > 0) fvel = max(fvel-8*TICSPERFRAME,-128);
	if (keystatus[keys[12]] > 0) svel = min(svel+8*TICSPERFRAME,127);
	if (keystatus[keys[13]] > 0) svel = max(svel-8*TICSPERFRAME,-128);

	if (avel < 0) avel = min(avel+12*TICSPERFRAME,0);
	if (avel > 0) avel = max(avel-12*TICSPERFRAME,0);
	if (svel < 0) svel = min(svel+2*TICSPERFRAME,0);
	if (svel > 0) svel = max(svel-2*TICSPERFRAME,0);
	if (fvel < 0) fvel = min(fvel+2*TICSPERFRAME,0);
	if (fvel > 0) fvel = max(fvel-2*TICSPERFRAME,0);

	if ((option[4] == 0) && (numplayers == 2))
	{
		if (keystatus[0x4f] == 0)
		{
			if (keystatus[0x4b] > 0) avel2 = max(avel2-16*TICSPERFRAME,-128);
			if (keystatus[0x4d] > 0) avel2 = min(avel2+16*TICSPERFRAME,127);
		}
		else
		{
			if (keystatus[0x4b] > 0) svel2 = min(svel2+8*TICSPERFRAME,127);
			if (keystatus[0x4d] > 0) svel2 = max(svel2-8*TICSPERFRAME,-128);
		}
		if (keystatus[0x48] > 0) fvel2 = min(fvel2+8*TICSPERFRAME,127);
		if (keystatus[0x4c] > 0) fvel2 = max(fvel2-8*TICSPERFRAME,-128);

		if (avel2 < 0) avel2 = min(avel2+12*TICSPERFRAME,0);
		if (avel2 > 0) avel2 = max(avel2-12*TICSPERFRAME,0);
		if (svel2 < 0) svel2 = min(svel2+2*TICSPERFRAME,0);
		if (svel2 > 0) svel2 = max(svel2-2*TICSPERFRAME,0);
		if (fvel2 < 0) fvel2 = min(fvel2+2*TICSPERFRAME,0);
		if (fvel2 > 0) fvel2 = max(fvel2-2*TICSPERFRAME,0);
	}

	if (keystatus[0x1a] != 0) screentilt += ((4*TICSPERFRAME)<<(keystatus[0x2a]|keystatus[0x36]));
	if (keystatus[0x1b] != 0) screentilt -= ((4*TICSPERFRAME)<<(keystatus[0x2a]|keystatus[0x36]));

	i = (TICSPERFRAME<<1);
	while ((screentilt != 0) && (i > 0))
		{ screentilt = ((screentilt+ksgn(screentilt-1024))&2047); i--; }
	if (keystatus[0x28] != 0) screentilt = 1536;


	loc.fvel = min(max(fvel,-128+8),127-8);
	loc.svel = min(max(svel,-128+8),127-8);
	loc.avel = min(max(avel,-128+16),127-16);

	getmousevalues(&mousx,&mousy,&bstatus);
	loc.avel = min(max(loc.avel+(mousx<<3),-128),127);
	loc.fvel = min(max(loc.fvel-(mousy<<3),-128),127);

	loc.bits = (locselectedgun<<13);
	if (typemode == 0)           //if normal game keys active
	{
		loc.bits |= (keystatus[0x32]<<9);                 //M (be master)
		loc.bits |= ((keystatus[keys[14]]==1)<<12);       //Map mode
	}
	loc.bits |= keystatus[keys[8]];                   //Stand high
	loc.bits |= (keystatus[keys[9]]<<1);              //Stand low
	loc.bits |= (keystatus[keys[16]]<<4);             //Zoom in
	loc.bits |= (keystatus[keys[17]]<<5);             //Zoom out
	loc.bits |= (keystatus[keys[4]]<<8);                 //Run
	loc.bits |= (keystatus[keys[10]]<<2);                //Look up
	loc.bits |= (keystatus[keys[11]]<<3);                //Look down
	loc.bits |= ((keystatus[keys[7]]==1)<<10);           //Space
	loc.bits |= ((keystatus[keys[6]]==1)<<11);           //Shoot
	loc.bits |= (((bstatus&6)>(oldmousebstatus&6))<<10); //Space
	loc.bits |= (((bstatus&1)>(oldmousebstatus&1))<<11); //Shoot

	oldmousebstatus = bstatus;
	if (((loc.bits&2048) > 0) && (locselectedgun == 1))
		oldmousebstatus &= ~1;     //Allow continous fire with mouse for chain gun

		//PRIVATE KEYS:
	if (keystatus[0xb7] > 0)  //Printscreen
	{
		keystatus[0xb7] = 0;
		printscreeninterrupt();
	}
	if (keystatus[0x57] > 0)  //F11 - brightness
	{
		keystatus[0x57] = 0;
		brightness++;
		if (brightness > 8) brightness = 0;
		setbrightness(brightness,(char *)&palette[0]);
	}

	if (typemode == 0)           //if normal game keys active
	{
		if (keystatus[0x19] > 0)  //P
		{
			keystatus[0x19] = 0;
			parallaxtype++;
			if (parallaxtype > 2) parallaxtype = 0;
		}
		if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
		{
			if (keystatus[0x4a] > 0)  // Keypad -
				visibility = min(visibility+(visibility>>3),16384);
			if (keystatus[0x4e] > 0)  // Keypad +
				visibility = max(visibility-(visibility>>3),128);
		}

		/*if ((option[1] == 1) && (option[4] >= 5))
		{
			if ((keystatus[0x13] > 0) && (recsnddone == 1) && (recording == -2))      //R (record)
			{
				wrec(32768L);
				recording = -1;
			}
			if ((recording == -1) && ((keystatus[0x13] == 0) || (recsnddone == 1)))
			{
				continueplay();
				recsnddone = 1;
				recording = 0;
				wsay("recordedvoice",2972L,255L,255L);
			}
			if ((recording >= 0) && (screenpeek != myconnectindex))
			{
				packbuf[0] = 3;

				ptr = (char *)(recsndoffs+recording);
				for(i=0;i<256;i++)
					packbuf[i+1] = *ptr++;
				recording += 256;

				sendpacket(screenpeek,packbuf,257L);
				if (recording >= 32768)
					recording = -2;
			}
		}*/

		if ((keystatus[keys[18]]) > 0)   //Typing mode
		{
			keystatus[keys[18]] = 0;
			typemode = 1;
			keyfifoplc = keyfifoend;      //Reset keyboard fifo
		}
	}
	else
	{
		while (keyfifoplc != keyfifoend)
		{
			ch = keyfifo[keyfifoplc];
			keystate = keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
			keyfifoplc = ((keyfifoplc+2)&(KEYFIFOSIZ-1));

			if (keystate != 0)
			{
				if (ch == 0xe)   //Backspace
				{
					if (typemessageleng == 0) { typemode = 0; break; }
					typemessageleng--;
				}
				if (ch == 0xf)
				{
					keystatus[0xf] = 0;
					typemode = 0;
					break;
				}
				if ((ch == 0x1c) || (ch == 0x9c))  //Either ENTER
				{
					keystatus[0x1c] = 0; keystatus[0x9c] = 0;
					if (typemessageleng > 0)
					{
						packbuf[0] = 2;          //Sending text is message type 4
						for(j=typemessageleng-1;j>=0;j--)
							packbuf[j+1] = typemessage[j];

						for(i=connecthead;i>=0;i=connectpoint2[i])
							if (i != myconnectindex)
								sendpacket(i,packbuf,typemessageleng+1);

						typemessageleng = 0;
					}
					typemode = 0;
					break;
				}

				if ((typemessageleng < 159) && (ch < 128))
				{
					if ((keystatus[0x2a]|keystatus[0x36]) != 0)
						ch = scantoascwithshift[ch];
					else
						ch = scantoasc[ch];

					if (ch != 0) typemessage[typemessageleng++] = ch;
				}
			}
		}
			//Here's a trick of making key repeat after a 1/2 second
		if (keystatus[0xe] > 0)
		{
			if (keystatus[0xe] < 30)
				keystatus[0xe] += TICSPERFRAME;
			else
			{
				if (typemessageleng == 0)
					typemode = 0;
				else
					typemessageleng--;
			}
		}
	}
}

initplayersprite(short snum)
{
	long i;

	if (playersprite[snum] >= 0) return;

	spawnsprite(playersprite[snum],posx[snum],posy[snum],posz[snum]+(32<<8),
		1+256,0,snum,32,64,64,0,0,DOOMGUY,ang[snum],0,0,0,snum+4096,
		cursectnum[snum],8,0,0,0);

	switch(snum)
	{
		case 1: for(i=0;i<32;i++) tempbuf[i+192] = i+128; break; //green->red
		case 2: for(i=0;i<32;i++) tempbuf[i+192] = i+32; break;  //green->blue
		case 3: for(i=0;i<32;i++) tempbuf[i+192] = i+224; break; //green->pink
		case 4: for(i=0;i<32;i++) tempbuf[i+192] = i+64; break;  //green->brown
		case 5: for(i=0;i<32;i++) tempbuf[i+192] = i+96; break;
		case 6: for(i=0;i<32;i++) tempbuf[i+192] = i+160; break;
		case 7: for(i=0;i<32;i++) tempbuf[i+192] = i+192; break;
		default: for(i=0;i<256;i++) tempbuf[i] = i; break;
	}
	makepalookup(snum,tempbuf,0,0,0,1);
}

playback()
{
	long i, j, k;

	ready2send = 0;
	recstat = 0; i = reccnt;
	while (keystatus[1] == 0)
	{
		while (totalclock >= lockclock+TICSPERFRAME)
		{
			if (i >= reccnt)
			{
				prepareboard(boardfilename);
				for(i=connecthead;i>=0;i=connectpoint2[i])
					initplayersprite((short)i);
				resettiming(); ototalclock = 0; gotlastpacketclock = 0;
				i = 0;
			}

			k = 0;
			for(j=connecthead;j>=0;j=connectpoint2[j])
			{
				fsync[j].fvel = recsync[i][k].fvel;
				fsync[j].svel = recsync[i][k].svel;
				fsync[j].avel = recsync[i][k].avel;
				fsync[j].bits = recsync[i][k].bits;
				k++;
			}
			movethings(); domovethings();
			i++;
		}
		drawscreen(screenpeek,(totalclock-lockclock)*(65536/TICSPERFRAME));

		if (keystatus[keys[15]] > 0)
		{
			keystatus[keys[15]] = 0;

			screenpeek = connectpoint2[screenpeek];
			if (screenpeek < 0) screenpeek = connecthead;
		}
		if (keystatus[keys[14]] > 0)
		{
			keystatus[keys[14]] = 0;
			dimensionmode[screenpeek]++;
			if (dimensionmode[screenpeek] > 3) dimensionmode[screenpeek] = 1;
		}
	}

	musicoff();
	uninitmultiplayers();
	uninittimer();
	uninitkeys();
	uninitengine();
	uninitsb();
	uninitgroupfile();
	setvmode(0x3);        //Set back to text mode
	exit(0);
}

setup3dscreen()
{
	long i, dax, day, dax2, day2;

	setgamemode();

	  //Make that ugly pink into black in case it ever shows up!
	outp(0x3c8,255); outp(0x3c9,0); outp(0x3c9,0); outp(0x3c9,0);

	if (screensize > xdim)
	{
		dax = 0; day = 0;
		dax2 = xdim-1; day2 = ydim-1;
	}
	else
	{
		dax = ((xdim-screensize)>>1);
		dax2 = dax+screensize-1;
		day = (((ydim-32)-scale(screensize,ydim-32,xdim))>>1);
		day2 = day + scale(screensize,ydim-32,xdim)-1;
		setview(dax,day,dax2,day2);
	}

	if (screensize < xdim)
		drawtilebackground(0L,0L,BACKGROUND,8,0L,0L,xdim-1L,ydim-1L,0);      //Draw background

	if (screensize <= xdim)
	{
		rotatesprite((xdim-320)<<15,(ydim-32)<<16,65536L,0,STATUSBAR,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
		i = ((xdim-320)>>1);
		while (i >= 8) i -= 8, rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL8,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
		if (i >= 4) i -= 4, rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL4,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L);
		i = ((xdim-320)>>1)+320;
		while (i <= xdim-8) rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL8,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L), i += 8;
		if (i <= xdim-4) rotatesprite(i<<16,(ydim-32)<<16,65536L,0,STATUSBARFILL4,0,0,8+16+64+128,0L,0L,xdim-1L,ydim-1L), i += 4;

		sprintf(&tempbuf,"Deaths: %d",deaths[screenpeek]);
		printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-16,tempbuf,ALPHABET,80);

		sprintf(&tempbuf,"Health: %3d",health[screenpeek]);
		printext((xdim>>1)-(strlen(tempbuf)<<2),ydim-24,tempbuf,ALPHABET,80);
	}
}

findrandomspot(long *x, long *y, short *sectnum)
{
	short startwall, endwall, s, dasector;
	long dax, day, daz, minx, maxx, miny, maxy, cnt;

	for(cnt=256;cnt>=0;cnt--)
	{
		do
		{
			dasector = mulscale16(krand(),numsectors);
		} while ((sector[dasector].ceilingz+(8<<8) >= sector[dasector].floorz) || ((sector[dasector].lotag|sector[dasector].hitag) != 0) || ((sector[dasector].floorstat&1) != 0));

		startwall = sector[dasector].wallptr;
		endwall = startwall+sector[dasector].wallnum;
		if (endwall <= startwall) continue;

		dax = 0L;
		day = 0L;
		minx = 0x7fffffff; maxx = 0x80000000;
		miny = 0x7fffffff; maxy = 0x80000000;

		for(s=startwall;s<endwall;s++)
		{
			dax += wall[s].x;
			day += wall[s].y;
			if (wall[s].x < minx) minx = wall[s].x;
			if (wall[s].x > maxx) maxx = wall[s].x;
			if (wall[s].y < miny) miny = wall[s].y;
			if (wall[s].y > maxy) maxy = wall[s].y;
		}

		if ((maxx-minx <= 256) || (maxy-miny <= 256)) continue;

		dax /= (endwall-startwall);
		day /= (endwall-startwall);

		if (inside(dax,day,dasector) == 0) continue;

		daz = sector[dasector].floorz-(32<<8);
		if (pushmove(&dax,&day,&daz,&dasector,128L,4<<8,4<<8,0) < 0) continue;

		*x = dax; *y = day; *sectnum = dasector;
		return;
	}
}

warp(long *x, long *y, long *z, short *daang, short *dasector)
{
	short startwall, endwall, s;
	long i, j, dax, day, ox, oy;

	ox = *x; oy = *y;

	for(i=0;i<warpsectorcnt;i++)
		if (warpsectorlist[i] == *dasector)
		{
			j = sector[*dasector].hitag;
			do
			{
				i++;
				if (i >= warpsectorcnt) i = 0;
			} while (sector[warpsectorlist[i]].hitag != j);
			*dasector = warpsectorlist[i];
			break;
		}

		//Find center of sector
	startwall = sector[*dasector].wallptr;
	endwall = startwall+sector[*dasector].wallnum;
	dax = 0L, day = 0L;
	for(s=startwall;s<endwall;s++)
	{
		dax += wall[s].x, day += wall[s].y;
		if (wall[s].nextsector >= 0)
			i = s;
	}
	*x = dax / (endwall-startwall);
	*y = day / (endwall-startwall);
	*z = sector[*dasector].floorz-(32<<8);
	updatesector(*x,*y,dasector);
	dax = ((wall[i].x+wall[wall[i].point2].x)>>1);
	day = ((wall[i].y+wall[wall[i].point2].y)>>1);
	*daang = getangle(dax-*x,day-*y);

	wsayfollow("warp.wav",3072L+(krand()&127)-64,192L,&ox,&oy,0);
	wsayfollow("warp.wav",4096L+(krand()&127)-64,256L,x,y,0);
}

warpsprite(short spritenum)
{
	short dasectnum;

	dasectnum = sprite[spritenum].sectnum;
	warp(&sprite[spritenum].x,&sprite[spritenum].y,&sprite[spritenum].z,
		  &sprite[spritenum].ang,&dasectnum);

	copybuf(&sprite[spritenum].x,&osprite[spritenum].x,3);
	changespritesect(spritenum,dasectnum);

	show2dsprite[spritenum>>3] &= ~(1<<(spritenum&7));
	if (show2dsector[dasectnum>>3]&(1<<(dasectnum&7)))
		show2dsprite[spritenum>>3] |= (1<<(spritenum&7));
}

initlava()
{
	long x, y, z, r;

	for(x=-16;x<=16;x++)
		for(y=-16;y<=16;y++)
		{
			r = ksqrt(x*x + y*y);
			lavaradx[r][lavaradcnt[r]] = x;
			lavarady[r][lavaradcnt[r]] = y;
			lavaradcnt[r]++;
		}

	for(z=0;z<16;z++)
		lavadropsizlookup[z] = 8 / (ksqrt(z)+1);

	for(z=0;z<LAVASIZ;z++)
		lavainc[z] = klabs((((z^17)>>4)&7)-4)+12;

	lavanumdrops = 0;
	lavanumframes = 0;
}

#pragma aux addlava =\
	"mov al, byte ptr [ebx-133]",\
	"mov dl, byte ptr [ebx-1]",\
	"add al, byte ptr [ebx-132]",\
	"add dl, byte ptr [ebx+131]",\
	"add al, byte ptr [ebx-131]",\
	"add dl, byte ptr [ebx+132]",\
	"add al, byte ptr [ebx+1]",\
	"add al, dl",\
	parm [ebx]\
	modify exact [eax edx]\

movelava(char *dapic)
{
	long i, j, x, y, z, zz, dalavadropsiz, dadropsizlookup;
	long dalavax, dalavay, *ptr, *ptr2;

	for(z=min(LAVAMAXDROPS-lavanumdrops-1,3);z>=0;z--)
	{
		lavadropx[lavanumdrops] = (rand()&(LAVASIZ-1));
		lavadropy[lavanumdrops] = (rand()&(LAVASIZ-1));
		lavadropsiz[lavanumdrops] = 1;
		lavanumdrops++;
	}

	for(z=lavanumdrops-1;z>=0;z--)
	{
		dadropsizlookup = lavadropsizlookup[lavadropsiz[z]]*(((z&1)<<1)-1);
		dalavadropsiz = lavadropsiz[z];
		dalavax = lavadropx[z]; dalavay = lavadropy[z];
		for(zz=lavaradcnt[lavadropsiz[z]]-1;zz>=0;zz--)
		{
			i = (((lavaradx[dalavadropsiz][zz]+dalavax)&(LAVASIZ-1))<<LAVALOGSIZ);
			i += ((lavarady[dalavadropsiz][zz]+dalavay)&(LAVASIZ-1));
			dapic[i] += dadropsizlookup;
			if (dapic[i] < 192) dapic[i] = 192;
		}

		lavadropsiz[z]++;
		if (lavadropsiz[z] > 10)
		{
			lavanumdrops--;
			lavadropx[z] = lavadropx[lavanumdrops];
			lavadropy[z] = lavadropy[lavanumdrops];
			lavadropsiz[z] = lavadropsiz[lavanumdrops];
		}
	}

		//Back up dapic with 1 pixel extra on each boundary
		//(to prevent anding for wrap-around)
	ptr = (long *)dapic;
	ptr2 = (long *)((LAVASIZ+4)+1+((long)lavabakpic));
	for(x=0;x<LAVASIZ;x++)
	{
		for(y=(LAVASIZ>>2);y>0;y--) *ptr2++ = ((*ptr++)&0x1f1f1f1f);
		ptr2++;
	}
	for(y=0;y<LAVASIZ;y++)
	{
		lavabakpic[y+1] = (dapic[y+((LAVASIZ-1)<<LAVALOGSIZ)]&31);
		lavabakpic[y+1+(LAVASIZ+1)*(LAVASIZ+4)] = (dapic[y]&31);
	}
	for(x=0;x<LAVASIZ;x++)
	{
		lavabakpic[(x+1)*(LAVASIZ+4)] = (dapic[(x<<LAVALOGSIZ)+(LAVASIZ-1)]&31);
		lavabakpic[(x+1)*(LAVASIZ+4)+(LAVASIZ+1)] = (dapic[x<<LAVALOGSIZ]&31);
	}
	lavabakpic[0] = (dapic[LAVASIZ*LAVASIZ-1]&31);
	lavabakpic[LAVASIZ+1] = (dapic[LAVASIZ*(LAVASIZ-1)]&31);
	lavabakpic[(LAVASIZ+4)*(LAVASIZ+1)] = (dapic[LAVASIZ-1]&31);
	lavabakpic[(LAVASIZ+4)*(LAVASIZ+2)-1] = (dapic[0]&31);

	ptr = (long *)dapic;
	for(x=0;x<LAVASIZ;x++)
	{
		i = (long)&lavabakpic[(x+1)*(LAVASIZ+4)+1];
		j = i+LAVASIZ;
		for(y=i;y<j;y+=4)
		{
			*ptr++ = ((addlava(y+0)&0xf8)>>3)+
						((addlava(y+1)&0xf8)<<5)+
						((addlava(y+2)&0xf8)<<13)+
						((addlava(y+3)&0xf8)<<21)+
						0xc2c2c2c2;
		}
	}

	lavanumframes++;
}

doanimations()
{
	long i, j;

	for(i=animatecnt-1;i>=0;i--)
	{
		j = *animateptr[i];

		if (j < animategoal[i])
			j = min(j+animatevel[i]*TICSPERFRAME,animategoal[i]);
		else
			j = max(j-animatevel[i]*TICSPERFRAME,animategoal[i]);
		animatevel[i] += animateacc[i];

		*animateptr[i] = j;

		if (j == animategoal[i])
		{
			animatecnt--;
			if (i != animatecnt)
			{
				stopinterpolation(animateptr[i]);
				animateptr[i] = animateptr[animatecnt];
				animategoal[i] = animategoal[animatecnt];
				animatevel[i] = animatevel[animatecnt];
				animateacc[i] = animateacc[animatecnt];
			}
		}
	}
}

getanimationgoal(long animptr)
{
	long i;

	for(i=animatecnt-1;i>=0;i--)
		if (animptr == animateptr[i]) return(i);
	return(-1);
}

setanimation(long *animptr, long thegoal, long thevel, long theacc)
{
	long i, j;

	if (animatecnt >= MAXANIMATES) return(-1);

	j = animatecnt;
	for(i=animatecnt-1;i>=0;i--)
		if (animptr == animateptr[i])
			{ j = i; break; }

	setinterpolation(animptr);

	animateptr[j] = animptr;
	animategoal[j] = thegoal;
	animatevel[j] = thevel;
	animateacc[j] = theacc;
	if (j == animatecnt) animatecnt++;
	return(j);
}

checkmasterslaveswitch()
{
	long i, j;

	if (option[4] == 0) return;

	j = 0;
	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (sync[i].bits&512) j++;
	if (j != 1) return;

	i = connecthead;
	for(j=connectpoint2[i];j>=0;j=connectpoint2[j])
	{
		if (sync[j].bits&512)
		{
			connectpoint2[i] = connectpoint2[j];
			connectpoint2[j] = connecthead;
			connecthead = (short)j;

			oloc.fvel = loc.fvel+1;
			oloc.svel = loc.svel+1;
			oloc.avel = loc.avel+1;
			oloc.bits = loc.bits+1;
			for(i=0;i<MAXPLAYERS;i++)
			{
				osync[i].fvel = fsync[i].fvel+1;
				osync[i].svel = fsync[i].svel+1;
				osync[i].avel = fsync[i].avel+1;
				osync[i].bits = fsync[i].bits+1;
			}

			syncvalhead = othersyncvalhead = syncvaltail = 0L;
			totalclock = ototalclock = gotlastpacketclock = lockclock;

			j = 1;
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				if (myconnectindex == i) break;
				j++;
			}
			if (j == 1)
				strcpy(getmessage,"Player 1 (Master)");
			else
				sprintf(getmessage,"Player %ld (Slave)",j);
			getmessageleng = strlen(getmessage);
			getmessagetimeoff = totalclock+120;

			return;
		}
		i = j;
	}
}

inittimer()
{
	outp(0x43,54); outp(0x40,9942&255); outp(0x40,9942>>8);  //120 times/sec
	oldtimerhandler = _dos_getvect(0x8);
	_disable(); _dos_setvect(0x8, timerhandler); _enable();
}

uninittimer()
{
	outp(0x43,54); outp(0x40,255); outp(0x40,255);           //18.2 times/sec
	_disable(); _dos_setvect(0x8, oldtimerhandler); _enable();
}

void __interrupt __far timerhandler()
{
	totalclock++;
	_chain_intr(oldtimerhandler);
	outp(0x20,0x20);
}

initkeys()
{
	long i;

	keyfifoplc = 0; keyfifoend = 0;
	for(i=0;i<256;i++) keystatus[i] = 0;
	oldkeyhandler = _dos_getvect(0x9);
	_disable(); _dos_setvect(0x9, keyhandler); _enable();
}

uninitkeys()
{
	short *ptr;

	_dos_setvect(0x9, oldkeyhandler);
		//Turn off shifts to prevent stucks with quitting
	ptr = (short *)0x417; *ptr &= ~0x030f;
}

void __interrupt __far keyhandler()
{
	koutp(0x20,0x20);
	oldreadch = readch; readch = kinp(0x60);
	keytemp = kinp(0x61); koutp(0x61,keytemp|128); koutp(0x61,keytemp&127);
	if ((readch|1) == 0xe1) { extended = 128; return; }
	if (oldreadch != readch)
	{
		if ((readch&128) == 0)
		{
			keytemp = readch+extended;
			if (keystatus[keytemp] == 0)
			{
				keystatus[keytemp] = 1;
				keyfifo[keyfifoend] = keytemp;
				keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 1;
				keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
			}
		}
		else
		{
			keytemp = (readch&127)+extended;
			keystatus[keytemp] = 0;
			keyfifo[keyfifoend] = keytemp;
			keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 0;
			keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
		}
	}
	extended = 0;
}

testneighborsectors(short sect1, short sect2)
{
	short i, startwall, num1, num2;

	num1 = sector[sect1].wallnum;
	num2 = sector[sect2].wallnum;
	if (num1 < num2) //Traverse walls of sector with fewest walls (for speed)
	{
		startwall = sector[sect1].wallptr;
		for(i=num1-1;i>=0;i--)
			if (wall[i+startwall].nextsector == sect2)
				return(1);
	}
	else
	{
		startwall = sector[sect2].wallptr;
		for(i=num2-1;i>=0;i--)
			if (wall[i+startwall].nextsector == sect1)
				return(1);
	}
	return(0);
}

loadgame()
{
	long i;
	FILE *fil;

	if ((fil = fopen("save0000.gam","rb")) == 0) return(-1);
	fread(&numplayers,2,1,fil);
	fread(&myconnectindex,2,1,fil);
	fread(&connecthead,2,1,fil);
	fread(connectpoint2,MAXPLAYERS<<1,1,fil);

		//Make sure palookups get set, sprites will get overwritten later
	for(i=connecthead;i>=0;i=connectpoint2[i]) initplayersprite((short)i);

	fread(posx,MAXPLAYERS<<2,1,fil);
	fread(posy,MAXPLAYERS<<2,1,fil);
	fread(posz,MAXPLAYERS<<2,1,fil);
	fread(horiz,MAXPLAYERS<<2,1,fil);
	fread(zoom,MAXPLAYERS<<2,1,fil);
	fread(hvel,MAXPLAYERS<<2,1,fil);
	fread(ang,MAXPLAYERS<<1,1,fil);
	fread(cursectnum,MAXPLAYERS<<1,1,fil);
	fread(ocursectnum,MAXPLAYERS<<1,1,fil);
	fread(playersprite,MAXPLAYERS<<1,1,fil);
	fread(deaths,MAXPLAYERS<<1,1,fil);
	fread(lastchaingun,MAXPLAYERS<<2,1,fil);
	fread(health,MAXPLAYERS<<2,1,fil);
	fread(numbombs,MAXPLAYERS<<1,1,fil);
	fread(oflags,MAXPLAYERS<<1,1,fil);
	fread(dimensionmode,MAXPLAYERS,1,fil);
	fread(revolvedoorstat,MAXPLAYERS,1,fil);
	fread(revolvedoorang,MAXPLAYERS<<1,1,fil);
	fread(revolvedoorrotang,MAXPLAYERS<<1,1,fil);
	fread(revolvedoorx,MAXPLAYERS<<2,1,fil);
	fread(revolvedoory,MAXPLAYERS<<2,1,fil);

	fread(&numsectors,2,1,fil);
	fread(sector,sizeof(sectortype)*numsectors,1,fil);

	fread(&numwalls,2,1,fil);
	fread(wall,sizeof(walltype)*numwalls,1,fil);

		//Store all sprites (even holes) to preserve indeces
	fread(sprite,sizeof(spritetype)*MAXSPRITES,1,fil);
	fread(headspritesect,(MAXSECTORS+1)<<1,1,fil);
	fread(prevspritesect,MAXSPRITES<<1,1,fil);
	fread(nextspritesect,MAXSPRITES<<1,1,fil);
	fread(headspritestat,(MAXSTATUS+1)<<1,1,fil);
	fread(prevspritestat,MAXSPRITES<<1,1,fil);
	fread(nextspritestat,MAXSPRITES<<1,1,fil);

	fread(&fvel,4,1,fil);
	fread(&svel,4,1,fil);
	fread(&avel,4,1,fil);

	fread(&locselectedgun,4,1,fil);
	fread(&loc.fvel,1,1,fil);
	fread(&oloc.fvel,1,1,fil);
	fread(&loc.svel,1,1,fil);
	fread(&oloc.svel,1,1,fil);
	fread(&loc.avel,1,1,fil);
	fread(&oloc.avel,1,1,fil);
	fread(&loc.bits,2,1,fil);
	fread(&oloc.bits,2,1,fil);

	fread(&locselectedgun2,4,1,fil);
	fread(&loc2.fvel,sizeof(input),1,fil);

	fread(sync,sizeof(input)*MAXPLAYERS,1,fil);
	fread(osync,sizeof(input)*MAXPLAYERS,1,fil);

	fread(boardfilename,80,1,fil);
	fread(&screenpeek,2,1,fil);
	fread(&oldmousebstatus,2,1,fil);
	fread(&brightness,2,1,fil);
	fread(&neartagsector,2,1,fil);
	fread(&neartagwall,2,1,fil);
	fread(&neartagsprite,2,1,fil);
	fread(&lockclock,4,1,fil);
	fread(&neartagdist,4,1,fil);
	fread(&neartaghitdist,4,1,fil);

	fread(turnspritelist,16<<1,1,fil);
	fread(&turnspritecnt,2,1,fil);
	fread(warpsectorlist,16<<1,1,fil);
	fread(&warpsectorcnt,2,1,fil);
	fread(xpanningsectorlist,16<<1,1,fil);
	fread(&xpanningsectorcnt,2,1,fil);
	fread(ypanningwalllist,64<<1,1,fil);
	fread(&ypanningwallcnt,2,1,fil);
	fread(floorpanninglist,64<<1,1,fil);
	fread(&floorpanningcnt,2,1,fil);
	fread(dragsectorlist,16<<1,1,fil);
	fread(dragxdir,16<<1,1,fil);
	fread(dragydir,16<<1,1,fil);
	fread(&dragsectorcnt,2,1,fil);
	fread(dragx1,16<<2,1,fil);
	fread(dragy1,16<<2,1,fil);
	fread(dragx2,16<<2,1,fil);
	fread(dragy2,16<<2,1,fil);
	fread(dragfloorz,16<<2,1,fil);
	fread(&swingcnt,2,1,fil);
	fread(swingwall,(32*5)<<1,1,fil);
	fread(swingsector,32<<1,1,fil);
	fread(swingangopen,32<<1,1,fil);
	fread(swingangclosed,32<<1,1,fil);
	fread(swingangopendir,32<<1,1,fil);
	fread(swingang,32<<1,1,fil);
	fread(swinganginc,32<<1,1,fil);
	fread(swingx,(32*8)<<2,1,fil);
	fread(swingy,(32*8)<<2,1,fil);
	fread(revolvesector,4<<1,1,fil);
	fread(revolveang,4<<1,1,fil);
	fread(&revolvecnt,2,1,fil);
	fread(revolvex,(4*16)<<2,1,fil);
	fread(revolvey,(4*16)<<2,1,fil);
	fread(revolvepivotx,4<<2,1,fil);
	fread(revolvepivoty,4<<2,1,fil);
	fread(subwaytracksector,(4*128)<<1,1,fil);
	fread(subwaynumsectors,4<<1,1,fil);
	fread(&subwaytrackcnt,2,1,fil);
	fread(subwaystop,(4*8)<<2,1,fil);
	fread(subwaystopcnt,4<<2,1,fil);
	fread(subwaytrackx1,4<<2,1,fil);
	fread(subwaytracky1,4<<2,1,fil);
	fread(subwaytrackx2,4<<2,1,fil);
	fread(subwaytracky2,4<<2,1,fil);
	fread(subwayx,4<<2,1,fil);
	fread(subwaygoalstop,4<<2,1,fil);
	fread(subwayvel,4<<2,1,fil);
	fread(subwaypausetime,4<<2,1,fil);
	fread(waterfountainwall,MAXPLAYERS<<1,1,fil);
	fread(waterfountaincnt,MAXPLAYERS<<1,1,fil);
	fread(slimesoundcnt,MAXPLAYERS<<1,1,fil);

		//Warning: only works if all pointers are in sector structures!
	fread(animateptr,MAXANIMATES<<2,1,fil);
	for(i=MAXANIMATES-1;i>=0;i--)
		animateptr[i] = (long *)(animateptr[i]+((long)sector));
	fread(animategoal,MAXANIMATES<<2,1,fil);
	fread(animatevel,MAXANIMATES<<2,1,fil);
	fread(animateacc,MAXANIMATES<<2,1,fil);
	fread(&animatecnt,4,1,fil);

	fread(&totalclock,4,1,fil);
	fread(&numframes,4,1,fil);
	fread(&randomseed,4,1,fil);
	fread(&numpalookups,2,1,fil);

	fread(&visibility,4,1,fil);
	fread(&parallaxvisibility,4,1,fil);
	fread(&parallaxtype,1,1,fil);
	fread(&parallaxyoffs,4,1,fil);
	fread(pskyoff,MAXPSKYTILES<<1,1,fil);
	fread(&pskybits,2,1,fil);

	fclose(fil);

	for(i=connecthead;i>=0;i=connectpoint2[i]) initplayersprite((short)i);

	totalclock = lockclock;
	ototalclock = lockclock;

	strcpy(getmessage,"Game loaded.");
	getmessageleng = strlen(getmessage);
	getmessagetimeoff = totalclock+360+(getmessageleng<<4);
	return(0);
}

savegame()
{
	long i;
	FILE *fil;

	if ((fil = fopen("save0000.gam","wb")) == 0) return(-1);

	fwrite(&numplayers,2,1,fil);
	fwrite(&myconnectindex,2,1,fil);
	fwrite(&connecthead,2,1,fil);
	fwrite(connectpoint2,MAXPLAYERS<<1,1,fil);

	fwrite(posx,MAXPLAYERS<<2,1,fil);
	fwrite(posy,MAXPLAYERS<<2,1,fil);
	fwrite(posz,MAXPLAYERS<<2,1,fil);
	fwrite(horiz,MAXPLAYERS<<2,1,fil);
	fwrite(zoom,MAXPLAYERS<<2,1,fil);
	fwrite(hvel,MAXPLAYERS<<2,1,fil);
	fwrite(ang,MAXPLAYERS<<1,1,fil);
	fwrite(cursectnum,MAXPLAYERS<<1,1,fil);
	fwrite(ocursectnum,MAXPLAYERS<<1,1,fil);
	fwrite(playersprite,MAXPLAYERS<<1,1,fil);
	fwrite(deaths,MAXPLAYERS<<1,1,fil);
	fwrite(lastchaingun,MAXPLAYERS<<2,1,fil);
	fwrite(health,MAXPLAYERS<<2,1,fil);
	fwrite(numbombs,MAXPLAYERS<<1,1,fil);
	fwrite(oflags,MAXPLAYERS<<1,1,fil);
	fwrite(dimensionmode,MAXPLAYERS,1,fil);
	fwrite(revolvedoorstat,MAXPLAYERS,1,fil);
	fwrite(revolvedoorang,MAXPLAYERS<<1,1,fil);
	fwrite(revolvedoorrotang,MAXPLAYERS<<1,1,fil);
	fwrite(revolvedoorx,MAXPLAYERS<<2,1,fil);
	fwrite(revolvedoory,MAXPLAYERS<<2,1,fil);

	fwrite(&numsectors,2,1,fil);
	fwrite(sector,sizeof(sectortype)*numsectors,1,fil);

	fwrite(&numwalls,2,1,fil);
	fwrite(wall,sizeof(walltype)*numwalls,1,fil);

		//Store all sprites (even holes) to preserve indeces
	fwrite(sprite,sizeof(spritetype)*MAXSPRITES,1,fil);
	fwrite(headspritesect,(MAXSECTORS+1)<<1,1,fil);
	fwrite(prevspritesect,MAXSPRITES<<1,1,fil);
	fwrite(nextspritesect,MAXSPRITES<<1,1,fil);
	fwrite(headspritestat,(MAXSTATUS+1)<<1,1,fil);
	fwrite(prevspritestat,MAXSPRITES<<1,1,fil);
	fwrite(nextspritestat,MAXSPRITES<<1,1,fil);

	fwrite(&fvel,4,1,fil);
	fwrite(&svel,4,1,fil);
	fwrite(&avel,4,1,fil);

	fwrite(&locselectedgun,4,1,fil);
	fwrite(&loc.fvel,1,1,fil);
	fwrite(&oloc.fvel,1,1,fil);
	fwrite(&loc.svel,1,1,fil);
	fwrite(&oloc.svel,1,1,fil);
	fwrite(&loc.avel,1,1,fil);
	fwrite(&oloc.avel,1,1,fil);
	fwrite(&loc.bits,2,1,fil);
	fwrite(&oloc.bits,2,1,fil);

	fwrite(&locselectedgun2,4,1,fil);
	fwrite(&loc2.fvel,sizeof(input),1,fil);

	fwrite(sync,sizeof(input)*MAXPLAYERS,1,fil);
	fwrite(osync,sizeof(input)*MAXPLAYERS,1,fil);

	fwrite(boardfilename,80,1,fil);
	fwrite(&screenpeek,2,1,fil);
	fwrite(&oldmousebstatus,2,1,fil);
	fwrite(&brightness,2,1,fil);
	fwrite(&neartagsector,2,1,fil);
	fwrite(&neartagwall,2,1,fil);
	fwrite(&neartagsprite,2,1,fil);
	fwrite(&lockclock,4,1,fil);
	fwrite(&neartagdist,4,1,fil);
	fwrite(&neartaghitdist,4,1,fil);

	fwrite(turnspritelist,16<<1,1,fil);
	fwrite(&turnspritecnt,2,1,fil);
	fwrite(warpsectorlist,16<<1,1,fil);
	fwrite(&warpsectorcnt,2,1,fil);
	fwrite(xpanningsectorlist,16<<1,1,fil);
	fwrite(&xpanningsectorcnt,2,1,fil);
	fwrite(ypanningwalllist,64<<1,1,fil);
	fwrite(&ypanningwallcnt,2,1,fil);
	fwrite(floorpanninglist,64<<1,1,fil);
	fwrite(&floorpanningcnt,2,1,fil);
	fwrite(dragsectorlist,16<<1,1,fil);
	fwrite(dragxdir,16<<1,1,fil);
	fwrite(dragydir,16<<1,1,fil);
	fwrite(&dragsectorcnt,2,1,fil);
	fwrite(dragx1,16<<2,1,fil);
	fwrite(dragy1,16<<2,1,fil);
	fwrite(dragx2,16<<2,1,fil);
	fwrite(dragy2,16<<2,1,fil);
	fwrite(dragfloorz,16<<2,1,fil);
	fwrite(&swingcnt,2,1,fil);
	fwrite(swingwall,(32*5)<<1,1,fil);
	fwrite(swingsector,32<<1,1,fil);
	fwrite(swingangopen,32<<1,1,fil);
	fwrite(swingangclosed,32<<1,1,fil);
	fwrite(swingangopendir,32<<1,1,fil);
	fwrite(swingang,32<<1,1,fil);
	fwrite(swinganginc,32<<1,1,fil);
	fwrite(swingx,(32*8)<<2,1,fil);
	fwrite(swingy,(32*8)<<2,1,fil);
	fwrite(revolvesector,4<<1,1,fil);
	fwrite(revolveang,4<<1,1,fil);
	fwrite(&revolvecnt,2,1,fil);
	fwrite(revolvex,(4*16)<<2,1,fil);
	fwrite(revolvey,(4*16)<<2,1,fil);
	fwrite(revolvepivotx,4<<2,1,fil);
	fwrite(revolvepivoty,4<<2,1,fil);
	fwrite(subwaytracksector,(4*128)<<1,1,fil);
	fwrite(subwaynumsectors,4<<1,1,fil);
	fwrite(&subwaytrackcnt,2,1,fil);
	fwrite(subwaystop,(4*8)<<2,1,fil);
	fwrite(subwaystopcnt,4<<2,1,fil);
	fwrite(subwaytrackx1,4<<2,1,fil);
	fwrite(subwaytracky1,4<<2,1,fil);
	fwrite(subwaytrackx2,4<<2,1,fil);
	fwrite(subwaytracky2,4<<2,1,fil);
	fwrite(subwayx,4<<2,1,fil);
	fwrite(subwaygoalstop,4<<2,1,fil);
	fwrite(subwayvel,4<<2,1,fil);
	fwrite(subwaypausetime,4<<2,1,fil);
	fwrite(waterfountainwall,MAXPLAYERS<<1,1,fil);
	fwrite(waterfountaincnt,MAXPLAYERS<<1,1,fil);
	fwrite(slimesoundcnt,MAXPLAYERS<<1,1,fil);

		//Warning: only works if all pointers are in sector structures!
	for(i=MAXANIMATES-1;i>=0;i--)
		animateptr[i] = (long *)(animateptr[i]-((long)sector));
	fwrite(animateptr,MAXANIMATES<<2,1,fil);
	for(i=MAXANIMATES-1;i>=0;i--)
		animateptr[i] = (long *)(animateptr[i]+((long)sector));
	fwrite(animategoal,MAXANIMATES<<2,1,fil);
	fwrite(animatevel,MAXANIMATES<<2,1,fil);
	fwrite(animateacc,MAXANIMATES<<2,1,fil);
	fwrite(&animatecnt,4,1,fil);

	fwrite(&totalclock,4,1,fil);
	fwrite(&numframes,4,1,fil);
	fwrite(&randomseed,4,1,fil);
	fwrite(&numpalookups,2,1,fil);

	fwrite(&visibility,4,1,fil);
	fwrite(&parallaxvisibility,4,1,fil);
	fwrite(&parallaxtype,1,1,fil);
	fwrite(&parallaxyoffs,4,1,fil);
	fwrite(pskyoff,MAXPSKYTILES<<1,1,fil);
	fwrite(&pskybits,2,1,fil);

	fclose(fil);

	strcpy(getmessage,"Game saved.");
	getmessageleng = strlen(getmessage);
	getmessagetimeoff = totalclock+360+(getmessageleng<<4);
	return(0);
}

faketimerhandler()
{
	short other, packbufleng;
	long i, j, k, l;

	if ((totalclock < ototalclock+TICSPERFRAME) || (ready2send == 0)) return;
	ototalclock += TICSPERFRAME;

	getpackets(); if (getoutputcirclesize() >= 16) return;
	getinput();

		//MASTER (or 1 player game)
	if ((myconnectindex == connecthead) || (option[4] == 0))
	{
		fsync[myconnectindex].fvel = loc.fvel;
		fsync[myconnectindex].svel = loc.svel;
		fsync[myconnectindex].avel = loc.avel;
		fsync[myconnectindex].bits = loc.bits;

		if (option[4] != 0)
		{
			packbuf[0] = 0;
			j = ((numplayers+1)>>1)+1;
			for(k=1;k<j;k++) packbuf[k] = 0;
			k = (1<<3);
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				l = 0;
				if (fsync[i].fvel != osync[i].fvel) packbuf[j++] = fsync[i].fvel, l |= 1;
				if (fsync[i].svel != osync[i].svel) packbuf[j++] = fsync[i].svel, l |= 2;
				if (fsync[i].avel != osync[i].avel) packbuf[j++] = fsync[i].avel, l |= 4;
				if (fsync[i].bits != osync[i].bits)
				{
					packbuf[j++] = (fsync[i].bits&255);
					packbuf[j++] = ((fsync[i].bits>>8)&255);
					l |= 8;
				}
				packbuf[k>>3] |= (l<<(k&7));
				k += 4;

				osync[i].fvel = fsync[i].fvel;
				osync[i].svel = fsync[i].svel;
				osync[i].avel = fsync[i].avel;
				osync[i].bits = fsync[i].bits;
			}

			while (syncvalhead != syncvaltail)
			{
				packbuf[j] = (char)(syncval[syncvaltail]&255);
				packbuf[j+1] = (char)((syncval[syncvaltail]>>8)&255);
				j += 2;
				syncvaltail = ((syncvaltail+1)&(MOVEFIFOSIZ-1));
			}

			for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				sendpacket(i,packbuf,j);
		}
		else if (numplayers == 2)
		{
			if (keystatus[0xb5] > 0)
			{
				keystatus[0xb5] = 0;
				locselectedgun2++; if (locselectedgun2 >= 3) locselectedgun2 = 0;
			}

				//Second player on 1 computer mode
			loc2.fvel = min(max(fvel2,-128+8),127-8);
			loc2.svel = min(max(svel2,-128+8),127-8);
			loc2.avel = min(max(avel2,-128+16),127-16);
			loc2.bits = (locselectedgun2<<13);
			loc2.bits |= keystatus[0x45];                  //Stand high
			loc2.bits |= (keystatus[0x47]<<1);             //Stand low
			loc2.bits |= (1<<8);                           //Run
			loc2.bits |= (keystatus[0x49]<<2);             //Look up
			loc2.bits |= (keystatus[0x37]<<3);             //Look down
			loc2.bits |= (keystatus[0x50]<<10);            //Space
			loc2.bits |= (keystatus[0x52]<<11);            //Shoot

			other = connectpoint2[myconnectindex];
			if (other < 0) other = connecthead;

			fsync[other].fvel = loc2.fvel;
			fsync[other].svel = loc2.svel;
			fsync[other].avel = loc2.avel;
			fsync[other].bits = loc2.bits;
		}
		movethings();  //Move EVERYTHING (you too!)
	}
	else                        //I am a SLAVE
	{
		packbuf[0] = 1; packbuf[1] = 0; j = 2;
		if (loc.fvel != oloc.fvel) packbuf[j++] = loc.fvel, oloc.fvel = loc.fvel, packbuf[1] |= 1;
		if (loc.svel != oloc.svel) packbuf[j++] = loc.svel, oloc.svel = loc.svel, packbuf[1] |= 2;
		if (loc.avel != oloc.avel) packbuf[j++] = loc.avel, oloc.avel = loc.avel, packbuf[1] |= 4;
		if ((loc.bits^oloc.bits)&0x00ff) packbuf[j++] = (loc.bits&255), packbuf[1] |= 8;
		if ((loc.bits^oloc.bits)&0xff00) packbuf[j++] = ((loc.bits>>8)&255), packbuf[1] |= 16;
		oloc.bits = loc.bits;
		sendpacket(connecthead,packbuf,j);
	}
}

getpackets()
{
	long i, j, k, l;
	short other, packbufleng, movecnt;

	if (option[4] == 0) return;

	movecnt = 0;
	while ((packbufleng = getpacket(&other,packbuf)) > 0)
	{
		switch(packbuf[0])
		{
			case 0:  //[0] (receive master sync buffer)
				j = ((numplayers+1)>>1)+1; k = (1<<3);
				for(i=connecthead;i>=0;i=connectpoint2[i])
				{
					l = (packbuf[k>>3]>>(k&7));
					if (l&1) fsync[i].fvel = packbuf[j++];
					if (l&2) fsync[i].svel = packbuf[j++];
					if (l&4) fsync[i].avel = packbuf[j++];
					if (l&8)
					{
						fsync[i].bits = ((short)packbuf[j])+(((short)packbuf[j+1])<<8);
						j += 2;
					}
					k += 4;
				}
				while (j != packbufleng)
				{
					othersyncval[othersyncvalhead] = ((long)packbuf[j]);
					othersyncval[othersyncvalhead] += (((long)packbuf[j+1])<<8);
					j += 2;
					othersyncvalhead = ((othersyncvalhead+1)&(MOVEFIFOSIZ-1));
				}

				if ((syncvalhead != syncvaltail) && (othersyncvalhead != syncvaltail))
				{
					syncstat = 0;
					do
					{
						syncstat |= (syncval[syncvaltail]^othersyncval[syncvaltail]);
						syncvaltail = ((syncvaltail+1)&(MOVEFIFOSIZ-1));

					} while ((syncvalhead != syncvaltail) && (othersyncvalhead != syncvaltail));
				}

				movethings();        //Move all players and sprites
				movecnt++;
				break;
			case 1:  //[1] (receive slave sync buffer)
				j = 2; k = packbuf[1];
				if (k&1) fsync[other].fvel = packbuf[j++];
				if (k&2) fsync[other].svel = packbuf[j++];
				if (k&4) fsync[other].avel = packbuf[j++];
				if (k&8) fsync[other].bits = ((fsync[other].bits&0xff00)|((short)packbuf[j++]));
				if (k&16) fsync[other].bits = ((fsync[other].bits&0x00ff)|(((short)packbuf[j++])<<8));
				break;
			case 2:
				getmessageleng = packbufleng-1;
				for(j=getmessageleng-1;j>=0;j--) getmessage[j] = packbuf[j+1];
				getmessagetimeoff = totalclock+360+(getmessageleng<<4);
				break;
			case 3:
				wsay("getstuff.wav",4096L,63L,63L);
				break;
			case 5:
				playerreadyflag[other] = packbuf[1];
				if ((other == connecthead) && (packbuf[1] == 2))
					sendpacket(connecthead,packbuf,2);
				break;
			case 255:  //[255] (logout)
				keystatus[1] = 1;
				break;
		}
	}
	if ((myconnectindex != connecthead) && ((movecnt&1) == 0))
	{
		if (rand()&1) ototalclock += (TICSPERFRAME>>1);
					else ototalclock -= (TICSPERFRAME>>1);
	}
}

drawoverheadmap(long cposx, long cposy, long czoom, short cang)
{
	long i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
	long dax, day, cosang, sinang, xspan, yspan, sprx, spry;
	long xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
	long xvect, yvect, xvect2, yvect2;
	char col;
	walltype *wal, *wal2;
	spritetype *spr;

	xvect = sintable[(-cang)&2047] * czoom;
	yvect = sintable[(1536-cang)&2047] * czoom;
	xvect2 = mulscale16(xvect,yxaspect);
	yvect2 = mulscale16(yvect,yxaspect);

		//Draw red lines
	for(i=0;i<numsectors;i++)
	{
		startwall = sector[i].wallptr;
		endwall = sector[i].wallptr + sector[i].wallnum;

		z1 = sector[i].ceilingz; z2 = sector[i].floorz;

		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			k = wal->nextwall; if (k < 0) continue;

			if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;
			if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) continue;

			if (sector[wal->nextsector].ceilingz == z1)
				if (sector[wal->nextsector].floorz == z2)
					if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

			col = 152;

			if (dimensionmode[screenpeek] == 2)
			{
				if (sector[i].floorz != sector[i].ceilingz)
					if (sector[wal->nextsector].floorz != sector[wal->nextsector].ceilingz)
						if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0)
							if (sector[i].floorz == sector[wal->nextsector].floorz) continue;
				if (sector[i].floorpicnum != sector[wal->nextsector].floorpicnum) continue;
				if (sector[i].floorshade != sector[wal->nextsector].floorshade) continue;
				col = 12;
			}

			ox = wal->x-cposx; oy = wal->y-cposy;
			x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			wal2 = &wall[wal->point2];
			ox = wal2->x-cposx; oy = wal2->y-cposy;
			x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			drawline256(x1,y1,x2,y2,col);
		}
	}

		//Draw sprites
	k = playersprite[screenpeek];
	for(i=0;i<numsectors;i++)
		for(j=headspritesect[i];j>=0;j=nextspritesect[j])
			if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
			{
				spr = &sprite[j]; if (spr->cstat&0x8000) continue;
				col = 56;
				if (spr->cstat&1) col = 248;
				if (j == k) col = 31;

				k = statrate[spr->statnum];
				sprx = spr->x;
				spry = spr->y;
				if (k >= 0)
				{
					switch(k)
					{
						case 0: l = smoothratio; break;
						case 1: l = (smoothratio>>1)+(((nummoves-j)&1)<<15); break;
						case 3: l = (smoothratio>>2)+(((nummoves-j)&3)<<14); break;
						case 7: l = (smoothratio>>3)+(((nummoves-j)&7)<<13); break;
						case 15: l = (smoothratio>>4)+(((nummoves-j)&15)<<12); break;
					}
					sprx = osprite[j].x+mulscale16(sprx-osprite[j].x,l);
					spry = osprite[j].y+mulscale16(spry-osprite[j].y,l);
				}

				switch (spr->cstat&48)
				{
					case 0:
						ox = sprx-cposx; oy = spry-cposy;
						x1 = dmulscale16(ox,xvect,-oy,yvect);
						y1 = dmulscale16(oy,xvect2,ox,yvect2);

						if (dimensionmode[screenpeek] == 1)
						{
							ox = (sintable[(spr->ang+512)&2047]>>7);
							oy = (sintable[(spr->ang)&2047]>>7);
							x2 = dmulscale16(ox,xvect,-oy,yvect);
							y2 = dmulscale16(oy,xvect,ox,yvect);

							if (j == playersprite[screenpeek])
							{
								x2 = 0L;
								y2 = -(czoom<<5);
							}

							x3 = mulscale16(x2,yxaspect);
							y3 = mulscale16(y2,yxaspect);

							drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
							drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
							drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
											x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
						}
						else
						{
							if (((gotsector[i>>3]&(1<<(i&7))) > 0) && (czoom > 96))
							{
								daang = (spr->ang-cang)&2047;
								if (j == playersprite[screenpeek]) { x1 = 0; y1 = 0; daang = 0; }
								rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),mulscale16(czoom*spr->yrepeat,yxaspect),daang,spr->picnum,spr->shade,spr->pal,(spr->cstat&2)>>1,windowx1,windowy1,windowx2,windowy2);
							}
						}
						break;
					case 16:
						x1 = sprx; y1 = spry;
						tilenum = spr->picnum;
						xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)spr->xoffset);
						if ((spr->cstat&4) > 0) xoff = -xoff;
						k = spr->ang; l = spr->xrepeat;
						dax = sintable[k&2047]*l; day = sintable[(k+1536)&2047]*l;
						l = tilesizx[tilenum]; k = (l>>1)+xoff;
						x1 -= mulscale16(dax,k); x2 = x1+mulscale16(dax,l);
						y1 -= mulscale16(day,k); y2 = y1+mulscale16(day,l);

						ox = x1-cposx; oy = y1-cposy;
						x1 = dmulscale16(ox,xvect,-oy,yvect);
						y1 = dmulscale16(oy,xvect2,ox,yvect2);

						ox = x2-cposx; oy = y2-cposy;
						x2 = dmulscale16(ox,xvect,-oy,yvect);
						y2 = dmulscale16(oy,xvect2,ox,yvect2);

						drawline256(x1+(xdim<<11),y1+(ydim<<11),
										x2+(xdim<<11),y2+(ydim<<11),col);

						break;
					case 32:
						if (dimensionmode[screenpeek] == 1)
						{
							tilenum = spr->picnum;
							xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)spr->xoffset);
							yoff = (long)((signed char)((picanm[tilenum]>>16)&255))+((long)spr->yoffset);
							if ((spr->cstat&4) > 0) xoff = -xoff;
							if ((spr->cstat&8) > 0) yoff = -yoff;

							k = spr->ang;
							cosang = sintable[(k+512)&2047]; sinang = sintable[k];
							xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
							yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

							dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
							x1 = sprx + dmulscale16(sinang,dax,cosang,day);
							y1 = spry + dmulscale16(sinang,day,-cosang,dax);
							l = xspan*xrepeat;
							x2 = x1 - mulscale16(sinang,l);
							y2 = y1 + mulscale16(cosang,l);
							l = yspan*yrepeat;
							k = -mulscale16(cosang,l); x3 = x2+k; x4 = x1+k;
							k = -mulscale16(sinang,l); y3 = y2+k; y4 = y1+k;

							ox = x1-cposx; oy = y1-cposy;
							x1 = dmulscale16(ox,xvect,-oy,yvect);
							y1 = dmulscale16(oy,xvect2,ox,yvect2);

							ox = x2-cposx; oy = y2-cposy;
							x2 = dmulscale16(ox,xvect,-oy,yvect);
							y2 = dmulscale16(oy,xvect2,ox,yvect2);

							ox = x3-cposx; oy = y3-cposy;
							x3 = dmulscale16(ox,xvect,-oy,yvect);
							y3 = dmulscale16(oy,xvect2,ox,yvect2);

							ox = x4-cposx; oy = y4-cposy;
							x4 = dmulscale16(ox,xvect,-oy,yvect);
							y4 = dmulscale16(oy,xvect2,ox,yvect2);

							drawline256(x1+(xdim<<11),y1+(ydim<<11),
											x2+(xdim<<11),y2+(ydim<<11),col);

							drawline256(x2+(xdim<<11),y2+(ydim<<11),
											x3+(xdim<<11),y3+(ydim<<11),col);

							drawline256(x3+(xdim<<11),y3+(ydim<<11),
											x4+(xdim<<11),y4+(ydim<<11),col);

							drawline256(x4+(xdim<<11),y4+(ydim<<11),
											x1+(xdim<<11),y1+(ydim<<11),col);

						}
						break;
				}
			}

		//Draw white lines
	for(i=0;i<numsectors;i++)
	{
		startwall = sector[i].wallptr;
		endwall = sector[i].wallptr + sector[i].wallnum;

		k = -1;
		for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
		{
			if (wal->nextwall >= 0) continue;

			if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;

			if (tilesizx[wal->picnum] == 0) continue;
			if (tilesizy[wal->picnum] == 0) continue;

			if (j == k)
				{ x1 = x2; y1 = y2; }
			else
			{
				ox = wal->x-cposx; oy = wal->y-cposy;
				x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
				y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
			}

			k = wal->point2; wal2 = &wall[k];
			ox = wal2->x-cposx; oy = wal2->y-cposy;
			x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
			y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

			drawline256(x1,y1,x2,y2,24);
		}
	}
}

	//New movesprite using getzrange.  Note that I made the getzrange
	//parameters global (&globhiz,&globhihit,&globloz,&globlohit) so they
	//don't need to be passed everywhere.  Also this should make this
	//movesprite function compatible with the older movesprite functions.
movesprite(short spritenum, long dx, long dy, long dz, long ceildist, long flordist, char cliptype)
{
	long daz, zoffs, templong;
	short retval, dasectnum, datempshort;
	spritetype *spr;

	spr = &sprite[spritenum];

	if ((spr->cstat&128) == 0)
		zoffs = -((tilesizy[spr->picnum]*spr->yrepeat)<<1);
	else
		zoffs = 0;

	dasectnum = spr->sectnum;  //Can't modify sprite sectors directly becuase of linked lists
	daz = spr->z+zoffs;  //Must do this if not using the new centered centering (of course)
	retval = clipmove(&spr->x,&spr->y,&daz,&dasectnum,dx,dy,
							((long)spr->clipdist)<<2,ceildist,flordist,cliptype);

	if (dasectnum < 0) retval = -1;

	if ((dasectnum != spr->sectnum) && (dasectnum >= 0))
		changespritesect(spritenum,dasectnum);

		//Set the blocking bit to 0 temporarly so getzrange doesn't pick up
		//its own sprite
	datempshort = spr->cstat; spr->cstat &= ~1;
	getzrange(spr->x,spr->y,spr->z-1,spr->sectnum,
				 &globhiz,&globhihit,&globloz,&globlohit,
				 ((long)spr->clipdist)<<2,cliptype);
	spr->cstat = datempshort;

	daz = spr->z+zoffs + dz;
	if ((daz <= globhiz) || (daz > globloz))
	{
		if (retval != 0) return(retval);
		return(16384+dasectnum);
	}
	spr->z = daz-zoffs;
	return(retval);
}

waitforeverybody()
{
	long i, j, oldtotalclock;

	if (numplayers < 2) return;

	if (myconnectindex == connecthead)
	{
		for(j=1;j<=2;j++)
		{
			for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				playerreadyflag[i] = 0;
			oldtotalclock = totalclock-8;
			do
			{
				getpackets();
				if (totalclock >= oldtotalclock+8)
				{
					oldtotalclock = totalclock;
					packbuf[0] = 5;
					packbuf[1] = j;
					for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
						if (playerreadyflag[i] != j) sendpacket(i,packbuf,2);
				}
				for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
					if (playerreadyflag[i] != j) break;
			} while (i >= 0);
		}
	}
	else
	{
		playerreadyflag[connecthead] = 0;
		while (playerreadyflag[connecthead] != 2)
		{
			getpackets();
			if (playerreadyflag[connecthead] == 1)
			{
				playerreadyflag[connecthead] = 0;
				sendpacket(connecthead,packbuf,2);
			}
		}
	}
}

getsyncstat()
{
	long i, j;
	unsigned short crc;
	spritetype *spr;

	crc = 0;
	updatecrc16(crc,randomseed); updatecrc16(crc,randomseed>>8);
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		updatecrc16(crc,posx[i]); updatecrc16(crc,posx[i]>>8); updatecrc16(crc,posx[i]>>16); updatecrc16(crc,posx[i]>>24);
		updatecrc16(crc,posy[i]); updatecrc16(crc,posy[i]>>8); updatecrc16(crc,posy[i]>>16); updatecrc16(crc,posy[i]>>24);
		updatecrc16(crc,posz[i]); updatecrc16(crc,posz[i]>>8); updatecrc16(crc,posz[i]>>16); updatecrc16(crc,posz[i]>>24);
		updatecrc16(crc,ang[i]); updatecrc16(crc,ang[i]>>8);
		updatecrc16(crc,horiz[i]); updatecrc16(crc,horiz[i]>>8); updatecrc16(crc,horiz[i]>>16); updatecrc16(crc,horiz[i]>>24);
		updatecrc16(crc,health[i]); updatecrc16(crc,health[i]>>8); updatecrc16(crc,health[i]>>16); updatecrc16(crc,health[i]>>24);
		updatecrc16(crc,cursectnum[i]); updatecrc16(crc,cursectnum[i]>>8);
	}

	for(i=7;i>=0;i--)
		for(j=headspritestat[i];j>=0;j=nextspritestat[j])
		{
			spr = &sprite[j];
			updatecrc16(crc,spr->x); updatecrc16(crc,spr->x>>8); updatecrc16(crc,spr->x>>16); updatecrc16(crc,spr->x>>24);
			updatecrc16(crc,spr->y); updatecrc16(crc,spr->y>>8); updatecrc16(crc,spr->y>>16); updatecrc16(crc,spr->y>>24);
			updatecrc16(crc,spr->z); updatecrc16(crc,spr->z>>8); updatecrc16(crc,spr->z>>16); updatecrc16(crc,spr->z>>24);
			updatecrc16(crc,spr->ang); updatecrc16(crc,spr->ang>>8);
		}
	return(crc);
}

searchmap(short startsector)
{
	long i, j, dasect, splc, send, startwall, endwall;
	short dapic;
	walltype *wal;

	if ((startsector < 0) || (startsector >= numsectors)) return;
	for(i=0;i<(MAXSECTORS>>3);i++) show2dsector[i] = 0;
	for(i=0;i<(MAXWALLS>>3);i++) show2dwall[i] = 0;
	for(i=0;i<(MAXSPRITES>>3);i++) show2dsprite[i] = 0;

	automapping = 0;

		//Search your area recursively & set all show2dsector/show2dwalls
	tempshort[0] = startsector;
	show2dsector[startsector>>3] |= (1<<(startsector&7));
	dapic = sector[startsector].ceilingpicnum;
	if (waloff[dapic] == 0) loadtile(dapic);
	dapic = sector[startsector].floorpicnum;
	if (waloff[dapic] == 0) loadtile(dapic);
	for(splc=0,send=1;splc<send;splc++)
	{
		dasect = tempshort[splc];
		startwall = sector[dasect].wallptr;
		endwall = startwall + sector[dasect].wallnum;
		for(i=startwall,wal=&wall[startwall];i<endwall;i++,wal++)
		{
			show2dwall[i>>3] |= (1<<(i&7));
			dapic = wall[i].picnum;
			if (waloff[dapic] == 0) loadtile(dapic);
			dapic = wall[i].overpicnum;
			if (((dapic&0xfffff000) == 0) && (waloff[dapic] == 0)) loadtile(dapic);

			j = wal->nextsector;
			if ((j >= 0) && ((show2dsector[j>>3]&(1<<(j&7))) == 0))
			{
				show2dsector[j>>3] |= (1<<(j&7));

				dapic = sector[j].ceilingpicnum;
				if (waloff[dapic] == 0) loadtile(dapic);
				dapic = sector[j].floorpicnum;
				if (waloff[dapic] == 0) loadtile(dapic);

				tempshort[send++] = (short)j;
			}
		}

		for(i=headspritesect[dasect];i>=0;i=nextspritesect[i])
		{
			show2dsprite[i>>3] |= (1<<(i&7));
			dapic = sprite[i].picnum;
			if (waloff[dapic] == 0) loadtile(dapic);
		}
	}
}

setinterpolation(long *posptr)
{
	long i;

	if (numinterpolations >= MAXINTERPOLATIONS) return;
	for(i=numinterpolations-1;i>=0;i--)
		if (curipos[i] == posptr) return;
	curipos[numinterpolations] = posptr;
	oldipos[numinterpolations] = *posptr;
	numinterpolations++;
}

stopinterpolation(long *posptr)
{
	long i;

	for(i=numinterpolations-1;i>=startofdynamicinterpolations;i--)
		if (curipos[i] == posptr)
		{
			numinterpolations--;
			oldipos[i] = oldipos[numinterpolations];
			bakipos[i] = bakipos[numinterpolations];
			curipos[i] = curipos[numinterpolations];
		}
}

updateinterpolations()  //Stick at beginning of domovethings
{
	long i;

	for(i=numinterpolations-1;i>=0;i--) oldipos[i] = *curipos[i];
}

dointerpolations()       //Stick at beginning of drawscreen
{
	long i, j, odelta, ndelta;

	ndelta = 0; j = 0;
	for(i=numinterpolations-1;i>=0;i--)
	{
		bakipos[i] = *curipos[i];
		odelta = ndelta; ndelta = (*curipos[i])-oldipos[i];
		if (odelta != ndelta) j = mulscale16(ndelta,smoothratio);
		*curipos[i] = oldipos[i]+j;
	}
}

restoreinterpolations()  //Stick at end of drawscreen
{
	long i;

	for(i=numinterpolations-1;i>=0;i--) *curipos[i] = bakipos[i];
}

printext(long x, long y, char *buffer, short tilenum, char invisiblecol)
{
	long i;
	char ch;

	for(i=0;buffer[i]!=0;i++)
	{
		ch = buffer[i];
		rotatesprite((x-((8&15)<<3))<<16,(y-((8>>4)<<3))<<16,65536L,0,tilenum,0,0,8+16+64+128,x,y,x+7,y+7);
		rotatesprite((x-((ch&15)<<3))<<16,(y-((ch>>4)<<3))<<16,65536L,0,tilenum,0,0,8+16+128,x,y,x+7,y+7);
		x += 8;
	}
}

drawtilebackground (long thex, long they, short tilenum,
								  signed char shade, long cx1, long cy1,
								  long cx2, long cy2, char dapalnum)
{
	long x, y, xsiz, ysiz, tx1, ty1, tx2, ty2;

	xsiz = tilesizx[tilenum]; tx1 = cx1/xsiz; tx2 = cx2/xsiz;
	ysiz = tilesizy[tilenum]; ty1 = cy1/ysiz; ty2 = cy2/ysiz;

	for(x=tx1;x<=tx2;x++)
		for(y=ty1;y<=ty2;y++)
			rotatesprite(x*xsiz<<16,y*ysiz<<16,65536L,0,tilenum,shade,dapalnum,8+16+64+128,cx1,cy1,cx2,cy2);
}
