/*
User-modifiable configuration
In the near future, bring this to a menu, and load-edit them in SPIFFS
*/

//Default settings

extern float offset = 0.0044;  // device specific, in MHz. VHF: 0.0014 UHF: 0.0044
extern float frequency = 439.98750;

#define RICNUMBER 8 //Maximum number of RIC usable
#define RINGTONE 5 //Number of ringtones available
#define NOTENUMBER 8 //Number of tones per ringtone

#define STARTUPTONE 4 //Which tone to play when booting

#define ATONE 2730 //Frequencies in Hz of specific tones
#define BTONE 1005
#define CTONE 3201

struct ric{
    long ricvalue; //RIC adress itself
    char* name; //"Nickname"
    int ringtype; //TBD: ring "melody"
    bool placeholder1;
    bool placeholder2;
}

/*RICs the pager will respond to. As described by the struct above:
{RIC,"NAME",ringtone(see below),TBD,TBD}
*/
ric[RICNUMBER]={
        {216, "TIME", 0,0,0},
        {349440, "DAN", 3,0,0},
        {1067, "EMFCAMP", 2,0,0},
};

//"melodies", 130ms tones. Individual frequencies, 8 slots. 0 equals to a 130ms silence.
int beepTones[RINGTONE][NOTENUMBER]={
        {0,0,0,0,0,0,0,0},
        {ATONE,CTONE,ATONE,CTONE,ATONE,0,0,0},
        {CTONE,0,ATONE,0,BTONE,BTONE,0,ATONE},
        {ATONE,CTONE,CTONE,0,ATONE,CTONE,CTONE,0},
        {ATONE,0,0,CTONE,0,0,0,0}, //Startup ringtone
};
