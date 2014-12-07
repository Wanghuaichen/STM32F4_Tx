#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Magic macro to check enum size
//#define ctassert(n,e) extern unsigned char n[(e)?0:-1]
#define ctassert(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]


#define TEMPSTRINGLENGTH 400 //This is the max dialog size (80 characters * 5 lines)
                             //We could reduce this to ~240 on the 128x64 screens
                             //But only after all sprintf are replaced with snprintf
                             //Maybe move this to target_defs.h
extern char tempstring[TEMPSTRINGLENGTH];


typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "target.h"
#include "std.h"

//FATFS is defined by target_defs.h
struct FAT {
    char a[FILE_SIZE];
};


extern volatile s16 Channels[NUM_OUT_CHANNELS];
extern const char DeviationVersion[33];

/* Temproary definition until we have real translation */
#define _tr_noop(x) x
#ifdef NO_LANGUAGE_SUPPORT
#define _tr(x) x
#else
const char *_tr(const char *str);
#endif

void CONFIG_ReadLang(u8 idx);
void CONFIG_EnableLanguage(int state);
int CONFIG_IniParse(const char* filename,
         int (*handler)(void*, const char*, const char*, const char*),
         void* user);
u8 CONFIG_IsModelChanged(void);
u8 CONFIG_SaveModelIfNeeded(void);
void CONFIG_SaveTxIfNeeded(void);
extern const char * const MODULE_NAME[TX_MODULE_LAST];

/* LCD primitive functions */
void LCD_Clear(unsigned int color);
    /* Strings */
void LCD_PrintCharXY(unsigned int x, unsigned int y, u32 c);
void LCD_PrintChar(u32 c);
void LCD_PrintStringXY(unsigned int x, unsigned int y, const char *str);
void LCD_PrintString(const char *str);
void LCD_SetXY(unsigned int x, unsigned int y);
void LCD_GetStringDimensions(const u8 *str, u16 *width, u16 *height);
void LCD_GetCharDimensions(u32 c, u16 *width, u16 *height);
u8 LCD_SetFont(unsigned int idx);
u8  LCD_GetFont(void);
void LCD_SetFontColor(u16 color);
    /* Graphics */
void LCD_DrawCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_FillCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 color);
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void LCD_DrawFastHLine(u16 x, u16 y, u16 w, u16 color);
void LCD_DrawDashedVLine(int16_t x, int16_t y, int16_t h, int16_t space, uint16_t color);
void LCD_DrawDashedHLine(int16_t x, int16_t y, int16_t w, int16_t space, uint16_t color);
void LCD_DrawRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_FillRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_DrawRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_FillRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_DrawTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_FillTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawWindowedImageFromFile(u16 x, u16 y, const char *file, s16 w, s16 h, u16 x_off, u16 y_off);
void LCD_DrawImageFromFile(u16 x, u16 y, const char *file);
u8 LCD_ImageIsTransparent(const char *file);
u8 LCD_ImageDimensions(const char *file, u16 *w, u16 *h);
void LCD_DrawUSBLogo(int lcd_width, int lcd_height);

/* Music */

/* Mixer functions */
void MIXER_CalcChannels(void);

/* GUI Pages */
void PAGE_Init(void);
void PAGE_Change(int dir);
void PAGE_Event(void);
int PAGE_DialogVisible(void);
void PAGE_ShowSafetyDialog(void);
void PAGE_CloseBindingDialog(void);
void PAGE_ShowBindingDialog(u8 update);
void PAGE_ShowLowBattDialog(void);
void PAGE_DisableSafetyDialog(u8 disable);
void PAGE_ShowResetPermTimerDialog(void *guiObject, void *data);
void PAGE_ShowInvalidModule(void);
void PAGE_ShowModuleDialog(const char **missing);
void PAGE_ShowWarning(const char *title, const char *str);
const char *PAGE_GetName(int idx);
int PAGE_GetNumPages(void);
int PAGE_GetStartPage(void);

/* Protocol */
#define PROTODEF(proto, module, map, init, name) proto,
enum Protocols {
    PROTOCOL_NONE,
    #include "protocol/protocol.h"
    PROTOCOL_COUNT,
};
#undef PROTODEF
extern const u8 *ProtocolChannelMap[PROTOCOL_COUNT];
extern const char * const ProtocolNames[PROTOCOL_COUNT];
#define PROTO_MAP_LEN 5

enum ModelType {
    MODELTYPE_HELI,
    MODELTYPE_PLANE,
    MODELTYPE_LAST,
};

enum TxPower {
    TXPOWER_100uW,
    TXPOWER_300uW,
    TXPOWER_1mW,
    TXPOWER_3mW,
    TXPOWER_10mW,
    TXPOWER_30mW,
    TXPOWER_100mW,
    TXPOWER_150mW,
    TXPOWER_LAST,
};

void PROTOCOL_Init(u8 force);
void PROTOCOL_Load(int no_dlg);
void PROTOCOL_DeInit(void);
u8 PROTOCOL_WaitingForSafe(void);
u64 PROTOCOL_CheckSafe(void);
u32 PROTOCOL_Binding(void);
u8 PROTOCOL_AutoBindEnabled(void);
void PROTOCOL_Bind(void);
void PROTOCOL_SetBindState(u32 msec);
int PROTOCOL_NumChannels(void);
u8 PROTOCOL_GetTelemCapability(void);
int PROTOCOL_DefaultNumChannels(void);
void PROTOCOL_CheckDialogs(void);
u32 PROTOCOL_CurrentID(void);
const char **PROTOCOL_GetOptions(void);
void PROTOCOL_SetOptions(void);
int PROTOCOL_GetTelemetryState(void);
int PROTOCOL_MapChannel(int input, int default_ch);
int PROTOCOL_HasModule(int idx);
int PROTOCOL_HasPowerAmp(int idx);
int PROTOCOL_SetSwitch(int module);
int PROTOCOL_SticksMoved(int init);
void PROTOCOL_InitModules(void);


/* Input */
const char *INPUT_SourceName(char *str, unsigned src);
const char *INPUT_SourceNameReal(char *str, unsigned src);
const char *INPUT_SourceNameAbbrevSwitch(char *str, unsigned src);
const char *INPUT_SourceNameAbbrevSwitchReal(char *str, unsigned src);
int INPUT_GetAbbrevSource(int origval, int newval, int dir);
int INPUT_SwitchPos(unsigned src);
int INPUT_NumSwitchPos(unsigned src);
int INPUT_GetFirstSwitch(int src);
int INPUT_SelectSource(int src, int dir, u8 *changed);
int INPUT_SelectAbbrevSource(int src, int dir);

const char *INPUT_MapSourceName(unsigned idx, unsigned *val);
const char *INPUT_ButtonName(unsigned src);

/* Misc */
void Delay(u32 count);
u32 Crc(const void *buffer, u32 size);
const char *utf8_to_u32(const char *str, u32 *ch);
int exact_atoi(const char *str); //Like atoi but will not decode a number followed by non-number
size_t strlcpy(char* dst, const char* src, size_t bufsize);
void tempstring_cpy(const char* src);
int fexists(const char *file);
u32 rand32_r(u32 *seed, u8 update); //LFSR based PRNG
u32 rand32(void); //LFSR based PRNG
extern volatile u8 priority_ready;
void medium_priority_cb(void);
void debug_timing(u32 type, int startend); //This is only defined if TIMING_DEBUG is defined
/* Battery */
#define BATTERY_CRITICAL 0x01
#define BATTERY_LOW      0x02
u8 BATTERY_Check(void);

/* Mixer mode */
typedef enum {
    MIXER_ADVANCED = 0,
    MIXER_STANDARD,
    MIXER_ALL,
} MixerMode;
void PAGE_ShowInvalidStandardMixerDialog(void *guiObj);
void STDMIXER_Preset(void);
void STDMIXER_SetChannelOrderByProtocol(void);
unsigned STDMIXER_ValidateTraditionModel(void);
const char *STDMIXER_ModeName(int mode);
void STDMIXER_InitSwitches(void);
void STDMIXER_SaveSwitches(void);
const char *GetElemName(int type);
const char *GetBoxSource(char *str, int src);
const char *GetBoxSourceReal(char *str, int src);
void RemapChannelsForProtocol(const u8 *oldmap);
#define PPMin_Mode(void) (Model.num_ppmin >> 6)
#endif