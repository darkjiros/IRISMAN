/*
    (c) 2011 Hermes/Estwald <www.elotrolado.net>
    IrisManager (HMANAGER port) (c) 2011 D_Skywalk <http://david.dantoine.org>

    HMANAGER4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    HMANAGER4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HMANAGER4.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <lv2/process.h>
#include <sys/file.h>
#include <ppu-lv2.h>
#include <sys/stat.h>
#include <lv2/sysfs.h>

#include <sysutil/disc.h>

#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>
#include <jpgdec/jpgdec.h>

#include <io/pad.h>

#include <tiny3d.h>
#include "libfont2.h"
#include "language.h"
#include "syscall8.h"
#include "payload.h"
#include "payload341/payload_341.h"
#include "payload355/payload_355.h"
#include "payload355dex/payload_355dex.h"
#include "payload421/payload_421.h"
#include "payload421dex/payload_421dex.h"
#include "payload430/payload_430.h"
#include "payload431/payload_431.h"
#include "payload430dex/payload_430dex.h"
#include "payload440/payload_440.h"
#include "payload441/payload_441.h"
#include "payload441dex/payload_441dex.h"
#include "payload446/payload_446.h"
#include "payload446dex/payload_446dex.h"
#include "payload450/payload_450.h"
#include "payload450dex/payload_450dex.h"
#include "payload453/payload_453.h"
#include "payload455/payload_455.h"

#include "spu_soundmodule.bin.h" // load SPU Module
#include "spu_soundlib.h"

#include <gcmodplay.h>
#include "credits.h"
#include "main.h"

#include "ttf_render.h"
#include "gfx.h"
#include "utils.h"
#include "pad.h"

#include "ftp/ftp.h"
#include "ftp/functions.h"

#include "psx.h"
#include "file_manager.h"
#include "controlfan.h"
#include "sysregistry.h"
#include "osk_input.h"
#include "modules.h"

// include fonts

// font 2: 224 chr from 32 to 255, 16 x 32 pix 2 bit depth
#include "font_b.h"

#include "bluray_png_bin.h"
#include "dvd_png_bin.h"
#include "direct_png_bin.h"
#include "usb_png_bin.h"
#include "usb_png2_bin.h"
#include "missing_png_bin.h"
#include "ftp_png_bin.h"
#include "psone_png_bin.h"
#include "psoneiso_png_bin.h"
#include "pstwoiso_png_bin.h"
#include "pspiso_png_bin.h"
#include "folder_png_bin.h"
#include "file_png_bin.h"
#include "pkg_png_bin.h"
#include "self_png_bin.h"
#include "img_png_bin.h"
#include "iso_png_bin.h"
#include "background_jpg_bin.h"
#include "film_png_bin.h"
#include "retro_png_bin.h"

#include "music1_mod_bin.h"
#include "music2_mod_bin.h"
#include "music3_mod_bin.h"

#include "cricket_raw_bin.h"

#include "ps3_discless.h"

#include "event_threads.h"
#include "cobra.h"
#include "iso.h"

int use_cobra = 0;
int use_mamba = 0; // cobra app version

int signal_ntfs_mount = 0;

u64 restore_syscall8[2] = {0,0};

#define MODE_COVERFLOW  1

int cover_mode = 0;
int gui_mode = MODE_COVERFLOW; // GUI selector
int sort_mode = 0;

// grid config for gui1
int num_box = 12;
int cols = 4, rows = 3;

int bk_picture = 2;
int refresh_gui = 0;

int options_locked = 0; // to 1 when control parental < 9 and not 0 (Disabled)

#define MODE_NOBDVD      1
#define MODE_DISCLESS    2

int noBDVD = MODE_DISCLESS;
int stops_BDVD = 1;
int bdvd_is_usb = 0;

int is_ps3game_running = 0;

int mode_homebrew = 0;
int game_list_category = 0;
int homelaun = 0;
int ftp_inited = 0;

#define GIGABYTES 1073741824.0

#define LAUNCHMODE_TOCHECK -1
#define LAUNCHMODE_CHECKED -2
#define LAUNCHMODE_REFRESH -3
#define LAUNCHMODE_STARTED 0
static int autolaunch = LAUNCHMODE_TOCHECK;
static int psx_inserted = 0;

u64 syscall_base = 0ULL;

int (*lv2_unpatch_bdvdemu)(void) = NULL;
int (*lv2_patch_bdvdemu)(uint32_t flags) = NULL;
int (*lv2_patch_storage)(void) = NULL;
int (*lv2_unpatch_storage)(void) = NULL;

//Console id
u64 off_idps = 0;
u64 off_idps2 = 0;
u64 val_idps_part1 = 0;
u64 val_idps_part2 = 0;

u64 off_psid = 0;
u64 val_psid_part1 = 0;
u64 val_psid_part2 = 0;

char psid[33];
char console_id[33];
char default_console_id[33];

void draw_console_id_tools(float x, float y);
void get_console_id_eid5();

void set_console_id_lv2();
void get_console_id_lv2();
void get_console_id_val();
int get_console_id_keyb();
int is_valid_idps();

void set_psid_lv2();
void get_psid_lv2();
void get_psid_val();
int get_psid_keyb();
bool is_valid_psid();

bool is_hex_char(char c);

int save_spoofed_psid();
int load_spoofed_psid();

int save_spoofed_console_id();
int load_spoofed_console_id();

void UTF8_to_Ansi(char *utf8, char *ansi, int len); // from osk_input

u16 * ttf_texture;
int update_title_utf8 = 1;
u8 string_title_utf8[128] = "";
int width_title_utf8 = 0;

void enable_draw_background_pic1();
void draw_background_pic1();
void load_background_picture();

void mount_iso_game();

// music
#define MAX_SONGS 3

char * music[MAX_SONGS + MAX_SONGS] = {
            (char *) music1_mod_bin,
            (char *) music2_mod_bin,
            (char *) music3_mod_bin,
            "Song: stardust memories (1997)",
            "Song: elysium (1997)",
            "Song: Overture (2000)"
            };

int song_selected = 0;

// SPU
u32 spu = 0;
sysSpuImage spu_image;
#define SPU_SIZE(x) (((x)+127) & ~127)

MODPlay mod_track;

u64 frame_count = 0;

#define ROT_INC(x ,y , z) {x++; if(x > y) x = z;}
#define ROT_DEC(x ,y , z) {x--; if(x < y) x = z;}

int menu_screen = 0;
int mode_favourites = 1;

t_directories directories[MAX_DIRECTORIES];

int ndirectories = 0;

int currentdir = 0;
int currentgamedir = 0;
int int_currentdir = 0;

void unpatch_bdvdemu();
int patch_bdvdemu(u32 flags);
int move_origin_to_bdemubackup(char *path);
int move_bdemubackup_to_origin(u32 flags);

u8 * png_texture = NULL;

#define GET_ICON_FROM_ISO -666
#define ICON_LOAD_SUCCESS -6666

PngDatas Png_datas[MAX_PICTURES];
u32 Png_offset[MAX_PICTURES];
int Png_iscover[MAX_PICTURES];
int Png_index[MAX_PICTURES];

PngDatas Png_res[MAX_RESOURCES];
u32 Png_res_offset[MAX_RESOURCES];

char self_path[MAXPATHLEN] = "/"__MKDEF_MANAGER_FULLDIR__;

#define PATHS_SECTION "Paths"

char backgrounds_path[MAXPATHLEN];
char covers_path[MAXPATHLEN];
char retro_covers_path[MAXPATHLEN];
char updates_path[MAXPATHLEN];
char video_path[MAXPATHLEN];
char webman_path[MAXPATHLEN];
char psp_launcher_path[MAXPATHLEN];
char retroarch_path[MAXPATHLEN];
char ps2classic_path[MAXPATHLEN];

#define EXTENSIONS_SECTION "Extensions"

char video_extensions[300];
char audio_extensions[300];
char rom_extensions[300];
char browser_extensions[100];

#define ROMS_SECTION "RetroPaths"

char retro_root_path[ROMS_MAXPATHLEN];
char retro_snes_path[ROMS_MAXPATHLEN];
char retro_gba_path[ROMS_MAXPATHLEN];
char retro_gen_path[ROMS_MAXPATHLEN];
char retro_nes_path[ROMS_MAXPATHLEN];
char retro_mame_path[ROMS_MAXPATHLEN];
char retro_fba_path[ROMS_MAXPATHLEN];
char retro_doom_path[ROMS_MAXPATHLEN];
char retro_quake_path[ROMS_MAXPATHLEN];
char retro_pce_path[ROMS_MAXPATHLEN];
char retro_gb_path[ROMS_MAXPATHLEN];
char retro_gbc_path[ROMS_MAXPATHLEN];
char retro_atari_path[ROMS_MAXPATHLEN];
char retro_vb_path[ROMS_MAXPATHLEN];
char retro_nxe_path[ROMS_MAXPATHLEN];
char retro_wswam_path[ROMS_MAXPATHLEN];

#define GUI_SECTION "GUI"

u8 bShowPlayOverlay = 1;
u8 bShowUSBIcon = 1;
u8 bBackgroundGears = 1;
u8 bIconPulse = 1;
u8 bFileManager = 0;
u8 iTimeFormat = 1;
u8 bUnmountDevBlind = 1;
u8 bCachedGameList = 1;
u8 bShowPIC1 = 2;
u8 bLoadPIC1 = 0;
u8 bSkipPIC1 = 0;
u8 bShowVersion = 2;
u8 bSpoofVersion = 0;
u8 bHideCoverflowSortModeLabel = 0;
u16 iTimeoutByInactivity = 1;
u8 bshowpath = 0;

static u32 last_game_flag = 0;
static char last_game_id[64];
static int last_game_favourites = 0;
static int allow_restore_last_game = 1;

inline int get_currentdir(int i);

char temp_buffer[8192];

//void MSGBOX(char *text, char *text2) {sprintf(temp_buffer, "%s = %s", text, text2); DrawDialogOKTimer(temp_buffer, 5000.0f);}    //debug message
//void MSGBOX2(char *text, int i) {sprintf(temp_buffer, "%s = %i", text, i); DrawDialogOKTimer(temp_buffer, 5000.0f);}    //debug message

int LoadPNG(PngDatas *png, const char *filename)
{
    int ret;
    pngData png2;

    if(filename)
    {
        if(!strncmp(filename, "/ntfs", 5) || !strncmp(filename, "/ext", 4))
        {
            int file_size = 0;
            char *buff = LoadFile((char *) filename, &file_size);

            if(!buff) return FAILED;

            ret = pngLoadFromBuffer((const void *) buff, file_size, &png2);

            free(buff);
        }
        else
            ret = pngLoadFromFile(filename, &png2);
    }
    else
        ret = pngLoadFromBuffer((const void *) png->png_in, png->png_size, &png2);

    png->bmp_out = png2.bmp_out;
    png->wpitch  = png2.pitch;
    png->width   = png2.width;
    png->height  = png2.height;

    return ret;
}

int LoadJPG(JpgDatas *jpg, char *filename)
{
    int ret;

    jpgData jpg2;

    if(filename)
    {
        if(!strncmp(filename, "/ntfs", 5) || !strncmp(filename, "/ext", 4))
        {
            int file_size = 0;
            char *buff = LoadFile((char *) filename, &file_size);

            if(!buff) return FAILED;

            ret = jpgLoadFromBuffer((const void *) buff, file_size, &jpg2);

            free(buff);
        }
        else
            ret = jpgLoadFromFile(filename, &jpg2);
    }
    else ret = jpgLoadFromBuffer((const void *) jpg->jpg_in, jpg->jpg_size, &jpg2);

    jpg->bmp_out = jpg2.bmp_out;
    jpg->wpitch  = jpg2.pitch;
    jpg->width   = jpg2.width;
    jpg->height  = jpg2.height;

    return ret;
}

void Load_PNG_resources()
{
    int i;

    for(i = 0; i < MAX_RESOURCES; i++) Png_res[i].png_in = NULL;
    for(i = 0; i < MAX_PICTURES; i++) {Png_iscover[i] = 0; Png_index[i] = i;}

    // datas for PNG from memory

    Png_res[IMG_BLURAY_DISC].png_in   = (void *) bluray_png_bin;
    Png_res[IMG_BLURAY_DISC].png_size = bluray_png_bin_size;

    Png_res[IMG_USB_ICON].png_in   = (void *) usb_png_bin;
    Png_res[IMG_USB_ICON].png_size = usb_png_bin_size;

    Png_res[IMG_MISSING_ICON].png_in   = (void *) missing_png_bin;
    Png_res[IMG_MISSING_ICON].png_size = missing_png_bin_size;

    Png_res[IMG_DIRECT_ICON].png_in   = (void *) direct_png_bin;
    Png_res[IMG_DIRECT_ICON].png_size = direct_png_bin_size;

    Png_res[IMG_FTP_ICON].png_in   = (void *) ftp_png_bin;
    Png_res[IMG_FTP_ICON].png_size = ftp_png_bin_size;

    Png_res[IMG_PS1_DISC].png_in   = (void *) psone_png_bin;
    Png_res[IMG_PS1_DISC].png_size = psone_png_bin_size;

    Png_res[IMG_PS1_ISO].png_in   = (void *) psoneiso_png_bin;
    Png_res[IMG_PS1_ISO].png_size = psoneiso_png_bin_size;

    // file manager icons

    Png_res[IMG_FOLDER_ICON].png_in   = (void *) folder_png_bin;
    Png_res[IMG_FOLDER_ICON].png_size = folder_png_bin_size;

    Png_res[IMG_FILE_ICON].png_in   = (void *) file_png_bin;
    Png_res[IMG_FILE_ICON].png_size = file_png_bin_size;

    Png_res[IMG_PKG_ICON].png_in   = (void *) pkg_png_bin;
    Png_res[IMG_PKG_ICON].png_size = pkg_png_bin_size;

    Png_res[IMG_SELF_ICON].png_in   = (void *) self_png_bin;
    Png_res[IMG_SELF_ICON].png_size = self_png_bin_size;

    Png_res[IMG_IMAGE_ICON].png_in   = (void *) img_png_bin;
    Png_res[IMG_IMAGE_ICON].png_size = img_png_bin_size;

    Png_res[IMG_ISO_ICON].png_in   = (void *) iso_png_bin;
    Png_res[IMG_ISO_ICON].png_size = iso_png_bin_size;

    // end file manager icons

    Png_res[IMG_PS2_ISO].png_in   = (void *) pstwoiso_png_bin;
    Png_res[IMG_PS2_ISO].png_size = pstwoiso_png_bin_size;

    Png_res[IMG_USB_ICON2].png_in   = (void *) usb_png2_bin;
    Png_res[IMG_USB_ICON2].png_size = usb_png2_bin_size;

    Png_res[IMG_DVD_DISC].png_in   = (void *) dvd_png_bin;
    Png_res[IMG_DVD_DISC].png_size = dvd_png_bin_size;

    // load PNG from memory

    for(i = 0; i < 16; i++)
        if(Png_res[i].png_in != NULL) LoadPNG(&Png_res[i], NULL);

    // Default Background Picture
    Png_res[IMG_DEFAULT_BACKGROUND].png_in   = (void *) background_jpg_bin;
    Png_res[IMG_DEFAULT_BACKGROUND].png_size = background_jpg_bin_size;
    LoadJPG((JpgDatas *) &Png_res[IMG_DEFAULT_BACKGROUND], NULL);

    // UMD
    Png_res[IMG_PSP_ISO].png_in   = (void *) pspiso_png_bin;
    Png_res[IMG_PSP_ISO].png_size = pspiso_png_bin_size;
    LoadPNG(&Png_res[IMG_PSP_ISO], NULL);

    // retro
    Png_res[IMG_RETRO_ICON].png_in   = (void *) retro_png_bin;
    Png_res[IMG_RETRO_ICON].png_size = retro_png_bin_size;
    LoadPNG(&Png_res[IMG_RETRO_ICON], NULL);

    // film PNG
    Png_res[IMG_MOVIE_ICON].png_in   = (void *) film_png_bin;
    Png_res[IMG_MOVIE_ICON].png_size = film_png_bin_size;
    LoadPNG(&Png_res[IMG_MOVIE_ICON], NULL);
}

static PngDatas my_png_datas;

int LoadTexturePNG(char * filename, int index)
{

    u32 * texture_pointer2 = (u32 *) (png_texture + (index >= num_box ? num_box : Png_index[index]) * 4096 * 1024); // 4 MB reserved for PNG index
    //u32 * texture_pointer2 = (u32 *) (png_texture + (index >= BIG_PICT ? BIG_PICT : Png_index[index]) * 4096 * 1024); // 4 MB reserved for PNG index

    if(index == num_box || index == BIG_PICT) texture_pointer2 += 2048 * 1200; // reserves 2048 x 1200 x 4 for background picture

    // here you can add more textures using 'texture_pointer'. It is returned aligned to 16 bytes

    if(filename) memset(&my_png_datas, 0, sizeof(PngDatas));
    if(LoadPNG(&my_png_datas, filename) != SUCCESS) memset(&my_png_datas, 0, sizeof(PngDatas));

    my_png_datas.png_in = NULL;
    my_png_datas.png_size = 0;

    Png_offset[index] = 0;
    memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));

    if(my_png_datas.bmp_out)
    {
        if((index < num_box && my_png_datas.wpitch * my_png_datas.height > 4096 * 1024) ||
           (index > BIG_PICT && my_png_datas.wpitch * my_png_datas.height > 8192 * 1200))
        {   // too big!
            memset(texture_pointer2, 0, 64 * 64 * 4);
            my_png_datas.wpitch = 64 * 4;
            my_png_datas.height = my_png_datas.width = 64;
        }
        else
            memcpy(texture_pointer2, my_png_datas.bmp_out, my_png_datas.wpitch * my_png_datas.height);

        free(my_png_datas.bmp_out);

        my_png_datas.bmp_out= texture_pointer2;

        memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));
        Png_offset[index] = tiny3d_TextureOffset(my_png_datas.bmp_out);      // get the offset (RSX use offset instead address)

        return SUCCESS;
    }
    else
    {
         // fake PNG
        my_png_datas.bmp_out= texture_pointer2;

        int n;
        u32 * text = texture_pointer2;

        my_png_datas.width = my_png_datas.height = 64;

        my_png_datas.wpitch = my_png_datas.width * 4;

        for (n = 0; n < my_png_datas.width * my_png_datas.height; n++) *text++ = 0xff000000;

        memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));

        Png_offset[index] = tiny3d_TextureOffset(my_png_datas.bmp_out);
    }

    return FAILED;
}

int LoadTextureJPG(char * filename, int index)
{

    u32 * texture_pointer2 = (u32 *) (png_texture + (index >= num_box ? num_box : Png_index[index]) * 4096 * 1024); // 4 MB reserved for PNG index
    //u32 * texture_pointer2 = (u32 *) (png_texture + (index >= BIG_PICT ? BIG_PICT : Png_index[index]) * 4096 * 1024); // 4 MB reserved for PNG index

    if(index == num_box || index == BIG_PICT) texture_pointer2 += 2048 * 1200; // reserves 2048 x 1200 x 4 for background picture

    // here you can add more textures using 'texture_pointer'. It is returned aligned to 16 bytes

    memset(&my_png_datas, 0, sizeof(PngDatas));

    if(LoadJPG((JpgDatas *)&my_png_datas, filename) != SUCCESS) memset(&my_png_datas, 0, sizeof(PngDatas));

    Png_offset[index] = 0;
    memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));

    if(my_png_datas.bmp_out)
    {
        if((index < num_box && my_png_datas.wpitch * my_png_datas.height > 4096 * 1024) ||
           (index > BIG_PICT && my_png_datas.wpitch * my_png_datas.height > 8192 * 1200))
        {   // too big!
            memset(texture_pointer2, 0, 64 * 64 * 4);
            my_png_datas.wpitch = 64 * 4;
            my_png_datas.height = my_png_datas.width = 64;
        }
        else
            memcpy(texture_pointer2, my_png_datas.bmp_out, my_png_datas.wpitch * my_png_datas.height);

        free(my_png_datas.bmp_out);

        my_png_datas.bmp_out = texture_pointer2;

        memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));
        Png_offset[index] = tiny3d_TextureOffset(my_png_datas.bmp_out);      // get the offset (RSX use offset instead address)

        return SUCCESS;
    }
    else
    {
         // fake PNG
        my_png_datas.bmp_out= texture_pointer2;

        int n;
        u32 * text = texture_pointer2;

        my_png_datas.width = my_png_datas.height = 64;

        my_png_datas.wpitch = my_png_datas.width * 4;

        for (n = 0; n < my_png_datas.width * my_png_datas.height; n++) *text++ = 0xff000000;

        memcpy(&Png_datas[index], &my_png_datas, sizeof(PngDatas));

        Png_offset[index] = tiny3d_TextureOffset(my_png_datas.bmp_out);
    }

    return FAILED;
}

char path_name[MAXPATHLEN];

const char folder_mode[2][16] = {{"/"},{"/PS3_GAME/"}};

// no inline!
int get_icon(char * path, const int num_dir)
{
    struct stat s;
    char titleid[12];
    int is_update = (strlen(path) == 9) && (num_dir == 0);

    if((directories[num_dir].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
    {
        bool is_retro = (strstr(directories[num_dir].path_name, retro_root_path) != NULL);
        bool is_ps2_classic = !is_retro &&
                              (strstr(directories[num_dir].path_name, ps2classic_path) != NULL);

        if(cover_mode || is_retro || is_ps2_classic)
        {
            strcpy(path, directories[num_dir].path_name);

            if(is_ps2_classic && strlen(path) > 8)
            {
                path[strlen(path) - 8] = 0;

                int n = strlen(path);
                path[n] = 0; strcat(path, ".jpg");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".png");
                if(!stat(path, &s)) return 2;
                path[n] = 0; strcat(path, ".JPG");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".PNG");
                if(!stat(path, &s)) return 2;

                strcpy(path, directories[num_dir].path_name);
            }

            if(strlen(path) > 4)
            {
                path[strlen(path) - 4] = 0;

                int n = strlen(path);
                path[n] = 0; strcat(path, ".jpg");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".png");
                if(!stat(path, &s)) return 2;
                path[n] = 0; strcat(path, ".JPG");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".PNG");
                if(!stat(path, &s)) return 2;

                strcpy(path, directories[num_dir].path_name);
            }

            if(is_retro && strlen(path) > 3 && path[strlen(path) - 3] == '.')
            {
                path[strlen(path) - 3] = 0;

                int n = strlen(path);
                path[n] = 0; strcat(path, ".jpg");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".png");
                if(!stat(path, &s)) return 2;
                path[n] = 0; strcat(path, ".JPG");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".PNG");
                if(!stat(path, &s)) return 2;
            }
            else
            if(is_retro && strlen(path) > 5 && path[strlen(path) - 5] == '.')
            {
                path[strlen(path) - 5] = 0;

                int n = strlen(path);
                path[n] = 0; strcat(path, ".jpg");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".png");
                if(!stat(path, &s)) return 2;
                path[n] = 0; strcat(path, ".JPG");
                if(!stat(path, &s)) return 1;
                path[n] = 0; strcat(path, ".PNG");
                if(!stat(path, &s)) return 2;
            }
        }

        strcpy(path, directories[num_dir].path_name);

        if(is_retro || is_ps2_classic || stat(path, &s) != SUCCESS) return FAILED;
        return GET_ICON_FROM_ISO;
    }

    if (!is_update)
    {
        if(!strncmp(directories[num_dir].title_id, "HTSS00003", 9))
        {
            sprintf(path, "/dev_hdd0/game/HTSS00003/ICON0.PNG");
            return SUCCESS;
        }
        else if(!strncmp(directories[num_dir].title_id, "IRISMAN00", 9))
        {
            sprintf(path, "%s/USRDIR/browser.png", self_path);
            if(!stat(path, &s)) return SUCCESS;

            sprintf(path, "%s/ICON0.PNG", self_path);
            return SUCCESS;
        }

        // add PSX/PS2 iso
        if(directories[num_dir].flags & PS1_FLAG)
        {
            int cover_type = (((directories[num_dir].flags & (PS2_FLAG)) == (PS2_FLAG)) ? 1 : 2); // PSX cover

            sprintf(path, "%s%s_COV.JPG", retro_covers_path, directories[num_dir].title_id);
            if(!stat(path, &s)) return cover_type;

            if(parse_iso_titleid(directories[num_dir].path_name, titleid) == SUCCESS)
            {
                sprintf(path, "%s%s_COV.JPG", retro_covers_path, titleid);
                if(!stat(path, &s))
                {
                    strcpy(directories[num_dir].title_id, titleid);
                    return cover_type;
                }
            }

            strcpy(path, directories[num_dir].path_name);
            if(path[strlen(path) - 1] == '0') path[strlen(path) - 6] = 0; else path[strlen(path) - 4] = 0;

            int n = strlen(path);
            path[n] = 0; strcat(path, ".jpg");
            if(!stat(path, &s)) return cover_type;
            path[n] = 0; strcat(path, ".png");
            if(!stat(path, &s)) return cover_type;
            path[n] = 0; strcat(path, ".JPG");
            if(!stat(path, &s)) return cover_type;
            path[n] = 0; strcat(path, ".PNG");
            if(!stat(path, &s)) return cover_type;

            sprintf(path, "%s/COVER.JPG", directories[num_dir].path_name);
            if(stat(path, &s) != SUCCESS)
            {
                sprintf(path, "%s/cover.jpg", directories[num_dir].path_name);
                if(stat(path, &s) != SUCCESS) return SUCCESS; else return 2;
            }
            else return 2;
        }

        // bluray /dvd / mkv in Homebrew mode
        if((directories[num_dir].flags & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
        {
            strcpy(path, directories[num_dir].path_name);
            if(path[strlen(path) - 1] == '0') path[strlen(path) - 6] = 0; else path[strlen(path) - 4] = 0;
            int n = strlen(path);
            path[n] = 0; strcat(path, ".jpg");
            if(!stat(path, &s)) return 1;
            path[n] = 0; strcat(path, ".png");
            if(!stat(path, &s)) return 1;
            path[n] = 0; strcat(path, ".JPG");
            if(!stat(path, &s)) return 1;
            path[n] = 0; strcat(path, ".PNG");
            if(!stat(path, &s)) return 1;

            return FAILED;
        }

        if(cover_mode == 0 && !((directories[num_dir].flags & (PS3_FLAG | HOMEBREW_FLAG | BDVD_FLAG)) == PS3_FLAG))
        {
            sprintf(path, "%s%sICON0.PNG", directories[num_dir].path_name, &folder_mode[!(directories[num_dir].flags & D_FLAG_HOMEB)][0]);
            return SUCCESS;
        }
    }

    if(is_update || (directories[num_dir].flags & ((PS3_FLAG) | (HOMEBREW_FLAG) | (BDVD_FLAG))) == (PS3_FLAG))
    {   // PS3
        if((strlen(path) == 9) && (num_dir == 0))
            strcpy(titleid, path);
        else
            sprintf(titleid, "%c%c%c%c%s", directories[num_dir].title_id[0], directories[num_dir].title_id[1],
                                           directories[num_dir].title_id[2], directories[num_dir].title_id[3], &directories[num_dir].title_id[5]);

        sprintf(path, "%s%s.JPG", covers_path, titleid);
        if(stat(path, &s) != SUCCESS) {
            sprintf(path, "%s%s.PNG", covers_path, titleid);
            if(stat(path, &s) != SUCCESS) {
                sprintf(path, "%s%s.jpg", covers_path, titleid);
                if(stat(path, &s) != SUCCESS) {
                    sprintf(path, "%s%s.png", covers_path, titleid);
                    if(stat(path, &s) != SUCCESS) {
                        sprintf(path, "%s/USRDIR/covers/%s.JPG", MM_PATH, titleid);
                        if(stat(path, &s) != SUCCESS) {
                            sprintf(path, "%s/USRDIR/covers/%s.PNG", MM_PATH, titleid);
                            if(stat(path, &s) != SUCCESS) {
                                sprintf(path, "%s/covers/%s.JPG", self_path, titleid);
                                if(stat(path, &s) != SUCCESS) {
                                    sprintf(path, "%s/covers/%s.PNG", self_path, titleid);
                                    if(stat(path, &s) != SUCCESS) {
                                        sprintf(path, "%s/covers/%s.PNG", self_path, directories[num_dir].title_id);
                                        if(stat(path, &s) != SUCCESS) {
                                            // get covers from GAMES or GAMEZ
                                            if(!strcmp(hdd_folder, "GAMES") || !strcmp(hdd_folder, "dev_hdd0_2"))
                                              sprintf(path, "/dev_hdd0/GAMES/covers/%s.JPG", titleid);
                                            else
                                              sprintf(path, "/dev_hdd0/GAMEZ/covers/%s.JPG", titleid);

                                            if(stat(path, &s) != SUCCESS)
                                            {
                                                strcpy(path, directories[num_dir].path_name);
                                                if(path[strlen(path) - 1] == '0') path[strlen(path) - 6] = 0; else path[strlen(path) - 4] = 0;
                                                int n = strlen(path);
                                                strcat(path, ".jpg");
                                                if(!stat(path, &s)) return 1;
                                                path[n] = 0; strcat(path, ".JPG");
                                                if(!stat(path, &s)) return 1;
                                                path[n] = 0; strcat(path, ".png");
                                                if(!stat(path, &s)) return 2;
                                                path[n] = 0; strcat(path, ".PNG");
                                                if(!stat(path, &s)) return 2;

                                                sprintf(path, "%s", directories[num_dir].path_name);
                                                if(!stat(path, &s)) return GET_ICON_FROM_ISO;
                                                return FAILED;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return 1;
    }

    sprintf(titleid, "%c%c%c%c%s", directories[num_dir].title_id[0], directories[num_dir].title_id[1],
                                   directories[num_dir].title_id[2], directories[num_dir].title_id[3], &directories[num_dir].title_id[5]);

    sprintf(path, "%s%s.JPG", covers_path, titleid);
    if(stat(path, &s) != SUCCESS) {
        sprintf(path, "%s%s.PNG", covers_path, titleid);
        if(stat(path, &s) != SUCCESS) {
            sprintf(path, "%s%s.jpg", covers_path, titleid);
            if(stat(path, &s) != SUCCESS) {
                sprintf(path, "%s%s.png", covers_path, titleid);
                if(stat(path, &s) != SUCCESS) {
                    sprintf(path, "%s/USRDIR/covers/%s.JPG", MM_PATH, titleid);
                    if(stat(path, &s) != SUCCESS) {
                        sprintf(path, "%s/USRDIR/covers/%s.PNG", MM_PATH, titleid);
                        if(stat(path, &s) != SUCCESS) {
                            sprintf(path, "%s/covers/%s.PNG", self_path, titleid);
                            if(stat(path, &s) != SUCCESS) {
                                sprintf(path, "%s/covers/%s.JPG", self_path, titleid);
                                if(stat(path, &s) != SUCCESS) {
                                    sprintf(path, "%s/COVERS/%s.PNG", self_path, titleid);
                                    if(stat(path, &s) != SUCCESS) {
                                        sprintf(path, "%s/COVERS/%s.JPG", self_path, titleid);
                                        if(stat(path, &s) != SUCCESS) {
                                            sprintf(path, "%s/COVERS/%s.PNG", self_path, directories[num_dir].title_id);
                                            // get covers from GAMES or GAMEZ
                                            if(stat(path, &s) != SUCCESS) {
                                                sprintf(path, "/dev_hdd0/GAMES/covers/%s.JPG", titleid);
                                                if(stat(path, &s) != SUCCESS) sprintf(path, "/dev_hdd0/GAMEZ/covers/%s.JPG", titleid);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(stat(path, &s) != SUCCESS)
    {
        sprintf(path, "%s%sICON0.PNG", directories[num_dir].path_name, &folder_mode[!((directories[num_dir].flags>>D_FLAG_HOMEB_DPL) & 1)][0]);
        return SUCCESS;
    }
    else
        return FAILED;
}

static volatile int break_get_games = 0;

void get_games_2(void *empty)
{
    int n, f;

    stops_BDVD = 1;

    if(int_currentdir < 0 || int_currentdir >= ndirectories) int_currentdir = 0;

    if(mode_favourites)
    {
        for(f = 0; f < 3; f++) // priority loop: HDD/USB/BDVD Only
        for(n = 0; n < num_box; n++)
        {
            if(break_get_games) return;

            if(favourites.list[n].index < 0 || favourites.list[n].title_id[0] == 0 || favourites.list[n].index >= ndirectories) Png_offset[n] = 0;
            else
            {
                if(!Png_offset[n])
                {

                    if(f == 0 && !(directories[favourites.list[n].index].flags & D_FLAG_HDD0)) continue; // HDD Only
                    if(f == 1 && (directories[favourites.list[n].index].flags & (D_FLAG_HDD0 | D_FLAG_BDVD))) continue; // USB Only
                    if(f == 2 && !(directories[favourites.list[n].index].flags & D_FLAG_BDVD)) continue; // BDVD Only

                    strcpy(path_name, directories[favourites.list[n].index].path_name);

                    int r = get_icon(path_name, favourites.list[n].index);

                    if(r == GET_ICON_FROM_ISO)
                    {
                        Png_iscover[n] = -1;

                        int fd = ps3ntfs_open(path_name, O_RDONLY, 0);
                        if(fd > 0)
                        {
                            u32 flba;
                            u64 size;
                            int re;
                            char *mem = NULL;

                            if((directories[favourites.list[n].index].flags & (PSP_FLAG)) == (PSP_FLAG))
                                re = get_iso_file_pos(fd, "/PSP_GAME/ICON0.PNG", &flba, &size);
                            else
                                re = get_iso_file_pos(fd, "/PS3_GAME/ICON0.PNG;1", &flba, &size);

                            if(!re && (mem = malloc(size)) != NULL)
                            {
                                re = ps3ntfs_read(fd, (void *) mem, size);
                                ps3ntfs_close(fd);
                                if(re == size)
                                {
                                    memset(&my_png_datas, 0, sizeof(PngDatas));
                                    my_png_datas.png_in = mem;
                                    my_png_datas.png_size = size;
                                    if(LoadTexturePNG(NULL, n) == SUCCESS) Png_iscover[n] = 2;
                                }

                                free(mem);
                                continue;
                            }
                            else
                                ps3ntfs_close(fd);
                        }
                    }
                    else
                        Png_iscover[n] = r;

                    if(!strncmp(path_name + strlen(path_name) -4, ".JPG", 4) || !strncmp(path_name + strlen(path_name) -4, ".jpg", 4))
                        LoadTextureJPG(path_name, n);
                    else
                        LoadTexturePNG(path_name, n);
                }
            }
        }

        enable_draw_background_pic1();
        return;
    }

    int nn;

    for(f = 0; f < 3; f++) // priority loop: HDD/USB/BDVD Only
    for(n = 0; n < num_box; n++)
    {
        if(break_get_games) return;
        nn = (int_currentdir + n);

        if(nn < ndirectories)
        {
            if(!Png_offset[n])
            {
                if(f == 0 && !(directories[nn].flags & D_FLAG_HDD0)) continue; // HDD Only
                if(f == 1 && (directories[nn].flags & (D_FLAG_HDD0 | D_FLAG_BDVD))) continue; // USB Only
                if(f == 2 && !(directories[nn].flags & D_FLAG_BDVD)) continue; // BDVD Only

                strcpy(path_name, directories[nn].path_name);

                int r = get_icon(path_name, nn);

                if(r == GET_ICON_FROM_ISO)
                {
                    Png_iscover[n] = -1;

                    int fd = ps3ntfs_open(path_name, O_RDONLY, 0);
                    if(fd > 0)
                    {
                        u32 flba;
                        u64 size;
                        char *mem = NULL;
                        int re;

                        if((directories[nn].flags & (PSP_FLAG)) == (PSP_FLAG))
                            re = get_iso_file_pos(fd, "/PSP_GAME/ICON0.PNG", &flba, &size);
                        else
                            re = get_iso_file_pos(fd, "/PS3_GAME/ICON0.PNG;1", &flba, &size);

                        if(!re && (mem = malloc(size)) != NULL)
                        {
                            re = ps3ntfs_read(fd, (void *) mem, size);
                            ps3ntfs_close(fd);
                            if(re == size)
                            {
                                memset(&my_png_datas, 0, sizeof(PngDatas));
                                my_png_datas.png_in = mem;
                                my_png_datas.png_size = size;
                                if(LoadTexturePNG(NULL, n) == SUCCESS) Png_iscover[n] = 2;
                            }

                            free(mem);
                            continue;
                        }
                        else
                            ps3ntfs_close(fd);
                    }
                }
                else
                    Png_iscover[n] = r;

                if(!strncmp(path_name + strlen(path_name) -4, ".JPG", 4) || !strncmp(path_name + strlen(path_name) -4, ".jpg", 4))
                   LoadTextureJPG(path_name, n);
                else
                   LoadTexturePNG(path_name, n);
            }

        } else Png_offset[n] = 0;
    }

    enable_draw_background_pic1();
}

void get_grid_dimensions()
{
         if(gui_mode ==  1) {cols = 4; rows = 3;} // Coverflow
    else if(gui_mode ==  2) {cols = 3; rows = 2;} // Grid 3x2
    else if(gui_mode ==  3) {cols = 4; rows = 2;} // Grid 4x2
    else if(gui_mode ==  4) {cols = 3; rows = 3;} // Grid 3x3
    else if(gui_mode ==  5) {cols = 4; rows = 3;} // Grid 4x3
    else if(gui_mode ==  6) {cols = 5; rows = 3;} // Grid 5x3
    else if(gui_mode ==  7) {cols = 6; rows = 3;} // Grid 6x3
    else if(gui_mode ==  8) {cols = 4; rows = 4;} // Grid 4x4
    else if(gui_mode ==  9) {cols = 5; rows = 4;} // Grid 5x4
    else if(gui_mode == 10) {cols = 6; rows = 4;} // Grid 6x4
    else if(gui_mode == 11) {cols = 5; rows = 5;} // Grid 5x5
    else if(gui_mode == 12) {cols = 6; rows = 5;} // Grid 6x5
    else if(gui_mode == 13) {cols = 6; rows = 6;} // Grid 6x6
    else if(gui_mode == 14) {cols = 8; rows = 5;} // Grid 8x5
    else if(gui_mode == 15) {cols = 8; rows = 6;} // Grid 8x6
    else                    {cols = 4; rows = 3;} // Grid 4x3

    num_box = cols * rows;
}

void get_games_3(u64 var)
{

    int n;
    int indx;

    get_grid_dimensions();

    if(var == 1ULL)
    {
        indx = Png_index[0];

        for(n = 0; n < num_box; n++)
        {
            Png_iscover[n] = Png_iscover[n + 1];
            Png_offset[n] = Png_offset[n + 1];
            Png_datas[n] = Png_datas[n + 1];
            Png_index[n] = Png_index[n + 1];
            if(Png_offset[n]) ;
            else {Png_iscover[n] = 0; Png_offset[n] = 0;}
        }

        Png_iscover[num_box - 1] = 0; Png_offset[num_box - 1] = 0; Png_index[num_box - 1] = indx;

    }
    else
    {
        indx = Png_index[num_box - 1];
        for(n = num_box - 1; n > 0; n--)
        {
            Png_iscover[n] = Png_iscover[n - 1];
            Png_offset[n] = Png_offset[n - 1];
            Png_datas[n] = Png_datas[n - 1];
            Png_index[n] = Png_index[n - 1];
            if(Png_offset[n]) ;
            else {Png_iscover[n] = 0; Png_offset[n] = 0;}
        }

        Png_iscover[0] = 0; Png_offset[0] = 0;  Png_index[0] = indx;
    }

    for(n = num_box; n < BIG_PICT; n++)
    {
        Png_iscover[n] = 0; Png_offset[n] = 0;
    }

    get_games_2(NULL);
 }


void get_games()
{
    int n;

    break_get_games = 1; // for short wait
    wait_event_thread(); // wait previous event thread function
    break_get_games = 0;

    int_currentdir = currentdir;

    // reset icon datas
    for(n = 0; n < num_box; n++) {Png_iscover[n] = 0; Png_offset[n] = 0; Png_index[n] = n;}

    // program new event thread function
    event_thread_send(0x555ULL, (u64) get_games_2, 0);
}


void DrawCenteredBar2D(float y, float w, float h, u32 rgba)
{
    float x = (848.0f - w)/ 2.0f;

    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(x    , y    , 1.0f);
    tiny3d_VertexColor(rgba);

    tiny3d_VertexPos(x + w, y    , 1.0f);

    tiny3d_VertexPos(x + w, y + h, 1.0f);

    tiny3d_VertexPos(x    , y + h, 1.0f);
    tiny3d_End();
}


static u32 text_size = 0;

void LoadTexture()
{
    int i;

    u32 * texture_mem = tiny3d_AllocTexture(170 * 1024 * 1024); // alloc 170MB of space for textures (this pointer can be global)

    u32 * texture_pointer; // use to asign texture space without changes texture_mem

    if(!texture_mem) return; // fail!

    texture_pointer = texture_mem;

    ResetFont();

    //debug font
    texture_pointer = (u32 *) AddFontFromBitmapArray((u8 *) font_b, (u8 *) texture_pointer, 32, 255, 16, 32, 2, BIT0_FIRST_PIXEL);

/*
    TTFLoadFont(NULL, (void *) comfortaa_ttf_bin, comfortaa_ttf_bin_size);
    texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
    texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 20, 20, TTF_to_Bitmap);
    TTFUnloadFont();


    //new button font
    TTFLoadFont(NULL, (void *) comfortaa_bold_ttf_bin, comfortaa_bold_ttf_bin_size);
    texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 24, 24, TTF_to_Bitmap);
    TTFUnloadFont();
*/

    {
        struct stat s;
        sprintf(temp_buffer, "%s/font.ttf", self_path);

        if(stat(temp_buffer, &s) != SUCCESS || TTFLoadFont(0, temp_buffer, NULL, 0) != SUCCESS)
        {
            if(TTFLoadFont(0, "/dev_flash/data/font/SCE-PS3-SR-R-JPN.TTF", NULL, 0) != SUCCESS)
                if(TTFLoadFont(0, "/dev_flash/data/font/SCE-PS3-NR-R-JPN.TTF", NULL, 0) != SUCCESS)
                    exit(0);
                //TTFLoadFont(NULL, (void *) comfortaa_bold_ttf_bin, comfortaa_bold_ttf_bin_size);
        }

        TTFLoadFont(1, "/dev_flash/data/font/SCE-PS3-DH-R-CGB.TTF", NULL, 0);
        TTFLoadFont(2, "/dev_flash/data/font/SCE-PS3-SR-R-LATIN2.TTF", NULL, 0);
        TTFLoadFont(3, "/dev_flash/data/font/SCE-PS3-YG-R-KOR.TTF", NULL, 0);

    }


    Load_PNG_resources();

    for(i = 0; i < MAX_RESOURCES; i++)
    {
        if(Png_res[i].png_in == NULL) continue;

        Png_res_offset[i]  = 0;

        if(Png_res[i].bmp_out)
        {
            memcpy(texture_pointer, Png_res[i].bmp_out, Png_res[i].wpitch * Png_res[i].height);

            free(Png_res[i].bmp_out); // free the PNG because i don't need this datas

            Png_res_offset[i] = tiny3d_TextureOffset(texture_pointer);      // get the offset (RSX use offset instead address)

            texture_pointer += ((Png_res[i].wpitch * Png_res[i].height + 15) & ~15) / 4; // aligned to 16 bytes (it is u32) and update the pointer
         }
    }


    ttf_texture = (u16 *) texture_pointer;

    texture_pointer += 1024 * 16;

    texture_pointer = (u32 *) init_ttf_table((u16 *) texture_pointer);

    png_texture = (u8 *) texture_pointer;

    text_size = (u32) (u64)((png_texture + BIG_PICT * 4096 * 1024 + 1980 * 1080 * 4 + 2048 * 1200 * 4) - ((u8 *) texture_mem));
}

int show_custom_icons = 1;

int background_sel = 1;
int background_fx = 0;

#define GUI_MODES  15

#define MAX_COLORS  12

u32 background_colors[MAX_COLORS] = {
    0xff000000, //Animated Background
    0xff000010, //Dark blue
    0xff000033, //Navy
    0xff0040cf, //Cobalt
    0xff6699ff, //Sky blue
    0x10e8a010, //Orange
    0xff320200, //Dark red
    0xff220345, //Purple
    0xff734367, //Pink
    0xff0b4143, //Cyan
    0xff556b2f, //Olive
    0xff404040, //Gray
};

u32 background_colors2[24] = {
    0x000000FF, //Animated Background
    0x000000FF,

    0x040404ff, //BLACK
    0x030303ff,

    0x000060FF, //NAVY
    0x000000FF,

    0x2040B8ff, //BLUE
    0x102048ff,

    0x4040ffff, //BRIGHT BLUE
    0x0040a8ff,

    0xf8b010ff, //ORANGE
    0x481008ff,

    0x400000ff, //RED
    0x200000ff,

    0x624385ff, //PURPLE
    0x220345ff,

    0x784078ff, //PLUM
    0x281028ff,

    0x008080ff, //CYAN
    0x001010FF,

    0x206F00ff, //GREEN
    0x002000ff,

    0x808080ff, //GRAY
    0x000000FF,
};

void cls0()
{
    cls2();

    if(background_sel == 0) { GFX1_background(); return; }

    if(Png_offset[BACKGROUND_PICT])
    {
        if (Png_datas[BACKGROUND_PICT].wpitch > 0 && Png_datas[BACKGROUND_PICT].width > 0 && Png_datas[BACKGROUND_PICT].height > 0)
        {
            tiny3d_SetTextureWrap(0, Png_offset[BACKGROUND_PICT], Png_datas[BACKGROUND_PICT].width,
                Png_datas[BACKGROUND_PICT].height, Png_datas[BACKGROUND_PICT].wpitch,
                TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP, 1);

            DrawTextBox(-1, -1, 1000, 850, 514, (background_sel & 1) ? 0xffffffff : 0xcfcfcfff);
            return;
        }
    }

    u32 color1; //0x002088ff;
    u32 color2;

    color1 = background_colors2[(background_sel & 15) << 1];
    color2 = background_colors2[((background_sel & 15) << 1) + 1];

    tiny3d_SetPolygon(TINY3D_QUADS);

    tiny3d_VertexPos(-80, -80, 1000);
    tiny3d_VertexColor(color1);

    tiny3d_VertexPos(938, -80, 1000);
    tiny3d_VertexColor(color1);

    tiny3d_VertexPos(938, 400, 1000);
    tiny3d_VertexColor(color2);

    tiny3d_VertexPos(-80, 400, 1000);
    tiny3d_VertexColor(color2);

    tiny3d_VertexPos(-80, 400, 1000);
    tiny3d_VertexColor(color2);

    tiny3d_VertexPos(938, 400, 1000);
    tiny3d_VertexColor(color2);

    tiny3d_VertexPos(938, 592, 1000);
    tiny3d_VertexColor(0x080008ff);

    tiny3d_VertexPos(-80, 592, 1000);
    tiny3d_VertexColor(0x080008ff);

    tiny3d_End();
}

void cls()
{
    if(gui_mode == MODE_COVERFLOW)  {cls0(); return;}

    if(background_sel == 0) {cls2(); GFX1_background(); return;}

    draw_background_pic1();

    tiny3d_Clear(background_colors[background_sel], TINY3D_CLEAR_ALL);

    // Enable alpha Test
    tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

    // Enable alpha blending.
            tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
                TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
                TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
    reset_ttf_frame();
}

void cls2()
{
    draw_background_pic1();

    tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

    // Enable alpha Test
    tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

   // Enable alpha blending.
            tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
                TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
                TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
    reset_ttf_frame();
}

/******************************************************************************************************************************************************/
/* Payload functions                                                                                                                                  */
/******************************************************************************************************************************************************/

static int sys_ss_media_id(void * id)
{
    lv2syscall2(879, 0x10001ULL, (u64) id);
    return_to_user_prog(int);
}

u64 syscall_40(u64 cmd, u64 arg)
{
    lv2syscall2(40, cmd, arg);
    return_to_user_prog(u64);
}


int sys_set_leds(u64 color, u64 state)
{
    lv2syscall2(386,  (u64) color, (u64) state);
    return_to_user_prog(int);
}

int sys_game_get_temperature(int sel, u32 *temperature)
{
    u32 temp;

    lv2syscall2(383, (u64) sel, (u64) &temp);
    *temperature = (temp >> 24);
    return_to_user_prog(int);
}

int sys_fs_mount(char const* deviceName, char const* deviceFileSystem, char const* devicePath, int writeProt)
{
    lv2syscall8(837, (u64) deviceName, (u64) deviceFileSystem, (u64) devicePath, 0, (u64) writeProt, 0, 0, 0 );
    return_to_user_prog(int);
}

int sys_fs_umount(char const* devicePath)
{
    lv2syscall3(838,  (u64) devicePath, 0, 0 );
    return_to_user_prog(int);
}

u64 lv2peek(u64 addr)
{
    lv2syscall1(6, (u64) addr);
    return_to_user_prog(u64);

}

u64 lv2poke(u64 addr, u64 value)
{
    lv2syscall2(7, (u64) addr, (u64) value);
    return_to_user_prog(u64);
}


int lv2launch(u64 addr)
{
    lv2syscall8(9, (u64) addr, 0,0,0,0,0,0,0);
    return_to_user_prog(int);
}

/*
int syscall36(char * path)
{
    lv2syscall1(36, (u64) path);
    return_to_user_prog(int);

}
*/

u64 hmanager_key = 0x1759829723742374ULL;

/******************************************************************************************************************************************************/

// manager


char payload_str[256];

int videoscale_x = 0;
int videoscale_y = 0;

int flash;

int select_px = 0;
int select_py = 0;

u32 blockSize;
u64 freeSize;
float freeSpace[12];

int select_option = 0;

u32 fdevices = 0;
u32 fdevices_old = 0;
u32 forcedevices = 0;
int find_device = 0;

char hdd_folder[64]="12345";

char bluray_game[64]; // name of the game

static int exit_program = 0;

#define ROUND_UPX(x)  ((((x) + num_box - 1) / num_box) * num_box)

void draw_grid(float x, float y);
void draw_coverflow(float x, float y);

int gui_control();

void draw_options(float x, float y, int index);
void draw_iso_options(float x, float y, int index);
void draw_configs(float x, float y, int index);
void draw_gbloptions(float x, float y);
void draw_toolsoptions(float x, float y);
void draw_cachesel(float x, float y);
void draw_pkginstall(float x, float y);
void draw_device_mkiso(float x, float y, int index);
void draw_device_xtiso(float x, float y, int index);
void draw_device_cpyiso(float x, float y, int index);
void draw_app_version(float x, float y);

char app_ver[8];

struct {
    int videoscale_x[4];
    int videoscale_y[4];
    int background_sel;
    char hdd_folder[64];
    u32 usekey;
    char pad[156];
} manager_oldcfg;

struct {
    int videoscale_x[4];
    int videoscale_y[4];
    int background_sel;
    char hdd_folder[64];
    u32 usekey;
    u8 language;
    u8 noBDVD;
    u8 gui_mode;
    u8 bk_picture;
    char pad[148];
    u32 event_flag;
    u32 opt_flags;
    int cover_mode;
    int background_fx;
    int show_custom_icons;

    int mode_homebrew;
    int game_list_category;
    int mode_favourites;

} manager_cfg;

struct {
    int version;
    int perm;
    int useBDVD;
    int updates;
    int ext_ebootbin;
    int bdemu;
    int exthdd0emu;
    int direct_boot;
    int bdemu_ext;
    int pad[5];
} game_cfg;


int inited = 0;

#define INITED_SPU          2
#define INITED_SOUNDLIB     4
#define INITED_GCM_SYS      8
#define INITED_IO          16
#define INITED_PNGDEC      32
#define INITED_FS          64
#define INITED_JPGDEC      128
#define INITED_MODLIB      256
#define INITED_HTTPS       512

int set_install_pkg = 0;

void SaveManagerCfg()
{
    manager_cfg.mode_homebrew = mode_homebrew;
    manager_cfg.game_list_category = game_list_category;
    manager_cfg.mode_favourites = mode_favourites;

    sprintf(temp_buffer, "%s/config/manager_setup.bin", self_path);
    SaveFile(temp_buffer, (char *) &manager_cfg, sizeof(manager_cfg));
}

void LoadManagerCfg()
{
    sprintf(temp_buffer, "%s/config/manager_setup.bin", self_path);

    int file_size;
    char *file = LoadFile(temp_buffer, &file_size);

    if(file)
    {
        if(file_size != sizeof(manager_cfg))
        {
            file_size = sizeof(manager_oldcfg); // safe
            //manager_cfg.opt_flags |= OPTFLAGS_PLAYMUSIC; // enabled by default
            manager_cfg.gui_mode = 1;
            manager_cfg.background_sel = 1;
            manager_cfg.background_fx = 0;
            manager_cfg.bk_picture = 2;
            manager_cfg.show_custom_icons = 1;

            manager_cfg.mode_homebrew = 0;
            manager_cfg.game_list_category = 0;
            manager_cfg.mode_favourites = 1;
            SaveManagerCfg();
        }

        memcpy(&manager_cfg, file, file_size);
        free(file);
    }
    else
    {
        //manager_cfg.opt_flags |= OPTFLAGS_PLAYMUSIC; // enabled by default
        manager_cfg.gui_mode = 1;
        manager_cfg.background_sel = 1;
        manager_cfg.background_fx = 0;
        manager_cfg.bk_picture = 2;
        manager_cfg.show_custom_icons = 1;

        manager_cfg.mode_homebrew = 0;
        manager_cfg.game_list_category = 0;
        manager_cfg.mode_favourites = 1;
        SaveManagerCfg();
    }

    if((manager_cfg.gui_mode < 1) || (manager_cfg.gui_mode > GUI_MODES)) manager_cfg.gui_mode = 1;

    gui_mode = manager_cfg.gui_mode & 15;
    cover_mode = manager_cfg.cover_mode;

    mode_homebrew = manager_cfg.mode_homebrew;
    game_list_category = manager_cfg.game_list_category;
    mode_favourites = manager_cfg.mode_favourites;

    get_grid_dimensions();

    background_sel = manager_cfg.background_sel;
    background_fx = manager_cfg.background_fx;
    show_custom_icons = manager_cfg.show_custom_icons;

    switch(Video_Resolution.height)
    {
        case 480:
            videoscale_x = manager_cfg.videoscale_x[0];
            videoscale_y = manager_cfg.videoscale_y[0];
            break;
       case 576:
            videoscale_x = manager_cfg.videoscale_x[1];
            videoscale_y = manager_cfg.videoscale_y[1];
            break;
       case 720:
            videoscale_x = manager_cfg.videoscale_x[2];
            videoscale_y = manager_cfg.videoscale_y[2];
            break;
       default:
            videoscale_x = manager_cfg.videoscale_x[3];
            videoscale_y = manager_cfg.videoscale_y[3];
            break;
    }

    if(manager_cfg.language == 99)
        manager_cfg.language = (u8) get_system_language();

    if(manager_cfg.language < 0 || manager_cfg.language > LANGCOUNT)  manager_cfg.language = 0;

    sprintf(temp_buffer, "%s/config/language.ini", self_path);
    open_language(manager_cfg.language, temp_buffer);
}

void fun_exit()
{
    // multiple calls? (Yes!)
    static int one = 1;
    if(!one) return;
    one = 0;

    event_threads_finish();
    set_usleep_sm_main(250000);

    if(!is_ps3game_running && !use_cobra && lv2peek(0x80000000000004E8ULL)) syscall_40(1, 0); // disables PS3 Disc-less

    SaveManagerCfg();

    close_language();
    TTFUnloadFont();

    ftp_deinit();
    ftp_net_deinit();

    if(inited & INITED_SOUNDLIB)
    {
        if(inited & INITED_MODLIB)
            MODPlay_Unload (&mod_track);
        SND_End();
    }

    if(inited & INITED_SPU)
    {
        sleep(1);
        sysSpuRawDestroy(spu);
        sysSpuImageClose(&spu_image);
    }

    NTFS_UnMountAll();

    if(inited & INITED_GCM_SYS) sysModuleUnload(SYSMODULE_GCM_SYS);
    if(inited & INITED_IO)      sysModuleUnload(SYSMODULE_IO);
    if(inited & INITED_PNGDEC)  sysModuleUnload(SYSMODULE_PNGDEC);
    if(inited & INITED_JPGDEC)  sysModuleUnload(SYSMODULE_JPGDEC);
    if(inited & INITED_FS)      sysModuleUnload(SYSMODULE_FS);
    if(inited & INITED_HTTPS)   sysModuleUnload(SYSMODULE_HTTPS);
    sysModuleUnload(SYSMODULE_SYSUTIL);

    inited = 0;
    //if(manager_cfg.usekey) sys8_disable(hmanager_key);

    ioPadEnd();

    if(lv2_unpatch_storage) lv2_unpatch_storage();

    if(set_install_pkg)
    {
        unlink_secure("/dev_hdd0/tmp/turnoff");
    }

    sys8_perm_mode(0); // perms to 0 from exit()

    if(restore_syscall8[0]) sys8_pokeinstr(restore_syscall8[0], restore_syscall8[1]);

    if(game_cfg.direct_boot == 555 && use_cobra == 1)
        sysProcessExitSpawn2("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN", NULL, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);

    if(set_install_pkg) sys_reboot();
}

void auto_ftp(void)
{
    static int one = 1;
    static int counter = 0;
    if(!one) goto ftp_net;

    one= 0;
    if (manager_cfg.opt_flags & OPTFLAGS_FTP) // maybe we need add an icon to user...
    {
        int r = ftp_init();

        if(r == SUCCESS) ftp_inited = 1; //DrawDialogOK("FTP Service init on boot: OK");
        else
        {
            if(r == -1) DrawDialogOKTimer("Error in netInitialize()", 2000.0f);
            else if(r == -2) DrawDialogOKTimer("Error in netCtlInit()", 2000.0f);
            else if(r == -3) DrawDialogOKTimer("Error in netCtlGetInfo()", 2000.0f);
            else if(r == -4) DrawDialogOKTimer("Net Disconnected or Connection not Established", 2000.0f);
            else DrawDialogOK("FTP Unknown Error");
        }
    }

ftp_net:
    if(!ftp_inited) return;
    counter++;

    if(counter < 600)  return;

    counter = 0;

    int r = ftp_net_status();

    if(r == -4)
    {
       ftp_net_deinit();
       ftp_net_init();
       r = ftp_net_status();
    }

}

void RemoveNtfsFromGameList()
{
    delete_entries(directories, &ndirectories, NTFS_FLAG);

    for (int i = ndirectories; i < MAX_DIRECTORIES; i++)
    {
        directories[i].flags = 0;
        directories[i].title_id[0] = 0;
        directories[i].path_name[0] = 0;
    }

    sort_entries2(directories, &ndirectories, sort_mode);
}

void LoadGameList()
{
    if(!bCachedGameList) return;

    sprintf(temp_buffer, "%s/config/gamelist.bin", self_path);

    int file_size;
    char *file = LoadFile(temp_buffer, &file_size);

    if(file)
    {
        int i, d = 0;

        if(file_size == sizeof(directories))
        {
            memcpy(&directories, file, file_size);
            free(file);

            ndirectories = MAX_DIRECTORIES - 1;
            while (ndirectories > 0 && directories[ndirectories].title_id[0] == 0) ndirectories--;
            ndirectories++;

            RemoveNtfsFromGameList();

            find_device = 0;
            sysFsGetFreeSize("/dev_hdd0/", &blockSize, &freeSize);
            freeSpace[find_device] = ( ((u64)blockSize * freeSize));
            freeSpace[find_device] = freeSpace[find_device] / GIGABYTES;

            fdevices = 2;

            for (i = 1; i < 11; i++) freeSpace[i] = 0;

            for (i = 0; i < ndirectories; i++)
            {
                if(!is_file_exist(directories[i].path_name))
                {
                    directories[i].flags = 0;
                    directories[i].title_id[0] = 0;
                    directories[i].path_name[0] = 0;
                    d = 1;
                }
                else if (directories[i].path_name[5] == 'u' &&
                         directories[i].path_name[6] == 's' &&
                         directories[i].path_name[7] == 'b')
                {
                    // /dev_usb
                    find_device = (directories[i].path_name[10]);
                    if(find_device > 47 && find_device < 58) fdevices |= 1<<(find_device - 47);
                }
            }

            if(d) sort_entries2(directories, &ndirectories, sort_mode);

            fdevices_old = fdevices;

            // check bdemu
            char dev_path[12];
            for (find_device = 0; find_device < 11; find_device++)
            {
                if(find_device == HDD0_DEVICE)
                    sprintf(dev_path, "/dev_hdd0");
                else
                    sprintf(dev_path, "/dev_usb00%c", 47 + find_device);

                if(is_file_exist(dev_path))
                    move_bdemubackup_to_origin(1 << find_device);
            }
}
        else free(file);

        delete_entries(directories, &ndirectories, 0);
    }
}

void SaveLastGame()
{
    sprintf(temp_buffer, "%s/config/lastgame", self_path);
    memset(temp_buffer + 1024, 0, 72);
    memcpy(temp_buffer + 1024, directories[currentgamedir].title_id, 65);
    memcpy(temp_buffer + 1092, &directories[currentgamedir].flags, 4);
    SaveFile(temp_buffer, temp_buffer + 1024, 72);
}

void SaveGameList()
{
    SaveLastGame();

    if(!bCachedGameList) return;

    sprintf(temp_buffer, "%s/config/gamelist.bin", self_path);
    SaveFile(temp_buffer, (char *) &directories, sizeof(directories));
}

void LoadLastGame()
{
    sprintf(temp_buffer, "%s/config/lastgame", self_path);

    int size;
    char * mem = LoadFile(temp_buffer, &size);

    if(mem && size == 72)
    {
        mem[63] = 0;

        get_grid_dimensions();

        if(ndirectories <= 0)
        {
            get_games();
            load_gamecfg(-1); // force refresh game info
        }

        for(int n = 0; n < ndirectories; n++)
        {
            if(!strcmp(directories[n].title_id, mem) && !memcmp(&directories[n].flags, mem + 68, 4))
            {
                last_game_flag = directories[n].flags;

                frame_count = 32;
                //mode_favourites = 0;

                get_grid_dimensions();
                currentgamedir = n;

                if(gui_mode == MODE_COVERFLOW)
                {
                    select_py = 0;
                    if (n >= 3)
                        select_px = 3;
                    else
                        select_px = n;
                    currentdir = n - select_px;
                }
                else
                {
                    currentdir = (n / (rows * cols)) * (rows * cols);
                    select_py = ((n - currentdir) / cols);
                    select_px = (n - currentdir) %  cols;
                }

                return;
            }
        }
    }

    if(mem) free(mem);

    last_game_flag = 0;
    return;
}

void video_adjust()
{

    SetCurrentFont(FONT_TTF);
    while(1)
    {
        double sx = (double) Video_Resolution.width;
        double sy = (double) Video_Resolution.height;
        double psx = (double) (1000 + videoscale_x)/1000.0;
        double psy = (double) (1000 + videoscale_y)/1000.0;

        tiny3d_UserViewport(1,
            (float) ((sx - sx * psx) / 2.0), // 2D position
            (float) ((sy - sy * psy) / 2.0),
            (float) ((sx * psx) / 848.0),    // 2D scale
            (float) ((sy * psy) / 512.0),
            (float) ((sx / 1920.0) * psx),  // 3D scale
            (float) ((sy / 1080.0) * psy));

        tiny3d_Clear(background_colors[background_sel], TINY3D_CLEAR_ALL);


        // Enable alpha Test
        tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

        // Enable alpha blending.
                tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
                    TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
                    TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
        reset_ttf_frame();

        DrawAdjustBackground(0xffffffff) ; // light blue

        update_twat(0);
        SetFontSize(16, 24);
        SetFontColor(0xffffffff, 0x0);

        SetFontAutoCenter(1);

        DrawFormatString(0, (512 - 24)/2 - 64, "%s", language[VIDEOADJUST_POSITION]);

        DrawFormatString(0, (512 - 24)/2, language[VIDEOADJUST_SCALEINFO], videoscale_x, videoscale_y);

        DrawFormatString(0, (512 - 24)/2 + 64, "%s", language[VIDEOADJUST_EXITINFO]);

        DrawFormatString(0, (512 - 24)/2 + 96, "%s", language[VIDEOADJUST_DEFAULTS]);

        // Warning!! don´t traslate this string!
        DrawFormatString(0, (512 - 24)/2 + 128, "%s", "Press [] to English language");

        SetFontAutoCenter(0);

        tiny3d_Flip();
        ps3pad_read();

        if(!(frame_count & 3))
        {
            if(old_pad & BUTTON_UP) {if(videoscale_y > -179) videoscale_y--;}
            if(old_pad & BUTTON_DOWN) {if(videoscale_y < 10) videoscale_y++;}
            if(old_pad & BUTTON_LEFT) {if(videoscale_x > -199) videoscale_x--;}
            if(old_pad & BUTTON_RIGHT) {if(videoscale_x < 10) videoscale_x++;}
        }

        if(new_pad & BUTTON_SQUARE)
        {
            manager_cfg.language = 0;
            sprintf(temp_buffer, "%s/config/language.ini", self_path);
            open_language(manager_cfg.language, temp_buffer);
        }

        if(new_pad & (BUTTON_CROSS | BUTTON_START))
        {
            switch(Video_Resolution.height)
            {
               case 480:
                    manager_cfg.videoscale_x[0] = videoscale_x;
                    manager_cfg.videoscale_y[0] = videoscale_y;
                    break;
               case 576:
                    manager_cfg.videoscale_x[1] = videoscale_x;
                    manager_cfg.videoscale_y[1] = videoscale_y;
                    break;
               case 720:
                    manager_cfg.videoscale_x[2] = videoscale_x;
                    manager_cfg.videoscale_y[2] = videoscale_y;
                    break;
               default:
                    manager_cfg.videoscale_x[3] = videoscale_x;
                    manager_cfg.videoscale_y[3] = videoscale_y;
                    break;
            }


            SaveManagerCfg();

            break;
        }

        if(new_pad & BUTTON_CIRCLE)
        {
            if(videoscale_x == -120 && videoscale_y == -120)
            {
                videoscale_x = -40;
                videoscale_y = -60;
            }
            else
            {
                videoscale_x = videoscale_y = -120;
            }
        }

        frame_count++;
    }
}

void Select_games_folder()
{

    DIR  *dir, *dir2;
    int selected = 0;
    char tmp[256];

    dir = opendir ("/dev_hdd0/GAMES");
    if(dir)
    {
        closedir (dir);
        sprintf(temp_buffer, "%s %s %s", language[GAMEFOLDER_WANTUSE], "/dev_hdd0/GAMES", language[GAMEFOLDER_TOINSTALLNTR]);

        if(DrawDialogYesNoDefaultYes(temp_buffer) == 1)
        {
            strncpy(hdd_folder, "GAMES", 64);
            strncpy(manager_cfg.hdd_folder, "GAMES", 64);
            return;
        }
    }

    dir = opendir ("/host_root");
    if(dir)
    {
        closedir (dir);
        sprintf(temp_buffer, "%s %s %s", language[GAMEFOLDER_WANTUSE], "/host_root", language[GAMEFOLDER_TOINSTALLNTR]);

        if(DrawDialogYesNo(temp_buffer) == 1)
        {
            strncpy(hdd_folder, "host_root", 64);
            strncpy(manager_cfg.hdd_folder, "host_root", 64);
            return;
        }
    }

    dir = opendir ("/dev_hdd0/game");

    if(dir)
    {
        while(1)
        {
            struct dirent *entry = readdir (dir);

            if(!entry) break;

            if(entry->d_name[0] == '.' && (entry->d_name[1] == 0 || entry->d_name[1]== '.')) continue;

            if(!(entry->d_type & DT_DIR)) continue;

            sprintf(temp_buffer, "/dev_hdd0/game/%s/"__MKDEF_GAMES_DIR, entry->d_name);

            dir2 = opendir (temp_buffer);

            if(dir2)
            {
                closedir (dir2);

                sprintf(temp_buffer, "%s /%s %s", language[GAMEFOLDER_WANTUSE], entry->d_name, language[GAMEFOLDER_TOINSTALLNTR]);

                if(DrawDialogYesNo(temp_buffer) == 1)
                {
                    strncpy(hdd_folder, entry->d_name, 64);
                    strncpy(manager_cfg.hdd_folder, entry->d_name, 64);
                    selected = 1;
                    break;
                }
            }

          }
    closedir (dir);
    }

    dir = opendir ("/dev_hdd0/video");
    if(dir)
    {
        closedir (dir);
        sprintf(temp_buffer, "%s %s %s", language[GAMEFOLDER_WANTUSE], "/dev_hdd0/video", language[GAMEFOLDER_TOINSTALLNTR]);

        if(DrawDialogYesNoDefaultYes(temp_buffer) == 1)
        {
            strncpy(hdd_folder, "video", 64);
            strncpy(manager_cfg.hdd_folder, "video", 64);
            return;
        }
    }

    if(!selected)
    {
        sprintf(temp_buffer, "%s %s %s", language[GAMEFOLDER_WANTUSE], "/dev_hdd0/" __MKDEF_GAMES_DIR, language[GAMEFOLDER_TOINSTALLNTR]);

        if(DrawDialogYesNo(temp_buffer) == 1)
        {
            strncpy(hdd_folder, "dev_hdd0", 64);
            strncpy(manager_cfg.hdd_folder, "dev_hdd0", 64);
            mkdir_secure("/dev_hdd0/" __MKDEF_GAMES_DIR);
        }
        else
        {
            strncpy(hdd_folder, __MKDEF_MANAGER_DIR__, 64);
            strncpy(manager_cfg.hdd_folder, __MKDEF_MANAGER_DIR__, 64);
            sprintf(tmp, "/%s/" __MKDEF_GAMES_DIR, __MKDEF_MANAGER_FULLDIR__);
            mkdir_secure(tmp);

            sprintf(temp_buffer, "%s %s %s", language[GAMEFOLDER_USING], tmp, language[GAMEFOLDER_TOINSTALL]);
            DrawDialogOK(temp_buffer);
        }
    }
}

void pause_music(int pause)
{
    if((!pause)&&(!(manager_cfg.opt_flags & OPTFLAGS_PLAYMUSIC)))
        return;

    SND_Pause(pause);
}

void init_music(int select_song)
{
    MODPlay_Init(&mod_track);

    int file_size;
    char *file = NULL;

    if(select_song < 0)
    {
        sprintf(temp_buffer, "%s/music.mod", self_path);
        file = LoadFile(temp_buffer, &file_size);

        if(!file)
        {
            sprintf(temp_buffer, "%s/MUSIC.MOD", self_path);
            file = LoadFile(temp_buffer, &file_size);
        }
    }

    if(select_song >= 0 && select_song < MAX_SONGS)
    {
        song_selected = select_song;
        file = (char *) music[song_selected];
    }
    else if(!file)
    {
        srand(time(0)); // randomize seed
        song_selected = rand() % MAX_SONGS;
        file = (char *) music[song_selected];
    }
    else
    {
        // paranoic code to copy the .mod in aligned and large memory
        char *file2 = memalign(32, file_size + 32768);
        if(file2) {memcpy(file2, file, file_size);free(file); file = file2;}
    }

    if(MODPlay_SetMOD (&mod_track, file) < 0)
    {
        MODPlay_Unload (&mod_track);
    }

    MODPlay_SetVolume( &mod_track, 6, 6); // fix the volume to 16 (max 64)
    MODPlay_Start (&mod_track); // Play the MOD
    inited |= INITED_MODLIB;
    SND_Pause(1); //force pause here

}


int payload_mode = 0;

int firmware = 0;
int fw_ver = 0;

int load_from_bluray = 0;

static char filename[0x420];

/******************************************************************************************************************************************************/

static volatile int bdvd_notify = 0;

static volatile int bdvd_ejected = 1;

static volatile int disc_less_on = 0;

void DiscEjectCallback(void)
{
    bdvd_notify = -1;
    bdvd_ejected = 1;
}

void DiscInsertCallback(u32 discType, char *title)
{

    bdvd_notify = 0;

    if(!noBDVD && lv2_patch_storage)
    {
        if(firmware < 0x421C)
            Reset1_BDVD();
        else
            Reset2_BDVD();
    }

    if(noBDVD != MODE_DISCLESS)
    {
        bdvd_notify = 1;
        bdvd_ejected = 0;

        mode_favourites = 0;
        select_option = 0;
        //menu_screen = 0;
        select_px = 0;
        select_py = 0;

        currentgamedir = currentdir = 0;
    }
    else
        disc_less_on = 1;
}

int is_libfs_patched(void)
{
    struct stat s;
    char path[256];

    sprintf(path, "%s/libfs_patched.sprx", self_path);
    return (stat(path, &s) == SUCCESS);
}


/******************************************************************************************************************************************************/

bool test_ftp_working()
{
    if(get_ftp_activity())
    {
        if(DrawDialogYesNo("FTP is working now\nDo you want to interrupt the FTP activity?\n\nEl FTP esta trabajando ahora mismo\nDesea interrumpir su actividad?\n\nService FTP en cours\nL'intérompre?") == 1)
        {
            ftp_deinit();
            ftp_inited = 0;
            return false;
        }
        else
            return true;
    } return false;
}

int is_payload_loaded(void)
{
    u64 addr = lv2peek(0x80000000000004f0ULL);

    if((addr>>32) == 0x534B3145)
    {
        addr&= 0xffffffff;
        if(addr && peekq(0x80000000000004f8ULL))
        {
            restore_syscall8[0]= lv2peek(0x80000000000004f8ULL); // (8*8)
            restore_syscall8[1]= lv2peek(restore_syscall8[0]);
            lv2poke(restore_syscall8[0], 0x8000000000000000ULL + (u64) (addr + 0x20));
            return 2;
        }

        return 1;
    }

    return 0;
}

void set_last_game()
{
    int i = select_px + select_py * cols;
    currentgamedir = get_currentdir(i);
    if(currentgamedir < 0 || currentgamedir >= ndirectories || ndirectories <= 0) return;
    last_game_favourites = mode_favourites;
    last_game_flag = directories[currentgamedir].flags;
    strncpy(last_game_id, directories[currentgamedir].title_id, 63);
    last_game_id[63] = 0;
}

void locate_last_game()
{

    u32 flags = 0, f;

    int n, pos = -1;

    if(last_game_flag == 0 || ndirectories <= 0 || allow_restore_last_game == 0) return;

    if(mode_favourites && last_game_favourites)
    {
        for(n = 0; n < num_box; n++)
        {
            if(favourites.list[n].index >= 0 && favourites.list[n].index < ndirectories &&
               favourites.list[n].title_id[0] != 0 && directories[favourites.list[n].index].flags)
            {
                int i = favourites.list[n].index;
                if(!strcmp(directories[i].title_id, last_game_id))
                {
                    f = directories[i].flags;

                    if(f & D_FLAG_HDD0)
                    {
                        flags = f;
                        pos = n;
                        break;
                    }
                    else if((f & NTFS_FLAG) && !(flags & D_FLAG_HDD0))
                    {
                        flags = f;
                        pos = n;
                    }
                    else if(!(flags & (NTFS_FLAG | D_FLAG_HDD0)))
                    {
                        flags = f;
                        pos = n;
                    }
                }
            }
        }

        if(pos >= 0)
        {
            currentdir = 0;

            currentgamedir =  favourites.list[pos].index;
            get_grid_dimensions();

            select_py = (pos / cols);
            select_px = (pos % cols);
            last_game_flag = 0;
            return;
        }

    }

    for(n = 0; n < ndirectories; n++)
    {
        if(!strcmp(directories[n].title_id, last_game_id))
        {
            f = directories[n].flags;
            if((f & last_game_flag) == last_game_flag)
            {
                flags = f;
                pos = n;
                break;
            }
            else if(f & D_FLAG_HDD0)
            {
                flags = f;
                pos = n;
            }
            else if((f & NTFS_FLAG) && !(flags & D_FLAG_HDD0))
            {
                flags = f;
                pos = n;
            }
            else if(!(flags & (NTFS_FLAG | D_FLAG_HDD0)))
            {
                flags = f;
                pos = n;
            }
        }
    }

    if(pos >= 0)
    {
        mode_favourites = 0;

        get_grid_dimensions();

        if(gui_mode != 1)
        {
            currentdir = (pos/num_box) * num_box;
            select_py = ((pos - currentdir) / cols);
            select_px = (pos - currentdir) % cols;
        }
        else
        {
            currentdir = pos;

            if(currentdir <= (cols - 1) )
            {
                select_px = currentdir; select_py = 0;
                currentdir = 0;
            }
            else
            {
                currentdir -= (cols - 1);
                select_px = (cols - 1); select_py = 0;
            }
        }

        int i = select_px + select_py * cols;

        currentgamedir = (currentdir + i);

        get_games();
        load_gamecfg(-1); // force refresh game info

    }

    last_game_flag = 0;
}

void read_settings()
{
    char * file = NULL;
    int file_size = 0;
    struct stat s;

    char ShowPlayOverlay[2];
    char ShowUSBIcon[2];
    char BackgroundGears[2];
    char IconPulse[2];
    char FileManager[2];
    char TimeFormat[2];
    char UnmountDevBlind[2];
    char CachedGameList[2];
    char ShowPIC1[2];
    char ShowVersion[2];
    char SpoofVersion[2];
    char HideCoverflowSortModeLabel[2];
    char TimeoutByInactivity[2];

    // set default values
    sprintf(covers_path, "%s/USRDIR/covers/", MM_PATH);
    sprintf(retro_covers_path, "%s/USRDIR/covers_retro/psx/", MM_PATH);
    sprintf(backgrounds_path, "%s/USRDIR/background/", self_path);
    sprintf(updates_path, "/dev_hdd0/packages");
    sprintf(video_path, "/MKV");
    sprintf(webman_path, "/dev_hdd0/webftp_server.sprx");
    sprintf(psp_launcher_path, "/dev_hdd0/game/PSPC66820");
    sprintf(retroarch_path, "/dev_hdd0/game/SSNE10000");
    sprintf(ps2classic_path, "/PS2ISO");

    sprintf(retro_root_path, "/ROMS/");
    sprintf(retro_snes_path, "%sSNES", retro_root_path);
    sprintf(retro_gba_path, "%sGBA", retro_root_path);
    sprintf(retro_gen_path, "%sGEN", retro_root_path);
    sprintf(retro_nes_path, "%sNES", retro_root_path);
    sprintf(retro_mame_path, "%sMAME", retro_root_path);
    sprintf(retro_fba_path, "%sFBA", retro_root_path);
    sprintf(retro_doom_path, "%sDOOM", retro_root_path);
    sprintf(retro_quake_path, "%sQUAKE", retro_root_path);
    sprintf(retro_pce_path, "%sPCE", retro_root_path);
    sprintf(retro_gb_path, "%sGB", retro_root_path);
    sprintf(retro_gbc_path, "%sGBC", retro_root_path);
    sprintf(retro_atari_path, "%sATARI", retro_root_path);
    sprintf(retro_vb_path, "%sVB", retro_root_path);
    sprintf(retro_nxe_path, "%sNXE", retro_root_path);
    sprintf(retro_wswam_path, "%sWSWAM", retro_root_path);

    sprintf(video_extensions, ".MKV .MP4 .AVI .MPG .MPEG .MOV .M2TS .VOB .FLV .WMV .ASF .DIVX .XVID .PAM .BIK .BINK .VP6 .MTH .3GP .RMVB .OGM .OGV .M2T .MTS .TS .TTS .RM .RV .VP3 .VP5 .VP8 .264 .M1V .M2V .M4B .M4P .M4R .M4V .MP4V .MPE .BDMV .DVB");
    sprintf(audio_extensions, ".MP3 .WAV .WMA .AAC .AC3 .AT3 .OGG .OGA .MP2 .MPA .M4A .FLAC .RA .RAM .AIF .AIFF .MOD .S3M .XM .IT .MTM .STM .UMX .MO3 .NED .669 .MP1 .M1A .M2A .M4B .AA3 .OMA .AIFC");
    sprintf(rom_extensions, ".ZIP .GBA .NES .UNIF .GB .GBC .DMG .MD .SMD .GEN .SMS .GG .SG .BIN .CUE .IOS .FLAC .NGP .NGC .PCE .SGX .CUE .VB .VBOY .BIN .WS .WSC .FDS .EXE .WAD .IWAD .SMC .FIG .SFC .GD3 .GD7 .DX2 .BSX .SWC .A26 .BIN .PAK");
    sprintf(browser_extensions, ".HTML .HTM .TXT .INI .CFG .GIF");

    sprintf(ShowPlayOverlay, "1");
    sprintf(ShowUSBIcon, "1");
    sprintf(BackgroundGears, "1");
    sprintf(IconPulse, "1");
    sprintf(TimeFormat, "1");
    sprintf(UnmountDevBlind, "1");
    sprintf(CachedGameList, "1");
    sprintf(ShowPIC1, "2");
    sprintf(ShowVersion, "2");
    sprintf(HideCoverflowSortModeLabel, "0");
    sprintf(TimeoutByInactivity, "1");
    sprintf(SpoofVersion, "0");

    ftp_port = 21;

    // read settings.ini
    sprintf(temp_buffer, "%s/USRDIR/settings.ini", self_path);

    if(stat(temp_buffer, &s) != SUCCESS) //if settings.ini is not found, then use the settings_default.ini
    {
       sprintf(temp_buffer, "%s/USRDIR/settings_default.ini", self_path);

       if(!stat(temp_buffer, &s))
          file = LoadFile(temp_buffer, &file_size);

       if(file)
       {
           sprintf(temp_buffer, "%s/USRDIR/settings.ini", self_path);
           SaveFile(temp_buffer, (char *) file, file_size);
       }
    }
    else if(!stat(temp_buffer, &s))
            file = LoadFile(temp_buffer, &file_size);

    if(file)
    {
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "Covers", covers_path, MAXPATHLEN - 1, covers_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "RetroCovers", retro_covers_path, MAXPATHLEN - 1, retro_covers_path);

        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "Background", backgrounds_path, MAXPATHLEN - 1, backgrounds_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "Packages", updates_path, MAXPATHLEN - 1, updates_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "Video", video_path, MAXPATHLEN - 1, video_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "webMAN", webman_path, MAXPATHLEN - 1, webman_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "PSPLauncher", psp_launcher_path, MAXPATHLEN - 1, psp_launcher_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "RetroArch", retroarch_path, MAXPATHLEN - 1, retroarch_path);
        getConfigMemValueString((char *) file, file_size, PATHS_SECTION, "PS2Classics", ps2classic_path, MAXPATHLEN - 1, ps2classic_path);

        getConfigMemValueString((char *) file, file_size, EXTENSIONS_SECTION, "Video", video_extensions, 300, video_extensions);
        getConfigMemValueString((char *) file, file_size, EXTENSIONS_SECTION, "Audio", audio_extensions, 300, audio_extensions);
        getConfigMemValueString((char *) file, file_size, EXTENSIONS_SECTION, "Roms", rom_extensions, 300, rom_extensions);
        getConfigMemValueString((char *) file, file_size, EXTENSIONS_SECTION, "Browser", browser_extensions, 100, browser_extensions);

        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "ROMS", retro_root_path, ROMS_MAXPATHLEN - 1, retro_root_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "SNES", retro_snes_path, ROMS_MAXPATHLEN - 1, retro_snes_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "GBA", retro_gba_path, ROMS_MAXPATHLEN - 1, retro_gba_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "GEN", retro_gen_path, ROMS_MAXPATHLEN - 1, retro_gen_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "NES", retro_nes_path, ROMS_MAXPATHLEN - 1, retro_nes_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "MAME", retro_mame_path, ROMS_MAXPATHLEN - 1, retro_mame_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "FBA", retro_fba_path, ROMS_MAXPATHLEN - 1, retro_fba_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "DOOM", retro_doom_path, ROMS_MAXPATHLEN - 1, retro_doom_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "QUAKE", retro_quake_path, ROMS_MAXPATHLEN - 1, retro_quake_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "PCE", retro_pce_path, ROMS_MAXPATHLEN - 1, retro_pce_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "GB", retro_gb_path, ROMS_MAXPATHLEN - 1, retro_gb_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "GBC", retro_gbc_path, ROMS_MAXPATHLEN - 1, retro_gbc_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "ATARI", retro_atari_path, ROMS_MAXPATHLEN - 1, retro_atari_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "VB", retro_vb_path, ROMS_MAXPATHLEN - 1, retro_vb_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "NXE", retro_nxe_path, ROMS_MAXPATHLEN - 1, retro_nxe_path);
        getConfigMemValueString((char *) file, file_size, ROMS_SECTION, "WSWAM", retro_wswam_path, ROMS_MAXPATHLEN - 1, retro_wswam_path);

        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "ShowPlayOverlay", ShowPlayOverlay, 2, ShowPlayOverlay);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "ShowUSBIcon", ShowUSBIcon, 2, ShowUSBIcon);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "BackgroundGears", BackgroundGears, 2, BackgroundGears);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "IconPulse", IconPulse, 2, IconPulse);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "FileManager", FileManager, 2, FileManager);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "TimeFormat", TimeFormat, 2, TimeFormat);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "UnmountDevBlind", UnmountDevBlind, 2, UnmountDevBlind);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "CachedGameList", CachedGameList, 2, CachedGameList);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "ShowPIC1", ShowPIC1, 2, ShowPIC1);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "ShowVersion", ShowVersion, 2, ShowVersion);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "SpoofVersion", SpoofVersion, 2, SpoofVersion);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "HideCoverflowSortModeLabel", HideCoverflowSortModeLabel, 2, HideCoverflowSortModeLabel);
        getConfigMemValueString((char *) file, file_size, GUI_SECTION, "TimeoutByInactivity", TimeoutByInactivity, 2, TimeoutByInactivity);

        ftp_port = getConfigMemValueInt((char *) file, file_size, GUI_SECTION, "FTPport", 21);

        free(file);

        bShowPlayOverlay = strncmp(ShowPlayOverlay, "0", 1);
        bShowUSBIcon = strncmp(ShowUSBIcon, "0", 1);
        bBackgroundGears = strncmp(BackgroundGears, "0", 1);
        bIconPulse = strncmp(IconPulse, "0", 1);
        bFileManager = strncmp(FileManager, "0", 1);
        bUnmountDevBlind = strncmp(UnmountDevBlind, "0", 1);
        bCachedGameList = strncmp(CachedGameList, "0", 1);
        bHideCoverflowSortModeLabel = strncmp(HideCoverflowSortModeLabel, "0", 1);
        bSpoofVersion = strncmp(SpoofVersion, "0", 1);

        if(!strncmp(ShowVersion, "0", 1)) bShowVersion = 0; else //Don't show
        if(!strncmp(ShowVersion, "1", 1)) bShowVersion = 1; else //Show for all PS3 games
                                          bShowVersion = 2;      //Show except for NTFS games

        if(!strncmp(ShowPIC1, "0", 1)) bShowPIC1 = 0; else //Don't show
        if(!strncmp(ShowPIC1, "1", 1)) bShowPIC1 = 1; else //Show for all PS3 games
                                       bShowPIC1 = 2;      //Show except for NTFS games

        if(!strncmp(TimeFormat, "0", 1)) iTimeFormat = 0; else //Hidden
        if(!strncmp(TimeFormat, "1", 1)) iTimeFormat = 1; else //24h
        if(!strncmp(TimeFormat, "2", 1)) iTimeFormat = 2; else //12h
        if(!strncmp(TimeFormat, "3", 1)) iTimeFormat = 3; else //Full date + time (24h)
                                         iTimeFormat = 4;      //Date + time (12h)

        if(!strncmp(TimeoutByInactivity, "1", 1)) iTimeoutByInactivity = 1; else //1hr
        if(!strncmp(TimeoutByInactivity, "2", 1)) iTimeoutByInactivity = 2; else //2hrs
        if(!strncmp(TimeoutByInactivity, "3", 1)) iTimeoutByInactivity = 3; else //3hrs
        if(!strncmp(TimeoutByInactivity, "4", 1)) iTimeoutByInactivity = 4; else //4hrs
        if(!strncmp(TimeoutByInactivity, "5", 1)) iTimeoutByInactivity = 5; else //5hrs
        if(!strncmp(TimeoutByInactivity, "6", 1)) iTimeoutByInactivity = 6; else //6hrs
        if(!strncmp(TimeoutByInactivity, "7", 1)) iTimeoutByInactivity = 7; else //7hrs
        if(!strncmp(TimeoutByInactivity, "8", 1)) iTimeoutByInactivity = 8; else //8hrs
                                                  iTimeoutByInactivity = 0;      //disabled
    }

    //supported file extensions
    strtoupper(video_extensions);
    strcat(video_extensions, " ");

    strtoupper(audio_extensions);
    strcat(audio_extensions, " ");

    strtoupper(rom_extensions);
    strcat(rom_extensions, " ");

    strtoupper(browser_extensions);
    strcat(browser_extensions, " ");

    // reset default psp launcher (if don't exist)
    if(stat(psp_launcher_path, &s) != SUCCESS)
       sprintf(psp_launcher_path, "/dev_hdd0/game/PSPC66820");

    // reset default covers path (if don't exist)
    if(stat(covers_path, &s) != SUCCESS)
        sprintf(covers_path, "/dev_hdd0/GAMES/covers/");

    // reset default RetroArch path (if don't exist)
    if(stat(retroarch_path, &s) != SUCCESS)
        sprintf(retroarch_path, "/dev_hdd0/game/SSNE10000");

    mkdir_secure(covers_path);
    mkdir_secure(backgrounds_path);
    mkdir_secure(updates_path);
}


s32 main(s32 argc, const char* argv[])
{
    int n;

    u32 entry = 0;
    u32 segmentcount = 0;
    sysSpuSegment * segments;

    if(lv2peek(0x80000000000004E8ULL)) syscall_40(1, 0); // disables PS3 Disc-less

    NTFS_init_system_io();

    event_threads_init();

    atexit(fun_exit);

    int must_patch = 0;

    if(lv2peek(0x80000000007EF220ULL) == 0x45737477616C6420ULL && is_payload_loaded())
    {
        must_patch = 1;

        sys8_path_table(0LL); // break libfs.sprx re-direction

        if(restore_syscall8[0]) sys8_pokeinstr(restore_syscall8[0], restore_syscall8[1]);
    }

    if(sysModuleLoad(SYSMODULE_FS)      == SUCCESS) inited|= INITED_FS;      else exit(0);
    if(sysModuleLoad(SYSMODULE_PNGDEC)  == SUCCESS) inited|= INITED_PNGDEC;  else exit(0);
    if(sysModuleLoad(SYSMODULE_JPGDEC)  == SUCCESS) inited|= INITED_JPGDEC;  else exit(0);
    if(sysModuleLoad(SYSMODULE_IO)      == SUCCESS) inited|= INITED_IO;      else exit(0);
    if(sysModuleLoad(SYSMODULE_GCM_SYS) == SUCCESS) inited|= INITED_GCM_SYS; else exit(0);
    if(sysModuleLoad(SYSMODULE_HTTPS)   == SUCCESS) inited|= INITED_HTTPS;   else exit(0);

    sysModuleLoad(SYSMODULE_SYSUTIL);
    sysSpuInitialize(6, 5);

    sysSpuRawCreate(&spu, NULL);

    sysSpuElfGetInformation(spu_soundmodule_bin, &entry, &segmentcount);

    size_t segmentsize = sizeof(sysSpuSegment) * segmentcount;
    segments = (sysSpuSegment*)memalign(128, SPU_SIZE(segmentsize)); // must be aligned to 128 or it break malloc() allocations
    memset(segments, 0, segmentsize);

    sysSpuElfGetSegments(spu_soundmodule_bin, segments, segmentcount);

    sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0);

    sysSpuRawImageLoad(spu, &spu_image);

    inited |= INITED_SPU;

    if(SND_Init(spu) == SUCCESS) inited |= INITED_SOUNDLIB;

    if(argc > 0 && argv)
    {
        int n;

        if(!strncmp(argv[0], "/dev_hdd0/game/", 15))
        {
            strcpy(self_path, argv[0]);

            n = 15; while(self_path[n] != '/' && self_path[n] != 0) n++;

            if(self_path[n] == '/') self_path[n] = 0;
        }
        else if(!strncmp(argv[0], "/dev_bdvd/PS3_GAME/", 19))
        {
            strcpy(self_path, "/dev_bdvd/PS3_GAME");
        }
    }

    if (strstr(self_path, "/BLES80608")) sprintf(self_path, "/dev_hdd0/game/IRISMAN00");

    if(is_firm_341())
    {
        firmware = 0x341C;
        fw_ver = 0x8534;
        off_idps = 0x80000000003BA880ULL;
        off_idps2 = 0x800000000044A174ULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_341();
    }
    else if(is_firm_355())
    {
        firmware = 0x355C;
        fw_ver = 0x8AAC;
        off_idps = 0x80000000003C2EF0ULL;
        off_idps2 = 0x8000000000452174ULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_355();
    }
    else if(is_firm_355dex())
    {
        firmware = 0x355D;
        fw_ver = 0x8AAC;
        off_idps = 0x80000000003DE170ULL;
        off_idps2 = 0x8000000000472174ULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_355dex();
    }
    else if(is_firm_421())
    {
        firmware = 0x421C;
        fw_ver = 0xA474;
        off_idps = 0x80000000003D9230ULL;
        off_idps2 = 0x8000000000477E9CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_421();
    }
    else if(is_firm_421dex())
    {
        firmware = 0x421D;
        fw_ver = 0xA474;
        off_idps = 0x80000000003F7A30ULL;
        off_idps2 = 0x800000000048FE9CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_421dex();
    }
    else if(is_firm_430())
    {
        firmware = 0x430C;
        fw_ver = 0xA7F8;
        off_idps = 0x80000000003DB1B0ULL;
        off_idps2 = 0x8000000000476F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_430();
    }
    else if(is_firm_431())
    {
        firmware = 0x431C;
        fw_ver = 0xA85C;
        off_idps = 0x80000000003DB1B0ULL;
        off_idps2 = 0x8000000000476F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_431();
    }
    else if(is_firm_430dex())
    {
        firmware = 0x430D;
        fw_ver = 0xA7F8;
        off_idps = 0x80000000003F9930ULL;
        off_idps2 = 0x8000000000496F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_430dex();
    }
    else if(is_firm_440())
    {
        firmware = 0x440C;
        fw_ver = 0xABE0;
        off_idps = 0x80000000003DB830ULL;
        off_idps2 = 0x8000000000476F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_440();
    }
    else if(is_firm_441())
    {
        firmware = 0x441C;
        fw_ver = 0xAC44;
        off_idps = 0x80000000003DB830ULL;
        off_idps2 = 0x8000000000476F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_441();
    }
    else if(is_firm_441dex())
    {
        //if( is_file_exist( "/dev_flash/ps3itald.self" ) == 1 && is_file_exist( "/dev_flash/ps3ita" ) == 1) is_ps3ita = 1;
        firmware = 0x441D;
        fw_ver = 0xAC44;
        off_idps = 0x80000000003FA2B0ULL;
        off_idps2 = 0x8000000000496F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_441dex();
    }
    else if(is_firm_446())
    {
        firmware = 0x446C;
        fw_ver = 0xAE38;
        off_idps = 0x80000000003DBE30ULL;
        off_idps2 = 0x8000000000476F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_446();
    }
    else if(is_firm_446dex())
    {
        firmware = 0x446D;
        fw_ver = 0xAE38;
        off_idps = 0x80000000003FA8B0ULL;
        off_idps2 = 0x8000000000496F3CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_446dex();
    }
    else if(is_firm_450())
    {
        firmware = 0x450C;
        fw_ver = 0xAFC8;
        off_idps = 0x80000000003DE230ULL;
        off_idps2 = 0x800000000046CF0CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_450();
    }
    else if(is_firm_450dex())
    {
        //if( is_file_exist( "/dev_flash/ps3itald.self" ) == 1 && is_file_exist( "/dev_flash/ps3ita" ) == 1) is_ps3ita = 1;
        firmware = 0x450D;
        fw_ver = 0xAFC8;
        off_idps = 0x8000000000402AB0ULL;
        off_idps2 = 0x8000000000494F0CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_450dex();
    }
    else if(is_firm_453())
    {
        firmware = 0x453C;
        fw_ver = 0xB0F4;
        off_idps = 0x80000000003DE430ULL;
        off_idps2 = 0x800000000046CF0CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_453();
    }
    else if(is_firm_455())
    {
        firmware = 0x455C;
        fw_ver = 0xB1BC;
        off_idps = 0x80000000003E17B0ULL;
        off_idps2= 0x8000000000474F1CULL;
        off_psid = off_idps2 + 0x18ULL;
        payload_mode = is_payload_loaded_455();
    }

    if(is_cobra_based()) use_cobra = 1;

    //sprintf(temp_buffer + 0x1000, "firmware: %xex payload %i", firmware, payload_mode);

    ///////////////////////////

    switch(firmware)
    {
        case 0x341C:
            set_bdvdemu_341(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_341(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case HERMES_PAYLOAD:
                    break;
            }
            break;
        case 0x355C:
            set_bdvdemu_355(payload_mode);
            switch(payload_mode)
            {
                case WANIN_PAYLOAD:
                case ZERO_PAYLOAD: //no payload installed
                    install_new_poke(); /* need for patch lv2 */

                    if (!map_lv1())
                    {
                        remove_new_poke();

                        tiny3d_Init(1024*1024);
                        ioPadInit(7);
                        DrawDialogOK("Error Loading Payload: map failed?!");
                        exit(0);
                    }

                    patch_lv2_protection(); /* yaw */
                    remove_new_poke(); /* restore pokes */

                    unmap_lv1();  /* 3.55 need unmap? */

                    __asm__("sync");
                    sleep(1); /* dont touch! nein! */

                    //please, do not translate this strings - i preffer this errors in english for better support...
                    if(payload_mode == WANIN_PAYLOAD)
                    {
                        sys8_disable_all = 1;
                        sprintf(temp_buffer, "WANINV2 DETECTED\nOLD SYSCALL 36 LOADED (mode=%i)\n\n - no big files allowed with this payload -", payload_mode);
                        sprintf(payload_str, "wanin cfw - old syscall36, no bigfiles allowed");
                    }
                    else
                    {
                        load_payload_355(payload_mode);

                        __asm__("sync");
                        sleep(1); /* maybe need it, maybe not */
                    }
                    break;
                case SYS36_PAYLOAD:
                    sys8_disable_all = 1;
                    sprintf(temp_buffer, "OLD SYSCALL 36 RESIDENT, RESPECT!\nNEW PAYLOAD NOT LOADED...\n\n - no big files allowed with this payload -");
                    sprintf(payload_str, "syscall36 resident - new payload no loaded, no bigfiles allowed");
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x355D: //355dex
            set_bdvdemu_355dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    install_new_poke_355dex();
                    if (!map_lv1_355dex())
                    {
                        remove_new_poke_355dex();

                        tiny3d_Init(1024*1024);
                        ioPadInit(7);
                        DrawDialogOK("Error Loading Payload: map failed?!");
                        exit(0);
                    }
                    patch_lv2_protection_355dex(); /* yaw */

                    remove_new_poke_355dex(); /* restore pokes */
                    unmap_lv1_355dex();  /* 3.55 need unmap? */
                    __asm__("sync");

                    load_payload_355dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x421C:
            set_bdvdemu_421(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_421(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x421D: //4.21 dex
            set_bdvdemu_421dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_421dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x430C:
            set_bdvdemu_430(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_430(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x430D:
            set_bdvdemu_430dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_430dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x431C:
            set_bdvdemu_431(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_431(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x440C:
            set_bdvdemu_440(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_440(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x441C:
            set_bdvdemu_441(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_441(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x441D:
            set_bdvdemu_441dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_441dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x446C:
            set_bdvdemu_446(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_446(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */

                    if(!use_cobra)
                    {
                        load_ps3_mamba_payload();
                        use_mamba = 1;
                    }
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x446D:
            set_bdvdemu_446dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_446dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x450C:
            set_bdvdemu_450(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_450(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x450D:
            set_bdvdemu_450dex(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_450dex(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x453C:
            set_bdvdemu_453(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_453(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */

                    if(!use_cobra)
                    {
                        load_ps3_mamba_payload();
                        use_mamba = 1;
                    }
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        case 0x455C:
            set_bdvdemu_455(payload_mode);
            switch(payload_mode)
            {
                case ZERO_PAYLOAD: //no payload installed
                    load_payload_455(payload_mode);
                    __asm__("sync");
                    sleep(1); /* maybe need it, maybe not */

                    if(!use_cobra)
                    {
                        load_ps3_mamba_payload();
                        use_mamba = 1;
                    }
                    break;
                case SKY10_PAYLOAD:
                    break;
            }
            break;
        default:
            tiny3d_Init(1024*1024);
            ioPadInit(7);
            DrawDialogOK("Error: Unsupported firmware!");
            exit(0);
            break;
    }

    usleep(250000);

    if(payload_mode >= ZERO_PAYLOAD && sys8_disable_all == 0)
    {
        int test = 0x100;

        //check syscall8 status
        test = sys8_enable(0ULL);
        if((test & 0xff00) == 0x300)
        {
            if(payload_mode == ZERO_PAYLOAD)
            {
                if(firmware== 0x341C)
                    sprintf(payload_str, "payload-hermes - new syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                else
                {
                    if(use_mamba)
                        sprintf(payload_str, "payload-sk1e - 'Mamba' syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                    else if(use_cobra)
                        sprintf(payload_str, "payload-sk1e - 'Cobra' syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                    else
                        sprintf(payload_str, "payload-sk1e - new syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                }
            }
            else if (payload_mode == SKY10_PAYLOAD)
            {
                if(use_cobra && sys8_mamba() == 0x666)
                    sprintf(payload_str, "payload-sk1e - 'Mamba' syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                else if(use_cobra)
                    sprintf(payload_str, "payload-sk1e - 'Cobra' syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
                else
                    sprintf(payload_str, "payload-sk1e - new syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");
            }
            else
                sprintf(payload_str, "payload-hermes resident - new syscall8 v%i (libfs_patched %s)", test & 0xff, is_libfs_patched()? "found!": "not found");

        }
        else
        {       sys8_disable_all = 1;
                sprintf(payload_str, "payload-sk10 - new syscall8 Err?! v(%i)", test);
        }
    }

    // ADVERTENCIA: este codigo no debe ser removido de aqui para evitar pantalla negra por los parches (problemas de cache)
    // CAUTION: this code should not be removed from here to avoid black screen caused by the patches (e.g. cache problems)
    tiny3d_Init(1024*1024);
    ioPadInit(7);
    usleep(250000);

    if(sys8_disable_all != 0)
    {
         if(DrawDialogYesNo2("Syscall 8 very old or not detected\n\nDo you want to REBOOT the PS3?\n\n(Select NO to exit to XMB)") == 1)
         {
             set_install_pkg = 1;
             game_cfg.direct_boot = 0;
             exit(0);
         }
         else
             exit(0);
    }

    if(!use_mamba)
    {
        if(must_patch) sys8_pokeinstr(0x80000000007EF220ULL, 0x0ULL);

        // disable ps1 emulation
        unload_psx_payload();

        {
            FILE *fp = fopen("/dev_hdd0/game/HOMELAUN1/path.bin", "rb");
            if(fp)
            {
                fclose(fp); homelaun = 1;
                fp = fopen("/dev_hdd0/game/HOMELAUN1/path2.bin", "rb");
                if(fp) {fclose(fp); homelaun = 2;}
            }
        }

        // turn off
        sys8_perm_mode(1);
        usleep(5000);


        // sys8_perm_mode(0);
        sys8_path_table(0LL);


        if(lv2_patch_storage)
        {   // for PSX games
            if(lv2_patch_storage() < 0) lv2_patch_storage = NULL;
        }
    }
    else
        lv2_patch_storage = lv2_unpatch_storage = NULL;


    if(payload_mode < ZERO_PAYLOAD) //if mode is wanin or worse, launch advert
    {
        DrawDialogOK(temp_buffer);
    }

    // read custom settings
    read_settings();

    // spoof firmware
    if(use_cobra && !use_mamba)
    {
        u8 cconfig[15];
        CobraConfig *cobra_config = (CobraConfig*) cconfig;
        memset(cobra_config, 0, 15);
        cobra_read_config(cobra_config);

        cobra_config->spoof_version  = 0;
        cobra_config->spoof_revision = 0;

        if(bSpoofVersion != 0 && (firmware == 0x446C || firmware == 0x453C))
        {
            cobra_config->spoof_version  = 0x0455;
            cobra_config->spoof_revision = 62848;
        }

        if(cobra_config->ps2softemu == 0 && cobra_get_ps2_emu_type() == PS2_EMU_SW)
            cobra_config->ps2softemu = 1;

        cobra_write_config(cobra_config);
    }

    sprintf(temp_buffer, "%s/config", self_path);
    mkdir_secure(temp_buffer);

    sprintf(temp_buffer, "%s/cache", self_path);
    mkdir_secure(temp_buffer);

    //sprintf(temp_buffer, "%s/PKG", self_path);
    mkdir_secure(updates_path);

    // initialize manager conf
    memset(&manager_cfg, 0, sizeof(manager_cfg));

    for(n = 0; n < 4; n++)
        manager_cfg.videoscale_x[n] = 1024;

    manager_cfg.background_sel = 0;
    manager_cfg.noBDVD = MODE_DISCLESS;
    manager_cfg.language = 99;

    // get default console id
    get_console_id_eid5();
    strcpy(default_console_id, console_id);

    // spoof console id (idps)/psid
    load_spoofed_console_id();
    load_spoofed_psid();

    // load cfg and language strings
    LoadManagerCfg();

    bk_picture = manager_cfg.bk_picture;

    noBDVD = manager_cfg.noBDVD;
    gui_mode = manager_cfg.gui_mode & 15;

    sort_mode = 0;
    if(gui_mode == MODE_COVERFLOW) sort_mode = (manager_cfg.gui_mode>>4);

    get_grid_dimensions();

    if(noBDVD == MODE_NOBDVD)
    {
        use_cobra = 0;
        use_mamba = 0;
    }

    // load ps3 disc less payload
    if(noBDVD == MODE_DISCLESS && !use_cobra && !use_mamba)
    {
        //lv2poke(syscall_base +(u64) (40 * 8), lv2peek(syscall_base));
        load_ps3_discless_payload();

        if(firmware == 0x341C || firmware == 0x355C)
            syscall_40(6, 0x8d000B04);
        else /*if(firmware == 0x421C)
            syscall_40(6, 0x8d001A04);
        else*/
            if(firmware != 0x430C && firmware != 0x431C && firmware != 0x440C && manager_cfg.event_flag)
                syscall_40(6, manager_cfg.event_flag);

    }

    load_controlfan_config();

    set_device_wakeup_mode(0);

    LoadPSXOptions(NULL);

    // Load texture

    LoadTexture();

    init_twat();

    if(videoscale_x >= 1024)
    {
        videoscale_x = videoscale_y = 0;
        video_adjust();
    }

    if(manager_cfg.hdd_folder[0] == 0)
    {
        Select_games_folder();

        if(manager_cfg.hdd_folder[0] == 0) strcpy(manager_cfg.hdd_folder, __MKDEF_MANAGER_DIR__);
        SaveManagerCfg();
    }

    strncpy(hdd_folder, manager_cfg.hdd_folder, 64);


    double sx = (double) Video_Resolution.width;
    double sy = (double) Video_Resolution.height;
    double psx = (double) (1000 + videoscale_x)/1000.0;
    double psy = (double) (1000 + videoscale_y)/1000.0;

    tiny3d_UserViewport(1,
        (float) ((sx - sx * psx) / 2.0), // 2D position
        (float) ((sy - sy * psy) / 2.0),
        (float) ((sx * psx) / 848.0),    // 2D scale
        (float) ((sy * psy) / 512.0),
        (float) ((sx / 1920.0) * psx),  // 3D scale
        (float) ((sy / 1080.0) * psy));

    if(use_mamba && !use_cobra)
    {
        syscall_40(1, 0);
        fun_exit();
        restore_syscall8[1]= lv2peek(restore_syscall8[0]); // use mamba vector

        // relaunch iris manager to get vsh process for mamba
        sprintf(temp_buffer, "%s/USRDIR/RELOAD.SELF", self_path);
        sysProcessExitSpawn2(temp_buffer, NULL, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
        exit(0);
    }

    // disable cobra and mamba flags if "no BDVD device" mode is set
    if(noBDVD == MODE_NOBDVD)
    {
        use_cobra = 0;
        use_mamba = 0;
    }
    else
    {
        // from reload, use_mamba is zero: this code detect if mamba is present and set to 1
        if(sys8_mamba() == 0x666) use_mamba = 1;
    }

    select_px = select_py = 0;

    fdevices = 0;
    fdevices_old = 0;
    forcedevices = 0;
    find_device = 0;
    app_ver[0] = 0;

    //syscall36("/dev_bdvd");
    add_sys8_bdvd(NULL, NULL);

    //sys8_perm_mode((u64) 0);

    unpatch_bdvdemu();

    if(noBDVD && !use_cobra)
    {
        //sys_fs_umount("/dev_ps2disc");
        sys_fs_umount("/dev_bdvd");
        sleep(0);
        sys_fs_mount("CELL_FS_UTILITY:HDD1", "CELL_FS_FAT", "/dev_bdvd", 1);
    }

    //sys_fs_umount("/dev_bdvd");
    //    sleep(0);
    //sys_fs_mount("CELL_FS_IOS:BDVD_DRIVE", "CELL_FS_ISO9660", "/dev_bdvd", 1);

    // eject disc with cobra method
    if(noBDVD && use_cobra)
    {
        cobra_send_fake_disc_eject_event();
        cobra_umount_disc_image();
        cobra_unload_vsh_plugin(0); // unload ISO plugin in slot 0 (reserved)
    }

    init_music(-1);

    if(!noBDVD && lv2_patch_storage)
    {
        if(!bdvd_notify)
        {
            DIR  *dir;
            dir = opendir("/dev_bdvd");

            if (dir)
                closedir (dir);
            else if(firmware < 0x421C)
                Reset1_BDVD();
            else
                Reset2_BDVD();
        }

        bdvd_notify = 1;
    }


    sysDiscRegisterDiscChangeCallback(&DiscEjectCallback, &DiscInsertCallback);

    if(bUnmountDevBlind)
    {
        sys_fs_umount("/dev_blind");
        sys_fs_umount("/dev_habib");
        sys_fs_umount("/dev_rewrite");
    }

    sys_fs_umount("/dev_moo");

    if(lv2peek(0x80000000000004E8ULL) && noBDVD == MODE_DISCLESS && !use_cobra)
    {
        syscall_40(1, 2);
    }

    if(noBDVD == MODE_DISCLESS && !use_cobra && syscall_40(3, 0) == 0)
    {
        DrawDialogTimer(language[PLUG_STORAGE1], 2000.0f);
    }

    // read xRegitry datas
    read_from_registry();

    if(sys_parental_level != 0 && sys_parental_level < 9) options_locked = 1;

    if(use_cobra)
    {
        struct stat s;
        sprintf(temp_buffer, "%s/sprx_iso", self_path);
        if(stat(temp_buffer, &s) != SUCCESS || s.st_size != SIZE_SPRX_ISO)
            SaveFile(temp_buffer, (char *) sprx_iso, SIZE_SPRX_ISO);
    }

    u32 old_ntfs_ports = 0;

    // Launch PSX iso stored on ./USRDIR/ps1_iso
    if (autolaunch == LAUNCHMODE_TOCHECK)
    {
        float x = 0.0f, y = 0.0f;

        sprintf(filename, "%s/USRDIR/ps1_iso", self_path);
        if (fill_iso_entries_from_device(filename, PS1_FLAG, directories, &ndirectories) > 0)
        {
            sort_entries2(directories, &ndirectories, sort_mode);

            autolaunch = LAUNCHMODE_STARTED;
            directories[autolaunch].flags = PS1_FLAG;
            gui_mode = 1;

            ps3pad_read();

            if ((new_pad | old_pad) & (BUTTON_L1 | BUTTON_R1))
            {
                menu_screen = 444;

                while (menu_screen != 0)
                {
                    flash = (frame_count >> 5) & 1;
                    frame_count++;

                    cls();

                    update_twat(0);

                    switch(menu_screen)
                    {
                        case 444:
                            draw_psx_options(x, y, autolaunch);
                            break;
                        case 445:
                            draw_psx_options2(x, y, autolaunch);
                            break;
                        default:
                            menu_screen = 0;
                            break;
                    }

                    ps3pad_read();
                }
            }

            if (!((new_pad | old_pad) & BUTTON_SELECT))
            {
                if (strstr(directories[autolaunch].path_name, "/PSXISO/") != NULL)
                    launch_iso_game(directories[autolaunch].path_name, EMU_PSX);
                else
                {
                    reset_sys8_path_table();

                    //syscall36("/dev_bdvd");
                    add_sys8_bdvd(NULL, NULL);

                    if(lv2peek(0x80000000000004E8ULL) && !use_cobra) syscall_40(1, 0); // disables PS3 Disc-less

                    // load PSX options
                    LoadPSXOptions(filename);

                    if(psx_iso_prepare(filename, directories[autolaunch].title, NULL) == 0)  exit(0);

                    psx_launch();
                }

                exit(0);
            }
        }
    }

    //// Load Favorites
    sprintf(temp_buffer, "%s/config/", self_path);
    LoadFavourites(temp_buffer, GAMEBASE_MODE);

    sprintf(temp_buffer, "%s/config/", self_path);
    LoadFavourites(temp_buffer, HOMEBREW_MODE);

    sprintf(temp_buffer, "%s/config/", self_path);
    LoadFavourites(temp_buffer, HOMEBREW_MODE + 1);

    GetFavourites(mode_homebrew);
    //


    //// Load Background Picture
    load_background_picture();
    //

    //// Load Cached Game List
    ps3pad_read();
    if(bCachedGameList)
    {
        if((new_pad | old_pad) & (BUTTON_R1)) bCachedGameList = 0;
    }
    if (bFileManager || (new_pad | old_pad) & (BUTTON_L2 | BUTTON_R2))
    {
        autolaunch = LAUNCHMODE_CHECKED;
        file_manager(NULL, NULL);
    }

    // Load Cached Game List
    LoadGameList();

    // remove temporary showtime items
    sprintf(temp_buffer, "%s/USRDIR/TEMP/SHOWTIME.TXT", self_path);
    unlink_secure(temp_buffer);

    sprintf(temp_buffer, "%s/USRDIR/temp.html", self_path);
    unlink_secure(temp_buffer);

    sprintf(temp_buffer, "%s/USRDIR/TEMP", self_path);
    rmdir_secure(temp_buffer);
    //

    // get estimated texture used
    //sprintf(temp_buffer, "Textures used: %u bytes", text_size);
    //DrawDialogOK(temp_buffer);


    //////////////////////////////////////////
    // Main loop
    /////////////////////////////////////////
    while(!exit_program)
    {
        float x = 0.0f, y = 0.0f;

        flash = (frame_count >> 5) & 1;

        frame_count++;

        int count_devices = 0;

        int found_game_insert = 0;

        int found_game_remove = 0;

        if(tiny3d_MenuActive()) frame_count = 32; // to avoid the access to hdd when menu is active

        // NTFS Automount
        for(int i = 0; i < 8; i++)
        {
            int r = NTFS_Event_Mount(i);

            if(r == NTFS_DEVICE_MOUNT)
            {   // mount device
                NTFS_UnMount(i);
                mounts[i] = NULL;
                mountCount[i] = 0;

                mountCount[i] = ntfsMountDevice(disc_ntfs[i], &mounts[i], NTFS_DEFAULT | NTFS_RECOVER);

                if(autolaunch == LAUNCHMODE_CHECKED)
                {
                    SaveLastGame();
                    autolaunch = LAUNCHMODE_REFRESH;
                    frame_count = 0;
                    allow_restore_last_game = 1;
                }
            }
            else if(r == NTFS_DEVICE_UNMOUNT && mounts[i] != NULL)
            { // unmount device
               NTFS_UnMount(i);
               mounts[i] = NULL;
               mountCount[i] = 0;
            }
        }
        // NTFS Automount

        int signal_force = (fdevices == 0);

        if(forcedevices || (frame_count & 63) == 0 || signal_force)
        {
          for(find_device = 0; find_device < 12; find_device++)
          {

            if(find_device == HDD0_DEVICE)
                sprintf(filename, "/dev_hdd0");
            else if(find_device == BDVD_DEVICE)
                sprintf(filename, "/dev_bdvd");
            else
                sprintf(filename, "/dev_usb00%c", 47 + find_device);

            if((!forcedevices || (fdevices & D_FLAG_BDVD)) && find_device == BDVD_DEVICE && bdvd_notify == 0) {goto skip_bdvd;}

            sysFSStat dstat;
            int ret;

            if(find_device == BDVD_DEVICE && psx_inserted < 0)
                ret = FAILED;
            else
                ret = sysLv2FsStat(filename, &dstat);

            if (ret == SUCCESS)
            {

                // check bdemu
                if((fdevices & (1<<find_device)) == 0 && find_device >= 0 && find_device < 11)
                    move_bdemubackup_to_origin(1 << find_device);

                if(fdevices == 0xff) {fdevices_old = fdevices = 0;}
                fdevices|= 1<<find_device;

                // check psx
                if(find_device == BDVD_DEVICE)
                {
                    if(!noBDVD && (get_psx_region_cd() & 0x10) == 0x10) {psx_inserted |= 0x100;}
                    else
                    {
                        ret = sysLv2FsStat("/dev_bdvd/PS3_GAME", &dstat);
                        if(ret != SUCCESS) {psx_inserted = 0; fdevices^= 1<<find_device;}
                    }
                }

                if(find_device == BDVD_DEVICE && get_disc_ready()) bdvd_notify = 0;
            }
            else
            {
                // check psx
                if(find_device == BDVD_DEVICE && !noBDVD && (get_psx_region_cd() & 0x10) == 0x10)
                    {bdvd_notify = 0;fdevices|= 1<<find_device; psx_inserted |= 0x100;}
                else
                {
                    if(find_device == BDVD_DEVICE) psx_inserted = 0;
                    fdevices&= ~ (1<<find_device);
                }
            }

skip_bdvd:
            // limit to 3 the devices selectables
            if(((fdevices>>find_device) & 1) && find_device != BDVD_DEVICE)
            {
                count_devices++;

               if(count_devices > 3) fdevices&= ~ (1<<find_device);
            }

            // bdvd
            if(find_device == BDVD_DEVICE)
            {
                if((fdevices != fdevices_old || ((forcedevices>>find_device) & 1)))
                {
                    struct stat s;
                    found_game_insert = 1;
                    currentdir = 0;

                    // detect psx code
                    if(!noBDVD &&
                       (stat("/dev_bdvd/PSX.EXE", &s) == SUCCESS || stat("/dev_bdvd/SYSTEM.CNF", &s) == SUCCESS || (psx_inserted & 0x100) == 0x100))
                    {
                        psx_inserted &= ~0x100;
                        psx_inserted |= 0x80;
                        strncpy(bluray_game, "PSX-GAME", 64);
                        bdvd_ejected = 0;
                        if(!noBDVD && lv2_patch_storage)
                        {
                            psx_inserted|= get_psx_region_cd();
                            strncpy(bluray_game, (char *) psx_id, 64);
                            mode_favourites = 0;
                            select_option = 0;
                            menu_screen = 0;
                            select_px = 0;
                            select_py = 0;
                        }
                        bluray_game[63] = 0;
                    }
                    else
                    {
                        sprintf(filename, "/dev_bdvd/PS3_GAME/PARAM.SFO");
                        bluray_game[0] = 0;
                        // whatever... unused if -- remove later?
                        if(parse_param_sfo("/dev_bdvd/PS3_GAME/PARAM.SFO", bluray_game) == -1);
                        bluray_game[63] = 0;
                    }
                    found_game_insert = 1;
                    if(((fdevices>>BDVD_DEVICE) & 1)  && !mode_homebrew && !noBDVD)
                    {
                        if(ndirectories >= MAX_DIRECTORIES) ndirectories = MAX_DIRECTORIES - 1;

                        sprintf(directories[ndirectories].path_name, "/dev_bdvd");

                        memcpy(directories[ndirectories].title, bluray_game, 63);
                        directories[ndirectories].title[63] = 0;
                        directories[ndirectories].flags = BDVD_FLAG | ((psx_inserted & 0xff)<<16);
                        if(!psx_inserted)
                        {
                            sprintf(filename, "%s/%s", directories[ndirectories].path_name, "PS3_DISC.SFB" );
                            parse_ps3_disc((char *) filename, directories[ndirectories].title_id);
                            directories[ndirectories].title_id[63] = 0;
                        }
                        else
                            strncpy(directories[ndirectories].title_id, "PSX-GAME", 64);

                        ndirectories++;
                        found_game_insert = 1;

                        s32 fdr;

                        if(!noBDVD && !sysLv2FsOpen("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN", 0, &fdr, S_IRWXU | S_IRWXG | S_IRWXO, NULL, 0))
                        {
                            u64 bytes;
                            u32 dat;
                            bdvd_ejected = 0;
                            static int counter = 0;
                            if(sysLv2FsRead(fdr, (void *) &dat, 4, &bytes) != SUCCESS) bytes = 0LL;

                            if(bytes == 4 && dat != 0x53434500)
                            {
                                if(counter == 0)
                                {
                                    counter = 1;
                                    if(!noBDVD && lv2_patch_storage)
                                    {
                                        bdvd_notify = 0;
                                        Eject_BDVD(EJECT_BDVD);
                                        Eject_BDVD(NOWAIT_BDVD | LOAD_BDVD);
                                    }
                                    else DrawDialogOK("Warning! You must eject/load the disc");
                                }
                                else DrawDialogOK("Warning! Disc authentication failed");

                                ndirectories--;

                                psx_inserted = 0;
                                fdevices&= ~ BDVD_FLAG;
                                goto skip_bdvd;
                            }
                            else
                            {
                                counter = 0;
                                if(!sys_ss_media_id(temp_buffer + 2048))
                                {
                                    struct stat s;
                                    sprintf(temp_buffer, "%s/config/%s.did", self_path, directories[ndirectories - 1].title_id);
                                    if(stat(temp_buffer, &s) != SUCCESS || s.st_size != 0x10)
                                        SaveFile(temp_buffer, (char *) temp_buffer + 2048, 0x10);
                                }
                            }
                        }

                        //stops_BDVD = 0;
                        //Eject_BDVD(NOWAIT_BDVD | STOP_BDVD);
                        //parse_param_sfo_id(filename, directories[ndirectories].title_id);
                        //directories[ndirectories].title_id[63] = 0;
                    }
                    else
                    {
                        delete_entries(directories, &ndirectories, BDVD_FLAG);
                        found_game_remove = 1;
                    }

                    sort_entries2(directories, &ndirectories, sort_mode);
                }

                forcedevices &= ~ (1<<find_device);
                fdevices_old &= ~ (1<<find_device);
                fdevices_old |= fdevices & (1<<find_device);
            } else
            // refresh list
            if(fdevices!=fdevices_old || ((forcedevices>>find_device) & 1))
            {
                currentdir = 0;
                found_game_insert = 1;
                forcedevices &= ~ (1<<find_device);

                if(find_device == HDD0_DEVICE)
                {
                    if(mode_homebrew)
                        sprintf(filename, "/dev_hdd0/game");
                    else if (!memcmp(hdd_folder, "dev_hdd0", 9))
                        sprintf(filename, "/%s/" __MKDEF_GAMES_DIR, hdd_folder);
                    else if (!memcmp(hdd_folder, "GAMES", 6) || !memcmp(hdd_folder, "dev_hdd0_2", 11))
                        sprintf(filename, "/dev_hdd0/GAMES");
                    else if (!memcmp(hdd_folder, "host_root", 10))
                        sprintf(filename, "/host_root");
                    else if (!memcmp(hdd_folder, "video", 6))
                        sprintf(filename, "/dev_hdd0/video");
                    else if (strstr(hdd_folder, "/"))
                        sprintf(filename, hdd_folder);
                    else
                        sprintf(filename, "/dev_hdd0/game/%s/" __MKDEF_GAMES_DIR, hdd_folder);

                    sysFsGetFreeSize("/dev_hdd0/", &blockSize, &freeSize);
                    freeSpace[find_device] = ( ((u64)blockSize * freeSize));
                    freeSpace[find_device] = freeSpace[find_device] / GIGABYTES;

                }
                else
                {
                    sprintf(filename, "/dev_usb00%c/", 47 + find_device);
                    sysFsGetFreeSize(filename, &blockSize, &freeSize);
                    double space = ( ((double)blockSize) * ((double) freeSize) ) /  GIGABYTES;
                    freeSpace[find_device] = (float) space;
                    if(!mode_homebrew)
                        sprintf(filename, "/dev_usb00%c/GAMES", 47 + find_device);
                    else
                        sprintf(filename, "/dev_usb00%c/game", 47 + find_device);
                }

                // BDISO, DVDISO, MKV
                if(mode_homebrew != 0 && use_cobra && noBDVD == MODE_DISCLESS && ((fdevices>>find_device) & 1) && find_device == HDD0_DEVICE)
                {
                    // isos BR-DVD
                    int n;
                    char file[0x420];

                    delete_entries(directories, &ndirectories, (1<<find_device));

                    strncpy(file, filename, 0x420);
                    n = 1; while(file[n] != '/' && file[n] != 0) n++;

                    file[n] = 0; strcat(file, "/BDISO");
                    fill_iso_entries_from_device(file, D_FLAG_HOMEB | D_FLAG_HOMEB_BD | (1<<find_device), directories, &ndirectories);

                    file[n] = 0; strcat(file, "/DVDISO");
                    fill_iso_entries_from_device(file, D_FLAG_HOMEB | D_FLAG_HOMEB_DVD | (1<<find_device), directories, &ndirectories);

                    file[n] = 0; strcat(file, "/VIDEO");
                    fill_iso_entries_from_device(file, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | (1<<find_device), directories, &ndirectories);

                    file[n] = 0; strcat(file, "/MOVIES");
                    fill_iso_entries_from_device(file, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | (1<<find_device), directories, &ndirectories);

                    file[n] = 0; strcat(file, video_path);
                    fill_iso_entries_from_device(file, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | (1<<find_device), directories, &ndirectories);

                    found_game_insert = 1;
                }
                else if(((fdevices>>find_device) & 1) && (!mode_homebrew || (mode_homebrew && find_device != 0)) )
                {
                    if (fill_entries_from_device(filename, directories, &ndirectories, (1<<find_device) | (HOMEBREW_FLAG * (mode_homebrew != 0)), 0 | (2 * (mode_homebrew != 0)), 0) == SUCCESS)
                    {
                        //append gamez, games_dup, games_bad
                        if(!mode_homebrew || (mode_homebrew && find_device != 0))
                        {
                            sprintf(filename, "/dev_usb00%c/GAMEZ", 47 + find_device);
                            if (fill_entries_from_device(filename, directories, &ndirectories, (1<<find_device) | (HOMEBREW_FLAG * (mode_homebrew != 0)), 0 | (2 * (mode_homebrew != 0)), 1) == SUCCESS)
                            {
                                sprintf(filename, "/dev_usb00%c/GAMES_DUP", 47 + find_device);
                                if (fill_entries_from_device(filename, directories, &ndirectories, (1<<find_device) | (HOMEBREW_FLAG * (mode_homebrew != 0)), 0 | (2 * (mode_homebrew != 0)), 1) == SUCCESS)
                                {
                                    sprintf(filename, "/dev_usb00%c/GAMES_BAD", 47 + find_device);
                                    fill_entries_from_device(filename, directories, &ndirectories, (1<<find_device) | (HOMEBREW_FLAG * (mode_homebrew != 0)), 0 | (2 * (mode_homebrew != 0)), 1);
                                }
                            }
                        }

                    }
                    found_game_insert = 1;
                }
                else
                {
                    delete_entries(directories, &ndirectories, (1<<find_device));
                    found_game_remove = 1;
                }

                sort_entries2(directories, &ndirectories, sort_mode);

                fdevices_old&= ~ (1<<find_device);
                fdevices_old|= fdevices & (1<<find_device);
            }
          }
        }

        // NTFS/EXTx
        if(noBDVD == MODE_DISCLESS && use_cobra)
        {
            u32 ports_plug_cnt = 0;
            signal_ntfs_mount = 0;
            for(find_device = 0; find_device < 8; find_device++)
            {
                if(automountCount[find_device] > 0) signal_ntfs_mount = 1;
                if(mountCount[find_device]) ports_plug_cnt|= 1<<find_device; else ports_plug_cnt&= ~(1<<find_device);
            }

            if(old_ntfs_ports != ports_plug_cnt || forcedevices || signal_force)
            {
                if(old_ntfs_ports == ports_plug_cnt) old_ntfs_ports = 0;
                if(delete_entries(directories, &ndirectories, NTFS_FLAG)) found_game_remove = 1;

                if( automountCount[0] == 0 && automountCount[1] == 0 && automountCount[2] == 0 && automountCount[3] == 0 &&
                    automountCount[4] == 0 && automountCount[5] == 0 && automountCount[6] == 0 && automountCount[7] == 0)
                {
                    for(find_device = 0; find_device < 8; find_device++)
                    {
                        if(mounts[find_device])
                        {
                            int k;
                            for (k = 0; k < mountCount[find_device]; k++)
                            {
                                if((mounts[find_device]+k)->name[0])
                                {
                                    if(mode_homebrew == GAMEBASE_MODE)
                                    {
                                        if(game_list_category != 2)
                                        {
                                            sprintf(filename, "/%s:/PS3ISO", (mounts[find_device]+k)->name);
                                            fill_iso_entries_from_device(filename, NTFS_FLAG, directories, &ndirectories);
                                            found_game_insert = 1;
                                        }

                                        if(mode_homebrew == 0  && game_list_category != 1)
                                        {
                                            sprintf(filename, "/%s:/PSXISO", (mounts[find_device]+k)->name);
                                            fill_iso_entries_from_device(filename, PS1_FLAG | D_FLAG_NTFS, directories, &ndirectories);

                                            sprintf(filename, "/%s:/PSXGAMES", (mounts[find_device]+k)->name);
                                            fill_psx_iso_entries_from_device(filename, PS1_FLAG | D_FLAG_NTFS, directories, &ndirectories);

                                            sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, ps2classic_path);
                                            fill_psx_iso_entries_from_device(filename, PS2_CLASSIC_FLAG | D_FLAG_NTFS, directories, &ndirectories);

                                            //RETRO
                                            char cfg_path[MAXPATHLEN];
                                            sprintf(cfg_path, "%s/USRDIR/cores", self_path);

                                            if(is_file_exist(cfg_path))
                                            {
                                                sprintf(cfg_path, "%s/USRDIR/cores/snes-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_snes_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/gba-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_gba_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/gen-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_gen_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/nes-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_nes_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/mame-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_mame_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/fba-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_fba_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/quake-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_quake_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/doom-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_doom_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/pce-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_pce_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/gbc-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_gbc_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);

                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_gb_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/atari-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_atari_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/vb-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_vb_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }

                                                sprintf(cfg_path, "%s/USRDIR/cores/nxe-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_nxe_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }


                                                sprintf(cfg_path, "%s/USRDIR/cores/wswam-retroarch.cfg", self_path);
                                                if(is_file_exist(cfg_path))
                                                {
                                                    sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, retro_wswam_path);
                                                    fill_iso_entries_from_device(filename, RETRO_FLAG | D_FLAG_NTFS, directories, &ndirectories);
                                                }
                                            }


                                            found_game_insert = 1;
                                        }
                                    }
                                    else
                                    {
                                        sprintf(filename, "/%s:/BDISO", (mounts[find_device]+k)->name);
                                        fill_iso_entries_from_device(filename, D_FLAG_HOMEB | D_FLAG_HOMEB_BD | D_FLAG_NTFS, directories, &ndirectories);

                                        sprintf(filename, "/%s:/DVDISO", (mounts[find_device]+k)->name);
                                        fill_iso_entries_from_device(filename, D_FLAG_HOMEB | D_FLAG_HOMEB_DVD | D_FLAG_NTFS, directories, &ndirectories);

                                        sprintf(filename, "/%s:/VIDEO", (mounts[find_device]+k)->name);
                                        fill_iso_entries_from_device(filename, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | D_FLAG_NTFS, directories, &ndirectories);

                                        sprintf(filename, "/%s:/MOVIES", (mounts[find_device]+k)->name);
                                        fill_iso_entries_from_device(filename, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | D_FLAG_NTFS, directories, &ndirectories);

                                        sprintf(filename, "/%s:%s", (mounts[find_device]+k)->name, video_path);
                                        fill_iso_entries_from_device(filename, D_FLAG_HOMEB | D_FLAG_HOMEB_MKV | D_FLAG_NTFS, directories, &ndirectories);

                                        found_game_insert = 1;
                                    }
                                }
                            }
                        }
                    }
                    if(found_game_insert)
                        sort_entries2(directories, &ndirectories, sort_mode);
                    old_ntfs_ports = ports_plug_cnt;
                }
            }
        }
        //-- endif (noBDVD == MODE_DISCLESS && use_cobra)


        if (found_game_insert || found_game_remove)
        {
           UpdateFavourites(directories, ndirectories);

           if(mode_favourites && !havefavourites) mode_favourites = 0;

           get_grid_dimensions();
           get_games();
           load_gamecfg(-1); // force refresh game info

           mode_favourites = mode_favourites != 0; // avoid insert favourites

           //select_option = 0;

           if(allow_restore_last_game == 1)
           {
               if(autolaunch == LAUNCHMODE_REFRESH)
               {
                    LoadLastGame();

                    get_games();
                    load_gamecfg(-1); // force refresh game info

                    autolaunch = LAUNCHMODE_CHECKED;
               }
               else
                    locate_last_game();
           }
        }

        found_game_remove = 0;
        found_game_insert = 0;

        pause_music(0);

        if(0)
        if(stops_BDVD)
        {
            stops_BDVD++;
            if(stops_BDVD >= 60 * 2)
            {
                stops_BDVD = 0;
                Eject_BDVD(NOWAIT_BDVD | STOP_BDVD);
            }
        }

        /////////////////////////////////////

        if(autolaunch == LAUNCHMODE_TOCHECK)
        {
            LoadLastGame();

            ps3pad_read();
            if((new_pad | old_pad) & (BUTTON_L1 | BUTTON_R1)) {autolaunch = currentgamedir; gui_control(); fun_exit(); exit(0);}

            autolaunch = LAUNCHMODE_CHECKED;
        }

        /////////////////////////////////////

        if(gui_mode == MODE_COVERFLOW)
            cls0();
        else
        {
            cls();
            update_twat(1);
        }

        x = (848 - 640) / 2; y = (512 - 360) / 2;
//        DrawBox(x - 16, y - 16, 65535.0f, 640.0f + 32, 360 + 32, 0x00000028);
//        DrawBox(x, y, 65535.0f, 640.0f, 360, 0x30003018);


        x = 28; y = 0;

        if((old_pad & (BUTTON_L2 | BUTTON_R2 | BUTTON_START)) == (BUTTON_L2 | BUTTON_R2 | BUTTON_START))
        {
            videoscale_x = videoscale_y = 0;
            video_adjust();
        }

        // paranoic checks
        if(select_px < 0 || select_px > (cols - 1)) select_px = 0;
        if(select_py < 0 || select_py > (rows - 1)) select_py = 0;

        if(currentdir < 0 || currentdir >= ndirectories) currentdir = 0;
        if(currentgamedir < 0 || currentgamedir >= ndirectories) currentgamedir = 0;

        // paranoic favourite check
        if(mode_favourites)
        {
            for(n = 0; n < num_box; n++)
            {
                if(favourites.list[n].index >= 0)
                {
                    if(favourites.list[n].title_id[0] == 0) exit(0);
                    if(favourites.list[n].index >= ndirectories) exit(0);
                    if(directories[favourites.list[n].index].flags == 0) exit(0);
                }
            }
            for (; n < 48; n++) favourites.list[n].index = -1;
        }

        // fake disc insertion
        if(noBDVD == MODE_DISCLESS && disc_less_on == 1 && !use_cobra && syscall_40(3, 0) == 1)
        {
            int u;

            s16 * sound = memalign(32, (cricket_raw_bin_size + 31) & ~31);

            if(sound)
            {
                memset((void *) sound, 0, (cricket_raw_bin_size + 31) & ~31);
                memcpy((void *) sound, (void *)  cricket_raw_bin + 1, cricket_raw_bin_size - 1);

                SND_SetVoice(1, VOICE_MONO_16BIT, 22050, 0, sound, cricket_raw_bin_size - 1, 0x40, 0x20, NULL);
                SND_SetVoice(1, VOICE_MONO_16BIT, 22050, 1000, sound, cricket_raw_bin_size - 1, 0x20, 0x40, NULL);
            }

            for(u = 0; u <190; u++)
            {
                cls();

                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                      Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                update_twat(1);
                DrawTextBox((848 - 300)/2,(512 - 300)/2, 0, 300, 300, ((u >= 64) ? 0xff : u<<2) | 0xffffff00);
                SetCurrentFont(FONT_TTF);
                SetFontColor(((u & 16)) ? 0x0 : 0xffffffff, 0x00000000);

                SetFontSize(32, 48);

                SetFontAutoCenter(1);
                DrawString(0, 512 - 75,  language[PLUG_STORAGE2]);
                SetFontAutoCenter(0);
                tiny3d_Flip();
                ps3pad_read();
            }

            if(sound) free(sound);

            if(!use_cobra)
            {
                u32 eid = (u32) syscall_40(4, 0);

                if(eid != SUCCESS)
                {
                    eid -= 0x100;
                    manager_cfg.event_flag = eid;
                    SaveManagerCfg();
                }
            }

            cls();
            update_twat(1);
            disc_less_on = 2;

            if(new_pad & BUTTON_CROSS) new_pad ^= BUTTON_CROSS;
            if(new_pad & BUTTON_CIRCLE) new_pad ^= BUTTON_CIRCLE;
        }

        switch(menu_screen)
        {
            case 0:
                if(gui_mode == MODE_COVERFLOW) {if(gui_control() == SUCCESS) draw_coverflow(x, y);}
                else                           {if(gui_control() == SUCCESS) draw_grid(x, y);}
                break;
            case 1:
                draw_options(x, y, currentgamedir);
                break;
            case 2:
                draw_configs(x, y, currentgamedir);
                break;
            case 3:
                draw_gbloptions(x, y);
                break;
            case 4:
                draw_toolsoptions(x, y);
                break;
            case 5:
                draw_cachesel(x, y);
                break;
            case 128:
                draw_iso_options(x, y, currentgamedir);
                break;
            case 222:
                draw_console_id_tools(x, y);
                break;
            case 444:
                draw_psx_options(x, y, currentgamedir);
                break;
            case 445:
                draw_psx_options2(x, y, currentgamedir);
                break;
            case 777:
                draw_device_mkiso(x, y, currentgamedir);
                break;
            case 778:
                draw_device_xtiso(x, y, currentgamedir);
                break;
            case 779:
                draw_device_cpyiso(x, y, currentgamedir);
                break;
            default:
                menu_screen = 0;
                break;
        }

        auto_ftp(); // auto enable the ftp

    }

    return SUCCESS;
}

void load_background_picture()
{
    if(bk_picture)
    {
        if(bk_picture == 1 || bk_picture == 2)
        {
          int retry = 0, bg;
          srand(time(0)); // randomize seed
          while(retry < 30)
          {
            bg = rand() % 15;
            // load background picture
            sprintf(temp_buffer, "%sPICT%i.JPG", backgrounds_path, bg);
            if(LoadTextureJPG(temp_buffer, BACKGROUND_PICT) != SUCCESS) Png_offset[BACKGROUND_PICT] = 0; else break;
            retry++;
            set_usleep_sm_main(100000);
          }
        }
        else if(bk_picture > 2)
        {
          // load background picture
          sprintf(temp_buffer, "%sPICT%i.JPG", backgrounds_path, bk_picture - 3);
          if(LoadTextureJPG(temp_buffer, BACKGROUND_PICT) != SUCCESS) {Png_offset[BACKGROUND_PICT] = 0; bk_picture = 0;}
        }
    }
    else
        Png_offset[BACKGROUND_PICT] = 0;

}

// draw_cachesel
struct {
    u64 size;
    char title[64];
    char title_id[64];
} cache_list[64];

int ncache_list = 0;

void LoadCacheDatas()
{
    DIR  *dir, *dir2;

    sprintf(temp_buffer, "%s/cache", self_path);
    dir = opendir (temp_buffer);
    if(!dir) return;

    sysFsGetFreeSize("/dev_hdd0/", &blockSize, &freeSize);
    freeSpace[0] = ( ((u64)blockSize * freeSize));
    freeSpace[0] = freeSpace[0] / GIGABYTES;

    ncache_list = 0;

    while(1)
    {
        struct dirent *entry = readdir (dir);

        if(!entry) break;
        if(entry->d_name[0] == '.' && (entry->d_name[1] == 0 || entry->d_name[1] == '.')) continue;

        if(!(entry->d_type & DT_DIR)) continue;

        strncpy(cache_list[ncache_list].title_id, entry->d_name, 64);
        cache_list[ncache_list].title_id[63] = 0;

        cache_list[ncache_list].size = 0ULL;

        sprintf(temp_buffer + 1024, "%s/cache/%s/name_entry", self_path, entry->d_name);
        int size;
        char *name = LoadFile(temp_buffer + 1024, &size);

        memset(cache_list[ncache_list].title, 0, 64);
        if(name)
        {
            memcpy(cache_list[ncache_list].title, name, (size < 64) ? size : 63);
            free(name);
        }

        sprintf(temp_buffer + 1024, "%s/cache/%s", self_path, entry->d_name);
        dir2 = opendir (temp_buffer + 1024);
        if(dir2)
        {
            while(1)
            {
                struct dirent *entry2 = readdir (dir2);
                struct stat s;

                if(!entry2) break;
                if(entry2->d_name[0] == '.' && (entry2->d_name[1] == 0 || entry2->d_name[1] == '.')) continue;

                if((entry2->d_type & DT_DIR)) continue;

                sprintf(temp_buffer + 2048, "%s/cache/%s/%s", self_path, entry->d_name, entry2->d_name);
                if(stat(temp_buffer + 2048, &s) == 0)
                    cache_list[ncache_list].size += s.st_size;
            }
        }

        ncache_list++; if(ncache_list >= 64) break;
    }
}

inline int get_currentdir(int i)
{
    if(mode_favourites != 0)
        return favourites.list[i].index;

    return (currentdir + i);
}

inline int get_int_currentdir(int i)
{
    if(mode_favourites != 0)
        return favourites.list[i].index;

    return (int_currentdir + i);
}

void load_gamecfg(int current_dir)
{
    static int last_selected = -1;

    char path_file[0x420];

    if(current_dir < 0) //check reset info
    {
        bshowpath = 0;
        last_selected = current_dir;
        memset(&game_cfg, 0, sizeof(game_cfg));
        return;
    }

    if(last_selected == current_dir)
        return;

    last_selected = current_dir; //prevents load again
    bshowpath = 0;

    sprintf(path_file, "%s/config/%s.cfg", self_path, directories[current_dir].title_id);
    memset(&game_cfg, 0, sizeof(game_cfg));

    int file_size;
    char *file = LoadFile(path_file, &file_size);
    if(file)
    {
        if(file_size > sizeof(game_cfg)) file_size = sizeof(game_cfg);
        memcpy(&game_cfg, file, file_size);
        free(file);
    }
}

int check_disc(void)
{
    int get_user = 1;

    while(get_user == 1)
    {
        DIR  *dir;
        dir = opendir ("/dev_bdvd");

        if(dir)
        {
            closedir (dir);
            return 1;
        }
        else
            get_user = DrawDialogYesNo(language[DRAWSCREEN_REQBR]);
    }

    return -1;
}

void mount_custom(char *path)
{
    int size, n;
    char * mem;
    sprintf(temp_buffer, "%s/ps2disc.txt", path);

    mem = LoadFile(temp_buffer, &size);

    if(mem)
    {
        n = 0; while(mem[n] >= 32) n++;

        mem[n] = 0;
        add_sys8_path_table("/dev_ps2disc", mem);
        free(mem);
    }

}

static u8 BdId[0x10];

void set_BdId(int index)
{
    struct stat s;
    int readed;
    int ret = 0;
    void *mem;

    if(use_cobra || !lv2peek(0x80000000000004E8ULL)) return;

    sprintf(temp_buffer, "%s/config/%s.did", self_path, directories[index].title_id);
    sprintf(temp_buffer + 1024, "%s/BDMEDIA_ID", directories[index].path_name);

    memset(BdId, 0, 0x10);

    ret = FAILED;

    if(stat(temp_buffer + 1024, &s) == 0 && s.st_size == 0x10)
    {
        mem = LoadFile((void *) temp_buffer + 1024, &readed);
        if(!mem || readed != 0x10) {if(mem) free(mem); ret = FAILED;}
        else {ret = SUCCESS; memcpy(BdId, mem, 0x10); free(mem);}
    }

    if(ret != SUCCESS && stat(temp_buffer, &s) == 0 && s.st_size == 0x10)
    {
        mem = LoadFile((void *) temp_buffer, &readed);
        if(!mem || readed != 0x10) {if(mem) free(mem); ret = -1;}
        else
        {
            ret = 0; memcpy(BdId, mem, 0x10);
            SaveFile(temp_buffer + 1024, (char *) BdId, 0x10);
            free(mem);
        }
    }

    if(!use_cobra) syscall_40(7, (u64) (BdId)); // set BD Media Id
}

void get_pict(int *index)
{
    char dir[0x420];
    char dir2[0x420];
    char temp_buffer[0x420];

    sysFSStat dstat;

    int fd;
    int i = *index;

    bSkipPIC1 = 0;
    Png_offset[BIG_PICT] = 0;

    if(!mode_favourites || (mode_favourites != 0 && favourites.list[i].index >= 0))
    {
        sprintf(dir, "%s%s", directories[get_int_currentdir(i)].path_name,
            &folder_mode[!((directories[get_int_currentdir(i)].flags>>D_FLAG_HOMEB_DPL) & 1)][0]);

        sprintf(dir2, "%s%s", directories[(mode_favourites != 0)
                ? favourites.list[i].index : (int_currentdir + i)].path_name,
                &folder_mode[!((directories[(mode_favourites != 0)
                    ? favourites.list[i].index : (int_currentdir + i)].flags>>D_FLAG_HOMEB_DPL) & 1)][0]);

        int ind = (mode_favourites != 0) ? favourites.list[i].index : (int_currentdir + i);


        bool is_psp = (directories[ind].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG);

        if(is_psp)
        {
            bool is_retro = (strstr(directories[ind].path_name, retro_root_path) != NULL);
            bool is_ps2_classic = !is_retro &&
                                  (strstr(directories[ind].path_name, ps2classic_path) != NULL);

            if(is_retro || is_ps2_classic) goto default_pict;
            goto get_pic_from_iso;
        }

        // ISOS
        if((directories[ind].flags & ((PS3_FLAG) | (HOMEBREW_FLAG) | (BDVD_FLAG))) == (PS3_FLAG))
        {
            if((directories[ind].flags & (PS2_FLAG)) == (PS2_FLAG))
            { // PS2
            }
            else
            { // PS3 / PSP
get_pic_from_iso:
                fd = ps3ntfs_open(directories[ind].path_name, O_RDONLY, 0);
                if(fd > 0)
                {
                    u32 flba;
                    u64 size;
                    char *mem = NULL;
                    int re;

                    if(is_psp)
                    {
                               re = get_iso_file_pos(fd, "/PSP_GAME/PIC1.PNG", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PSP_GAME/PIC0.PNG", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PSP_GAME/ICON0.PNG", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PSP_GAME/PIC2.PNG", &flba, &size);
                    }
                    else
                    {
                               re = get_iso_file_pos(fd, "/PS3_GAME/PIC1.PNG;1", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PS3_GAME/PIC0.PNG;1", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PS3_GAME/ICON0.PNG;1", &flba, &size);
                        if(re) re = get_iso_file_pos(fd, "/PS3_GAME/PIC2.PNG;1", &flba, &size);
                    }

                    Png_offset[BIG_PICT] = 0;

                    if(!re && (mem = malloc(size)) != NULL)
                    {
                        re = ps3ntfs_read(fd, (void *) mem, size);
                        ps3ntfs_close(fd);
                        if(re == size)
                        {
                            memset(&my_png_datas, 0, sizeof(PngDatas));
                            my_png_datas.png_in = mem;
                            my_png_datas.png_size = size;
                            if(LoadTexturePNG(NULL, BIG_PICT) == SUCCESS) ;
                            else
                                Png_offset[BIG_PICT] = 0;
                        }
                        free(mem);
                    }
                    else
                        ps3ntfs_close(fd);
                }
            }       // PS3

            if(bk_picture == 2)
            {
                if(Png_offset[BIG_PICT] == 0)
                    load_background_picture();
                else
                {
                    Png_offset[BACKGROUND_PICT] = Png_offset[BIG_PICT];
                    Png_datas[BACKGROUND_PICT] = Png_datas[BIG_PICT];
                }
            }

            return;
        }

        // GAMES / GAMEZ
        sprintf(temp_buffer, "%sPIC1.PNG", dir);
        if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS || LoadTexturePNG(temp_buffer, BIG_PICT) == FAILED)
        {
            sprintf(temp_buffer, "%sPIC0.PNG", dir2);
            if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS || LoadTexturePNG(temp_buffer, BIG_PICT) == FAILED)
            {
                sprintf(temp_buffer, "%sICON0.PNG", dir2);
                if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS || LoadTexturePNG(temp_buffer, BIG_PICT) == FAILED)
                {
                    sprintf(temp_buffer, "%sPIC2.PNG", dir2);
                    if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS || LoadTexturePNG(temp_buffer, BIG_PICT) == FAILED) Png_offset[BIG_PICT] = 0;
                }
            }
        }

default_pict:
        if(bk_picture == 2)
        {
            bSkipPIC1 = 1;
            if(Png_offset[BIG_PICT] == 0)
                load_background_picture();
            else
            {
                Png_offset[BACKGROUND_PICT] = Png_offset[BIG_PICT];
                Png_datas[BACKGROUND_PICT] = Png_datas[BIG_PICT];
            }
        }
    }
}

void draw_grid(float x, float y)
{
    int i, n, m;

    float x2;

    static char str_home[5][16] = {" Homebrew", "", " PS3", " Retro", " Films"};

    int str_type = (mode_homebrew != GAMEBASE_MODE) ? ((mode_homebrew == HOMEBREW_MODE) ? 0 : 4) : 1 + game_list_category;

    int selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 18, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    if(mode_favourites >= 0x20000)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVSWAP], &str_home[str_type][0]);
    else if(mode_favourites >= 0x10000)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVINSERT], &str_home[str_type][0]);
    else if(mode_favourites)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVORITES], &str_home[str_type][0]);
    else if(str_type == 4)
        DrawFormatString(x, y, " %s %i/%i (%i %s)", language[DRAWSCREEN_PAGE], currentdir / num_box + 1, ROUND_UPX(ndirectories) / num_box,
                                                    ndirectories, &str_home[str_type][0]);
    else
        DrawFormatString(x, y, " %s %i/%i (%i %s)%s", language[DRAWSCREEN_PAGE], currentdir / num_box + 1, ROUND_UPX(ndirectories) / num_box,
                                                      ndirectories, language[DRAWSCREEN_GAMES], &str_home[str_type][0]);

    // list device space

    m = selected;

    if(Png_offset[m])
    {

        i = -1;

        if(!mode_favourites || ((mode_favourites != 0) && favourites.list[m].index >= 0))
        {
            for(i = 0; i < 11; i++)
                if((directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].flags & GAMELIST_FILTER) == (1<<i)) break;
            if(i == 11)
                if((directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].flags & GAMELIST_FILTER) == NTFS_FLAG)
                {
                     i = 99;
                     strcpy(temp_buffer, directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].path_name);
                }
        }
        m = i;
    } else m = -1;

    x2 = 1200;
    for(n = 0; n < 2; n++)
    {
        if(m == 99)
        {
            char *p = (char *) temp_buffer + 1; while(*p && *p != ':') p++; if(*p == ':') p[1] = 0;
            SetFontColor(0xafd836ff, 0x00000000);
            x2 = DrawFormatString(x2, 0, "%s .ISO", (void *) (temp_buffer + 1));
        }
        else
            for(i = 0; i < 11; i++)
            {
                if(((fdevices>>i) & 1))
                {
                    if(freeSpace[i] <= 0)
                    {
                        sprintf(temp_buffer, "/dev_usb00%c/", (47 + i));
                        sysFsGetFreeSize(temp_buffer, &blockSize, &freeSize);
                        double space = ( ((double)blockSize) * ((double) freeSize) ) /  GIGABYTES;
                        freeSpace[i] = (float) space;
                    }

                    if(m == i) SetFontColor(0xafd836ff, 0x00000000); else SetFontColor(0xffffff44, 0x00000000);

                    if(i == HDD0_DEVICE)
                        x2 = DrawFormatString(x2, 0, "hdd0: %.2fGB ", freeSpace[i]);
                    else
                        x2 = DrawFormatString(x2, 0, "usb00%c: %.2fGB ", 47 + i, freeSpace[i]);
                }
            }

        x2 = 848 -(x2 - 1200) - x;
    }

    SetFontAutoCenter(0);
    SetFontSize(18, 20);

    SetFontColor(0xffffffff, 0x00000000);

    y += 24;

    int ww = 800 / cols;
    int hh = ww * 150 / 200;
    int xx = x, yy = y;

    if(hh * rows > 450)
    {
        hh = 450 / rows;

        ww = (hh * 200 / 150);

        if(ww * cols > 800) ww = 800 / cols;
        else
            xx += (800 - ww * cols) / 2;

    }
    else
    {
        yy += (450 - hh * rows) / 2;
    }

    #define FIX_X(a) ((a) * ww / 200)

    i = 0;
    int flash2 = 0;
    #define MAX_FLASH 32

    if(bIconPulse)
    {
        if (frame_count & MAX_FLASH)
            flash2 = (MAX_FLASH - 1) - (frame_count & (MAX_FLASH - 1));
        else
            flash2 = (frame_count & (MAX_FLASH - 1));
    }
    else
        flash2 = (MAX_FLASH - 1);

    ///////////////////////

    for(n = 0; n < rows; n++)
        for(m = 0; m < cols; m++)
        {
            int f = (select_px == m && select_py == n);
            float f2 = (int) f;

            if(cover_mode) f2 = 2.1f * ((float) (flash2 *(select_px == m && select_py == n))) / ((float) MAX_FLASH);

            DrawBox(xx + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, (ww-8) + 8 * f2, (hh-8) + 8 * f2, 0x00000028 + (flash2 * f) );

            //draw Splited box
            //if(directories[currentgamedir].splitted)
            //    DrawBox(x + 198 * m, (y - 2) + n * 150, 0, 194, 144, 0x55ff3328 );

            int set_ps3_cover = 0;

            if(Png_offset[i])
            {
                if(Png_iscover[i] == 1) set_ps3_cover = 1; // PS3 cover

                if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
                {
                     if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD)
                     {
                         if((directories[get_currentdir(i)].flags & D_FLAG_HOMEB_MKV) == D_FLAG_HOMEB_MKV)
                         tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MOVIE_ICON], Png_res[IMG_MOVIE_ICON].width,
                                               Png_res[IMG_MOVIE_ICON].height, Png_res[IMG_MOVIE_ICON].wpitch,
                                               TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                         else
                         tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DVD_DISC], Png_res[IMG_DVD_DISC].width,
                                               Png_res[IMG_DVD_DISC].height, Png_res[IMG_DVD_DISC].wpitch,
                                               TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                    else tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                               Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                               TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                }
                else if((directories[get_currentdir(i)].flags  & ((BDVD_FLAG) | (PS1_FLAG))) == ((BDVD_FLAG) | (PS1_FLAG)))
                {
                    Png_iscover[i] = -1;
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS1_DISC], Png_res[IMG_PS1_DISC].width,
                                          Png_res[IMG_PS1_DISC].height, Png_res[IMG_PS1_DISC].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                }
                else if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & (PS1_FLAG)))
                {
                    // add PSP / PS2 / PSX ISO icon
                    if((directories[get_currentdir(i)].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
                    {
                        bool is_retro = (strstr(directories[get_currentdir(i)].path_name, retro_root_path) != NULL);
                        bool is_ps2_classic = !is_retro &&
                                              (strstr(directories[get_currentdir(i)].path_name, ps2classic_path) != NULL);

                        if(is_retro)
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_RETRO_ICON], Png_res[IMG_RETRO_ICON].width,
                                                  Png_res[IMG_RETRO_ICON].height, Png_res[IMG_RETRO_ICON].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        else if(is_ps2_classic)
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                                  Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        else
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PSP_ISO], Png_res[IMG_PSP_ISO].width,
                                                  Png_res[IMG_PSP_ISO].height, Png_res[IMG_PSP_ISO].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                    else if((directories[get_currentdir(i)].flags & (PS2_FLAG)) == (PS2_FLAG))
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                              Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    else
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS1_ISO], Png_res[IMG_PS1_ISO].width,
                                              Png_res[IMG_PS1_ISO].height, Png_res[IMG_PS1_ISO].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                }
                else if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & ((PS3_FLAG) | (BDVD_FLAG))) == (PS3_FLAG))
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                              Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                else
                        tiny3d_SetTextureWrap(0, Png_offset[i], Png_datas[i].width,
                                              Png_datas[i].height, Png_datas[i].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                //if((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (PS1_FLAG)) {
                //    set_ps3_cover = 2; // PSX cover
                //    if((directories[get_currentdir(i)].flags & (PS2_FLAG)) == (PS2_FLAG)) set_ps3_cover = 3 + 1 * (Png_iscover[i] < 0); // PS2 cover
                //}

                if((directories[get_currentdir(i)].flags  & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
                {
                    set_ps3_cover = 3;
                    if(Png_iscover[i] < 0) set_ps3_cover = 1;
                    else if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD) set_ps3_cover = 1;
                    if(set_ps3_cover == 2 || set_ps3_cover == 4)
                        DrawTextBoxCover(xx + FIX_X(16) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(160) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f), set_ps3_cover - 1);
                    else
                        DrawTextBoxCover(xx + FIX_X(36) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(124) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f), set_ps3_cover - 1);
                }
                else if(set_ps3_cover && gui_mode != 0)
                {
                    if(set_ps3_cover == 2 || set_ps3_cover == 4)
                        DrawTextBoxCover(xx + FIX_X(16) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(160) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f), set_ps3_cover - 1);
                    else
                        DrawTextBoxCover(xx + FIX_X(36) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(124) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f), set_ps3_cover - 1);
                }
                else if(Png_iscover[i] == -1)
                    DrawTextBox(xx + FIX_X(25) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(142) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f));
                else if(Png_iscover[i] == 1)
                    DrawTextBox(xx + FIX_X(36) + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, FIX_X(124) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f));
                else
                    DrawTextBox(xx + ww * m - 4 * f2, yy + n * hh - 4 * f2, 0, (ww-8) + 8 * f2, (hh-8) + 8 * f2, (0xffffffff- (MAX_FLASH * 2 * f)) + (flash2 * 2 * f));

                //if((mode_favourites != 0) && favourites.list[i].index < 0) exit(0);

                if(!mode_favourites || ((mode_favourites != 0) && favourites.list[i].index >= 0))
                {
                    // ignore Bluray icon
                    if((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (BDVD_FLAG | PS1_FLAG)) ;
                    // draw Bluray icon
                    else if((directories[get_currentdir(i)].flags  & GAMELIST_FILTER) == BDVD_FLAG)
                    {
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                              Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        DrawTextBox(xx + ww * m + 4 - 4 * f2, yy + n * hh + 4 - 4 * f2, 0, FIX_X(32), FIX_X(32), 0xffffffcf);
                    }
                    else
                    // draw USB icon
                    if(bShowUSBIcon && (directories[get_currentdir(i)].flags  & GAMELIST_FILTER) > 1)
                    {
                        int ii = 1 + 13 * ((directories[get_currentdir(i)].flags & NTFS_FLAG) != 0);
                        tiny3d_SetTextureWrap(0, Png_res_offset[ii], Png_res[ii].width,
                        Png_res[ii].height, Png_res[ii].wpitch,
                            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                        if(directories[get_currentdir(i)].splitted)
                            DrawTextBox(xx + ww * m + 4 - 4 * f2, yy + n * hh + 4 - 4 * f2, 0, FIX_X(32), FIX_X(24), 0xff9999aa);
                        else
                            DrawTextBox(xx + ww * m + 4 - 4 * f2, yy + n * hh + 4 - 4 * f2, 0, FIX_X(32), FIX_X(24), 0xffffffcf);

                    }
                }

            }
            else if(mode_favourites && favourites.list[i].title_id[0] != 0)
            {
                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MISSING_ICON], Png_res[IMG_MISSING_ICON].width,
                                      Png_res[IMG_MISSING_ICON].height, Png_res[IMG_MISSING_ICON].wpitch,
                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                DrawTextBox(xx + ww * m + FIX_X(32) - 4 * f2, yy + n * hh + FIX_X(7) - 4 * f2, 0, FIX_X(128) + 8 * f2, FIX_X(128) + 8 * f2, 0xffffff3f);
            }

            i++;
        }

        #undef FIX_X

    i = selected;

    if(flash)
    {
        int png_on = 0;

        //DrawBox(x + 200 * select_px - 4, y + select_py * 150 - 4 , 0, 200, 150, 0xa0a06080);


        if(mode_favourites >= 0x10000)
        {
            if(mode_favourites < 0x20000)
            {
                if(Png_offset[BIG_PICT])
                {
                    tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                        Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    png_on = 1;
                }
            }
            else
            {
                i = mode_favourites - 0x20000;

                if(i>= 0 && i < num_box)
                {
                    if(!Png_offset[i] && favourites.list[i].title_id[0] != 0)
                    {
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MISSING_ICON], Png_res[IMG_MISSING_ICON].width,
                                              Png_res[IMG_MISSING_ICON].height, Png_res[IMG_MISSING_ICON].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        png_on = 1;

                    }
                    else if(Png_offset[i])
                    {
                        png_on = 1;
                        tiny3d_SetTextureWrap(0, Png_offset[i], Png_datas[i].width,
                        Png_datas[i].height, Png_datas[i].wpitch,
                            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                }
            }

            if(png_on)
                DrawTextBox(xx + ww * select_px - 4, yy + select_py * hh - 4 , 0, ww, hh, 0x8fff8fcf);
        }

    }

    SetFontColor(0xffffffff, 0x00000000);

    // display temperature
    if((frame_count & 0x100))
    {
        static u32 temp = 0;
        static u32 temp2 = 0;
        int y2;

        if(temp == 0 || (frame_count & 0x1f) == 0x0 )
        {
            sys_game_get_temperature(0, &temp);
            sys_game_get_temperature(1, &temp2);
        }

        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);

        x2 = DrawFormatString(1024, 0, " Temp CPU: 99ºC RSX: 99ºC ");

        y2= y + 3 * 150 - 4 + 12;
        SetFontColor(0xffffffff, 0x00000000);
        x2 = DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y2, " Temp CPU: ");
        if(temp < 80) SetFontColor(0xfff000ff, 0x00000000); else SetFontColor(0xff0000ff, 0x00000000);
        x2 = DrawFormatString(x2, y2, "%uºC",  temp);
        SetFontColor(0xffffffff, 0x00000000);
        x2 = DrawFormatString(x2, y2, " RSX: ");
        if(temp2 < 75) SetFontColor(0xfff000ff, 0x00000000); else SetFontColor(0xff0000ff, 0x00000000);
        x2 = DrawFormatString(x2, y2, "%uºC ", temp2);

        SetFontColor(0xffffffff, 0x00000000);
    }
    else if(Png_offset[i])
    {
        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_SOPTIONS]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 - 2, " %s ", language[DRAWSCREEN_SOPTIONS]);

    }
    else if(mode_favourites && mode_favourites < 0x10000 && favourites.list[i].title_id[0] != 0)
    {
        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_SDELETE]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 - 2, " %s ", language[DRAWSCREEN_SDELETE]);
    }


    if(!(frame_count & 0x100))
    {
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_STGLOPT]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 + 18, " %s ", language[DRAWSCREEN_STGLOPT]);
    }

    if(bShowPlayOverlay != 0)
    {
        if(!(mode_favourites && mode_favourites < 0x10000 && favourites.list[i].title_id[0] != 0))
        {
            //DrawBox(xx + ww * select_px , yy + select_py * hh , 0, ww-8, hh-8, 0x404040a0);
            DrawBox(xx + ww * select_px - 4, yy + select_py * hh - 4 + hh - 40, 0, ww, 40, 0x40404080);
            SetCurrentFont(FONT_TTF); // get default
            SetFontSize(20, 20);
        }

        if(((Png_offset[i]) || (mode_favourites && mode_favourites < 0x10000 && favourites.list[i].title_id[0] != 0)))
        {
            DrawBox(xx + ww * select_px - 4, yy + select_py * hh - 4 + hh - 40, 0, ww, 40, 0x40404080);
            SetCurrentFont(FONT_TTF);
            SetFontSize(24 * 4 / cols, 24);

            x2 = DrawFormatString(xx + ww * select_px - 4, yy + select_py * hh - 4 + hh - 40, "  %s", language[DRAWSCREEN_PLAY]);
        }
    }

    // draw game name
    i = selected;

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffee, 0x00000000);

    if((Png_offset[i] && !mode_favourites) || (mode_favourites && favourites.list[i].title_id[0] != 0))
    {
        u32 str_color = 0xffffffee;

        if(mode_favourites)
        {
            if(strncmp((char *) string_title_utf8, favourites.list[i].title, 64))
            {
                strncpy((char *) string_title_utf8, favourites.list[i].title, 128);
                update_title_utf8 = 1;
            }

        }
        else if((directories[(currentdir + i)].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            str_color = 0xafd836ee;
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }

        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[(currentdir + i)].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[(currentdir + i)].title, 128);
                update_title_utf8 = 1;
            }
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
        DrawTextBox(x + 3, y + 3 * 150 , 0, 768, 32, str_color);


        SetFontAutoCenter(0);

        load_gamecfg (get_currentdir(i)); // refresh game info

        // BD, DVD, MKV

        if((directories[get_currentdir(i)].flags  & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
        {
             if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD)
             {
                if((directories[get_currentdir(i)].flags & D_FLAG_HOMEB_MKV) == D_FLAG_HOMEB_MKV)
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MOVIE_ICON], Png_res[IMG_MOVIE_ICON].width,
                                          Png_res[IMG_MOVIE_ICON].height, Png_res[IMG_MOVIE_ICON].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                else
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DVD_DISC], Png_res[IMG_DVD_DISC].width,
                                          Png_res[IMG_DVD_DISC].height, Png_res[IMG_DVD_DISC].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
             }
             else tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                        Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                        TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

             DrawTextBox(xx + ww * select_px + ww - 52 + 8, yy + select_py * hh - 4 + hh - 36, 0, 32, 32, 0xffffff99);
        }
        else
        {
            if((game_cfg.useBDVD) || (game_cfg.direct_boot == 2) || (directories[get_currentdir(i)].flags  & GAMELIST_FILTER) == NTFS_FLAG)
            {
                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                      Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                DrawTextBox(xx + ww * select_px + ww - 52 + 8, yy + select_py * hh - 4 + hh - 36, 0, 32, 32, 0xffffff99);
            }

            if(game_cfg.direct_boot  && (directories[get_currentdir(i)].flags  & GAMELIST_FILTER) != NTFS_FLAG)
            {
                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DIRECT_ICON], Png_res[IMG_DIRECT_ICON].width,
                                      Png_res[IMG_DIRECT_ICON].height, Png_res[IMG_DIRECT_ICON].wpitch,
                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                DrawTextBox(xx + ww * select_px + ww - 52  + 8, yy + select_py * hh - 4 + hh - 36, 0, 32, 32, 0xffffffff);
            }
        }

    }

    if(flash && ftp_inited)
    {
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_FTP_ICON], Png_res[IMG_FTP_ICON].width,
                              Png_res[IMG_FTP_ICON].height, Png_res[IMG_FTP_ICON].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBox(230 /*200 * 4 -32*/, y - 32, 0, 64, 32, 0xffffffff);
    }

    //SetCurrentFont(FONT_DEFAULT);
    if(signal_ntfs_mount && (frame_count & MAX_FLASH))
    {
        int ii = 1 + 13;
        tiny3d_SetTextureWrap(0, Png_res_offset[ii], Png_res[ii].width,
        Png_res[ii].height, Png_res[ii].wpitch,
            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBoxLine(848 - 64 - 16, 24, 0, 64, 48, 0xffffffcf, 0x404040ff);
    }

    tiny3d_Flip();
}

static int anim_mode = 0, anim_step = 0;
static int g_saved = 0;
static int g_rel_posx[2];

void draw_coverflow(float x, float y)
{
    int i, n, m;

    float x2;

    static char str_home[5][16] = {" Homebrew", "", " PS3", " Retro", " Films"};

    int str_type = (mode_homebrew != GAMEBASE_MODE) ? ((mode_homebrew == HOMEBREW_MODE) ? 0 : 4) : 1 + game_list_category;

    int selected = select_px + select_py * 4;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 18, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    if(mode_favourites >= 0x20000)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVSWAP], &str_home[str_type][0]);
    else if(mode_favourites >= 0x10000)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVINSERT], &str_home[str_type][0]);
    else if(mode_favourites)
        DrawFormatString(x, y, " %s%s", language[DRAWSCREEN_FAVORITES], &str_home[str_type][0]);
    else if(str_type == 4)
        DrawFormatString(x, y, " %i/%i %s", ndirectories ? select_px + currentdir + 1 : 0, ndirectories, &str_home[str_type][0]);
    else
        DrawFormatString(x, y, " %i/%i %s%s", ndirectories ? select_px + currentdir + 1 : 0, ndirectories, language[DRAWSCREEN_GAMES], &str_home[str_type][0]);

    // Show Date/Time
    if (iTimeFormat)
    {
        u32 hh = 0, mm = 0, ss = 0, day = 0, month = 0, year = 0;

        PS3GetDateTime(&hh, &mm, &ss, &day, &month, &year);

        SetFontColor(0xffffffff, 0x00000000);
        SetCurrentFont(FONT_BUTTON);

        SetFontSize(8, 20);

        SetFontAutoCenter(0);

        float y2 = y + 80;

        y2 = y + 3 * 150 - 4 + 12 + 24;
        x2 = x;

        if(iTimeFormat == 2)
        {
            if(hh > 12)
            {
                hh = hh - 12;
                DrawFormatString(x2, y2, " %02u:%02u pm ", hh, mm);
            }
            else
            {
                if(hh == 0) hh = 12;
                DrawFormatString(x2, y2, " %02u:%02u am ", hh, mm);
            }
        }
        else if(iTimeFormat == 3)
        {
            if(sys_dateformat == 0)
                DrawFormatString(x2, y2, " %s, %04u/%02u/%02u %02u:%02u:%02u ", PS3TimeZone[sys_timezone % 110].name, year, month, day, hh, mm, ss);
            else if(sys_dateformat == 2)
                DrawFormatString(x2, y2, " %s, %02u/%02u/%04u %02u:%02u:%02u ", PS3TimeZone[sys_timezone % 110].name, month, day, year, hh, mm, ss);
            else
                DrawFormatString(x2, y2, " %s, %02u/%02u/%04u %02u:%02u:%02u ", PS3TimeZone[sys_timezone % 110].name, day, month, year, hh, mm, ss);
        }
        else if(iTimeFormat == 4)
        {
            if(hh > 12)
            {
                hh = hh - 12;
                if(sys_dateformat == 0 || sys_dateformat == 2)
                    DrawFormatString(x2, y2, " %02u/%02u %02u:%02u pm ", month, day, hh, mm);
                else
                    DrawFormatString(x2, y2, " %02u/%02u %02u:%02u pm ", day, month, hh, mm);
            }
            else
            {
                if(hh == 0) hh = 12;
                if(sys_dateformat == 0 || sys_dateformat == 2)
                    DrawFormatString(x2, y2, " %02u/%02u %02u:%02u am ", month, day, hh, mm);
                else
                    DrawFormatString(x2, y2, " %02u/%02u %02u:%02u am ", day, month, hh, mm);
            }
        }
        else
        {
            DrawFormatString(x2, y2, " %02u:%02u:%02u ", hh, mm, ss);
        }
    }

    ////////

    SetFontAutoCenter(0);

    SetFontColor(0xffffffff, 0x00000000);

    SetCurrentFont(FONT_TTF);
    SetFontSize(18, 20);

    // list device space

    m = selected;

    if(Png_offset[m])
    {
        i = -1;

        if(!mode_favourites || ((mode_favourites != 0) && favourites.list[m].index >= 0))
        {
            for(i = 0; i < 11; i++)
                if((directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].flags & GAMELIST_FILTER) == (1<<i)) break;
            if(i == 11)
                if((directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].flags & GAMELIST_FILTER) == NTFS_FLAG)
                {
                    i = 99;
                    strcpy(temp_buffer, directories[(mode_favourites != 0) ? favourites.list[m].index : (currentdir + m)].path_name);
                }
        }
        m = i;
    }
    else
        m = -1;

    x2 = 1200;
    for(n = 0; n < 2; n++)
    {
        if(m == 99)
        {
            char *p = (char *) temp_buffer + 1; while(*p && *p != ':') p++; if(*p == ':') p[1] = 0;
            SetFontColor(0xafd836ff, 0x00000000);
            x2 = DrawFormatString(x2, 0, "%s .ISO", (void *) (temp_buffer + 1));
        }
        else
            for(i = 0; i < 11; i++)
            {
                if(((fdevices>>i) & 1))
                {
                    if(freeSpace[i] <= 0)
                    {
                        sprintf(temp_buffer, "/dev_usb00%c/", (47 + i));
                        sysFsGetFreeSize(temp_buffer, &blockSize, &freeSize);
                        double space = ( ((double)blockSize) * ((double) freeSize) ) /  GIGABYTES;
                        freeSpace[i] = (float) space;
                    }

                    if(m == i) SetFontColor(0xafd836ff, 0x00000000); else SetFontColor(0xffffff44, 0x00000000);

                    if(i == HDD0_DEVICE)
                        x2 = DrawFormatString(x2, 0, "hdd0: %.2fGB ", freeSpace[i]);
                    else
                        x2 = DrawFormatString(x2, 0, "usb00%c: %.2fGB ", 47 + i, freeSpace[i]);
                }
            }

        x2 = 848 -(x2 - 1200) - x;
    }

    SetFontAutoCenter(0);
    SetFontSize(18, 20);

    SetFontColor(0xffffffff, 0x00000000);

    y += 24;

    i = 0;
    int flash2 = 0;
    #define MAX_FLASH 32

    if(bIconPulse)
    {
        if (frame_count & MAX_FLASH)
            flash2 = (MAX_FLASH - 1) - (frame_count & (MAX_FLASH - 1));
        else
            flash2 = (frame_count & (MAX_FLASH - 1));
    }
    else
        flash2 = (MAX_FLASH - 1);

    ///////////////////////

    int rel_posx[12];
    int rel_posy[12];
    int rel_widthx[12];
    int rel_widthy[12];

    int relx = 0;
    int centerx = 0;

    int icony = 128;

    int l;


    for(n = 0; n < 3; n++)
    {
        for(m = 0; m < 4; m++)
        {
            rel_widthy[i] = 142;
            rel_posy[i] = y + icony;
            if(Png_offset[i])
            {
                if((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (BDVD_FLAG | PS1_FLAG))
                    Png_iscover[i] = -1;
                else if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & PS1_FLAG))
                    Png_iscover[i] = -1;
            }

            if((select_px == m && select_py == n))
            {
                centerx = relx;
                if(Png_iscover[i] == -1 && ((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (PS1_FLAG))) centerx+= (162 + 10)/2;
                else if(Png_iscover[i] == -1) centerx+= (142 + 10)/2;
                else if(Png_iscover[i] ==  1) centerx+= (124 + 10)/2;
                else centerx+= (162 + 10)/2;
            }

            if(Png_offset[i])
            {
                if(Png_iscover[i] == -1 && ((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (PS1_FLAG)))
                    {rel_posx[i] = relx; rel_widthx[i] = 162; relx+= rel_widthx[i] + 10;}
                else if(Png_iscover[i] == -1)
                    {rel_posx[i] = relx; rel_widthx[i] = 142; relx+= rel_widthx[i] + 10;}
                else if(Png_iscover[i] == 1)
                    {rel_posx[i] = relx; rel_widthx[i] = 124; relx+= rel_widthx[i] + 10;}
                else
                    {rel_posx[i] = relx; rel_posy[i]+= 16; rel_widthx[i] = 162; rel_widthy[i] = 142 - 16;relx+= rel_widthx[i] + 10;}
            } else
                {rel_posx[i] = relx; rel_widthx[i] = 128; relx+= rel_widthx[i] + 10;}

            i++;
        }
    }
    //////////////////////

    u32 color_line = 0x404040ff;

    // get_games_3

    if(centerx < 848/2) centerx = -(848/2 - centerx); else centerx -= 848/2;

    if((rel_posx[0] - centerx) > 30)  centerx = -30;

    // -> anim_mode
    if(!anim_mode || !g_saved)
    {
        // don´t scroll the covers
        n = select_px + select_py * 4;

        if((n - 1) < 0) g_rel_posx[0] = rel_posx[0] - centerx; else g_rel_posx[0] = rel_posx[n - 1] - centerx;
        if((n + 1) > 11) g_rel_posx[1] = rel_posx[11] - centerx; else g_rel_posx[1] = rel_posx[n + 1] - centerx;

        anim_mode = 0;
        g_saved = 1;
    }
    else
    {
        if(anim_mode == 2)
        {
            int a = g_rel_posx[1] - (rel_posx[select_px + select_py * 4] - centerx);

            centerx -= (a * (4 - anim_step))/5;

            anim_step++; if(anim_step > 4) {anim_mode = 0; g_saved = 0;}
        }
        else if(anim_mode == 1)
        {
            int a = g_rel_posx[0] - (rel_posx[select_px + select_py * 4] - centerx);

            centerx -= (a * (4 - anim_step))/5;

            anim_step++; if(anim_step > 4) {anim_mode = 0; g_saved = 0;}
        }
    }
    // -> end anim_mode

    float f3 = 0.0f;

    for(l = 0; l < 2; l++)
    {
        i = 0;

        for(n = 0; n < 3; n++)
            for(m = 0; m < 4; m++)
            {
                int f = (select_px == m && select_py == n);
                float f2 = (int) f;

                if(l != 0 && f) {i++;continue;}
                if(l == 0 && !f) {i++;continue;}

                if(!mode_favourites && get_currentdir(i) >= ndirectories) break;

                f2 =  ((float) (flash2 *(select_px == m && select_py == n))) / ((float) MAX_FLASH);


                if(f) {f2 = 3 + 2 *f2; f3 = f2;}

                int set_ps3_cover = 0;

                if(Png_offset[i])
                {
                    if(Png_iscover[i] == 1) set_ps3_cover = 1; // PS3 cover

                    if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
                    {
                        if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD)
                        {
                             if((directories[get_currentdir(i)].flags & D_FLAG_HOMEB_MKV) == D_FLAG_HOMEB_MKV)
                             tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MOVIE_ICON], Png_res[IMG_MOVIE_ICON].width,
                                                   Png_res[IMG_MOVIE_ICON].height, Png_res[IMG_MOVIE_ICON].wpitch,
                                                   TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                             else
                             tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DVD_DISC], Png_res[IMG_DVD_DISC].width,
                                                   Png_res[IMG_DVD_DISC].height, Png_res[IMG_DVD_DISC].wpitch,
                                                   TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        }
                        else tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                                   Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                                   TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                    }
                    else if((directories[get_currentdir(i)].flags  & ((BDVD_FLAG) | (PS1_FLAG))) == ((BDVD_FLAG) | (PS1_FLAG)))
                    {
                        //Png_iscover[i] = -1;
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS1_DISC], Png_res[IMG_PS1_DISC].width,
                                              Png_res[IMG_PS1_DISC].height, Png_res[IMG_PS1_DISC].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                    else if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & (PS1_FLAG)))
                    {
                        // add PSP / PS2 / PSX ISO icon
                        if((directories[get_currentdir(i)].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
                        {
                            bool is_retro = (strstr(directories[get_currentdir(i)].path_name, retro_root_path) != NULL);
                            bool is_ps2_classic = !is_retro &&
                                                  (strstr(directories[get_currentdir(i)].path_name, ps2classic_path) != NULL);

                            if(is_retro)
                                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_RETRO_ICON], Png_res[IMG_RETRO_ICON].width,
                                                      Png_res[IMG_RETRO_ICON].height, Png_res[IMG_RETRO_ICON].wpitch,
                                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                            else if(is_ps2_classic)
                                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                                      Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                            else
                                tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PSP_ISO], Png_res[IMG_PSP_ISO].width,
                                                      Png_res[IMG_PSP_ISO].height, Png_res[IMG_PSP_ISO].wpitch,
                                                      TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        }
                        else if((directories[get_currentdir(i)].flags & (PS2_FLAG)) == (PS2_FLAG))
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                                  Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        else
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS1_ISO], Png_res[IMG_PS1_ISO].width,
                                                  Png_res[IMG_PS1_ISO].height, Png_res[IMG_PS1_ISO].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                    else if(Png_iscover[i] < 1 && (directories[get_currentdir(i)].flags  & ((PS3_FLAG) | (BDVD_FLAG))) == (PS3_FLAG))
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                                  Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    else
                            tiny3d_SetTextureWrap(0, Png_offset[i], Png_datas[i].width,
                                                 Png_datas[i].height, Png_datas[i].wpitch,
                                                 TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                    //if((directories[get_currentdir(i)].flags  & ((BDVD_FLAG) | (PS1_FLAG))) == (PS1_FLAG)) {
                    //    set_ps3_cover = 2; // PSX cover
                    //    if((directories[get_currentdir(i)].flags  & (PS2_FLAG)) == IS_PS2_FLAG) set_ps3_cover = 3 + 1 *(Png_iscover[i] < 0); // PS2 cover
                    //}

                    // BD, DVD, MKV
                    if((directories[get_currentdir(i)].flags  & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
                    {
                        set_ps3_cover = 3;
                        if(Png_iscover[i] < 0) set_ps3_cover = 1;
                        else if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD) set_ps3_cover = 1;
                    }

                    if(!set_ps3_cover)
                        DrawBox(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, rel_widthx[i] + 8 * f2, rel_widthy[i] + 8 * f2, 0x00000028);

                    if(!set_ps3_cover)
                        DrawTextBoxLine(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, rel_widthx[i] + 8 * f2, rel_widthy[i] + 8 * f2, 0xffffffff, color_line);
                    else
                        DrawTextBoxCover(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, rel_widthx[i] + 8 * f2, rel_widthy[i] + 8 * f2, 0xffffffff, set_ps3_cover - 1);

                    if(!set_ps3_cover)
                    {
                        DrawBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, rel_widthx[i] + 8 * f2, (rel_widthy[i]  * 5 / 8 + 8 * f2), 0x00000028);
                        DrawTextBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, rel_widthx[i] + 8 * f2, (rel_widthy[i]  * 5 / 8 + 8 * f2), 0x60606090);
                    }
                    else
                    {
                        DrawTextBoxCoverShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, rel_widthx[i] + 8 * f2, (rel_widthy[i]  * 5 / 8 + 8 * f2), 0x60606090, set_ps3_cover - 1);
                    }

                    //if((mode_favourites != 0) && favourites.list[i].index < 0) exit(0);

                    if(!mode_favourites || ((mode_favourites != 0) && favourites.list[i].index >= 0))
                    {
                        // ignore bluray icon
                        if((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG)) == (BDVD_FLAG | PS1_FLAG)) ;
                        // draw Bluray icon
                        else if((directories[get_currentdir(i)].flags  & GAMELIST_FILTER) == BDVD_FLAG)
                        {
                            tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                                  Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                                  TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                            DrawTextBox(rel_posx[i] - centerx - 4 * f2 + (rel_widthx[i] - 124)/2, rel_posy[i] + 4 + (rel_widthy[i] - 124)/2 - 4 * f2, !f ? 100 : 0,  (124) + 8 * f2, (124) + 8 * f2, 0xffffffcf);

                            DrawTextBoxShadow(rel_posx[i] - centerx - 4 * f2 + (rel_widthx[i] - 124)/2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0,  (124) + 8 * f2, (124 * 5 / 8) + 8 * f2, 0x60606090);
                        } else
                        // draw Usb icon
                        if(bShowUSBIcon && (directories[get_currentdir(i)].flags  & GAMELIST_FILTER) > 1)
                        {
                            int ii = 1 + 13 * ((directories[get_currentdir(i)].flags & NTFS_FLAG) != 0);
                            tiny3d_SetTextureWrap(0, Png_res_offset[ii], Png_res[ii].width,
                            Png_res[ii].height, Png_res[ii].wpitch,
                                TINY3D_TEX_FORMAT_A8R8G8B8, TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                            int x_cor = 0, y_cor = 0;

                            if(set_ps3_cover == 1 || set_ps3_cover == 3)
                            {
                                x_cor = 0; y_cor = 16;
                            }

                            if(set_ps3_cover == 2 || set_ps3_cover == 4)
                            {
                                x_cor = 12; y_cor = 0;
                            }

                            if(directories[get_currentdir(i)].splitted)
                                DrawTextBoxLine(x_cor + rel_posx[i] - centerx - 4 * f2 + 4, y_cor + rel_posy[i] + 4 - 4 * f2, !f ? 100 : 0, 32, 24, 0xff9999aa, color_line);
                            else
                                DrawTextBoxLine(x_cor + rel_posx[i] - centerx - 4 * f2 + 4, y_cor + rel_posy[i] + 4 - 4 * f2, !f ? 100 : 0, 32, 24, 0xffffffcf, color_line);
                        }
                    }


                }
                else if(mode_favourites && favourites.list[i].title_id[0] != 0)
                {
                    DrawBoxLine(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, 128 + 8 * f2, rel_widthy[i] + 8 * f2, 0xa0a0a028, color_line);
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MISSING_ICON], Png_res[IMG_MISSING_ICON].width,
                                          Png_res[IMG_MISSING_ICON].height, Png_res[IMG_MISSING_ICON].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                    DrawTextBox(rel_posx[i] - centerx - 4 * f2, y  + 7 - 4 * f2 + icony, !f ? 100 : 0, 128 + 8 * f2, 128 + 8 * f2, 0xffffff3f);
                    DrawBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, 128 + 8 * f2, (rel_widthy[i] * 5 / 8 + 8 * f2), 0x8f8f8f28);
                    DrawTextBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, 128 + 8 * f2, (rel_widthy[i] * 5 / 8 + 8 * f2), 0x40404090);
                }
                else
                {
                    // draw Bluray icon with empty icon
                    if((directories[get_currentdir(i)].flags  & GAMELIST_FILTER) == BDVD_FLAG &&
                        ((directories[get_currentdir(i)].flags  & (BDVD_FLAG | PS1_FLAG))!= (BDVD_FLAG | PS1_FLAG)))
                    {

                        DrawBoxLine(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, 128 + 8 * f2, rel_widthy[i] + 8 * f2, 0xa0a0a040, color_line);
                        DrawBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, 128 + 8 * f2, (rel_widthy[i] * 5 / 8 + 8 * f2), 0x80808040);

                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                                     Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                                     TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        DrawTextBox(rel_posx[i] - centerx - 4 * f2 + (rel_widthx[i] - 124)/2, rel_posy[i] + 4 + (rel_widthy[i] - 124)/2 - 4 * f2, !f ? 100 : 0,  (124) + 8 * f2, (124) + 8 * f2, 0xffffffcf);

                        DrawTextBoxShadow(rel_posx[i] - centerx - 4 * f2 + (rel_widthx[i] - 124)/2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0,  (124) + 8 * f2, (124 * 5 / 8) + 8 * f2, 0x60606090);

                    }
                    else
                    {
                        DrawBoxLine(rel_posx[i] - centerx - 4 * f2, rel_posy[i] - 4 * f2, !f ? 100 : 0, 128 + 8 * f2, rel_widthy[i] + 8 * f2, 0xa0a0a040, color_line);
                        DrawBoxShadow(rel_posx[i] - centerx - 4 * f2, y + icony - 4 * f2 + 142 + 8 * f2 + 8, !f ? 100 : 0, 128 + 8 * f2, (rel_widthy[i] * 5 / 8 + 8 * f2), 0x80808040);
                    }
                }

                i++;
            }
    }

    i = selected;

    if(flash)
    {
        int png_on = 0;

        //DrawBox(x + 200 * select_px - 4, y + select_py * 150 - 4 , 0, 200, 150, 0xa0a06080);

        if(mode_favourites >= 0x10000)
        {
            if(mode_favourites < 0x20000)
            {
                if(Png_offset[BIG_PICT])
                {
                    tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                        Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    png_on = 1;
                }
            }
            else
           {
                i = mode_favourites - 0x20000;

                if(i>= 0 && i < num_box)
                {
                    if(!Png_offset[i] && favourites.list[i].title_id[0] != 0)
                    {
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MISSING_ICON], Png_res[IMG_MISSING_ICON].width,
                                              Png_res[IMG_MISSING_ICON].height, Png_res[IMG_MISSING_ICON].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                        png_on = 1;
                    }
                    else if(Png_offset[i])
                    {
                        png_on = 1;
                        tiny3d_SetTextureWrap(0, Png_offset[i], Png_datas[i].width,
                        Png_datas[i].height, Png_datas[i].wpitch,
                            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    }
                }
            }

            int ii = select_px + 4 * select_py;

            if(png_on)
                DrawTextBox(rel_posx[ii] - centerx - 4 * f3, rel_posy[ii] - 4 * f3 , 0 , rel_widthx[ii] + 8 * f3, rel_widthy[ii] + 8 * f3, 0x8fff8fcf);
            else
                DrawBox(rel_posx[ii] - centerx - 4 * f3, rel_posy[ii] - 4 * f3 , 0 , rel_widthx[ii] + 8 * f3, rel_widthy[ii] + 8 * f3, 0x8fff8fcf);
        }

    }

    SetFontColor(0xffffffff, 0x00000000);

    // display temp
    if((frame_count & 0x100))
    {
        static u32 temp = 0;
        static u32 temp2 = 0;
        int y2;

        if(temp == 0 || (frame_count & 0x1f) == 0x0 )
        {
            sys_game_get_temperature(0, &temp);
            sys_game_get_temperature(1, &temp2);
        }

        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);

        x2 = DrawFormatString(1024, 0, " Temp CPU: 99ºC RSX: 99ºC ");

        y2 = y + 3 * 150 - 4 + 12;
        SetFontColor(0xffffffff, 0x00000000);
        x2 = DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y2, " Temp CPU: ");
        if(temp < 80) SetFontColor(0xfff000ff, 0x00000000); else SetFontColor(0xff0000ff, 0x00000000);
        x2 = DrawFormatString(x2, y2, "%uºC",  temp);
        SetFontColor(0xffffffff, 0x00000000);
        x2 = DrawFormatString(x2, y2, " RSX: ");
        if(temp2 < 75) SetFontColor(0xfff000ff, 0x00000000); else SetFontColor(0xff0000ff, 0x00000000);
        x2 = DrawFormatString(x2, y2, "%uºC ", temp2);

        SetFontColor(0xffffffff, 0x00000000);
    }
    else if(Png_offset[i])
    {
        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_SOPTIONS]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 - 2, " %s ", language[DRAWSCREEN_SOPTIONS]);

    }
    else if(mode_favourites && mode_favourites < 0x10000 && favourites.list[i].title_id[0] != 0)
    {
        SetCurrentFont(FONT_TTF);
        SetFontSize(20, 20);
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_SDELETE]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 - 2, " %s ", language[DRAWSCREEN_SDELETE]);
    }
    else
    {
        //DrawBox(x + 200 * select_px , y + select_py * 150 , 0, 192, 142, 0x404040a0);
        SetCurrentFont(FONT_TTF); // get default
        SetFontSize(20, 20);
    }

    if(!(frame_count & 0x100))
    {
        x2 = DrawFormatString(1024, 0, " %s ", language[DRAWSCREEN_STGLOPT]);
        DrawFormatString(x + 4 * 200 - (x2 - 1024) - 12 , y + 3 * 150 + 18, " %s ", language[DRAWSCREEN_STGLOPT]);
    }

    // draw game name
    i = selected;

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffee, 0x00000000);

    if((Png_offset[i] && !mode_favourites) || (mode_favourites && favourites.list[i].title_id[0] != 0))
    {
        u32 str_color = 0xffffffee;

        if(mode_favourites)
        {
            if(strncmp((char *) string_title_utf8, favourites.list[i].title, 64))
            {
                strncpy((char *) string_title_utf8, favourites.list[i].title, 128);
                update_title_utf8 = 1;
            }
        }
        else if((directories[(currentdir + i)].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            str_color = 0xafd836ee;
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }

        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[(currentdir + i)].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[(currentdir + i)].title, 128);
                update_title_utf8 = 1;
            }
        }

        n = 0;
        while(string_title_utf8[n])
        {
            if(string_title_utf8[n] == 13 || string_title_utf8[n] == 10) string_title_utf8[n] = '/';
            n++;
        }

        SetCurrentFont(FONT_TTF);
        SetFontSize(16, 32);
        SetFontAutoCenter(1);
        SetFontColor(str_color, 0x00000020);
        DrawFormatString(0 , y + 80 - 32, " %s ", string_title_utf8);
        SetFontColor(0xffffffff, 0x00000000);
/*
        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBox(x + 3, y + 3 * 150 , 0, 768, 32, str_color);
*/

        SetFontAutoCenter(0);

        load_gamecfg (get_currentdir(i)); // refresh game info

        x2 = 0;


        // BD, DVD, MKV
        if((directories[get_currentdir(i)].flags  & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB)
        {
             if(directories[get_currentdir(i)].flags & D_FLAG_HOMEB_DVD)
             {
                   if((directories[get_currentdir(i)].flags & D_FLAG_HOMEB_MKV) == D_FLAG_HOMEB_MKV)
                   tiny3d_SetTextureWrap(0, Png_res_offset[IMG_MOVIE_ICON], Png_res[IMG_MOVIE_ICON].width,
                                     Png_res[IMG_MOVIE_ICON].height, Png_res[IMG_MOVIE_ICON].wpitch,
                                     TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                   else
                   tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DVD_DISC], Png_res[IMG_DVD_DISC].width,
                                         Png_res[IMG_DVD_DISC].height, Png_res[IMG_DVD_DISC].wpitch,
                                         TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
             }
             else tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                        Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                        TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

             DrawTextBox(x + x2, y + 3 * 150 - 48, 0, 32, 32, 0xffffff99);

             x2+= 40;

             u32 ff = directories[get_currentdir(i)].flags & D_FLAG_HOMEB_MKV;

             if(ff == D_FLAG_HOMEB_BD) x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "BD ISO");
             else if(ff == D_FLAG_HOMEB_DVD) x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "DVD ISO");
             else if(ff == D_FLAG_HOMEB_MKV) x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "VIDEO");

        }
        else
        {
            if((directories[get_currentdir(i)].flags & PS1_FLAG) || (directories[get_currentdir(i)].flags & D_FLAG_PS3_ISO) || (game_cfg.useBDVD) || (game_cfg.direct_boot == 2))
            {
                // add PSP / PS2 / PSX ISO icon
                if((directories[get_currentdir(i)].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
                {
                    bool is_retro = (strstr(directories[get_currentdir(i)].path_name, retro_root_path) != NULL);
                    bool is_ps2_classic = !is_retro &&
                                          (strstr(directories[get_currentdir(i)].path_name, ps2classic_path) != NULL);

                    if(is_retro)
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_RETRO_ICON], Png_res[IMG_RETRO_ICON].width,
                                              Png_res[IMG_RETRO_ICON].height, Png_res[IMG_RETRO_ICON].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    else if(is_ps2_classic)
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                              Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                    else
                        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PSP_ISO], Png_res[IMG_PSP_ISO].width,
                                              Png_res[IMG_PSP_ISO].height, Png_res[IMG_PSP_ISO].wpitch,
                                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                }
                else if((directories[get_currentdir(i)].flags & (PS2_FLAG)) == (PS2_FLAG))
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS2_ISO], Png_res[IMG_PS2_ISO].width,
                                          Png_res[IMG_PS2_ISO].height, Png_res[IMG_PS2_ISO].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                else if((directories[get_currentdir(i)].flags & (PS1_FLAG)) == (PS1_FLAG))
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_PS1_ISO], Png_res[IMG_PS1_ISO].width,
                                          Png_res[IMG_PS1_ISO].height, Png_res[IMG_PS1_ISO].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
                else
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_BLURAY_DISC], Png_res[IMG_BLURAY_DISC].width,
                                          Png_res[IMG_BLURAY_DISC].height, Png_res[IMG_BLURAY_DISC].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                DrawTextBox(x + x2,  y + 3 * 150 - 48, 0, 32, 32, 0xffffff99);

                x2+= 40;
            }


            if(!(directories[get_currentdir(i)].flags & D_FLAG_PS3_ISO))
            {
                if(game_cfg.direct_boot)
                {
                    tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DIRECT_ICON], Png_res[IMG_DIRECT_ICON].width,
                                          Png_res[IMG_DIRECT_ICON].height, Png_res[IMG_DIRECT_ICON].wpitch,
                                          TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

                    DrawTextBox(x + x2,  y + 3 * 150 - 48, 0, 32, 32, 0xffffffff);

                    x2+= 40;
                }

                n = (directories[get_currentdir(i)].flags & D_FLAG_HDD0) ?  game_cfg.bdemu : game_cfg.bdemu_ext;

                SetFontColor(0xffffffee, 0x00000000);
                SetCurrentFont(FONT_TTF);
                SetFontSize(12, 24);

                if((directories[get_currentdir(i)].flags & (PS1_FLAG)) == (PS1_FLAG))
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "PSX ISO");
                else if(n == 1)
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "BD EMU");
                else if(n == 2)
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "LIBFS");
            }
            else if((directories[get_currentdir(i)].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
            {
                bool is_retro = (strstr(directories[get_currentdir(i)].path_name, retro_root_path) != NULL);
                bool is_ps2_classic = !is_retro &&
                                      (strstr(directories[get_currentdir(i)].path_name, ps2classic_path) != NULL);

                if(is_retro)
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "RETRO");
                else if(is_ps2_classic)
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "PS2 CLASSIC");
                else
                    x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "PSP ISO");
            }
            else if((directories[get_currentdir(i)].flags & (PS2_FLAG)) == (PS2_FLAG))
                x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "PS2 ISO");
            else
                x2 = DrawString(x + x2,  y + 3 * 150 - 48 + 4, "PS3 ISO");
        }
    }

    if(bHideCoverflowSortModeLabel);
    else if(str_type == 1 || str_type == 2)
    {
        SetFontColor(0xffffffee, 0x00000000);
        SetCurrentFont(FONT_TTF);
        SetFontSize(12, 24);

        SetFontAutoCenter(1);

        if(sort_mode == 1)
            DrawString(0,  y + 3 * 150 - 48 + 4, "PS3 > PSX");
        else if(sort_mode == 2)
            DrawString(0,  y + 3 * 150 - 48 + 4, "PSX > PS3");
        else
            DrawString(0,  y + 3 * 150 - 48 + 4, "PS3 - PSX");

        SetFontAutoCenter(0);
    }

    if(bshowpath)
    {
        SetFontColor(0xffffff88, 0x00000000);
        SetCurrentFont(FONT_TTF);
        SetFontSize(10, 16);

        SetFontAutoCenter(1);
        DrawString(0,  y + 3 * 150 - 80, directories[get_currentdir(i)].path_name);
    }

// draw box config
/*
    tiny3d_SetPolygon(TINY3D_LINE_STRIP);
    tiny3d_VertexPos(x - 4    , y + 3 * 150 - 52    , 0);
    tiny3d_VertexColor(0x808080ff);

    tiny3d_VertexPos(x + 124 + 8, y + 3 * 150 - 52    , 0);

    tiny3d_VertexPos(x + 124 + 8, y + 3 * 150 - 52 + 40, 0);

    tiny3d_VertexPos(x - 4   , y + 3 * 150 - 52 + 40, 0);

    tiny3d_VertexPos(x - 4   , y + 3 * 150 - 52    , 0);
    tiny3d_End();
*/

    if(flash && ftp_inited)
    {
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_FTP_ICON], Png_res[IMG_FTP_ICON].width,
                              Png_res[IMG_FTP_ICON].height, Png_res[IMG_FTP_ICON].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBox(200 * 4 -32, y + 3 * 150 - 48, 0, 64, 32, 0xffffffff);
    }

    //SetCurrentFont(FONT_DEFAULT);

    if(signal_ntfs_mount && (frame_count & MAX_FLASH))
    {
        int ii = 1 + 13;
        tiny3d_SetTextureWrap(0, Png_res_offset[ii], Png_res[ii].width,
        Png_res[ii].height, Png_res[ii].wpitch,
            TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBoxLine(848 - 64 - 16, 24, 0, 64, 48, 0xffffffcf, 0x404040ff);
    }

    tiny3d_Flip();

}

int gui_control()
{
    int i = 0, n = 0, r = SUCCESS;

    int selected = select_px + select_py * cols;

    ps3pad_read();

    if(menu_screen == 0)
    {
        if(autolaunch == LAUNCHMODE_TOCHECK) LoadLastGame();

        if(autolaunch >= LAUNCHMODE_STARTED)
        {
            i = autolaunch;
            currentdir = 0;

            selected = autolaunch;
            r = 1; //do not refresh gui
            load_gamecfg(autolaunch);

            mode_favourites = 0;
            autolaunch = LAUNCHMODE_CHECKED;
            goto autolaunch_proc;
        }
    }

    autolaunch = LAUNCHMODE_CHECKED;

    if(mode_favourites && (old_pad & BUTTON_L2) && (new_pad & BUTTON_TRIANGLE))
    {
        mode_favourites = 0x20000 | (selected);
        return r;
    }

    if(new_pad & BUTTON_CIRCLE)
    {
        if(mode_favourites >= 0x10000) mode_favourites = 1;
        else
        {
            if(!test_ftp_working())
            {
                if(old_pad & BUTTON_L2)
                {
                    fun_exit();
                    SaveGameList();

                    // relaunch iris manager
                    sprintf(temp_buffer, "%s/USRDIR/RELOAD.SELF", self_path);
                    sysProcessExitSpawn2(temp_buffer, NULL, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
                    exit(0);
                }

                if (old_pad & (BUTTON_SELECT))
                {
                    if(DrawDialogYesNo(language[DRAWSCREEN_RESTART]) == 1) {SaveGameList(); set_install_pkg = 1; game_cfg.direct_boot = 0; exit(0);}
                }
                else
                {
                    if(DrawDialogYesNo(language[DRAWSCREEN_EXITXMB]) == 1) {SaveGameList(); exit_program = 1; return r;}
                }
            }
        }
    }

    if(new_pad & BUTTON_SQUARE)
    {
        if(old_pad & BUTTON_SELECT)
        {
            if(cover_mode) cover_mode = 0; else cover_mode = 1;
            manager_cfg.cover_mode = cover_mode;

            if(mode_homebrew != HOMEBREW_MODE)
            {
                get_games();
                load_gamecfg(-1); // force refresh game info
            }
        }

        if(!mode_homebrew)
        {
            if(!bdvd_ejected)
                Eject_BDVD(NOWAIT_BDVD | EJECT_BDVD);
            else
                Eject_BDVD(NOWAIT_BDVD | LOAD_BDVD);
        }

    }

    if(new_pad & BUTTON_CROSS)
    {
        i = selected;

        if(mode_favourites >= 0x20000)
        {
            // swap favourites
            entry_favourites swap = favourites.list[i];

            favourites.list[i] = favourites.list[mode_favourites - 0x20000]; favourites.list[mode_favourites - 0x20000] = swap;

            sprintf(temp_buffer, "%s/config/", self_path);
            SaveFavourites(temp_buffer, mode_homebrew);

            mode_favourites = 1;
            get_games();

            return r;
        }

        if(mode_favourites >= 0x10000)
        {
            // insert favourites
            DeleteFavouritesIfExits(directories[mode_favourites - 0x10000].title_id);
            AddFavourites(i, directories, mode_favourites - 0x10000);

            sprintf(temp_buffer, "%s/config/", self_path);
            SaveFavourites(temp_buffer, mode_homebrew);

            mode_favourites = 1;
            get_games();

            return r;
        }

        if(test_ftp_working()) return r;

        if(Png_offset[i])
        {
            if(mode_favourites != 0 && favourites.list[i].index < 0)
            {
                DrawDialogOK(language[DRAWSCREEN_CANRUNFAV]); return r;
            }
            else
            {
autolaunch_proc:
                //////////////////////////////////////////////////////////////////////////////////////////

                if(autolaunch >= LAUNCHMODE_STARTED)
                    currentgamedir = autolaunch;
                else
                    currentgamedir = (mode_favourites != 0) ? favourites.list[i].index : (currentdir + i);

                if(currentgamedir < 0 || currentgamedir >= ndirectories) return r;

                int use_cache = 0;

                reset_sys8_path_table();

                SaveGameList();

                if(!strncmp(directories[currentgamedir].title_id, "HTSS00003", 9) || !strncmp(directories[currentgamedir].title_id, "IRISMAN00", 9))
                {
                    sysProcessExitSpawn2(directories[currentgamedir].path_name, NULL, NULL, NULL, 0, 3071, SYS_PROCESS_SPAWN_STACK_SIZE_1M);
                    return r;
                }

                // Launch Retro / ROM
                bool is_retro = ((directories[currentgamedir].flags & (RETRO_FLAG)) == (RETRO_FLAG)) &&
                                (strstr(directories[currentgamedir].path_name, retro_root_path) != NULL);

                if(is_retro)
                {
                    char rom_path[MAXPATHLEN];
                    sprintf(rom_path, "%s", directories[currentgamedir].path_name);
                    launch_retro(rom_path);
                    return r;
                }

                // Mount PS2 Classic ISO (*.ENC.BIN)
                bool is_ps2_classic = ((directories[currentgamedir].flags & (PS2_CLASSIC_FLAG)) == (PS2_CLASSIC_FLAG)) &&
                                      (strstr(directories[currentgamedir].path_name, ps2classic_path) != NULL);

                if(is_ps2_classic)
                {
                    launch_ps2classic(directories[currentgamedir].path_name, directories[currentgamedir].title);
                    return r;
                }

                if((directories[currentgamedir].flags & (BDVD_FLAG | HOMEBREW_FLAG | PS1_FLAG)) == PS1_FLAG)
                {
                    if(!(directories[currentgamedir].flags & PS3_FLAG))
                    {
                        if (strstr(directories[currentgamedir].path_name, "/PSXISO/") != NULL)
                            launch_iso_game(directories[currentgamedir].path_name, 0);
                        else
                        {
                            reset_sys8_path_table();

                            //syscall36("/dev_bdvd");
                            add_sys8_bdvd(NULL, NULL);

                            if(lv2peek(0x80000000000004E8ULL) && !use_cobra) syscall_40(1, 0); // disables PS3 Disc-less

                            // load PSX options
                            LoadPSXOptions(directories[currentgamedir].path_name);

                            if(psx_iso_prepare(directories[currentgamedir].path_name, directories[currentgamedir].title, NULL) == 0)
                                return r;

                            psx_launch();
                        }
                        return r;
                    }
                }

                if(/*noBDVD &&*/ use_cobra && (directories[currentgamedir].flags & (BDVD_FLAG | HOMEBREW_FLAG | PS3_FLAG)) == PS3_FLAG)
                {
                    mount_iso_game();
                    return r;
                }
                if(!(directories[currentgamedir].flags & BDVD_FLAG) && !(directories[currentgamedir].flags & D_FLAG_HOMEB))
                {
                    struct stat s;
                    memcpy((char *) temp_buffer, (char *) directories[currentgamedir].title_id, 4);
                    strncpy((char *) &temp_buffer[4], (char *) &directories[currentgamedir].title_id[5], 58);

                    sprintf(temp_buffer, "/dev_hdd0/game/%s/USRDIR/EBOOT.BIN", temp_buffer);

                    if(stat(temp_buffer, &s) != SUCCESS)
                        sprintf(temp_buffer, "%s/PS3_GAME/USRDIR/EBOOT.BIN", directories[currentgamedir].path_name);

                    if(!stat(temp_buffer, &s))
                    {
                        int r =  patch_exe_error_09(temp_buffer);
                        if(r == 1)
                        {
                            pause_music(1);

                            test_game(currentgamedir);
                        }
                        else if(r == -1)
                        {
                            DrawDialogOKTimer("This game requires a higher CFW or rebuild the SELFs/SPRX\n\nEste juego requiere un CFW superior o reconstruir los SELFs/SPRX\n\nCe jeu à besoin d'un CFW supérieur ou reconstruire les SELFs/SPRX", 2000.0f);
                            return r;
                        }
                    }


                    if(noBDVD && use_cobra)
                    {
                        char *files[1];

                        memcpy((char *) temp_buffer, (char *) directories[currentgamedir].title_id, 4);
                        strncpy((char *) &temp_buffer[4], (char *) &directories[currentgamedir].title_id[5], 58);

                        char *blank_iso = build_blank_iso((char *) temp_buffer);

                        if (blank_iso)
                        {
                            files[0] = blank_iso;
                            int ret = cobra_mount_ps3_disc_image(files, 1);
                            free(blank_iso);

                            if (ret == 0)
                            {
                                cobra_send_fake_disc_insert_event();

                                //DrawDialogOKTimer("PS3 Disc inserted", 2000.0f);
                            }
                        }
                    }
                }
                if(!((directories[currentgamedir].flags & (BDVD_FLAG | PS1_FLAG)) == (BDVD_FLAG | PS1_FLAG)) || // !psx CD
                    (directories[currentgamedir].flags & D_FLAG_HOMEB))
                {  // !homebrew
                    if(!(directories[currentgamedir].flags & BDVD_FLAG) && lv2peek(0x80000000000004E8ULL))
                        set_BdId(currentgamedir);
                }
                if(directories[currentgamedir].splitted == 1)
                {
                    if( payload_mode >= ZERO_PAYLOAD )
                    {
                        sprintf(temp_buffer, "%s/cache/%s/%s",
                                self_path, directories[currentgamedir].title_id, "/paths.dir");

                        struct stat s;

                        if(stat(temp_buffer, &s) != SUCCESS)
                        {
                            sprintf(temp_buffer + 1024, "%s\n\n%s",
                            directories[currentgamedir].title, language[DRAWSCREEN_MARKNOTEXEC]);
                            DrawDialogOK(temp_buffer + 1024);

                            copy_to_cache(currentgamedir, self_path);

                            sprintf(temp_buffer, "%s/cache/%s/%s", self_path,
                            directories[currentgamedir].title_id, "/paths.dir");
                            if(stat(temp_buffer, &s) != SUCCESS) return r; // cannot launch without cache files
                        }

                        use_cache = 1;
                    }
                    else
                    {
                        sprintf(temp_buffer,
                            "%s\n\n%s",
                            directories[get_currentdir(i)].title, language[DRAWSCREEN_MARKNOTEX4G]);
                        DrawDialogOK(temp_buffer);return r;
                    }
                }
                /// cache
                if(use_cache && ((game_cfg.bdemu == 1     &&  (directories[currentgamedir].flags & D_FLAG_HDD0)) ||
                                 (game_cfg.bdemu_ext == 1 && !(directories[currentgamedir].flags & D_FLAG_HDD0))))
                {

                    DrawDialogOKTimer("BD EMU cannot work with big files in cache data\n\nBD EMU no puede trabajar con ficheros grandes en cache de datos\n\nBD EMU ne peut pas travailler avec de gros fichiers en Cache", 2000.0f);
                    return r;
                }
                if(game_cfg.exthdd0emu)
                {
                    if((directories[currentgamedir].flags & D_FLAG_USB) != 0)
                    {
                        for(n = 1; n < 11 ; n++) if((directories[currentgamedir].flags  & GAMELIST_FILTER) == (1 << n)) break;
                        sprintf(temp_buffer, "/dev_usb00%c/GAMEI", 47 + n);
                        mkdir_secure(temp_buffer);
                        add_sys8_path_table(self_path, self_path);

                        if(((directories[currentgamedir].flags & D_FLAG_HDD0) && game_cfg.bdemu) || (!(directories[currentgamedir].flags & D_FLAG_HDD0) && game_cfg.bdemu_ext))
                            add_sys8_path_table("/dev_hdd0/game", "/dev_bdvd/GAMEI");
                        else
                            add_sys8_path_table("/dev_hdd0/game", temp_buffer);

                    }
                    else if((fdevices & D_FLAG_USB) != 0)
                    {
                        for(n = 1; n < 11; n++)
                        {
                            // searching directory
                            if(fdevices & (1 << n))
                            {
                                DIR  *dir;

                                sprintf(temp_buffer, "/dev_usb00%c/GAMEI", 47 + n);
                                dir = opendir (temp_buffer);
                                if(dir)
                                {
                                    closedir (dir);

                                    add_sys8_path_table(self_path, self_path);
                                    add_sys8_path_table("/dev_hdd0/game", temp_buffer);
                                    break;
                                }
                            }
                        }

                        if(n == BDVD_DEVICE)
                        {
                             // directory not found, Asking to create one
                             for(n = 1; n < 11 ; n++)
                             {
                                if(fdevices & (1 << n))
                                {
                                    sprintf(temp_buffer, "%s\n\n%s%c?"
                                            , language[DRAWSCREEN_GAMEINOFMNT], language[DRAWSCREEN_GAMEIASKDIR], 47 + n);
                                    if(DrawDialogYesNo(temp_buffer) == 1)
                                    {
                                        sprintf(temp_buffer, "/dev_usb00%c/GAMEI", 47 + n);
                                        mkdir_secure(temp_buffer);
                                        add_sys8_path_table(self_path, self_path);
                                        add_sys8_path_table("/dev_hdd0/game", temp_buffer);
                                        break;
                                    }
                                }
                             }

                             if(n == 11)
                             {
                                 sprintf(temp_buffer, "%s\n\n%s", language[DRAWSCREEN_GAMEICANTFD], language[DRAWSCREEN_GAMEIWLAUNCH]);
                                 if(DrawDialogYesNo(temp_buffer) != 1) return r;
                             }
                        }
                    }

                    if((fdevices & D_FLAG_USB) == 0)
                    {
                        sprintf(temp_buffer, "%s\n\n%s", language[DRAWSCREEN_GAMEICANTFD], language[DRAWSCREEN_GAMEIWLAUNCH]);
                        if(DrawDialogYesNo(temp_buffer) != 1) return r;
                    }

                }
                if((game_cfg.useBDVD && (fdevices & D_FLAG_BDVD) == 0) || (game_cfg.direct_boot == 2))
                {
                    load_from_bluray |= 1;
                    if(!noBDVD && check_disc() == -1)
                    return r;
                }
                if(!(directories[currentgamedir].flags & D_FLAG_BDVD))
                    param_sfo_util(directories[currentgamedir].path_name, (game_cfg.updates != 0));

                if(!game_cfg.ext_ebootbin)
                    sys8_path_table(0LL);
                else
                {
                    set_device_wakeup_mode(bdvd_is_usb ? 0xFFFFFFFF : directories[currentgamedir].flags);

                    sprintf(temp_buffer, "%s/self", self_path);
                    mkdir_secure(temp_buffer);

                    sprintf(temp_buffer, "%s/self/%s.BIN", self_path,
                        directories[get_currentdir(i)].title_id);

                    FILE *fp = fopen(temp_buffer, "rb");

                    if(!fp)
                    {
                        sprintf(temp_buffer, " %s.BIN\n %s\n\n%s",
                            directories[currentgamedir].title_id, language[DRAWSCREEN_EXTEXENOTFND], language[DRAWSCREEN_EXTEXENOTCPY]);
                        DrawDialogOK(temp_buffer);
                        goto skip_sys8;
                    }
                    else
                    {
                        fclose(fp);
                        add_sys8_path_table("/dev_bdvd/PS3_GAME/USRDIR/EBOOT.BIN", temp_buffer);
                    }
                }
                load_from_bluray = game_cfg.useBDVD;

                if(!game_cfg.useBDVD || noBDVD) sys8_sys_configure(CFG_XMB_DEBUG); else sys8_sys_configure(CFG_XMB_RETAIL);


                sys8_sys_configure(CFG_UNPATCH_APPVER + (game_cfg.updates != 0));

                if((game_cfg.bdemu && (directories[currentgamedir].flags & D_FLAG_HDD0)) ||
                     (!(directories[currentgamedir].flags & D_FLAG_HDD0) && game_cfg.bdemu_ext == 1))
                {
                    load_from_bluray |= 1;

                    if(!noBDVD && check_disc() == -1)
                        return r;

                    if(!noBDVD || use_cobra)
                        sys_fs_mount("CELL_FS_IOS:BDVD_DRIVE", "CELL_FS_ISO9660", "/dev_ps2disc", 1);

                    if((game_cfg.bdemu == 2 && (directories[currentgamedir].flags & D_FLAG_HDD0)) ||
                       (!(directories[currentgamedir].flags & D_FLAG_HDD0) && game_cfg.bdemu_ext == 2))
                    {
                         // new to add BD-Emu 2
                         n = 0;
                    }
                    else
                    {
                        n = move_origin_to_bdemubackup(directories[currentgamedir].path_name);
                        sprintf(temp_buffer, "%s/PS3_DISC.SFB", directories[currentgamedir].path_name);
                        add_sys8_path_table("/dev_bdvd/PS3_DISC.SFB", temp_buffer);
                    }

                    if(n < 0)
                    {
                        //syscall36("/dev_bdvd"); // in error exits
                        //sys8_perm_mode((u64) 0);

                        add_sys8_bdvd(NULL, NULL);

                        //if(game_cfg.ext_ebootbin)
                        build_sys8_path_table(); //prepare extern eboot

                        exit_program = 1;
                        return r;
                    }

                    is_ps3game_running = 1;

                   // if(n == 1) game_cfg.bdemu = 0; // if !dev_usb... bdemu is not usable
                }


                // HDD BDEMU-LIBFS / USB BDEMU
                if(((game_cfg.bdemu          &&  (directories[currentgamedir].flags & D_FLAG_HDD0)) ||
                    (game_cfg.bdemu_ext == 1 && !(directories[currentgamedir].flags & D_FLAG_HDD0))) &&
                     patch_bdvdemu(directories[currentgamedir].flags & GAMELIST_FILTER) == 0)
                {
                    // syscall36("//dev_bdvd"); // for hermes special flag see syscall36-3.41-hermes "//"
                    add_sys8_bdvd(NULL, NULL);

                    //we dont want libfs on USB devices
                    if(is_libfs_patched() && (game_cfg.bdemu && (directories[currentgamedir].flags & D_FLAG_HDD0)))
                    {
                        sprintf(temp_buffer + 1024, "%s/libfs_patched.sprx", self_path);
                        add_sys8_path_table("/dev_flash/sys/external/libfs.sprx", temp_buffer + 1024);
                        sys8_pokeinstr(0x80000000007EF220ULL, 0x45737477616C6420ULL);
                    }


                    // HDD LIBFS
                   if((game_cfg.bdemu == 2 && (directories[currentgamedir].flags & D_FLAG_HDD0)))
                   {    // new to add BD-Emu 2
                        mount_custom(directories[currentgamedir].path_name);

                        if(!game_cfg.useBDVD)
                        {
                            sprintf(temp_buffer, "%s/PS3_DISC.SFB", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/PS3_DISC.SFB", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/PS3_GAME", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/USRDIR", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home", temp_buffer);
                        }

                        add_sys8_bdvd(directories[currentgamedir].path_name, NULL);

                        //syscall36(directories[currentgamedir].path_name);
                   }
                   else
                   {
                        mount_custom(directories[currentgamedir].path_name);
                        if(!game_cfg.useBDVD)
                        {
                            sprintf(temp_buffer, "%s/PS3_DISC.SFB", "/dev_bdvd");
                            add_sys8_path_table("/app_home/PS3_DISC.SFB", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME", "/dev_bdvd");
                            add_sys8_path_table("/app_home/PS3_GAME", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", "/dev_bdvd");
                            add_sys8_path_table("/app_home/USRDIR", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", "/dev_bdvd");
                            add_sys8_path_table("/app_home", temp_buffer);
                        }
                   }

                }
                else
                {
                    // USB LIBFS (for game cacheds)
                    if(is_libfs_patched() && (!(directories[currentgamedir].flags & D_FLAG_HDD0) && game_cfg.bdemu_ext == 2) &&
                       !(directories[currentgamedir].flags & D_FLAG_BDVD))
                    {
                            // only with external bdemu LIBFS for cached files
                            sprintf(temp_buffer + 1024, "%s/libfs_patched.sprx", self_path);
                            add_sys8_path_table("/dev_flash/sys/external/libfs.sprx", temp_buffer + 1024);
                            sys8_pokeinstr(0x80000000007EF220ULL, 0x45737477616C6420ULL);
                    }

                    if(use_cache)
                    {
                        sprintf(temp_buffer + 1024, "%s/cache/%s/%s", self_path,  //check replace (1024)
                        directories[currentgamedir].title_id, "/paths.dir");

                        char *path = LoadFile(temp_buffer + 1024, &n);
                        char *mem = path;

                        if(path && path[0x400] == 0 && path[0x420] != 0)
                        {
                            // repair bug in some Iris Manager versions
                            memcpy(&path[0x400], &path[0x420], 0x380);
                            SaveFile(temp_buffer + 1024, path, n);
                        }

                        n = n & ~2047; // if file truncated break bad datas...

                        if(path)
                        {
                            while(n > 0)
                            {
                                char *t = path;

                                sprintf(temp_buffer + 1024, "%s/cache/%s/%s", self_path,
                                directories[currentgamedir].title_id, path + 0x400);

                                path = strstr(path, "PS3_GAME/");

                                sprintf(temp_buffer, "/dev_bdvd/%s", path);

                                add_sys8_path_table(temp_buffer, temp_buffer + 1024);

                                path = t + 2048;
                                n   -= 2048;
                            }
                            free(mem);
                        }
                    }
                    if(!(directories[currentgamedir].flags & D_FLAG_HOMEB) &&
                        (directories[currentgamedir].flags & (D_FLAG_PS3_ISO | D_FLAG_PSX_ISO)) == D_FLAG_PSX_ISO)
                    {   // add PSX iso
                        if(!lv2_patch_storage && (directories[currentgamedir].flags & BDVD_FLAG))
                        {
                            DrawDialogOKTimer("PSX Unsupported", 2000.0f);
                            return r;
                        }

                        //syscall36("/dev_bdvd");
                        add_sys8_bdvd(NULL, NULL);

                        if(!(directories[currentgamedir].flags & BDVD_FLAG))
                        {
                            if(lv2peek(0x80000000000004E8ULL) && !use_cobra) syscall_40(1, 0); // disables PS3 Disc-less

                            // load PSX options
                            LoadPSXOptions(directories[currentgamedir].path_name);

                            if(psx_iso_prepare(directories[currentgamedir].path_name, directories[currentgamedir].title, NULL) == 0)
                                return r;
                        }
                        else
                            psx_cd_with_cheats();

                        set_device_wakeup_mode(bdvd_is_usb ? 0xFFFFFFFF : directories[currentgamedir].flags);

                        psx_launch();
                    }
                    else if(directories[currentgamedir].flags & D_FLAG_HOMEB)
                    {   // is homebrew
                        reset_sys8_path_table();

                        if((directories[currentgamedir].flags & D_FLAG_HOMEB_MKV) == D_FLAG_HOMEB_BD)
                        {
                            // Is BDISO
                            launch_iso_game(directories[currentgamedir].path_name, EMU_BD);        // launch BD Video
                            return r;
                        }
                        else if(directories[currentgamedir].flags & D_FLAG_HOMEB_DVD)
                        {
                            // Is DVDISO or MKV
                            int p = strlen(directories[currentgamedir].path_name);
                            if(!strcmp(&directories[currentgamedir].path_name[p - 4], ".mkv") ||!strcmp(&directories[currentgamedir].path_name[p - 4], ".MKV"))
                            {
                                sprintf(temp_buffer, "%s", directories[currentgamedir].path_name); // launch MKV/MP4/AVI
                                sprintf(temp_buffer + 2048, "%s/iris_manager.biso", self_path);
                                launch_iso_build(temp_buffer + 2048, temp_buffer, 1);
                            }
                            else
                            {
                                launch_iso_game(directories[currentgamedir].path_name, EMU_DVD);   // launch DVD Video
                            }

                            return r;
                        }

                        add_sys8_bdvd(NULL, "/dev_kk");

                        sprintf(temp_buffer, "%s/PARAM.SFO", directories[currentgamedir].path_name);

                        if(homelaun == 2)
                        {
                            // test if same title id to skip homelaun pass
                            if(!parse_param_sfo_id("/dev_hdd0/game/HOMELAUN1/PARAMX.SFO", temp_buffer + 1024))
                                if(strcmp(directories[currentgamedir].title_id, temp_buffer + 1024)) {homelaun = 1;}
                        }

                        i = param_sfo_patch_category_to_cb(temp_buffer, "/dev_hdd0/game/HOMELAUN1/PARAMX.SFO");

                        switch(i)
                        {
                          case -1:
                            DrawDialogOK("Error: I cannot find PARAM.SFO for HOMELAUN1 (game broken)");
                            goto skip_homebrew;
                          case -2:
                            DrawDialogOK("Error: External USB Loader not found! (install HOMELAUN1)");
                            goto skip_homebrew;
                        }

                        sprintf(temp_buffer, "%s", directories[currentgamedir].path_name);

                        sprintf(temp_buffer + 1024, "%s/homelaunc1.bin", self_path);
                        sprintf(temp_buffer + 2048, "%s/homelaunc1.bin", self_path);
                        SaveFile(temp_buffer + 2048, temp_buffer, 2048);

                        i = strlen(directories[currentgamedir].path_name);
                        while(directories[currentgamedir].path_name[i] != '/') i--;

                        sprintf(temp_buffer + 1024, "/dev_hdd0/game%s", &directories[currentgamedir].path_name[i]);

                        if(homelaun == 2) add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/ICON0.PNG", "//dev_hdd0/game/HOMELAUN1/GICON0.PNG");
                        else add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/ICON0.PNG", "//dev_hdd0/game/HOMELAUN1/BICON0.PNG");

                        if(homelaun != 2)
                            add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/USRDIR/EBOOT.BIN", "//dev_hdd0/game/HOMELAUN1/USRDIR/EBOOT.BIN");

                        if(homelaun != 1) add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/PARAM.SFO", "//dev_hdd0/game/HOMELAUN1/PARAMX.SFO");
                        else add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/PARAM.SFO", "//dev_hdd0/game/HOMELAUN1/PARAM.SFO");

                        add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/path.bin", temp_buffer + 2048);
                        if(homelaun == 2) add_sys8_path_table("/dev_hdd0/game/HOMELAUN1/path2.bin", temp_buffer + 2048);

                        add_sys8_path_table("/dev_hdd0/game/HOMELAUN1", temp_buffer);

                        for(i = 0; i < 11; i++) if((directories[currentgamedir].flags>>i) & 1) break;

                        if(i != 0)
                            add_sys8_path_table(temp_buffer + 1024, temp_buffer);

                        build_sys8_path_table();
                        set_device_wakeup_mode(bdvd_is_usb ? 0xFFFFFFFF : directories[currentgamedir].flags);
                        game_cfg.direct_boot = 0;
                        exit(0);

                        //////////////
                        skip_homebrew: ;
                        //////////////
                    }
                    else
                    {
                        mount_custom(directories[currentgamedir].path_name);
                        if(!game_cfg.useBDVD)
                        {
                            sprintf(temp_buffer, "%s/PS3_DISC.SFB", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/PS3_DISC.SFB", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/PS3_GAME", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home/USRDIR", temp_buffer);

                            sprintf(temp_buffer, "%s/PS3_GAME/USRDIR", directories[currentgamedir].path_name);
                            add_sys8_path_table("/app_home", temp_buffer);
                        }

                        is_ps3game_running = 1;

                        //syscall36(directories[currentgamedir].path_name); // is bdvd game

                        add_sys8_bdvd(directories[currentgamedir].path_name, NULL);
                    }
                }

                if(noBDVD && !use_cobra)
                {
                    struct stat s;
                    patch_bdvdemu(NTFS_FLAG/*directories[currentgamedir].flags & GAMELIST_FILTER*/);

                    if(load_from_bluray)
                    {
                        if((firmware & 0xF) == 0xD)
                            sprintf(temp_buffer, "%s/explore_plugin_%xdex.sprx", self_path, firmware>>4);
                        else
                            sprintf(temp_buffer, "%s/explore_plugin_%x.sprx", self_path, firmware>>4);

                        if(stat(temp_buffer, &s) != SUCCESS)
                        {
                             if(noBDVD == MODE_DISCLESS) strcat(temp_buffer, " not found\n\nIt reduces the game compatibility");
                             else strcat(temp_buffer, " not found\n\npath from /app_home");
                             DrawDialogOKTimer(temp_buffer+strlen(self_path)+1, 2000.0f);
                        }
                        else
                            add_sys8_path_table("/dev_flash/vsh/module/explore_plugin.sprx", temp_buffer);
                    }

                }

                build_sys8_path_table();

                exit_program = 1;

                if(game_cfg.direct_boot) game_cfg.direct_boot = 555;

                set_device_wakeup_mode(bdvd_is_usb ? 0xFFFFFFFF : directories[currentgamedir].flags);

                skip_sys8:
                return r;
            }
        }
    }

    // don't remove this code
    if(mode_favourites < 0x10000) set_last_game();

    if(new_pad & BUTTON_START)
    {
        if(old_pad & BUTTON_L2)
        {
            gui_mode++;
            if(gui_mode > GUI_MODES)
            {
                gui_mode = 1;
                if(cover_mode) cover_mode = 0; else cover_mode = 1;
            }
            get_grid_dimensions();
            manager_cfg.cover_mode = cover_mode;
            manager_cfg.gui_mode = ((sort_mode & 0xf)<<4) | (gui_mode & 0xf);
            select_option = 0;
            menu_screen = 0;
            SaveManagerCfg();
            currentgamedir = currentdir = 0;
            select_px = select_py = 0;
            select_option = 0;
            menu_screen = 0;
            load_background_picture();
            get_games();
            load_gamecfg(-1); // force refresh game info
            new_pad = 0;
        }
        else if(old_pad & BUTTON_SELECT)
        {
            if (file_manager(NULL, NULL) == 1)
            {
                select_px = select_py = 0;
                select_option = 0;
                menu_screen = 0;

                ndirectories = 0;

                fdevices = 0;
                fdevices_old = 0;
                forcedevices = 0;
                find_device = 0;
                bdvd_notify = 1;
                currentdir = 0;

                select_option = 0;
                menu_screen = 0;
            }
        }
        else
        {
            // Show Global Options menu
            if(options_locked)
                DrawDialogOKTimer(language[DRAWSCREEN_PARCONTROL], 2000.0f);
            else
            {
                app_ver[0] = 0;
                select_option = -1;
                menu_screen = 3; return r;
            }
        }
    }

    if(new_pad & BUTTON_TRIANGLE)
    {
        if(options_locked)
            DrawDialogOKTimer(language[DRAWSCREEN_PARCONTROL], 2000.0f);
        else
        {
            if(old_pad & BUTTON_SELECT) {bShowVersion = 1; bShowPIC1 = 1;} // SELECT + /\ = force show version / pic1 on ISO

            i = selected;

            select_option = 0;

            if(!Png_offset[i] && mode_favourites && mode_favourites < 0x10000 && favourites.list[i].title_id[0] != 0)
            {
                DeleteFavouritesIfExits(favourites.list[i].title_id);

                sprintf(temp_buffer, "%s/config/", self_path);
                SaveFavourites(temp_buffer, mode_homebrew);

                if(mode_favourites && !havefavourites) mode_favourites = 0;
                get_games();
                return r;
            }

            if(Png_offset[i])
            {
                stops_BDVD = 1;
                Png_offset[BIG_PICT] = 0;
                wait_event_thread();

                int_currentdir = currentdir;
                int indx = (mode_favourites != 0) ? favourites.list[i].index : (currentdir + i);

                if((bShowPIC1 == 1) || (bShowPIC1 == 2 && ((directories[indx].flags & (NTFS_FLAG)) != NTFS_FLAG)))
                {
                    // program new event thread function to show background picture
                    event_thread_send(0x555ULL, (u64) get_pict, (u64) &i);
                }

                currentgamedir = get_currentdir(i);
                if(currentgamedir >= 0 && currentgamedir < ndirectories)
                {
                    bool is_retro = ((directories[indx].flags & (RETRO_FLAG)) == (RETRO_FLAG)) &&
                                    (strstr(directories[indx].path_name, retro_root_path) != NULL);

                    if(is_retro) goto ask_delete_item;

                    bool is_ps2_classic = ((directories[indx].flags & (PS2_CLASSIC_FLAG)) == (PS2_CLASSIC_FLAG)) &&
                                          (strstr(directories[indx].path_name, ps2classic_path) != NULL);

                    if(is_ps2_classic) goto ask_delete_item;


                    // access to PSX configuration menu
                    if(!mode_homebrew && (directories[indx].flags & (ISO_FLAGS)) == PS1_FLAG) // add PSX iso
                    {
                        if(!(directories[indx].flags & BDVD_FLAG))
                            LoadPSXOptions(directories[indx].path_name);
                        else
                            LoadPSXOptions(NULL);

                        if(get_psx_memcards() == 0)
                            menu_screen = 444;
                    }
                    else
                    {
                       if(is_retro || is_ps2_classic || (mode_homebrew && (directories[indx].flags & D_FLAG_HOMEB_GROUP) > D_FLAG_HOMEB))
                       {
ask_delete_item:
                           // SELECT + TRIANGLE or L2 + TRIANGLE = Delete movie
                           if(old_pad & (BUTTON_SELECT | BUTTON_L2) || bshowpath == 1 || gui_mode != MODE_COVERFLOW)
                           {
                                if(!strncmp(directories[indx].path_name, "/dev_hdd0", 9))
                                    sprintf(temp_buffer + 1024, "%s\n\n%s HDD0?", directories[indx].title, language[GAMEDELSL_WANTDELETE]);
                                else if(!strncmp(directories[indx].path_name, "/dev_usb", 8))
                                    sprintf(temp_buffer + 1024, "%s\n\n%s USB00%c?", directories[indx].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[10]);
                                else if(!strncmp(directories[indx].path_name, "/ntfs", 5))
                                    sprintf(temp_buffer + 1024, "%s\n\n%s NTFS%c?", directories[indx].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[5]);
                                else if(!strncmp(directories[indx].path_name, "/ext", 4))
                                    sprintf(temp_buffer + 1024, "%s\n\n%s EXT%c?", directories[indx].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[4]);
                                else
                                    return r;

                                if(DrawDialogYesNo(temp_buffer + 1024) == 1)
                                {
                                    pause_music(1);

                                    delps3iso(directories[indx].path_name);

                                    pause_music(0);

                                    fdevices = 0;
                                    fdevices_old = 0;
                                    forcedevices = 0;
                                    find_device = 0;
                                    bdvd_notify = 1;
                                    currentgamedir = currentdir = 0;
                                    Png_offset[BIG_PICT] = 0;
                                    select_option = 0;
                                    menu_screen = 0;
                                    select_px = select_py = 0;
                                    stops_BDVD = 1;
                                }

                                return r;
                           }
                           else
                               bshowpath = 1;

                           return r; // ignore Options menu
                       }

                       if(!mode_homebrew && (directories[indx].flags & (BDVD_FLAG | PS3_FLAG)) == PS3_FLAG)
                            menu_screen = 128;
                       else
                            menu_screen = 1;

                       load_gamecfg(-1); // force refresh game info
                       load_gamecfg (get_currentdir(i));

                       if(!(directories[indx].flags & BDVD_FLAG) && lv2peek(0x80000000000004E8ULL))
                            set_BdId(indx);
                    }
                    return r;
                }
            }
        }
    }

    static int auto_up = 0, auto_down = 0, auto_left = 0, auto_right = 0;

/************************************************************************************************************/
/* GUI 0: GRID                                                                                              */
/************************************************************************************************************/
    if(gui_mode != MODE_COVERFLOW) {

    AUTO_BUTTON_REP(auto_up, BUTTON_UP)
    AUTO_BUTTON_REP(auto_down, BUTTON_DOWN)
    AUTO_BUTTON_REP(auto_left, BUTTON_LEFT)
    AUTO_BUTTON_REP(auto_right, BUTTON_RIGHT)

    if(new_pad & BUTTON_UP)
    {
        select_py--;

        auto_up = 1;

        if(select_py < 0)
        {
            select_py = (rows - 1);

            if(mode_favourites >= 0x10000) ;
            else if(mode_favourites) {mode_favourites = 0; get_games();}
            else if(currentdir >= num_box) {mode_favourites = 0; currentdir -= num_box; get_games();}
            else {mode_favourites = (!mode_favourites && havefavourites); currentdir = ROUND_UPX(ndirectories) - num_box; get_games();}
        }

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_DOWN)
    {
        select_py++;

        auto_down = 1;

        if(select_py > (rows - 1))
        {
            select_py = 0;

            if(mode_favourites >= 0x10000) ;
            else if(mode_favourites) {mode_favourites = 0; get_games();}
            else if(currentdir < (ROUND_UPX(ndirectories) - num_box)) {mode_favourites = 0; currentdir += num_box; get_games();}
            else {mode_favourites = (!mode_favourites && havefavourites); currentdir = 0; get_games();}
        }

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_LEFT)
    {
        select_px--;

        if(select_px < 0 && select_py == 0 && currentdir == 0 && ndirectories < 4 && !mode_favourites && !havefavourites)
        {
            select_px = ndirectories - 1; if(select_px < 0) select_px = 0; currentdir = 0;
        }

        auto_left = 1;

        if(select_px < 0)
        {
            select_px = (cols - 1);

            if(mode_favourites >= 0x10000) ;
            else if(mode_favourites) {mode_favourites = 0; get_games();}
            else if(currentdir >= num_box) {mode_favourites = 0; currentdir -= num_box; get_games();}
            else {mode_favourites = (!mode_favourites && havefavourites); currentdir = ROUND_UPX(ndirectories) - num_box; get_games();}

        }

        GFX1_mode = 2; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_RIGHT)
    {
        select_px++;

        auto_right = 1;

        if(select_px > (cols - 1))
        {
            select_px = 0;

            if(mode_favourites >= 0x10000) ;
            else if(mode_favourites) {mode_favourites = 0; get_games();}
            else if(currentdir < (ROUND_UPX(ndirectories) - num_box)) {mode_favourites = 0; currentdir += num_box; get_games();}
            else {mode_favourites = (!mode_favourites && havefavourites); currentdir = 0; get_games();}
        }

        GFX1_mode = 1; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_L1) //change page
    {
        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites) {mode_favourites = 0; get_games();}
        else if(currentdir >= num_box) {mode_favourites = 0; currentdir -= num_box; get_games();}
        else {mode_favourites = (!mode_favourites && havefavourites); currentdir = ROUND_UPX(ndirectories) - num_box; get_games();}

        GFX1_mode = 2; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_R1) //change page
    {
        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites) {mode_favourites = 0; get_games();}
        else if(currentdir < (ROUND_UPX(ndirectories) - num_box)) {mode_favourites = 0; currentdir += num_box; get_games();}
        else {mode_favourites = (!mode_favourites && havefavourites); currentdir = 0; get_games();}

        GFX1_mode = 1; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
// end of GUI 0: GRID

    } else {

/************************************************************************************************************/
/* GUI 1: COVERFLOW                                                                                         */
/************************************************************************************************************/
    // AUTO_BUTTON_REP(auto_up, BUTTON_UP)
    //AUTO_BUTTON_REP(auto_down, BUTTON_DOWN)
    AUTO_BUTTON_REP3(auto_left, BUTTON_LEFT)
    AUTO_BUTTON_REP3(auto_right, BUTTON_RIGHT)

    num_box = 12;
    cols = 4;
    rows = 3;

    if(new_pad & BUTTON_UP)
    {
        auto_up = 1;
        anim_mode = 0; anim_step = 0;

        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites)
        {
            mode_favourites = 0;
            currentdir = 0;
            select_px = select_py = 0;
            get_games();
        }
        else
        {
            if(!havefavourites)
                new_pad = BUTTON_R3;
            else
            {
                mode_favourites = (!mode_favourites && havefavourites);
                currentdir = 0;
                select_px = select_py = 0;
                get_games();
            }
        }

        if(havefavourites)
        {
            enable_draw_background_pic1();
            return r;
        }
    }
    else if(new_pad & BUTTON_DOWN)
    {
        auto_down = 1;
        anim_mode = 0; anim_step = 0;

        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites)
        {
            mode_favourites = 0;
            currentdir = 0;
            select_px = select_py = 0;
            get_games();
        }
        else
        {
            sort_mode++; if(sort_mode > 2) sort_mode = 0;

            sort_entries2(directories, &ndirectories, sort_mode);
            UpdateFavourites(directories, ndirectories);
            load_gamecfg(-1); // force refresh game info
            manager_cfg.cover_mode = cover_mode;
            manager_cfg.gui_mode = ((sort_mode & 0xf)<<4) | (gui_mode & 0xf);
            SaveManagerCfg();

            currentdir = 0;
            select_px = select_py = 0;
            get_games();
        }

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_LEFT)
    {
        int tmp_currentdir = currentdir;
        int update = 0;

        select_px--;

        if(select_px < 0 && select_py == 0 && currentdir == 0 && ndirectories < 4 && !mode_favourites && !havefavourites)
        {
            select_px = ndirectories - 1;
            if(select_px < 0) select_px = 0;
            currentdir = 0;
        }

        auto_left = 1;

        if(select_px < 0)
        {
            select_px = 3;
            select_py--;

            if(select_py < 0)
            {
                select_py = 2;

                if(mode_favourites >= 0x10000) ;
                else if(mode_favourites)
                {
                    mode_favourites = 0; update = 1;
                    currentdir = ndirectories - 1;
                    if(currentdir < 0) currentdir = 0;
                    if(currentdir >= 3) {select_px = 3; currentdir-= 2;} else select_px = currentdir;
                }
                else
                    select_py = 5;
            }
        }

        if(!mode_favourites)
        {
            if(currentdir > 0 || (currentdir == 0 && select_py != 5))
            {
                if(currentdir > 0) {currentdir--; if(!update) update = 2; if(currentdir <= 3 && select_px == 2) select_px = 3;}
                if(currentdir >= 3) select_px = 3; select_py = 0;
            }
            else
            {
                mode_favourites = (!mode_favourites && havefavourites);

                if(mode_favourites) {currentdir = (ndirectories/12) * 12; select_px = 3; select_py = 2;}
                else
                {
                    currentdir = ndirectories - 1;
                    if(currentdir < 0) currentdir = 0;
                    if(currentdir >= 3)
                    {
                        select_px = 3;
                        currentdir-= 3;
                    }
                    else
                        select_px = currentdir;

                    select_py = 0;
                }

                update = 1;
            }
        }


        if(currentdir < 0 || currentdir >= ndirectories)  {currentdir = 0; select_px = 0; select_py = 0; update = 1;}

        if(update)
        {
            if(update == 2 && currentdir == (tmp_currentdir - 1))
            {
                wait_event_thread(); // wait previous event thread function

                int_currentdir = currentdir;

                // program new event thread function
                event_thread_send(0x555ULL, (u64) get_games_3, 0);
            }
            else
                get_games();
        }

        anim_mode = 1; anim_step = 0;
        GFX1_mode = 2; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_RIGHT)
    {
        int tmp_currentdir = currentdir;
        int update = 0;
        select_px++;

        auto_right = 1;

        if(!mode_favourites && (currentdir + select_px) >= ndirectories) {currentdir = ndirectories; select_px = 4; select_py = 2;}

        if(select_px > 3)
        {
            select_px = 0;

            select_py++;

            if(select_py > 2)
            {
                select_py = 0;

                if(mode_favourites >= 0x10000) ;
                else if(mode_favourites)
                {
                    mode_favourites = 0;
                    if(!update) update = 1;
                    currentdir = 0;
                }
                else
                    select_py = 1;
            }
        }

        //

        if(!mode_favourites)
        {
            if(select_px <= 3 && select_py == 0 && currentdir <= 3) ;
            else if(currentdir < ndirectories)
            {
                currentdir++; update = 2;
                select_px = 3; select_py = 0;
            }
            else
            {
                mode_favourites = (!mode_favourites && havefavourites);
                currentdir = 0;
                select_px = 0; select_py = 0;
                update = 1;
            }
        }


        if(currentdir < 0 || currentdir >= ndirectories)  currentdir = 0;

        if(update)
        {
            if(update == 2 && currentdir == (tmp_currentdir + 1))
            {
                wait_event_thread(); // wait previous event thread function

                int_currentdir = currentdir;

                // program new event thread function
                event_thread_send(0x555ULL, (u64) get_games_3, 1ULL);
            }
            else
                get_games();
        }

        anim_mode = 2; anim_step = 0;
        GFX1_mode = 1; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_L1) //change page
    {
        anim_mode = 0; anim_step = 0;

        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites)
        {
            mode_favourites = 0;
            select_px = select_py = 0;
            get_games();
        }
        else if(currentdir >= 12)
        {
            mode_favourites = 0;
            currentdir -= 12;
            if(currentdir < 0) currentdir = 0;
            if(currentdir >= 3)
            {
                select_px = 3;
                currentdir-= 3;
            }
            else
                select_px = currentdir;

            select_py = 0;
            select_py = 0;
            get_games();
        }
        else
        {
            mode_favourites = (!mode_favourites && havefavourites);
            currentdir = ndirectories - 12;
            if(currentdir < 0) currentdir = 0;
            if(currentdir >= 3)
            {
                select_px = 3;
                currentdir-= 3;
            }
            else
                select_px = currentdir;
            select_py = 0;
            get_games();
        }

        GFX1_mode = 2; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
    else if(new_pad & BUTTON_R1) //change page
    {
        //maybe wait some seconds here...
        anim_mode = 0; anim_step = 0;

        if(mode_favourites >= 0x10000) ;
        else if(mode_favourites)
        {
            if(currentdir >= ndirectories)
            {
                currentdir = 0;
                select_px = select_py = 0;
            }

            mode_favourites = 0;
            get_games();
        }
        else if(currentdir + 12 < ndirectories)
        {
            mode_favourites = 0;
            currentdir += 12;
            select_px = select_py = 0;
            if(currentdir >= 3)
            {
                select_px = 3;
                currentdir-= 3;
            }
            else
                select_px = currentdir;

            get_games();
        }
        else
        {
            mode_favourites = (!mode_favourites && havefavourites);
            currentdir = 0;
            get_games();
        }

        GFX1_mode = 1; GFX1_counter = 20;

        enable_draw_background_pic1();
        return r;
    }
// end of GUI 1
    }

    if(new_pad & BUTTON_R3) //change games/homebrew
    {
        // refresh custom settings
        read_settings();

        select_px = select_py = 0;
        select_option = 0;
        menu_screen = 0;

        anim_mode = 0; anim_step = 0;

        ndirectories = 0;

        fdevices = 0;
        fdevices_old = 0;
        forcedevices = 0;
        find_device = 0;
        bdvd_notify = 1;
        currentdir = 0;

        if(old_pad & BUTTON_SELECT)
        {
            mode_homebrew = 0;
            game_list_category = 0;
            mode_favourites = 0;
        }
        else if(game_list_category == 0 && !mode_homebrew)
        {
            SetFavourites(mode_homebrew);
            mode_homebrew = HOMEBREW_MODE;
            GetFavourites(mode_homebrew);
            mode_favourites = 1;
        }
        else if(mode_homebrew)
        {
            game_list_category = 1;
            SetFavourites(mode_homebrew);

            if(mode_homebrew == HOMEBREW_MODE) mode_homebrew = HOMEBREW_MODE + 1;
            else if(mode_homebrew > HOMEBREW_MODE) mode_homebrew = GAMEBASE_MODE;

            GetFavourites(mode_homebrew);
            mode_favourites = 0;
        }
        else
        {
            mode_favourites = 0;
            game_list_category++; if(game_list_category > 2) {game_list_category = 0; mode_favourites = 1;}
        }
    }
    else if(new_pad & BUTTON_L3) //change games/homebrew
    {
        // refresh custom settings
        read_settings();

        select_px = select_py = 0;
        select_option = 0;
        menu_screen = 0;

        ndirectories = 0;

        anim_mode = 0; anim_step = 0;

        fdevices = 0;
        fdevices_old = 0;
        forcedevices = 0;
        find_device = 0;
        bdvd_notify = 1;
        currentdir = 0;

        if(old_pad & BUTTON_SELECT)
        {
            mode_homebrew = 0;
            game_list_category = 0;
            mode_favourites = 0;
        }
        else if(game_list_category == 1 && !mode_homebrew)
        {
            SetFavourites(mode_homebrew);
            mode_homebrew = HOMEBREW_MODE + 1;
            GetFavourites(mode_homebrew);
            mode_favourites = 1;
        }
        else if(mode_homebrew)
        {
            SetFavourites(mode_homebrew);
            if(mode_homebrew == HOMEBREW_MODE) mode_homebrew = GAMEBASE_MODE;
            else if(mode_homebrew > HOMEBREW_MODE) mode_homebrew = HOMEBREW_MODE;
            GetFavourites(mode_homebrew);
            game_list_category = 0;
            mode_favourites = 1;
        }
        else
        {
            mode_favourites = 0;
            game_list_category--;
            if(game_list_category < 0) {game_list_category = 2;}
        }
    }


    return r;
}

void enable_draw_background_pic1()
{
    frame_count = 32;
    allow_restore_last_game = 0;
    bLoadPIC1 = 1;
}

void draw_background_pic1()
{
    int selected = select_px + select_py * cols;

    if((bLoadPIC1 == 1 && bShowPIC1 > 0 && bk_picture == 2 && frame_count > 55) && Png_offset[selected])
    {
        int i = selected;
        stops_BDVD = 1;
        bLoadPIC1 = 0;

        Png_offset[BIG_PICT] = 0;
        wait_event_thread();

        int_currentdir = currentdir;
        int indx = (mode_favourites != 0) ? favourites.list[i].index : (currentdir + i);

        if(bSkipPIC1)
        {
            // ignore PS1/PS2/RETRO
            if((directories[indx].flags & (RETRO_FLAG | PS2_CLASSIC_FLAG)) == (RETRO_FLAG | PS2_CLASSIC_FLAG))
            {
                bool is_retro = (strstr(directories[indx].path_name, retro_root_path) != NULL);
                bool is_ps2_classic = !is_retro &&
                                      (strstr(directories[indx].path_name, ps2classic_path) != NULL);

                if(is_retro || is_ps2_classic) return;
            }
            else if((directories[indx].flags & (PS1_FLAG)) == (PS1_FLAG)) return;
        }


        if((bShowPIC1 == 1) || (bShowPIC1 == 2 && ((directories[indx].flags & (NTFS_FLAG)) != NTFS_FLAG)))
        {
            // program new event thread function to show background picture
            event_thread_send(0x555ULL, (u64) get_pict, (u64) &i);
        }
    }
}

static int list_box_devices[16];
static int max_list_box_devices = 0;

void mount_iso_game()
{
    wait_event_thread(); // wait previous event thread function

    if(directories[currentgamedir].flags & (PS1_FLAG)) goto mount_game;

    u16 cur_firm = ((firmware>>12) & 0xF) * 10000 + ((firmware>>8) & 0xF) * 1000 + ((firmware>>4) & 0xF) * 100;

    char str_version[8];
    sprintf(str_version, "%2.2u.%4.4u", cur_firm / 10000, cur_firm % 10000 );

    char version[8];

    int is_ntfs_dev = NTFS_DEVICE_UNMOUNT;

    if(directories[currentgamedir].flags && NTFS_FLAG)
    {
        is_ntfs_dev = NTFS_Test_Device(directories[currentgamedir].path_name + 1);
    }

    if(strncmp(directories[currentgamedir].path_name, "/ntfs", 5) && strncmp(directories[currentgamedir].path_name, "/ext", 4))
        sysLv2FsChmod(directories[currentgamedir].path_name, 0170000 | 0777);

    bool is_iso = strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".iso")   == 0 ||
                  strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".ISO")   == 0 ||
                  strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".iso.0") == 0 ||
                  strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".ISO.0") == 0;

    if(is_ntfs_dev >= 0 || is_iso)
    {
        int fd = ps3ntfs_open(directories[currentgamedir].path_name, O_RDONLY, 0);
        if(fd >= 0)
        {
            u32 flba;
            u64 size;
            int re;
            char *mem = NULL;

            re = get_iso_file_pos(fd, "/PS3_GAME/PARAM.SFO;1", &flba, &size);

            if(!re && (mem = malloc(size + 16)) != NULL)
            {
                memset(mem, 0, size + 16);
                re = ps3ntfs_read(fd, (void *) mem, size);
                ps3ntfs_close(fd);

                if(re == size)
                {
                    if(mem_parse_param_sfo((u8 *) mem, size, "PS3_SYSTEM_VER", version) == SUCCESS) version[5] = 0;
                    if(mem) free(mem);

                    if(strcmp(version, str_version) > 0) if(patchps3iso(directories[currentgamedir].path_name, -1)) return;
                }
                else
                    if(mem) free(mem);
            }
            else
            {
                ps3ntfs_close(fd);
            }
        }
    }

mount_game:
    launch_iso_game(directories[currentgamedir].path_name, -1);

    fun_exit();
    exit(0);
}

void draw_app_version(float x, float y)
{
    wait_event_thread(); // wait previous event thread function

    if(app_ver[0])
    {
        DrawFormatString(848 - x - strlen(app_ver) * 16 - 190, y, app_ver);
        return;
    }

    if(directories[currentgamedir].flags & (PS1_FLAG)) return;

    if((bShowVersion == 0) || (bShowVersion == 2 && ((directories[currentgamedir].flags & (NTFS_FLAG)) == NTFS_FLAG))) return;

    sprintf(temp_buffer, "/dev_hdd0/game/%c%c%c%c%s/PARAM.SFO", directories[currentgamedir].title_id[0], directories[currentgamedir].title_id[1],
                                                                directories[currentgamedir].title_id[2], directories[currentgamedir].title_id[3],
                                                                &directories[currentgamedir].title_id[5]);

    if(parse_param_sfo_appver(temp_buffer, app_ver))
    {
        int is_ntfs_dev = NTFS_DEVICE_UNMOUNT;

        if(directories[currentgamedir].flags && NTFS_FLAG)
            is_ntfs_dev = NTFS_Test_Device(directories[currentgamedir].path_name + 1);

        if(strncmp(directories[currentgamedir].path_name, "/ntfs", 5) && strncmp(directories[currentgamedir].path_name, "/ext", 4))
            sysLv2FsChmod(directories[currentgamedir].path_name, 0170000 | 0777);

        bool is_iso = strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".iso")   == 0 ||
                      strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".ISO")   == 0 ||
                      strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".iso.0") == 0 ||
                      strcmp(directories[currentgamedir].path_name + strlen(directories[currentgamedir].path_name) - 4, ".ISO.0") == 0;

        if(is_ntfs_dev >= 0 || is_iso)
        {
            int fd = ps3ntfs_open(directories[currentgamedir].path_name, O_RDONLY, 0);
            if(fd >= 0)
            {
                u32 flba;
                u64 size;
                int re;
                char *mem = NULL;

                if((directories[currentgamedir].flags & (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG)) == (PSP_FLAG | RETRO_FLAG | PS2_CLASSIC_FLAG))
                {
                    bool is_retro = (strstr(directories[currentgamedir].path_name, retro_root_path) != NULL);
                    bool is_ps2_classic = !is_retro &&
                                          (strstr(directories[currentgamedir].path_name, ps2classic_path) != NULL);

                    if(is_retro || is_ps2_classic)
                        re = FAILED;
                    else
                        re = get_iso_file_pos(fd, "/PSP_GAME/PARAM.SFO", &flba, &size);
                }
                else
                    re = get_iso_file_pos(fd, "/PS3_GAME/PARAM.SFO;1", &flba, &size);

                if(!re && (mem = malloc(size + 16)) != NULL)
                {
                    memset(mem, 0, size + 16);
                    re = ps3ntfs_read(fd, (void *) mem, size);
                    ps3ntfs_close(fd);

                    if(re == size)
                    {
                        if(mem_parse_param_sfo((u8 *) mem, size, "APP_VER", app_ver) == SUCCESS) app_ver[5] = 0;
                    }
                }
                else
                    ps3ntfs_close(fd);

                if(mem) free(mem);
            }
            else
                return;
        }
        else
        {
            sprintf(temp_buffer, "%s/PS3_GAME/PARAM.SFO", directories[currentgamedir].path_name);
            if(parse_param_sfo_appver(temp_buffer, app_ver)) return;
        }
    }

    DrawFormatString(848 - x - strlen(app_ver) * 16 - 190, y, app_ver);
}

void draw_device_mkiso(float x, float y, int index)
{
    int i, n;


    int selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMOPT_MKISO]);

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);
    temp_buffer[64] = 0;

    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    draw_app_version(x, y);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    SetCurrentFont(FONT_TTF);

    SetFontAutoCenter(1);
    DrawFormatString(0, y, "%s", language[GLUTIL_HOLDTRIANGLEAB]);
    SetFontAutoCenter(0);

    for(n = 0; n < max_list_box_devices; n++)
    {
        if(list_box_devices[n] & 128)
        {
            sprintf(temp_buffer, "/ntfs%i:", list_box_devices[n] & 127);
        }
        else if(list_box_devices[n] & 64)
        {
            sprintf(temp_buffer, "/dev_usb00%i", list_box_devices[n] & 63);
        }
        else
            sprintf(temp_buffer,"/dev_hdd0");

        if(n < 8)
            DrawButton1_UTF8(x + 32, y + 32 + 48 * n, 320, temp_buffer, (flash && select_option == n));
        else
            DrawButton1_UTF8(x + 32 + 320 + 16, y + 32 + 48 * (n - 8), 320, temp_buffer, (flash && select_option == n));
    }



    ////////////

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    i = selected;

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
        {
            strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
            update_title_utf8 = 1;
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 , 0, 768, 32, str_color);
    }

    tiny3d_Flip();
    ps3pad_read();

    if(new_pad & BUTTON_CROSS)
    {
        //select_option

        if(list_box_devices[select_option] & 128)
            sprintf(temp_buffer, "/ntfs%i:/PS3ISO", list_box_devices[select_option] & 127);
        else if(list_box_devices[select_option] & 64)
            sprintf(temp_buffer, "/dev_usb00%i/PS3ISO", list_box_devices[select_option] & 63);
        else
            sprintf(temp_buffer,"/dev_hdd0/PS3ISO");

        pause_music(1);

        ps3ntfs_mkdir(temp_buffer, 0777);

        if(makeps3iso(directories[currentgamedir].path_name, temp_buffer, (list_box_devices[select_option] & 64) != 0 ? 1 : 2) != -1)
        {
            pause_music(0);

            select_px = select_py = 0;
            select_option = 0;
            menu_screen = 0;

            ndirectories = 0;

            fdevices = 0;
            fdevices_old = 0;
            forcedevices = 0;
            find_device = 0;
            bdvd_notify = 1;
            currentgamedir = currentdir = 0;
            select_px = select_py = 0;
            Png_offset[BIG_PICT] = 0;
        }
        return;
    }
    else if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        menu_screen = 1; select_option = 5; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, max_list_box_devices - 1)
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, max_list_box_devices - 1, 0);
    }

}

void draw_device_xtiso(float x, float y, int index)
{
    int i, n;


    int selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMOPT_XTISO]);

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);
    temp_buffer[64] = 0;

    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    draw_app_version(x, y);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    SetCurrentFont(FONT_TTF);

    SetFontAutoCenter(1);
    DrawFormatString(0, y, "%s", language[GLUTIL_HOLDTRIANGLEAB]);
    SetFontAutoCenter(0);

    for(n = 0; n < max_list_box_devices; n++)
    {
        if(list_box_devices[n] & 128)
            sprintf(temp_buffer, "/ntfs%i:", list_box_devices[n] & 127);
        else if(list_box_devices[n] & 64)
            sprintf(temp_buffer, "/dev_usb00%i", list_box_devices[n] & 63);
        else
            sprintf(temp_buffer,"/dev_hdd0");

        if(n < 8)
            DrawButton1_UTF8(x + 32, y + 32 + 48 * n, 320, temp_buffer, (flash && select_option == n));
        else
            DrawButton1_UTF8(x + 32 + 320 + 16, y + 32 + 48 * (n - 8), 320, temp_buffer, (flash && select_option == n));
    }

    ////////////

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    i = selected;

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
        {
            strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
            update_title_utf8 = 1;
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 , 0, 768, 32, str_color);
    }

    tiny3d_Flip();
    ps3pad_read();

    if(new_pad & BUTTON_CROSS)
    {
        //select_option

        if(list_box_devices[select_option] & 128)
            sprintf(temp_buffer, "/ntfs%i:/_GAMES", list_box_devices[select_option] & 127);
        else if(list_box_devices[select_option] & 64)
            sprintf(temp_buffer, "/dev_usb00%i/GAMES", list_box_devices[select_option] & 63);
        else
        {
            if (!memcmp(hdd_folder,"dev_hdd0",9))
                sprintf(temp_buffer, "/%s/" __MKDEF_GAMES_DIR,hdd_folder);
            else if (!memcmp(hdd_folder, "GAMES", 6) || !memcmp(hdd_folder, "dev_hdd0_2", 11))
                sprintf(temp_buffer, "/dev_hdd0/GAMES");
            else if (!memcmp(hdd_folder,"host_root", 10))
                sprintf(temp_buffer, "/host_root");
            else if (!memcmp(hdd_folder,"video", 6))
                sprintf(temp_buffer, "/dev_hdd0/video");
            else if (strstr(hdd_folder, "/"))
                sprintf(temp_buffer, hdd_folder);
            else
                sprintf(temp_buffer, "/dev_hdd0/game/%s/" __MKDEF_GAMES_DIR, hdd_folder);
        }

        pause_music(1);

        ps3ntfs_mkdir(temp_buffer, 0777);

        extractps3iso(directories[currentgamedir].path_name, temp_buffer, (list_box_devices[select_option] & 64) != 0 ? 1 : 0);

        pause_music(0);

        select_px = select_py = 0;
        select_option = 0;
        menu_screen = 0;

        ndirectories = 0;

        fdevices = 0;
        fdevices_old = 0;
        forcedevices = 0;
        find_device = 0;
        bdvd_notify = 1;
        currentgamedir = currentdir = 0;
        select_px = select_py = 0;
        Png_offset[BIG_PICT] = 0;

        return;
    }
    else if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        menu_screen = 128; select_option = 3; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, max_list_box_devices - 1)
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, max_list_box_devices - 1, 0);
    }

}

void draw_device_cpyiso(float x, float y, int index)
{
    int i, n;

    char temp_buffer2[4096];


    int selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMOPT_CPYISO]);

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);
    temp_buffer[64] = 0;

    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    draw_app_version(x, y);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    SetCurrentFont(FONT_TTF);

    SetFontAutoCenter(1);
    DrawFormatString(0, y, "%s", language[GLUTIL_HOLDTRIANGLEAB]);
    SetFontAutoCenter(0);

    for(n = 0; n < max_list_box_devices; n++)
    {
        if(list_box_devices[n] & 128)
            sprintf(temp_buffer, "/ntfs%i:", list_box_devices[n] & 127);
        else if(list_box_devices[n] & 64)
            sprintf(temp_buffer, "/dev_usb00%i", list_box_devices[n] & 63);
        else
            sprintf(temp_buffer,"/dev_hdd0");

        if(n < 8)
            DrawButton1_UTF8(x + 32, y + 32 + 48 * n, 320, temp_buffer, (flash && select_option == n));
        else
            DrawButton1_UTF8(x + 32 + 320 + 16, y + 32 + 48 * (n - 8), 320, temp_buffer, (flash && select_option == n));
    }



    ////////////

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    i = selected;

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
                update_title_utf8 = 1;
            }
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 , 0, 768, 32, str_color);
    }

    tiny3d_Flip();
    ps3pad_read();

    if(new_pad & BUTTON_CROSS)
    {
        //select_option
        if(list_box_devices[select_option] & 128)
        {
            if((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG))
                sprintf(temp_buffer2, "/ntfs%i:/PS2ISO", list_box_devices[select_option] & 127);
            else
                sprintf(temp_buffer2, "/ntfs%i:/PS3ISO", list_box_devices[select_option] & 127);
        }
        else if(list_box_devices[select_option] & 64)
        {
            if((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG))
                sprintf(temp_buffer2, "/dev_usb00%i/PS2ISO", list_box_devices[select_option] & 63);
            else
                sprintf(temp_buffer2, "/dev_usb00%i/PS3ISO", list_box_devices[select_option] & 63);
        }
        else
        {
            if((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG))
                sprintf(temp_buffer2, "/dev_hdd0/PS2ISO");
            else
                sprintf(temp_buffer2, "/dev_hdd0/PS3ISO");
        }

        ps3ntfs_mkdir(temp_buffer2, 0777);

        u64 avail = get_disk_free_space(temp_buffer2);

        strcpy(temp_buffer2 + 1024, directories[currentgamedir].path_name);
        char * filename = strrchr(temp_buffer2 + 1024, '/');
        if(filename) *filename++ = 0; // break the string

        if(filename && strncmp(temp_buffer2 + 1024, temp_buffer2, strlen(temp_buffer2)))
        {
            copy_archive_file(temp_buffer2 + 1024, temp_buffer2, filename, avail);

            select_px = select_py = 0;
            select_option = 0;
            menu_screen = 0;

            ndirectories = 0;

            fdevices = 0;
            fdevices_old = 0;
            forcedevices = 0;
            find_device = 0;
            bdvd_notify = 1;
            currentgamedir = currentdir = 0;
            select_px = select_py = 0;
            Png_offset[BIG_PICT] = 0;
        }
        else
        {
            DrawDialogOKTimer("Error:! Cannot copy in the same device\n\nError!: No se puede copiar en el mismo dispositivo", 2000.0f);
            menu_screen = 128; select_option = 3; return;
        }

        return;
    }
    else if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        menu_screen = 128;
        select_option = 3;
        return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, max_list_box_devices - 1)
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, max_list_box_devices - 1, 0);
    }

}

void draw_options(float x, float y, int index)
{

    if(!strncmp(directories[currentgamedir].title_id, "HTSS00003", 9) || !strncmp(directories[currentgamedir].title_id, "IRISMAN00", 9))
    {
        sprintf(temp_buffer, "%s\n\nDo you want to remove Showtime and Internet Browser icons from the Game List?", directories[currentgamedir].title);
        if(DrawDialogYesNo(temp_buffer) == 1)
        {
            show_custom_icons = 0;
            manager_cfg.show_custom_icons = show_custom_icons;
            forcedevices = 1;
            SaveManagerCfg();
        }

        Png_offset[num_box] = 0;
        Png_offset[BIG_PICT] = 0;
        select_option = 0;
        menu_screen = 0;
        return;
    }

    int i, n;

    float y2;

    int copy_flag = 1;

    int selected;

    selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMOPT_OPTS]);

    if(directories[currentgamedir].flags & D_FLAG_HDD0)
    {
        copy_flag = 0;

        for(n = 1; n < 11; n++)
        {
            if((fdevices >> n) & 1) copy_flag = 1;
        }

    }
    else if(directories[currentgamedir].flags & D_FLAG_USB)
    {
        copy_flag = 0;

        if((fdevices >> 0) & 1) copy_flag = 1;
    }

    if(directories[currentgamedir].title_id[0] == 0 && select_option == 0)
    {
        select_option = 1;

        if(!copy_flag && select_option == 1) select_option++;

        if((directories[currentgamedir].flags & D_FLAG_BDVD) && (select_option == 2 || select_option == 6)) select_option++;
    }

    if(mode_homebrew >= HOMEBREW_MODE)
    {
        if(select_option < 2) select_option= 2;
        if(select_option == 5) select_option= 6;
        if(select_option == 7) select_option= 8;
    }

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);
    temp_buffer[64] = 0;

    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    draw_app_version(x, y);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    y2 = y - 4;

    if((directories[currentgamedir].flags & (BDVD_FLAG | PS1_FLAG)) == BDVD_FLAG){
        n = sys_ss_media_id(BdId);
        if(n == 0 || n== 0x80010006)
        {
            SetFontSize(10, 16);
            SetFontColor(0x00afffff, 0x00000080);
            DrawFormatString(x + 32, y2, "BD ID: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                BdId[0], BdId[1], BdId[2], BdId[3], BdId[4], BdId[5], BdId[6], BdId[7],
                BdId[8], BdId[9], BdId[10], BdId[11], BdId[12], BdId[13], BdId[14], BdId[15]);
        }
    }
    else if((BdId[0] | BdId[1] | BdId[2] | BdId[3] | BdId[4] | BdId[5] | BdId[6] | BdId[7] |
             BdId[8] | BdId[9] | BdId[10] | BdId[11] | BdId[12] | BdId[13] | BdId[14] | BdId[15]))
    {
        SetFontSize(10, 16);
        SetFontColor(0xffaf00ff, 0x00000080);
        DrawFormatString(x + 32, y2, "BD ID: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
            BdId[0], BdId[1], BdId[2], BdId[3], BdId[4], BdId[5], BdId[6], BdId[7],
            BdId[8], BdId[9], BdId[10], BdId[11], BdId[12], BdId[13], BdId[14], BdId[15]);

    }

    SetFontSize(12, 16);

    SetFontColor(0xffffffff, 0x00000000);

    y2 = y + 12;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_CFGGAME], (directories[currentgamedir].title_id[0] == 0 || mode_homebrew >= HOMEBREW_MODE) ? -1 : (flash && select_option == 0));

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_CPYGAME], (copy_flag != 0  && mode_homebrew == GAMEBASE_MODE) ? (flash && select_option == 1) : -1);

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_DELGAME], (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : ((flash && select_option == 2) ? 1 : 0));

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_FIXGAME], (flash && select_option == 3));

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_TSTGAME], (flash && select_option == 4));

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_BUILDISO], (directories[currentgamedir].title_id[0] == 0 || mode_homebrew >= HOMEBREW_MODE) ? -1 : (flash && select_option == 5));

    y2+= 48;

    if(!TestFavouritesExits(directories[currentgamedir].title_id))
        DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_CPYTOFAV], (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : (flash && select_option == 6));
    else
        DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_DELFMFAV], (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : (flash && select_option == 6));

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_GAMEUPDATE], (directories[currentgamedir].title_id[0] == 0 ||
                                                                       mode_homebrew >= HOMEBREW_MODE) ? -1 : (flash && select_option == 7));

    y2+= 48;


/*
    DrawButton1_UTF8(x + 32, y2, 320, language[GLOBAL_RETURN], (flash && select_option == 8));

    y2+= 48;
*/
    /*
    for(n = 0; n < 1; n++)
    {
        DrawButton1_UTF8(x + 32, y2, 320, "", -1);

        y2+= 48;
    }
    */

//
    SetFontSize(8, 10);

    utf8_to_ansi(directories[currentgamedir].path_name, temp_buffer, 128);
    temp_buffer[128] = 0;

    DrawFormatString(x + 8, y + 3 * 150 - 6, "%s", temp_buffer);
//

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150 + 7, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    i = selected;

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
                update_title_utf8 = 1;
            }
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 + 8, 0, 768, 32, str_color);

    }

    tiny3d_Flip();
    ps3pad_read();

    if((old_pad & BUTTON_SELECT) && (new_pad & BUTTON_TRIANGLE))
    {
        if (Png_offset[BIG_PICT])
        {
            Png_offset[BACKGROUND_PICT] = Png_offset[BIG_PICT];
            Png_datas[BACKGROUND_PICT] = Png_datas[BIG_PICT];
        }
    }

    if(new_pad & BUTTON_CROSS)
    {
        switch(select_option)
        {
            case 0: // Config. Game
                if(mode_homebrew >= HOMEBREW_MODE) break;
                select_option = 0;
                menu_screen = 2;

                // load game config
                sprintf(temp_buffer, "%s/config/%s.cfg", self_path, directories[currentgamedir].title_id);
                memset(&game_cfg, 0, sizeof(game_cfg));

                int file_size;
                char *file = LoadFile(temp_buffer, &file_size);
                if(file)
                {
                    if(file_size > sizeof(game_cfg)) file_size = sizeof(game_cfg);
                    memcpy(&game_cfg, file, file_size);
                    free(file);
                }
                return;

            case 1: // Copy Game
                if(mode_homebrew >= HOMEBREW_MODE) break;
                if(test_ftp_working()) break;
                 i = selected;

                 if(Png_offset[i])
                 {
                    pause_music(1);

                    copy_from_selection(currentgamedir);

                    pause_music(0);

                    currentgamedir = currentdir = 0;
                    select_px = select_py = 0;
                    select_option = 0;
                    menu_screen = 0;
                    stops_BDVD = 1;
                 }
                 return;

            case 2: // Delete Game
                 i = selected;
                 if(test_ftp_working()) break;

                 if(Png_offset[i])
                 {
                    pause_music(1);

                    delete_game(currentgamedir);

                    pause_music(0);

                    currentgamedir = currentdir = 0;
                    select_px = select_py = 0;
                    select_option = 0;
                    menu_screen = 0;
                    stops_BDVD = 1;
                 }
                 return;
            case 3: // Fix File Permissions
                 i = selected;

                 if(Png_offset[i])
                 {
                    pause_music(1);

                    // sys8_perm_mode(1);
                    FixDirectory(directories[currentgamedir].path_name, 0);
                    // sys8_perm_mode(0);

                    msgDialogAbort();
                    msgDialogClose(0);

                    pause_music(0);

                    DrawDialogOK(language[DRAWGMOPT_FIXCOMPLETE]);
                    stops_BDVD = 1;
                 }
                 break;
            case 4: // Test Game
                 i = selected;

                 if(Png_offset[i])
                 {
                    pause_music(1);

                    test_game(currentgamedir);

                    pause_music(0);
                    stops_BDVD = 1;
                 }
                 break;
            case 5: // Copy EBOOT.BIN from USB
                {
                if(mode_homebrew >= HOMEBREW_MODE) break;

/*
                // load game config
                sprintf(temp_buffer, "/dev_usb/ps3game/%s.BIN", directories[currentgamedir].title_id);

                int file_size;
                char *file = LoadFile(temp_buffer, &file_size);
                if(file)
                {
                    sprintf(temp_buffer, "%s/self/%s.BIN", self_path, directories[currentgamedir].title_id);
                    if(SaveFile(temp_buffer, file, file_size) == 0)
                    {
                        sprintf(temp_buffer, "%s/self/%s.BIN\n\nEBOOT.BIN %s", self_path, directories[currentgamedir].title_id, language[DRAWGMOPT_CPYOK]);
                        DrawDialogOK(temp_buffer);
                    } else {
                        sprintf(temp_buffer, "%s/self/%s.BIN\n\n%s EBOOT.BIN", self_path, directories[currentgamedir].title_id, language[DRAWGMOPT_CPYERR]);
                        DrawDialogOK(temp_buffer);
                    }
                    free(file);
                } else {
                    sprintf(temp_buffer, "/dev_usb/ps3game/%s.BIN\n\nEBOOT.BIN %s", directories[currentgamedir].title_id, language[DRAWGMOPT_CPYNOTFND]);
                    DrawDialogOK(temp_buffer);
                }
*/
                    i = selected;

                    if(Png_offset[i])
                    {
                        // get device list
                        list_box_devices[0] = 0;
                        max_list_box_devices = 1;

                        int i, k;
                        for(i = 0; i < 8 ; i++)
                        {
                            if(mounts[i])
                            {
                                for (k = 0; k < mountCount[i]; k++)
                                {
                                    if(max_list_box_devices < 16 && !strncmp((mounts[i]+k)->name, "ntfs", 4))
                                        {list_box_devices[max_list_box_devices]= 128 + ((mounts[i]+k)->name[4] - 48); max_list_box_devices++;}
                                }
                            }
                        }

                        for(i = 0; i < 8 ; i++)
                        {
                            sysFSStat dstat;
                            sprintf(filename, "/dev_usb00%c", 48+i);
                            if(max_list_box_devices < 16 && !sysLv2FsStat(filename, &dstat))
                                {list_box_devices[max_list_box_devices]= 64 + i; max_list_box_devices++;}
                        }

                        select_option = 0;
                        menu_screen = 777;
                    }
                }
                break;

            case 6: // Copy to Favorites
                if(TestFavouritesExits(directories[currentgamedir].title_id))
                {
                        DeleteFavouritesIfExits(directories[currentgamedir].title_id);

                        sprintf(temp_buffer, "%s/config/", self_path);
                        SaveFavourites(temp_buffer, mode_homebrew);

                        if(mode_favourites && !havefavourites)
                        {
                            mode_favourites = 0; get_games(); select_option = 0;
                            menu_screen = 0;
                            return;
                        }

                        get_games();
                }
                else
                {
                    mode_favourites = currentgamedir  | 0x10000;

                    int r = get_icon(path_name, currentgamedir);

                    if(r == GET_ICON_FROM_ISO)
                    {
                        int fd = ps3ntfs_open(path_name, O_RDONLY, 0);
                        if(fd > 0)
                        {
                            u32 flba;
                            u64 size;
                            char *mem = NULL;
                            int re;

                            if((directories[currentgamedir].flags & (PSP_FLAG)) == (PSP_FLAG))
                                re = get_iso_file_pos(fd, "/PSP_GAME/ICON0.PNG", &flba, &size);
                            else
                                re = get_iso_file_pos(fd, "/PS3_GAME/ICON0.PNG;1", &flba, &size);

                            if(!re && (mem = memalign(32, size + 128)) != 0)
                            {
                                re = ps3ntfs_read(fd, (void *) mem, size);
                                ps3ntfs_close(fd);
                                if(re == size)
                                {
                                    memset(&my_png_datas, 0, sizeof(PngDatas));
                                    my_png_datas.png_in = mem;
                                    my_png_datas.png_size = size;
                                    if(LoadTexturePNG(NULL, BIG_PICT) == SUCCESS) r = ICON_LOAD_SUCCESS;
                                }
                                free(mem);
                            }
                            else
                                ps3ntfs_close(fd);
                        }
                    }

                    if(r == ICON_LOAD_SUCCESS) ;
                    else if(!strncmp(path_name + strlen(path_name) -4, ".JPG", 4) || !strncmp(path_name + strlen(path_name) -4, ".jpg", 4))
                        LoadTextureJPG(path_name, BIG_PICT);
                    else
                        LoadTexturePNG(path_name, BIG_PICT);

                    get_games();
                    select_option = 0;
                    menu_screen = 0;
                    return;
                }
                break;

            case 7: // Update
            {
               int r = ftp_net_status();

               if(r == -1)
               {
                   ftp_net_init();
                   r = ftp_net_status();
               }

               if(r == -4)
               {
                   ftp_net_deinit();
                   ftp_net_init();
                   r = ftp_net_status();
               }

               if(r != SUCCESS) break;

               if(game_update(directories[currentgamedir].title_id) > 0)
               {
                    //sprintf(temp_buffer, "%s/PKG", self_path);
                    file_manager(updates_path, NULL);

                    select_px = select_py = 0;
                    ndirectories = 0;

                    fdevices = 0;
                    fdevices_old = 0;
                    forcedevices = 0;
                    find_device = 0;
                    bdvd_notify = 1;
                    currentgamedir = currentdir = 0;

                    select_option = 0;
                    menu_screen = 0;
                }
            }
            break;
/*
            case 8: // Return

                Png_offset[BIG_PICT] = 0;
                select_option = 0;
                menu_screen = 0;
                return;
*/
            default:
               break;
        }
       // menu_screen = 0; return;
    }
    else if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        Png_offset[BIG_PICT] = 0; menu_screen = 0; select_option = 0; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        select_option--;
        frame_count = 32;

        if((directories[currentgamedir].flags & D_FLAG_BDVD) && (select_option == 2 || select_option == 6)) select_option--;
        if(!copy_flag && select_option == 1) select_option--;

        if(directories[currentgamedir].title_id[0] == 0 && (select_option == 0 || select_option == 5 || select_option == 7)) select_option--;

        if(mode_homebrew >= HOMEBREW_MODE)
        {
            if(select_option < 2) select_option= -1;
            if(select_option == 5) select_option= 4;
            if(select_option == 7) select_option= 6;
        }

        if(select_option < 0) select_option = 7;
    }
    else if(new_pad & BUTTON_DOWN)
    {
        select_option++;
        frame_count = 32;

        if(!copy_flag && select_option == 1) select_option++;

        if((directories[currentgamedir].flags & D_FLAG_BDVD) &&  (select_option == 2 || select_option == 6)) select_option++;

        if(mode_homebrew == HOMEBREW_MODE)
        {
            if(select_option < 2) select_option = 2;
            if(select_option == 5) select_option = 6;
            if(select_option == 7) select_option= 8;
        }

        if(select_option > 7) select_option = 0;

        if(directories[currentgamedir].title_id[0] == 0 && (select_option == 0 || select_option == 5 || select_option == 7))
        {
            select_option++;

            if(!copy_flag && select_option == 1) select_option++;

            if((directories[currentgamedir].flags & D_FLAG_BDVD) && select_option == 2) select_option++;
        }
    }
}

void draw_iso_options(float x, float y, int index)
{

    int i, o, max_op;

    float y2;
    int is_ntfs_dev = NTFS_DEVICE_UNMOUNT;

    int selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    is_ntfs_dev = NTFS_Test_Device(directories[currentgamedir].path_name + 1);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMOPT_OPTS]);

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    draw_app_version(x, y);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);

    if((directories[currentgamedir].flags & (PS1_FLAG)) || (directories[currentgamedir].flags & (PS2_FLAG)))
        parse_iso_titleid(directories[currentgamedir].path_name, temp_buffer);

    temp_buffer[64] = 0;
    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    y2 = y + 8;

    SetFontSize(12, 16);

    SetFontColor(0xffffffff, 0x00000000);

    y2 = y + 32;

    if(!TestFavouritesExits(directories[currentgamedir].title_id))
        DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_CPYTOFAV], (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : (flash && select_option == 0));
    else
        DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_DELFMFAV], (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : (flash && select_option == 0));

    max_op = o = 1;

    if(is_ntfs_dev >= 0)
    {
        y2 += 48;

        sprintf(temp_buffer, "Unmount USB00%i Device", is_ntfs_dev);
        DrawButton1_UTF8(x + 32, y2, 320, temp_buffer, (directories[currentgamedir].flags & D_FLAG_BDVD) ? -1  : (flash && select_option == o));

        max_op++; o++;
    }

    y2+= 48;

    if(select_option == o &&(directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) select_option++;
    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_TSTGAME],
        ((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) ? -1 : (flash && select_option == o));
    max_op++; o++;

    y2+= 48;

    if(select_option == o &&(directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) select_option++;
    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_EXTRACTISO],
        ((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) ? -1 : (flash && select_option == o));
    max_op++; o++;

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_CPYGAME],(flash && select_option == o));
    max_op++; o++;

    y2+= 48;

    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_DELGAME], ((flash && select_option == o) ? 1 : 0));
    max_op++; o++;

    y2+= 48;

    if(select_option == o &&(directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) select_option++;
    DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMOPT_GAMEUPDATE],
        ((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG)) ? -1 : (flash && select_option == o));
    max_op++; o++;

    y2+= 48;
/*
    DrawButton1_UTF8(x + 32, y2, 320, language[GLOBAL_RETURN], (flash && select_option == o));
    max_op++; o++;

    y2+= 48;
*/
    /*
    for(int n = 0; n < ((is_ntfs_dev < 0) ? 1 : 0); n++)
    {
        DrawButton1_UTF8(x + 32, y2, 320, "", -1);

        y2+= 48;
    }
    */

//
    SetFontSize(8, 10);

    utf8_to_ansi(directories[currentgamedir].path_name, temp_buffer, 128);
    temp_buffer[128] = 0;

    DrawFormatString(x + 8, y + 3 * 150 - 6, "%s", temp_buffer);
//

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150 + 7, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    i = selected;

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags  & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
                update_title_utf8 = 1;
            }
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 + 8, 0, 768, 32, str_color);

    }

    tiny3d_Flip();
    ps3pad_read();

    if((old_pad & BUTTON_SELECT) && (new_pad & BUTTON_TRIANGLE))
    {
        if (Png_offset[BIG_PICT])
        {
            Png_offset[BACKGROUND_PICT] = Png_offset[BIG_PICT];
            Png_datas[BACKGROUND_PICT] = Png_datas[BIG_PICT];
        }
    }

    if(new_pad & BUTTON_CROSS)
    {
        int select_option2 = select_option;

        if(is_ntfs_dev < 0 && select_option2 > 0) select_option2++;

        switch(select_option2)
        {
            case 0:
                if(TestFavouritesExits(directories[currentgamedir].title_id))
                {
                        DeleteFavouritesIfExits(directories[currentgamedir].title_id);

                        sprintf(temp_buffer, "%s/config/", self_path);
                        SaveFavourites(temp_buffer, mode_homebrew);

                        if(mode_favourites && !havefavourites)
                        {
                            mode_favourites = 0; get_games(); select_option = 0;
                            menu_screen = 0;
                            return;
                        }

                        get_games();
                }
                else
                {
                    mode_favourites = currentgamedir  | 0x10000;

                    int r = get_icon(path_name, currentgamedir);

                    if(r == GET_ICON_FROM_ISO)
                    {
                        int fd = ps3ntfs_open(path_name, O_RDONLY, 0);
                        if(fd > 0)
                        {
                            u32 flba;
                            u64 size;
                            char *mem = NULL;
                            int re;

                            if((directories[currentgamedir].flags & (PSP_FLAG)) == (PSP_FLAG))
                                re = get_iso_file_pos(fd, "/PSP_GAME/ICON0.PNG", &flba, &size);
                            else
                                re = get_iso_file_pos(fd, "/PS3_GAME/ICON0.PNG;1", &flba, &size);

                            if(!re && (mem = memalign(32, size + 128)) != 0)
                            {
                                re = ps3ntfs_read(fd, (void *) mem, size);
                                ps3ntfs_close(fd);
                                if(re == size)
                                {
                                    memset(&my_png_datas, 0, sizeof(PngDatas));
                                    my_png_datas.png_in = mem;
                                    my_png_datas.png_size = size;
                                    if(LoadTexturePNG(NULL, BIG_PICT) == SUCCESS) r = ICON_LOAD_SUCCESS;
                                }
                                free(mem);
                            }
                            else
                                ps3ntfs_close(fd);
                        }
                    }

                    if(r == ICON_LOAD_SUCCESS) ;
                    else if(!strncmp(path_name + strlen(path_name) -4, ".JPG", 4) || !strncmp(path_name + strlen(path_name) -4, ".jpg", 4))
                        LoadTextureJPG(path_name, BIG_PICT);
                    else
                        LoadTexturePNG(path_name, BIG_PICT);

                    get_games();
                    select_option = 0;
                    menu_screen = 0;
                    return;
                }
                break;

            case 1:
                NTFS_UnMount(is_ntfs_dev);
                ndirectories = 0;

                fdevices = 0;
                fdevices_old = 0;
                forcedevices = 0;
                find_device = 0;
                bdvd_notify = 1;
                currentgamedir = currentdir = 0;
                select_px = select_py = 0;
                Png_offset[BIG_PICT] = 0;
                select_option = 0;
                menu_screen = 0;
                return;

             case 2:
                 i = selected;

                 if(Png_offset[i])
                 {
                    pause_music(1);

                    test_game(currentgamedir);
                    patchps3iso(directories[currentgamedir].path_name, 0);

                    pause_music(0);
                    stops_BDVD = 1;
                 }
                 break;
            case 3:
                {
                if(mode_homebrew >= HOMEBREW_MODE) break;

                i = selected;

                if(Png_offset[i])
                {
                    // get device list
                    list_box_devices[0] = 0;
                    max_list_box_devices = 1;

                    int i, k;
                    for(i = 0; i < 8 ; i++)
                    {
                        if(mounts[i])
                        {
                            for (k = 0; k < mountCount[i]; k++)
                            {
                                if(max_list_box_devices < 16 && !strncmp((mounts[i]+k)->name, "ntfs", 4))
                                    {list_box_devices[max_list_box_devices]= 128 + ((mounts[i]+k)->name[4] - 48); max_list_box_devices++;}
                            }
                        }
                    }

                    for(i = 0; i < 8 ; i++)
                    {
                        sysFSStat dstat;
                        sprintf(filename, "/dev_usb00%c", 48+i);
                        if(max_list_box_devices < 16 && !sysLv2FsStat(filename, &dstat))
                            {list_box_devices[max_list_box_devices]= 64 + i; max_list_box_devices++;}
                    }

                    select_option = 0;
                    menu_screen = 778;
                }
                }
                break;

            ///
            case 4:
                if(mode_homebrew >= HOMEBREW_MODE) break;

                i = selected;

                if(Png_offset[i])
                {
                    // get device list

                    max_list_box_devices = 0;

                    if(strncmp(directories[currentgamedir].path_name, "/dev_hdd0", 9))
                    {
                        list_box_devices[0] = 0;
                        max_list_box_devices = 1;
                    }

                    int i, k;
                    for(i = 0; i < 8 ; i++)
                    {
                        if(mounts[i])
                        {
                            for (k = 0; k < mountCount[i]; k++)
                            {
                                if(max_list_box_devices < 16 && !strncmp((mounts[i]+k)->name, "ntfs", 4)
                                    && strncmp(&directories[currentgamedir].path_name[1], (mounts[i]+k)->name, 5))
                                    {list_box_devices[max_list_box_devices]= 128 + ((mounts[i]+k)->name[4] - 48); max_list_box_devices++;}
                            }
                        }
                    }

                    for(i = 0; i < 8 ; i++)
                    {
                        sysFSStat dstat;
                        sprintf(filename, "/dev_usb00%c", 48+i);
                        if(max_list_box_devices < 16 && !sysLv2FsStat(filename, &dstat) &&
                            strncmp(directories[currentgamedir].path_name, filename, 11))
                            {list_box_devices[max_list_box_devices]= 64 + i; max_list_box_devices++;}
                    }

                    if(max_list_box_devices > 0)
                    {
                        select_option = 0;
                        menu_screen = 779;
                    }
                }
                break;

            case 5:
                 i = selected;

                 if(Png_offset[i])
                 {
                    if(!strncmp(directories[currentgamedir].path_name, "/dev_hdd0", 9))
                        sprintf(temp_buffer, "%s\n\n%s HDD0?", directories[currentgamedir].title, language[GAMEDELSL_WANTDELETE]);
                    else if(!strncmp(directories[currentgamedir].path_name, "/dev_usb", 8))
                        sprintf(temp_buffer, "%s\n\n%s USB00%c?", directories[currentgamedir].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[10]);
                    else if(!strncmp(directories[currentgamedir].path_name, "/ntfs", 5))
                        sprintf(temp_buffer, "%s\n\n%s NTFS%c?", directories[currentgamedir].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[5]);
                    else if(!strncmp(directories[currentgamedir].path_name, "/ext", 4))
                        sprintf(temp_buffer, "%s\n\n%s EXT%c?", directories[currentgamedir].title, language[GAMEDELSL_WANTDELETE], directories[currentgamedir].path_name[4]);
                    else break;

                    if(DrawDialogYesNo(temp_buffer) == 1)
                    {
                        pause_music(1);

                        delps3iso(directories[currentgamedir].path_name);

                        pause_music(0);

                        fdevices = 0;
                        fdevices_old = 0;
                        forcedevices = 0;
                        find_device = 0;
                        bdvd_notify = 1;
                        currentgamedir = currentdir = 0;
                        Png_offset[BIG_PICT] = 0;
                        select_option = 0;
                        menu_screen = 0;
                        select_px = select_py = 0;
                        stops_BDVD = 1;
                    }
                 }
                 return;

            case 6:
            {
               int r = ftp_net_status();

               if(r == -1)
               {
                   ftp_net_init();
                   r = ftp_net_status();
               }

               if(r == -4)
               {
                   ftp_net_deinit();
                   ftp_net_init();
                   r = ftp_net_status();
               }

               if(r != SUCCESS) break;

               if(game_update(directories[currentgamedir].title_id) > 0)
               {
                    //sprintf(temp_buffer, "%s/PKG", self_path);
                    file_manager(updates_path, NULL);

                    select_px = select_py = 0;
                    ndirectories = 0;

                    fdevices = 0;
                    fdevices_old = 0;
                    forcedevices = 0;
                    find_device = 0;
                    bdvd_notify = 1;
                    currentgamedir = currentdir = 0;

                    select_option = 0;
                    menu_screen = 0;
                }
            }
            break;
/*
            case 7:

                Png_offset[BIG_PICT] = 0;
                select_option = 0;
                menu_screen = 0;
                return;
*/
            default:
               break;
        }
       // menu_screen = 0; return;
    }
    else if(new_pad & (BUTTON_TRIANGLE | BUTTON_CIRCLE))
    {
        Png_offset[BIG_PICT] = 0; menu_screen = 0; select_option = 0; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, max_op - 1)

        if((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG))
        {
            if(select_option >= 5) select_option = 4;
            if(select_option == 2) select_option = 0;
        }
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, max_op - 1, 0);

        if((directories[currentgamedir].flags & (PS2_FLAG)) == (PS2_FLAG))
        {
            if(select_option >= 5) select_option = 0;
        }
    }
}

void draw_configs(float x, float y, int index)
{

    int i;

    float y2, x2;

    int selected;
    selected = select_px + select_py * cols;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGMCFG_CFGS]);

    i = selected;

    SetCurrentFont(FONT_BUTTON);

    SetFontSize(16, 20);

    utf8_to_ansi(directories[currentgamedir].title_id, temp_buffer, 64);
    temp_buffer[64] = 0;

    DrawFormatString(848 - x - strlen(temp_buffer) * 16 - 8, y, temp_buffer);

    draw_app_version(x, y);

    y += 24;

    if(Png_offset[BIG_PICT])
        tiny3d_SetTextureWrap(0, Png_offset[BIG_PICT], Png_datas[BIG_PICT].width,
                              Png_datas[BIG_PICT].height, Png_datas[BIG_PICT].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
    else
        tiny3d_SetTextureWrap(0, Png_res_offset[IMG_DEFAULT_BACKGROUND], Png_res[IMG_DEFAULT_BACKGROUND].width,
                              Png_res[IMG_DEFAULT_BACKGROUND].height, Png_res[IMG_DEFAULT_BACKGROUND].wpitch,
                              TINY3D_TEX_FORMAT_A8R8G8B8,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);

    DrawTextBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0xffffffff);

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    x2 = x;
    y2 = y + 32;


#ifdef CONFIG_USE_SYS8PERMH4
    x2 = DrawButton1_UTF8(x + 32, y2, 320, "Fix Permissions", (flash && select_option == 0)) + 16; // do no translate this (3.44)

    x2 = DrawButton2_UTF8(x2, y2, 0, " Default ", (game_cfg.perm == 0) ) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, " PS jailbreak ", (game_cfg.perm == 1)) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, " v4 Perms (F1) ", (game_cfg.perm == 2)) + 8;

    y2+= 48;
#endif

    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMCFG_DSK], (flash && select_option == 0))  + 16;
    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_NO] , (game_cfg.useBDVD == 0)) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_YES], (game_cfg.useBDVD == 1)) + 8;

    y2+= 48;

    x2 = DrawButton1_UTF8(x + 32, y2, 320, "Direct Boot", (flash && select_option == 1)) + 16;

    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_NO], (game_cfg.direct_boot == 0) ) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_YES], (game_cfg.direct_boot == 1)) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, "With BR", (game_cfg.direct_boot == 2)) + 8;

    y2 += 48;

    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMCFG_EXTBOOT], (flash && select_option == 2))  + 16;

    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_ON] , (payload_mode >= ZERO_PAYLOAD) ? (game_cfg.ext_ebootbin != 0) : -1 ) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_OFF], (game_cfg.ext_ebootbin == 0)) + 8;

    y2 += 48;

    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMCFG_BDEMU], (flash && select_option == 3))  + 16;

    if(directories[currentgamedir].flags & D_FLAG_HDD0)
    {
        x2 = DrawButton2_UTF8(x2, y2, 0, "Mount BDVD" ,(game_cfg.bdemu == 1)) + 8;
        x2 = DrawButton2_UTF8(x2, y2, 0, "LIBFS" , (game_cfg.bdemu > 1)) + 8;
        x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_OFF], (game_cfg.bdemu == 0)) + 8;
    }
    else
    {
        x2 = DrawButton2_UTF8(x2, y2, 0, "Mount BDVD" ,(game_cfg.bdemu_ext == 1)) + 8;
        x2 = DrawButton2_UTF8(x2, y2, 0, "LIBFS" , (game_cfg.bdemu_ext > 1)) + 8;
        x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_OFF], (game_cfg.bdemu_ext == 0)) + 8;
    }

    y2+= 48;

    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMCFG_EXTHDD0GAME], (flash && select_option == 4))  + 16;

    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_ON] , (payload_mode >= ZERO_PAYLOAD) ? (game_cfg.exthdd0emu != 0): -1) + 8;
    x2 = DrawButton2_UTF8(x2, y2, 0, language[DRAWGMCFG_OFF], (game_cfg.exthdd0emu == 0)) + 8;

    y2+= 48;

    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[DRAWGMCFG_SAVECFG], (flash && select_option == 5))  + 16;
    y2+= 48;
/*
    x2 = DrawButton1_UTF8(x + 32, y2, 320, language[GLOBAL_RETURN], (flash && select_option == 6))  + 16;
    y2+= 48;
*/

    SetCurrentFont(FONT_TTF);

    // draw game name

    i = selected;

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    if(Png_offset[i])
    {
        u32 str_color = 0xffffffff;

        if((directories[currentgamedir].flags & GAMELIST_FILTER) == BDVD_FLAG)
        {
            if(strncmp((char *) string_title_utf8, bluray_game, 64))
            {
                strncpy((char *) string_title_utf8, bluray_game, 128);
                update_title_utf8 = 1;
            }
            str_color = 0x00ff00ff;
        }
        else
        {
            if(strncmp((char *) string_title_utf8, directories[currentgamedir].title, 64))
            {
                strncpy((char *) string_title_utf8, directories[currentgamedir].title, 128);
                update_title_utf8 = 1;
            }
        }

        if(update_title_utf8)
        {
            width_title_utf8 = Render_String_UTF8(ttf_texture, 768, 32, string_title_utf8, 16, 24);
            update_title_utf8 = 0;
        }

        tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(ttf_texture), 768,
                32, 768 * 2,
                TINY3D_TEX_FORMAT_A4R4G4B4,  TEXTWRAP_CLAMP, TEXTWRAP_CLAMP,1);
        DrawTextBox((848 - width_title_utf8) / 2, y + 3 * 150 , 0, 768, 32, str_color);
    }


    tiny3d_Flip();
    ps3pad_read();

    if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        menu_screen = 1; select_option = 0; return;
    }
    else if(new_pad & (BUTTON_START))
    {
        select_option = 5;
    }


    if(new_pad & (BUTTON_CROSS | BUTTON_START))
    {
        switch(select_option)
        {
            //removed sys8 calls not supported yet on 3.55
#ifdef CONFIG_USE_SYS8PERMH4
            case 0:
                ROT_INC(game_cfg.perm, 2, 0);
                break;
#endif
            case 0:
                ROT_INC(game_cfg.useBDVD, 1, 0);
                break;
            case 1:
                ROT_INC(game_cfg.direct_boot, 2, 0);
                break;
            case 2:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_INC(game_cfg.ext_ebootbin, 1, 0);
                break;
            case 3:
                if(directories[currentgamedir].flags & D_FLAG_HDD0)
                    ROT_INC(game_cfg.bdemu, 2, 0)
                else
                    ROT_INC(game_cfg.bdemu_ext, 2, 0)
                break;
            case 4:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_INC(game_cfg.exthdd0emu, 1, 0);
                break;
            case 5:
                // save game config
                sprintf(temp_buffer, "%s/config/%s.cfg", self_path, directories[currentgamedir].title_id);

                if(SaveFile(temp_buffer, (char *) &game_cfg, sizeof(game_cfg)) == SUCCESS)
                {
                    sprintf(temp_buffer, "%s/config/%s.cfg\n\n%s", self_path, directories[currentgamedir].title_id, language[GLOBAL_SAVED]);
                    DrawDialogOKTimer(temp_buffer, 2000.0f);
                    menu_screen = 1; select_option = 0; return;
                }

                break;
            default:
                menu_screen = 1; select_option = 0; return;
                break;
        }
    }

    if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, 5)
    }

    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, 5, 0);
    }

    else if(new_pad & BUTTON_LEFT)
    {
        switch(select_option)
        {
            //removed sys8 calls not supported yet on 3.55
#ifdef CONFIG_USE_SYS8PERMH4
            case 0:
                ROT_DEC(game_cfg.perm, 0, 2);
                break;
#endif
            case 0:
                ROT_DEC(game_cfg.useBDVD, 0, 1);
                break;
            case 1:
                ROT_DEC(game_cfg.direct_boot, 0, 2);
                break;
            case 2:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_DEC(game_cfg.ext_ebootbin, 0, 1);
                break;
            case 3:
                if(directories[currentgamedir].flags & D_FLAG_HDD0)
                    ROT_DEC(game_cfg.bdemu, 0, 2)
                else
                    ROT_DEC(game_cfg.bdemu_ext, 0, 2)
                break;
             case 4:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_DEC(game_cfg.exthdd0emu, 0, 1);
                break;
            default:
                break;
        }
     }

     else if(new_pad & BUTTON_RIGHT)
     {
        switch(select_option)
        {
            //removed sys8 calls not supported yet on 3.55
#ifdef CONFIG_USE_SYS8PERMH4
            case 0:
                ROT_INC(game_cfg.perm, 2, 0);
                break;
#endif
            case 0:
                ROT_INC(game_cfg.useBDVD, 1, 0);
                break;
            case 1:
                ROT_INC(game_cfg.direct_boot, 2, 0);
                break;
            case 2:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_INC(game_cfg.ext_ebootbin, 1, 0);
                break;
            case 3:
                if(directories[currentgamedir].flags & D_FLAG_HDD0)
                    ROT_INC(game_cfg.bdemu, 2, 0)
                else
                    ROT_INC(game_cfg.bdemu_ext, 2, 0)
                break;
            case 4:
                if(payload_mode >= ZERO_PAYLOAD)
                    ROT_INC(game_cfg.exthdd0emu, 1, 0);
                break;
            default:
                break;
        }
     }
}

/*
static char help1[]= {
    "Test of Character Set\n"
    "Hola Hello Bonjour Ciao Hallo مرحبا\n"
    "привет Γεια σας もしもし Merhaba ğ\n"
    "안녕하세요. 你好\n"
    "Dışişleri Bakanlığı\n"
    "Мне нужно немного водки\n"
    "我想貴國訪問\n"
    "Γαμημένοι πολιτικοί που έχουν προκαλέσει αναστάτωση\n"
    "スペインからのご挨拶\n"
    "تحيات من اسبانيا\n"
};
*/

void draw_gbloptions(float x, float y)
{

    float y2;
    static float x3 = -1;
    static int help = 0;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWGLOPT_OPTS]);

    if(x3 < 0)
    {
        x3 = 2000;
        x3 = DrawFormatString(x3, y,  music[song_selected + MAX_SONGS]); // calculate first time
        x3 = 848 -(x3 - 2000) - x;
    }
    else
        DrawFormatString(x3, y,  music[song_selected + MAX_SONGS]); //print current song name

    y += 24;

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    y2 = y + 32 - 24;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWGLOPT_TOOLS], (flash && select_option == -1));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWGLOPT_REFRESH], (flash && select_option == 0));
    y2+= 48;

    if(cover_mode)
    {
        if(gui_mode ==  1) sprintf(temp_buffer, "%s: Coverflow + Cover", language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  2) sprintf(temp_buffer, "%s: Grid 3x2 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  3) sprintf(temp_buffer, "%s: Grid 4x2 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  4) sprintf(temp_buffer, "%s: Grid 3x3 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  5) sprintf(temp_buffer, "%s: Grid 4x3 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  6) sprintf(temp_buffer, "%s: Grid 5x3 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  7) sprintf(temp_buffer, "%s: Grid 6x3 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  8) sprintf(temp_buffer, "%s: Grid 4x4 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  9) sprintf(temp_buffer, "%s: Grid 5x4 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 10) sprintf(temp_buffer, "%s: Grid 6x4 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 11) sprintf(temp_buffer, "%s: Grid 5x5 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 12) sprintf(temp_buffer, "%s: Grid 6x5 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 13) sprintf(temp_buffer, "%s: Grid 6x6 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 14) sprintf(temp_buffer, "%s: Grid 8x5 + Cover" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 15) sprintf(temp_buffer, "%s: Grid 8x6 + Cover" , language[DRAWGLOPT_CHANGEGUI]);
    }
    else
    {
        if(gui_mode ==  1) sprintf(temp_buffer, "%s: Coverflow", language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  2) sprintf(temp_buffer, "%s: Grid 3x2" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  3) sprintf(temp_buffer, "%s: Grid 4x2" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  4) sprintf(temp_buffer, "%s: Grid 3x3" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  5) sprintf(temp_buffer, "%s: Grid 4x3" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  6) sprintf(temp_buffer, "%s: Grid 5x3" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  7) sprintf(temp_buffer, "%s: Grid 6x3" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  8) sprintf(temp_buffer, "%s: Grid 4x4" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode ==  9) sprintf(temp_buffer, "%s: Grid 5x4" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 10) sprintf(temp_buffer, "%s: Grid 6x4" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 11) sprintf(temp_buffer, "%s: Grid 5x5" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 12) sprintf(temp_buffer, "%s: Grid 6x5" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 13) sprintf(temp_buffer, "%s: Grid 6x6" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 14) sprintf(temp_buffer, "%s: Grid 8x5" , language[DRAWGLOPT_CHANGEGUI]); else
        if(gui_mode == 15) sprintf(temp_buffer, "%s: Grid 8x6" , language[DRAWGLOPT_CHANGEGUI]);
    }

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, temp_buffer, (flash && select_option == 1));
    y2+= 48;

    if(background_sel == 0)
        sprintf(temp_buffer, "%s [FX %i]", language[DRAWGLOPT_CHANGEBCK], background_fx);
    else if(background_sel == 1)
    {
        if(bk_picture == 0)
          sprintf(temp_buffer, "Background Picture: None");
        else if(bk_picture == 1)
          sprintf(temp_buffer, "Background Picture: Random");
        else if(bk_picture == 2)
          sprintf(temp_buffer, "Background Picture: PIC1.PNG");
        else
          sprintf(temp_buffer, "Background Picture: PICT%i.JPG", bk_picture - 3);
    }
    else
        sprintf(temp_buffer, "%s [Color %i]", language[DRAWGLOPT_CHANGEBCK], background_sel - 1);

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, temp_buffer, (flash && select_option == 2));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWGLOPT_SCRADJUST], (flash && select_option == 3));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWGLOPT_CHANGEDIR], (flash && select_option == 4));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, (manager_cfg.opt_flags & OPTFLAGS_PLAYMUSIC)? language[DRAWGLOPT_SWMUSICOFF] : language[DRAWGLOPT_SWMUSICON] , (flash && select_option == 5));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, (ftp_ip_str[0]) ? ftp_ip_str : language[DRAWGLOPT_INITFTP], (flash && select_option == 6));
    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_COVERSDOWN], (flash && select_option == 7));
    y2+= 48;

    if(select_option >= 7)
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWGLOPT_CREDITS], (flash && select_option == 8));

    y2+= 48;

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);
    //y2+= 48;

    // draw sys version
    SetCurrentFont(FONT_TTF);
    SetFontColor(0xccccffff, 0x00000000);
    SetFontAutoCenter(1);

    if(select_option == 1)
    {
        SetFontSize(14, 20);

        if(show_custom_icons)
            DrawFormatString(500, y2 - 28, "SQUARE  Toggle Covers        R1/L1 - Showtime Icon: ON        <  >  Select GUI mode");
        else
            DrawFormatString(500, y2 - 28, "SQUARE  Toggle Covers        R1/L1 - Showtime Icon: OFF       <  >  Select GUI mode");
    }
    else if(select_option == 2)
    {
        SetFontSize(16, 20);
        if(background_sel == 0)
            DrawFormatString(500, y2 - 28, "X  Change Background           <  >  Select FX type");
        else if(background_sel == 1)
            DrawFormatString(500, y2 - 28, "X  Change Background           <  >  Select Picture");
        else
            DrawFormatString(500, y2 - 28, "X  Change Background           <  >  Select Color");
    }
    else if(select_option < 7)
    {
        SetFontSize(16, 20);
        DrawFormatString(0, y2 - 28, payload_str );
    }

    SetFontAutoCenter(0);

/*
    if(help)
    {
        DrawBox((848 - 624)/2, (512 - 424)/2, 0, 624, 424, 0x602060ff);
        DrawBox((848 - 616)/2, (512 - 416)/2, 0, 616, 416, 0x802080ff);
        set_ttf_window((848 - 600)/2, (512 - 416)/2, 600, 416, WIN_AUTO_LF);

        display_ttf_string(0, 0, help1, 0xffffffff, 0, 18, 24);

        SetFontAutoCenter(1);

        if(lv2peek(0x80000000000004E8ULL) && !use_cobra)
            DrawFormatString(0, (512 - 416)/2 - 20, "Event ID: %x / VSH ID %x", (u32) syscall_40(4, 0), manager_cfg.event_flag);
        SetFontAutoCenter(0);

        if(lv2peek(0x80000000000004E8ULL) && !use_cobra)
        {
            u32 eid= (u32) syscall_40(4, 0);

            if(eid != 0)
            {
                eid -= 0x100;
                if(eid != manager_cfg.event_flag)
                {
                    manager_cfg.event_flag = eid;
                    SaveManagerCfg();
                }
            }
        }
    }
*/

    tiny3d_Flip();
    ps3pad_read();

    //if(new_pad & ) help^=1;

    if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
exit_gbloptions:
       if(refresh_gui == 1)
       {
           refresh_gui = 0;
           get_grid_dimensions();

           manager_cfg.cover_mode = cover_mode;
           manager_cfg.gui_mode = ((sort_mode & 0xf)<<4) | (gui_mode & 0xf);
           SaveManagerCfg();
           currentgamedir = currentdir = 0;
           select_px = select_py = 0;

           locate_last_game();

           get_games();
           load_gamecfg(-1); // force refresh game info
        }
        help = 0;
        menu_screen = 0; return;
    }

    //if(help) return;

    if((new_pad & BUTTON_CROSS) ||
       ((select_option == 1 || ((select_option == 2) && (background_sel > 1)) || select_option == 5 || select_option == 6) && (new_pad & (BUTTON_LEFT | BUTTON_RIGHT))))
    {
        switch(select_option)
        {
            case -1: // Tools menu
                select_option = 0;
                menu_screen = 4;
                return;

            case 0: // Refresh Game List

                // refresh custom settings
                read_settings();

                select_px = select_py = 0;
                select_option = 0;
                menu_screen = 0;

                anim_mode = 0; anim_step = 0;

                ndirectories = 0;

                fdevices = 0;
                fdevices_old = 0;
                forcedevices = 0;
                find_device = 0;
                bdvd_notify = 1;
                currentdir = 0;

                mode_homebrew = 0;
                game_list_category = 0;
                mode_favourites = 0;

                select_option = 0;
                menu_screen = 0;

                return;

            case 1: // Change Current GUI

                if (new_pad & (BUTTON_LEFT)) gui_mode--; else gui_mode++;

                if(gui_mode > GUI_MODES)
                {
                    gui_mode = 1;
                    if(cover_mode) cover_mode = 0; else cover_mode = 1;
                }
                else if(gui_mode < 1)
                {
                    gui_mode = GUI_MODES;
                    if(cover_mode) cover_mode = 0; else cover_mode = 1;
                }
                refresh_gui = 1;
                break;
            case 2: // Change Background Color
                if (new_pad & BUTTON_LEFT) background_sel--; else background_sel++;;
                if(background_sel >= MAX_COLORS) background_sel = 0;
                manager_cfg.background_sel = background_sel;
                SaveManagerCfg();
                break;

            case 3: // Video Adjust
                video_adjust();
                select_option = 0;
                menu_screen = 0;
                return;

            case 4: // Change Game Directory
                menu_screen = 0;
                Select_games_folder();

                if(manager_cfg.hdd_folder[0] == 0) strcpy(manager_cfg.hdd_folder, __MKDEF_MANAGER_DIR__);
                SaveManagerCfg();
                currentgamedir = currentdir = 0;
                select_px = select_py = 0;
                select_option = 0;
                menu_screen = 0;

                ndirectories = 0;
                fdevices = 0;
                fdevices_old = 0;
                forcedevices = 0;
                find_device = 0;
                bdvd_notify = 1;
                load_gamecfg(-1); // force refresh game info

                return;

            case 5: // Switch Music On/Off
                if (new_pad & BUTTON_LEFT)
                {
                    if(song_selected > 0) song_selected--; else song_selected = MAX_SONGS - 1;
                    init_music(song_selected);
                }
                else if (new_pad & BUTTON_RIGHT)
                {
                    if(song_selected < MAX_SONGS) song_selected++; else song_selected = 0;
                    init_music(song_selected);
                }
                else
                {
                    manager_cfg.opt_flags ^= OPTFLAGS_PLAYMUSIC; //change bit
                    pause_music((manager_cfg.opt_flags & OPTFLAGS_PLAYMUSIC)? 0 : 1);
                    SaveManagerCfg();
                }
                break;

            case 6: // Initialize FTP server
                if(test_ftp_working()) break;
                if ((manager_cfg.opt_flags & OPTFLAGS_FTP) == 0)
                {
                    int r = ftp_net_status();

                    if(r == -1)
                    {
                       ftp_net_init();
                       r = ftp_net_status();
                    }

                    if(r == -4)
                    {
                       ftp_net_deinit();
                       ftp_net_init();
                       r = ftp_net_status();
                    }

                    r = ftp_init();
                    if(r == 0)
                    {
                        ftp_inited = 1;
                        if(DrawDialogYesNo(language[DRAWGLOPT_FTPINITED]) != 1)
                            break;
                    }
                    else
                    {
                        if(r == -1) DrawDialogOK("Error in netInitialize()");
                        else if(r == -2) DrawDialogOK("Error in netCtlInit()");
                        else if(r == -3) DrawDialogOK("Error in netCtlGetInfo()");
                        else if(r == -4) DrawDialogOK("Net Disconnected or Connection not Established");
                        else DrawDialogOK(language[DRAWGLOPT_FTPARINITED]);

                        break;
                    }
                }
                else
                {
                        DrawDialogOK(language[DRAWGLOPT_FTPSTOPED]);
                        ftp_deinit();
                        ftp_inited = 0;
                }
                manager_cfg.opt_flags ^= OPTFLAGS_FTP;
                SaveManagerCfg();
                break;

            case 7: // Download Covers
                select_option = 0;
                menu_screen = 0;

                int r = ftp_net_status();

                if(r == -1)
                {
                   ftp_net_init();
                   r = ftp_net_status();
                }

                if(r == -4)
                {
                   ftp_net_deinit();
                   ftp_net_init();
                   r = ftp_net_status();
                }

                if(r != SUCCESS) break;

                int n = covers_update(0);
                if(n == -1) n = covers_update(1); // try again
                wait_event_thread();
                get_games();
                if(n == 0)    DrawDialogOKTimer("Covers downloaded successfully\n\nCarátulas descargadas exitosamente", 2000.0f);
                if(n == -1)   DrawDialogOKTimer("Some covers could not be downloaded\n\nAlgunas carátulas no pueden descargarse", 2000.0f);
                if(n == -555) DrawDialogOKTimer("Aborted by the user\n\nAbortado por el usuario", 2000.0f);
                break;

            case 8: // Credits
                DrawDialogOK(credits_str1);
                DrawDialogOK(credits_str2);
                DrawDialogOK(credits_str3);
                break;

            default:
               break;
        }

    }

    if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, -1, 7);
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, 8, -1);
    }

    if(select_option == -1)
    {
        if(new_pad & BUTTON_START)
        {
            select_option = 0;
            menu_screen = 4;
            return;
        }
    }
    // Change Current GUI
    else if(select_option == 1)
    {
        // Toggle Showtime / Internet Browser icons
        if(new_pad & (BUTTON_L1 | BUTTON_R1))
        {
            ROT_INC(show_custom_icons, 1, 0);
            manager_cfg.show_custom_icons = show_custom_icons;
            forcedevices = 1;
            SaveManagerCfg();
        }
        // Toggle Cover mode
        else if(new_pad & BUTTON_SQUARE)
        {
            if(cover_mode) cover_mode = 0; else cover_mode = 1;
            refresh_gui = 1;
        }
        // Reset to default Coverflow mode
        else if(new_pad & BUTTON_SELECT)
        {
            gui_mode = MODE_COVERFLOW;
            refresh_gui = 1;
        }
        // Exit Global Options menu
        else if(new_pad & BUTTON_START)
        {
            goto exit_gbloptions;
        }
    }
    // Change Background Color
    else if(select_option == 2)
    {
        // Select animated background FXs
        if(background_sel == 0)
        {
            if(new_pad & (BUTTON_LEFT))
            {
              ROT_DEC(background_fx, 0, 7);
              manager_cfg.background_fx = background_fx;
              SaveManagerCfg();
            }
            else if(new_pad & (BUTTON_RIGHT))
            {
              ROT_INC(background_fx, 7, 0);
              manager_cfg.background_fx = background_fx;
              SaveManagerCfg();
            }
            else if(new_pad & BUTTON_SELECT)
            {
              background_fx = 0;
              load_background_picture();

              manager_cfg.background_fx = background_fx;
              SaveManagerCfg();
            }
        }
        // Select background picture
        else if(background_sel == 1)
        {
            if(new_pad & (BUTTON_LEFT))
            {
              if(bk_picture == 0)
              {
                  background_sel = 0;
              }
              else
              {
                  ROT_DEC(bk_picture, 0, 99);
                  load_background_picture();

                  manager_cfg.bk_picture = bk_picture;
                  SaveManagerCfg();
              }
            }
            else if(new_pad & (BUTTON_RIGHT))
            {
              ROT_INC(bk_picture, 99, 0);
              load_background_picture();

              manager_cfg.bk_picture = bk_picture;
              SaveManagerCfg();
            }
            else if(new_pad & BUTTON_SELECT)
            {
              bk_picture = 0;
              load_background_picture();

              manager_cfg.bk_picture = bk_picture;
              SaveManagerCfg();
            }
        }
        // Exit Global Options menu
        if(new_pad & BUTTON_START)
        {
            goto exit_gbloptions;
        }
    }
}

void draw_toolsoptions(float x, float y)
{

    float y2;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWTOOLS_TOOLS]);

    y += 24;

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    y2 = y + 12;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_ARCHIVEMAN], (flash && select_option == 0));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_PKGTOOLS], (flash && select_option == 1));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_DELCACHE], (flash && select_option == 2));

    y2+= 48;
/*
    if(manager_cfg.usekey)
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_SECDISABLE], (flash && select_option == 1));
    else
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_SECENABLE], (flash && select_option == 1));
*/

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_COPYFROM], (flash && select_option == 3));

    y2+= 48;

    if(!noBDVD)
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_WITHBDVD], (flash && select_option == 4));
    else if(noBDVD == MODE_DISCLESS)
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_NOBDVD2], (flash && select_option == 4));
    else
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_NOBDVD], (flash && select_option == 4));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, language[DRAWTOOLS_LANGUAGE_1 + (manager_cfg.language & 15)], (flash && select_option == 5));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Spoof Console ID", (flash && select_option == 6));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Control Fan & USB Wakeup", (flash && select_option == 7));

    y2+= 48;

    if(use_cobra)
    {
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Download latest webMAN", (flash && select_option == 8));

        y2+= 48;
    }

    /*
    for(int n = 0; n < 1; n++)
    {
        DrawButton1_UTF8((848 - 520) / 2, y2, 520, "", -1);

        y2+= 48;
    }
    */

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    tiny3d_Flip();
    ps3pad_read();

    if((new_pad & BUTTON_CROSS) ||
       ((select_option == 4 || select_option == 5) && (new_pad & (BUTTON_LEFT | BUTTON_RIGHT))))
    {

        switch(select_option)
        {
            case 0: // file manager
                if (file_manager(NULL, NULL) == 1)
                {
                    select_px = select_py = 0;
                    select_option = 0;
                    menu_screen = 0;

                    ndirectories = 0;

                    fdevices = 0;
                    fdevices_old = 0;
                    forcedevices = 0;
                    find_device = 0;
                    bdvd_notify = 1;
                    currentdir = 0;

                    select_option = 0;
                    menu_screen = 0;
                };

                return;

            case 1: //PKG Install
                DrawDialogOKTimer("For NTFS and EXT2/3/4 devices use the File Manager\n\nPara dispositivos NTFS y EXT2/3/4 usa el Manejador Archivos\n\nPour les lecteurs NTFS et EXT2/3/4 utiliser le gestionnaire de fichiers", 3000.0f);
                draw_pkginstall(x, y);
                break;

            case 2: //Delete Cache Tool

                LoadCacheDatas();
                // draw_cachesel

                if(ncache_list > 0)
                {
                    menu_screen = 5;
                    select_option = 0;
                }

                return;

            case 3: // Copy from /dev_usb/iris to Iris
                if(test_ftp_working()) break;
                copy_usb_to_iris(self_path);
                break;

            case 4: // Set noBDVD mode
                if(test_ftp_working()) break;

                if(new_pad & BUTTON_LEFT)
                {
                    if(noBDVD > 0) noBDVD--; else noBDVD = 2; //MODE_DISCLESS
                }
                else
                {
                    if(noBDVD < 2) noBDVD++; else noBDVD = 0; //WITH BDVD CONTROLLER
                }
                break;

            case 5: //Language
                if(new_pad & BUTTON_LEFT)
                {
                    if(manager_cfg.language > 0)
                        manager_cfg.language--;
                    else
                        manager_cfg.language = LANGCOUNT;
                }
                else
                {
                    if(manager_cfg.language < LANGCOUNT)
                        manager_cfg.language++;
                    else
                        manager_cfg.language = 0;
                }
                sprintf(temp_buffer, "%s/config/language.ini", self_path);
                open_language(manager_cfg.language, temp_buffer);
                //manager_cfg.usekey = manager_cfg.usekey == 0;
                SaveManagerCfg();

                break;

            case 6: // set console id
                select_option = 0;
                menu_screen = 222;
                return;

            case 7: // Control Fan & USB Wakeup
                select_option = 0;
                menu_screen = 0;
                draw_controlfan_options();
                return;

            case 8: // Download Latest webMAN
                if(use_cobra == 0 || use_mamba == 1)
                {
                    DrawDialogOK(credits_str1);
                    DrawDialogOK(credits_str2);
                    DrawDialogOK(credits_str3);
                    break;
                }

                struct stat s;

                sprintf(temp_buffer, "/dev_hdd0/plugins/webftp_server.sprx");

                if(stat(temp_buffer, &s) != SUCCESS)
                    sprintf(temp_buffer, webman_path);

                int r = ftp_net_status();

                if(r == -1)
                {
                   ftp_net_init();
                   r = ftp_net_status();
                }

                if(r == -4)
                {
                   ftp_net_deinit();
                   ftp_net_init();
                   r = ftp_net_status();
                }

                if(r != SUCCESS) break;

                if(download_file("http://www.deanbg.com/webftp_server.sprx", temp_buffer, 0, NULL) == 0)
                {
                    // Create boot_plugins.txt if it doesn't exist
                    if(stat("/dev_hdd0/boot_plugins.txt", &s) != SUCCESS)
                    {
                        sprintf(temp_buffer, "%s\n", temp_buffer);

                        FILE *fp;

                        // write plugin path
                        fp = fopen("/dev_hdd0/boot_plugins.txt", "w");
                        fputs (temp_buffer, fp);
                        fclose(fp);
                    }

                    DrawDialogOKTimer("webMAN has been updated successfully!", 2000.0f);
                }
                else
                    DrawDialogOKTimer("webMAN could not be downloaded!", 2000.0f);

                select_option = 0;
                menu_screen = 0;
                return;

            default:
                break;
        }

    }
    else if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE))
    {
        if (noBDVD != manager_cfg.noBDVD)
        {
            manager_cfg.noBDVD = noBDVD & 3;
            SaveManagerCfg();

            sys_fs_umount("/dev_bdvd");
            sys_fs_umount("/dev_ps2disc");

            if(noBDVD) Eject_BDVD(NOWAIT_BDVD | EJECT_BDVD);

            if(!manager_cfg.noBDVD)
                  sprintf(temp_buffer, "%s\n\n%s?", language[DRAWTOOLS_WITHBDVD], "Restarting the PS3...");
            else if(manager_cfg.noBDVD == MODE_DISCLESS)
                  sprintf(temp_buffer, "%s\n\n%s?", language[DRAWTOOLS_NOBDVD2], "Restarting the PS3...");
            else
                  sprintf(temp_buffer, "%s\n\n%s?", language[DRAWTOOLS_NOBDVD], "Restarting the PS3...");

            DrawDialogOKTimer(temp_buffer, 2500.0f);
            game_cfg.direct_boot = 0;

            ps3pad_read();

            if(new_pad & (BUTTON_CIRCLE | BUTTON_TRIANGLE)) exit(0);
            else
            {
                unlink_secure("/dev_hdd0/tmp/turnoff");
                fun_exit();
                sys_reboot();
            }
        }

        select_option = 0;
        menu_screen = 0; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        if(use_cobra)
        {
            ROT_DEC(select_option, 0, 8);
        }
        else
        {
            ROT_DEC(select_option, 0, 7);
        }
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        if(use_cobra)
        {
            ROT_INC(select_option, 8, 0);
        }
        else
        {
            ROT_INC(select_option, 7, 0);
        }
    }
}

void draw_cache_external()
{
    int menu = menu_screen;

    LoadCacheDatas();

    if(ncache_list > 0)
    {
        menu_screen = 4;
        select_option = 0;
    }
    else return;

    while(menu_screen != 0)
    {
        flash = (frame_count >> 5) & 1;

        frame_count++;
        cls();

        update_twat(1);
        menu_screen = 5;
        draw_cachesel(28, 0);
    }

    menu_screen = menu;
}

void draw_cachesel(float x, float y)
{

    int n;

    float y2, x2;

    SetCurrentFont(FONT_TTF);

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", language[DRAWCACHE_CACHE]);

    x2 = DrawFormatString(2000, 0, "hdd0: %.2fGB ", freeSpace[0]);
    x2 = 848 -(x2 - 2000) - x;
    DrawFormatString(x2, 0, "hdd0: %.2fGB ", freeSpace[0]);


    y += 24;

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);


    x2 = x;
    y2 = y + 32;

    for(n = (select_option / 8) * 8; (n < (select_option / 8) * 8 + 8); n++)
    {
        if(n < ncache_list)
        {
            sprintf(temp_buffer, "%s (%1.2f GB)", cache_list[n].title_id, ((double) cache_list[n].size) / GIGABYTES);
            DrawButton1_UTF8((848 - 520) / 2, y2, 520, temp_buffer, (flash && select_option == n));
        }
        else
            DrawButton1_UTF8((848 - 520) / 2, y2, 520, "", -1);

        y2+= 48;
    }

    SetCurrentFont(FONT_TTF);

    // draw game name

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);


    if(flash && cache_need_free != 0)
    {
        SetFontSize(20, 20);
        SetFontColor(0xffff00ff, 0x00000000);
        SetFontAutoCenter(1);
        DrawFormatString(0, y + 3 * 150 + 6, language[DRAWCACHE_ERRNEEDIT], cache_need_free);
        SetFontAutoCenter(0);
    }
    else if(select_option < ncache_list)
    {
        SetFontColor(0xffffffff, 0x00000000);

        utf8_truncate(cache_list[select_option].title, temp_buffer, 65);

        temp_buffer[65] = 0;

        if(strlen(temp_buffer) < 50)
            SetFontSize(18, 32);
        else
            SetFontSize(14, 32);

        SetFontAutoCenter(1);

        DrawFormatString(0, y + 3 * 150, temp_buffer);

        SetFontAutoCenter(0);
    }

    SetFontColor(0xffffffff, 0x00000000);


    tiny3d_Flip();
    ps3pad_read();

    if(new_pad & (BUTTON_CROSS))
    {
        if(select_option >= ncache_list) return;

        sprintf(temp_buffer, language[DRAWCACHE_ASKTODEL], cache_list[select_option].title_id);

        if(DrawDialogYesNo(temp_buffer) == 1)
        {
            sprintf(temp_buffer, "%s/cache/%s", self_path, cache_list[select_option].title_id);
            DeleteDirectory(temp_buffer);
            rmdir_secure(temp_buffer);
            LoadCacheDatas();

            if(ncache_list >= select_option) select_option = ncache_list - 1;

            if(ncache_list <= 0)
            {
                select_option = 0;
                menu_screen = 0;
            }
        }
        new_pad = 0;
        return;
    }
    else if(new_pad & BUTTON_TRIANGLE)
    {
        select_option = 0;
        menu_screen = 0; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, ncache_list - 1)
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, ncache_list - 1, 0);
    }

}

/******************************************************************************************************************************************************/
/* BDVDEMU FUNCTIONS                                                                                                                                  */
/******************************************************************************************************************************************************/

void unpatch_bdvdemu()
{
//LV2 Mount for 355 in his payload code

#ifdef PSDEBUG
    int flag = 0;
    flag =
#endif
    lv2_unpatch_bdvdemu();
}


int patch_bdvdemu(u32 flags)
{
    int n;
    int usb = -1;

    static int one = 0;

    if(one) return SUCCESS; // only one time


    if(((flags & 1) && game_cfg.bdemu     == 2) ||
      (!(flags & 1) && game_cfg.bdemu_ext == 2) || (flags & D_FLAG_BDVD) )
    {
        if(noBDVD == MODE_DISCLESS && !use_cobra) flags = NTFS_FLAG; else return SUCCESS; // new to add BD-Emu 2
    }

    one = 1;

    if(noBDVD && !use_cobra && (flags & NTFS_FLAG)) flags = 1;

    flags &= GAMELIST_FILTER;

    for(n = 1; n < 11; n++)
    {
        if(flags == (1 << n)) {usb = n - 1; break;}
    }

    if(usb == -1) bdvd_is_usb = 0; else bdvd_is_usb = 1;
/*
    if(usb < 0)
    {
        DrawDialogOK(language[PATCHBEMU_ERRNOUSB]);
        return FAILED;
    }
*/


    int save = noBDVD;

    if(use_cobra) noBDVD = 0;

    lv2_patch_bdvdemu(flags);

    noBDVD = save;

    return SUCCESS;
}

int move_origin_to_bdemubackup(char *path)
{
    if(strncmp(path, "/dev_usb00", 10) && strncmp(path, "/dev_hdd0", 9)) return 1;

    sprintf(temp_buffer, "%s/PS3_GAME/PS3PATH.BUP", path);
    sprintf(temp_buffer + 1024, "%s/PS3_GAME", path);

    if(SaveFile(temp_buffer, temp_buffer + 1024, strlen(temp_buffer + 1024)) != SUCCESS)
    {
        sprintf(temp_buffer + 1024, language[MOVEOBEMU_ERRSAVE], temp_buffer);
        DrawDialogOK(temp_buffer + 1024);

        return FAILED;
    }

    if(!strncmp(temp_buffer, "/dev_hdd0", 9)) strncpy(temp_buffer + 9, "/PS3_GAME", 16) ;
    else
        strncpy(temp_buffer + 11, "/PS3_GAME", 16);

    // sys8_perm_mode(1);
    int n = sysLv2FsRename(temp_buffer  + 1024, temp_buffer);
    // sys8_perm_mode(0);

    if(n != 0)
    {
        sprintf(temp_buffer + 256, language[MOVEOBEMU_ERRMOVE], temp_buffer);
        DrawDialogOK(temp_buffer + 256);

        return FAILED;
    }

    // PS3_GM01

    sprintf(temp_buffer, "%s/PS3_GM01/PS3PATH2.BUP", path);
    sprintf(temp_buffer + 1024, "%s/PS3_GM01", path);

    sysFSStat dstat;
    if(sysLv2FsStat(temp_buffer + 1024, &dstat) != SUCCESS)
    {
        sprintf(temp_buffer, "%s/PS3_GAME/PS3PATH.BUP", path);

        if(!strncmp(temp_buffer, "/dev_hdd0", 9)) strncpy(temp_buffer + 9, "/PS3_GAME", 16);
        else
            strncpy(temp_buffer + 11, "/PS3_GAME", 16);

        goto skip1;
    }

    if(SaveFile(temp_buffer, temp_buffer + 1024, strlen(temp_buffer + 1024)) != SUCCESS)
    {
        sprintf(temp_buffer + 1024, language[MOVEOBEMU_ERRSAVE], temp_buffer);
        DrawDialogOK(temp_buffer + 1024);

        return FAILED;
    }

    if(!strncmp(temp_buffer, "/dev_hdd0", 9)) strncpy(temp_buffer + 9, "/PS3_GM01", 16);
        else
            strncpy(temp_buffer + 11, "/PS3_GM01", 16);

    // sys8_perm_mode(1);
    n = sysLv2FsRename(temp_buffer  + 1024, temp_buffer);
    // sys8_perm_mode(0);

    if(n != SUCCESS)
    {
        sprintf(temp_buffer + 256, language[MOVEOBEMU_ERRMOVE], temp_buffer);
        DrawDialogOK(temp_buffer + 256);

        return FAILED;
     }

 skip1:

    if(autolaunch < LAUNCHMODE_STARTED)
    {
        sprintf(temp_buffer + 256, language[MOVEOBEMU_MOUNTOK], temp_buffer);
        DrawDialogOKTimer(temp_buffer + 256, 2000.0f);
    }

    return SUCCESS;
}

int move_bdemubackup_to_origin(u32 flags)
{
    int n;
    int usb = -1;

    static u32 olderrflags = 0;
    static u32 olderrflags2 = 0;

    flags &= GAMELIST_FILTER;

    for(n = 0; n < 11; n++)
    {
        if(flags == (1 << n)) {usb = n - 1; break;}
    }

    if(usb < 0)
    {
        sprintf(temp_buffer, "/dev_hdd0/PS3_GAME");
        sprintf(temp_buffer + 256, "/dev_hdd0/PS3_GAME/PS3PATH.BUP");
    }
    else
    {
        sprintf(temp_buffer, "/dev_usb00%c/PS3_GAME", 48 + usb);
        sprintf(temp_buffer + 256, "/dev_usb00%c/PS3_GAME/PS3PATH.BUP", 48 + usb);
    }

    int file_size;
    char *file;
    int ret = 0;

    sysFSStat dstat;
    if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS) {ret = -1; goto PS3_GM01;}

    file = LoadFile(temp_buffer + 256, &file_size);

    if(!file) {ret = -1; goto PS3_GM01;}

    memset(temp_buffer + 1024, 0, 0x420);

    if(file_size > 0x400) file_size = 0x400;

    memcpy(temp_buffer + 1024, file, file_size);

    free(file);

    for(n = 0; n < 0x400; n++)
    {
        if(temp_buffer[1024 + n] == 0) break;
        if(((u8)temp_buffer[1024 + n]) < 32) {temp_buffer[1024 + n] = 0; break;}
    }

    if(strncmp(temp_buffer, temp_buffer + 1024, 10))  {ret = -1;goto PS3_GM01;} // if not /dev_usb00x return

    if(!strncmp(temp_buffer, "/dev_hdd0", 9)) ;
    else
        memcpy(temp_buffer + 1024, temp_buffer, 11);

    // sys8_perm_mode(1);
    n = sysLv2FsRename(temp_buffer, temp_buffer + 1024);
    // sys8_perm_mode(0);

    if(n != SUCCESS)
    {
        if(!(olderrflags & flags))
        {
            sprintf(temp_buffer, language[MOVETBEMU_ERRMOVE], temp_buffer + 1024);
            DrawDialogOK(temp_buffer);
            olderrflags |= flags;
        }
        ret = FAILED; goto PS3_GM01;
    }

    // PS3_GM01

    PS3_GM01:

    if(usb < 0)
    {
        sprintf(temp_buffer, "/dev_hdd0/PS3_GM01");
        sprintf(temp_buffer + 256, "/dev_hdd0/PS3_GM01/PS3PATH2.BUP");
    }
    else
    {
        sprintf(temp_buffer, "/dev_usb00%c/PS3_GM01", 48 + usb);
        sprintf(temp_buffer + 256, "/dev_usb00%c/PS3_GM01/PS3PATH2.BUP", 48 + usb);
    }

    if(sysLv2FsStat(temp_buffer, &dstat) != SUCCESS) return FAILED;

    file = LoadFile(temp_buffer + 256, &file_size);

    if(!file) return FAILED;

    memset(temp_buffer + 1024, 0, 0x420);

    if(file_size > 0x400) file_size = 0x400;

    memcpy(temp_buffer + 1024, file, file_size);

    free(file);

    for(n = 0; n< 0x400; n++)
    {
        if(temp_buffer[1024 + n] == 0) break;
        if(((u8)temp_buffer[1024 + n]) < 32) {temp_buffer[1024 + n] = 0; break;}
    }

    if(strncmp(temp_buffer, temp_buffer + 1024, 10)) return FAILED; // if not /dev_usb00x return

    if(!strncmp(temp_buffer, "/dev_hdd0", 9)) ;
    else
        memcpy(temp_buffer + 1024, temp_buffer, 11);

    // sys8_perm_mode(1);
    n= sysLv2FsRename(temp_buffer, temp_buffer + 1024);
    // sys8_perm_mode(0);

    if(n != SUCCESS)
    {
        if(!(olderrflags2 & flags))
        {
            sprintf(temp_buffer, language[MOVETBEMU_ERRMOVE], temp_buffer + 1024);
            DrawDialogOK(temp_buffer);
            olderrflags2 |= flags;
        }
        return FAILED;
    }

    return ret;
}

//------- idps / console id ----------
s32 open_device( u64 device_ID, u32* fd )
{
    lv2syscall4( 600, device_ID, 0, (u64)fd, 0 );
    return_to_user_prog(s32);
}

s32 read_device( u32 fd, u64 start_read_offset, u64 byte_to_read, const void* buffer, u32 *number_byte_read, u64 flags )
{
    lv2syscall7( 602, fd, 0, start_read_offset, byte_to_read, (u64)buffer, (u64)number_byte_read, flags );
    return_to_user_prog(s32);
}

s32 close_device( u32 fd)
{
    lv2syscall1( 601, fd );
    return_to_user_prog(s32);
}

void get_psid_lv2()
{
    u64 uPSID[2] = {0,0};
    lv2syscall1(872, (u64) uPSID); //PSID

    if((off_psid > 0) && ((uPSID[0] == 0 && uPSID[1] == 0) || !strncmp(console_id, psid, 32)))
    {
        uPSID[0] = lv2peek( off_psid );
        uPSID[1] = lv2peek( off_psid + 8 );
    }

    snprintf( psid, 33, "%016llX%016llX", (long long unsigned int)uPSID[0], (long long unsigned int)uPSID[1] );
}

void get_console_id_lv2()
{
    u64 uIDPS[2] = {0,0};
    lv2syscall1(870, (u64) uIDPS); //IDPS

    if((off_idps > 0) && (uIDPS[0] == 0 && uIDPS[1] == 0))
    {
        uIDPS[0] = lv2peek( off_idps );
        uIDPS[1] = lv2peek( off_idps + 8 );
    }

    snprintf( console_id, 33, "%016llX%016llX", (long long unsigned int)uIDPS[0], (long long unsigned int)uIDPS[1] );
}

void set_psid_lv2()
{
    if(is_valid_psid() == false) {DrawDialogOKTimer("Invalid PSID", 2000.0f); return;}

    u64 uPSID[2] = {0,0};
    lv2syscall1(872, (u64) uPSID); //PSID

    if((off_psid > 0) && ((uPSID[0] == 0 && uPSID[1] == 0) || !strncmp(console_id, psid, 32)))
    {
        uPSID[0] = lv2peek( off_psid );
        uPSID[1] = lv2peek( off_psid + 8 );
    }

    get_psid_val(); //val_psid_part1 & val_psid_part2 from PSID[]

    if (uPSID[0] == 0 || uPSID[1] == 0 || val_psid_part1 == 0 || val_psid_part2 == 0)
    {
        DrawDialogOKTimer("Invalid PSID", 2000.0f);
        return;
    }

    for(u64 j = 0x8000000000000000ULL; j < 0x8000000000600000ULL; j++)
    {
        if((peekq(j) == uPSID[0]) && (peekq(j+8) == uPSID[1]))
        {
            if(uPSID[0] != val_idps_part1) pokeq(j, val_psid_part1);
            j+=8;
            if(uPSID[1] != val_idps_part2) pokeq(j, val_psid_part2);
            j+=8;
        }
    }
}

void set_console_id_lv2()
{
    if(is_valid_idps() == 0) {DrawDialogOKTimer("Invalid IDPS", 2000.0f); return;}

    u64 uIDPS[2] = {0,0};
    lv2syscall1(870, (u64) uIDPS); //IDPS

    if(uIDPS[0] == 0 && uIDPS[1] == 0 && off_idps > 0)
    {
        uIDPS[0] = lv2peek( off_idps );
        uIDPS[1] = lv2peek( off_idps + 8 );
    }

    get_console_id_val(); //val_idps_part1 & val_idps_part2 from IDPS[]

    if (uIDPS[0] == 0 || uIDPS[1] == 0 || val_idps_part1 == 0 || val_idps_part2 == 0)
    {
        DrawDialogOKTimer("Invalid IDPS", 2000.0f);
        return;
    }

    for(u64 j = 0x8000000000000000ULL; j < 0x8000000000600000ULL; j++)
    {
        if((peekq(j) == uIDPS[0]) && (peekq(j+8) == uIDPS[1]))
        {
            if(uIDPS[0] != val_idps_part1) pokeq(j, val_idps_part1);
            j+=8;
            if(uIDPS[1] != val_idps_part2) pokeq(j, val_idps_part2);
            j+=8;
        }
    }
}

void get_console_id_eid5()
{
    u32 source, read;
    u64 offset_c;
    u64 buffer[ 0x40 ];
    int ret = 666;

    ret = open_device( 0x100000000000004ull, &source );

    if( ret != SUCCESS ) //PS3 has nand
    {
        offset_c = 0x20D;  //0x20d * 0x200
        close_device( source );
        open_device( 0x100000000000001ull, &source );
    }
    else            //PS3 has nor
        offset_c = 0x181;  //0x181 * 0x200

    read_device( source, offset_c, 0x1, buffer, &read, 0x22 );

    snprintf( console_id, 33, "%016llX%016llX", (long long unsigned int)buffer[ 0x3a ], (long long unsigned int)buffer[ 0x3b ] );

    close_device( source );
}

void get_psid_val()
{
    char tmp[ 17 ], tmp2[ 17 ];
    int i, y;

    for( i = 0, y = 0; i <= 31; i++ )
    {
        if( i > 15 )
        {
            tmp2[ y ] = psid[ i ];
            y++;
        }
        else
            tmp[ i ] = psid[ i ];
    }
    tmp[ 16 ] = '\0';

    val_psid_part1 = string_to_ull( tmp );
    val_psid_part2 = string_to_ull( tmp2 );
}

void get_console_id_val()
{
    char tmp[ 17 ], tmp2[ 17 ];
    int i, y;

    for( i = 0, y = 0; i <= 31; i++ )
    {
        if( i > 15 )
        {
            tmp2[ y ] = console_id[ i ];
            y++;
        }
        else
            tmp[ i ] = console_id[ i ];
    }
    tmp[ 16 ] = '\0';

    val_idps_part1 = string_to_ull( tmp );
    val_idps_part2 = string_to_ull( tmp2 );
}

int get_psid_keyb()
{
    if( Get_OSK_String_no_lang("PSID:", psid, 32) == 0 )
    {
        if(is_valid_psid())
            return SUCCESS;
        else
            return FAILED;
    }
    else
        return 1;
}

int get_console_id_keyb()
{
    if( Get_OSK_String_no_lang("Console ID:", console_id, 32) == 0 )
    {
        if(is_valid_idps())
            return SUCCESS;
        else
            return FAILED;
    }
    else
        return 1;
}

bool is_valid_psid()
{
    for(int i = 0; i <= 31; i++)
      if (is_hex_char(psid[i]) == false) return false;
    return true;
}

int is_valid_idps()
{
    if( (console_id[0] == '0' && console_id[1] == '0' && console_id[2] == '0' && console_id[3] == '0' &&
         console_id[4] == '0' && console_id[5] == '0' && console_id[6] == '0' && console_id[7] == '1' &&
         console_id[8] == '0' && console_id[9] == '0' && console_id[10]== '8' &&
         is_hex_char(console_id[11]) != 0 &&
         console_id[12]== '0' && console_id[13]== '0' && console_id[14]== '0') &&
         is_hex_char(console_id[15]) != 0 &&
        (console_id[16]== '0' || console_id[16]== '1' || console_id[16]== 'F' || console_id[16]== 'f' ) &&
        (console_id[17]== '0' || console_id[17]== '4' ))
    {
        for(int i = 18; i <= 31; i++)
        {
          if (is_hex_char(console_id[i]) == 0) return false;
        }
        return true;
    }
    return false;
}

bool is_hex_char(char c)
{
  if(c == 'A' || c == 'a' || c == '1' || c == '6' ||
     c == 'B' || c == 'b' || c == '2' || c == '7' ||
     c == 'C' || c == 'c' || c == '3' || c == '8' ||
     c == 'D' || c == 'd' || c == '4' || c == '9' ||
     c == 'E' || c == 'e' || c == '5' || c == '0' ||
     c == 'F' || c == 'f' ) return true;
  return false;
}

int save_spoofed_psid()
{
    sprintf(temp_buffer, "%s/config/psid", self_path);
    return SaveFile(temp_buffer, (char *) &psid, sizeof(psid) - 1);
}

int load_spoofed_psid()
{
    sprintf(temp_buffer, "%s/config/psid", self_path);

    int file_size;
    char *file = LoadFile(temp_buffer, &file_size);

    if(!file) return FAILED;
    if(file_size < 32) {free(file); return FAILED;}

    char tmp_psid[33];
    memcpy(tmp_psid, file, 32);
    tmp_psid[32] = 0;

    if( strcmp(psid, tmp_psid) == SUCCESS )
    {
        free(file);
        return 1;
    }

    memcpy(psid, file, 32);
    psid[32] = 0;

    set_psid_lv2();
    free(file);
    return SUCCESS;
}

int save_spoofed_console_id()
{
    sprintf(temp_buffer, "%s/config/idps", self_path);
    return SaveFile(temp_buffer, (char *) &console_id, sizeof(console_id) - 1);
}

int load_spoofed_console_id()
{
    sprintf(temp_buffer, "%s/config/idps", self_path);

    int file_size;
    char *file = LoadFile(temp_buffer, &file_size);

    if(!file) return FAILED;
    if(file_size < 32) {free(file); return FAILED;}

    char tmp_console_id[33];
    memcpy(tmp_console_id, file, 32);
    tmp_console_id[32] = 0;

    if( strcmp(console_id, tmp_console_id) == SUCCESS )
    {
        free(file);
        return 1;
    }

    memcpy(console_id, file, 32);
    console_id[32] = 0;

    set_console_id_lv2();
    free(file);
    return SUCCESS;
}

void draw_console_id_tools(float x, float y)
{
    float y2, x2;
    int r;

    SetCurrentFont(FONT_TTF);
    get_console_id_lv2();

    // header title

    DrawBox(x, y, 0, 200 * 4 - 8, 20, 0x00000028);

    SetFontColor(0xffffffff, 0x00000000);

    SetFontSize(18, 20);

    SetFontAutoCenter(0);

    DrawFormatString(x, y, " %s", "Spoof console id");

    y += 24;

    DrawBox(x, y, 0, 200 * 4 - 8, 150 * 3 - 8, 0x00000028);

    x2 = x;
    y2 = y + 32;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Set new PSID", (flash && select_option == 0));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Set new console id", (flash && select_option == 1));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Load PSID & console id", (flash && select_option == 2));

    y2+= 48;

    DrawButton1_UTF8((848 - 520) / 2, y2, 520, "Restore default console id", (flash && select_option == 3));


    for(int n = 0; n < 5; n++) y2+= 48;

    DrawBox(x, y + 3 * 150, 0, 200 * 4 - 8, 40, 0x00000028);

    // draw sys version
    SetCurrentFont(FONT_TTF);

    SetFontColor(0xccccffff, 0x00000000);
    SetFontSize(18, 20);
    SetFontAutoCenter(1);
    DrawFormatString(0, y2 + 40, payload_str );
    SetFontAutoCenter(0);

    //Display Console id
    SetCurrentFont(FONT_TTF);

    SetFontColor(0xeeffaaff, 0x00000000);
    SetFontSize(14, 16);
    SetFontAutoCenter(1);
    get_console_id_lv2();
    DrawFormatString(500, y2, "Console id: %s", console_id );
    SetFontAutoCenter(0);

    if(strncmp(console_id, psid, 32))
    {
        SetFontAutoCenter(1);
        get_psid_lv2();
        DrawFormatString(500, y2 - 32, "PSID: %s", psid );
    }

    tiny3d_Flip();

    ps3pad_read();

    if(new_pad & BUTTON_CROSS)
    {
        switch(select_option)
        {
            case 0: //set new psid
                if( get_psid_keyb() == 0 )
                {
                    set_psid_lv2();
                    save_spoofed_psid();
                    sprintf(temp_buffer, "PSID has been set to:\n\n%s", psid);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                break;

            case 1: //set new console id
                if( get_console_id_keyb() == 0 )
                {
                    set_console_id_lv2();
                    save_spoofed_console_id();
                    sprintf(temp_buffer, "Console ID has been set to:\n\n%s", console_id);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                break;

            case 2: //load console id
                r = load_spoofed_console_id();
                if(r == SUCCESS)
                {
                    sprintf(temp_buffer, "Console ID has been set to:\n\n%s", console_id);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                else if(r  == 1)
                {
                    sprintf(temp_buffer, "Console ID is already set:\n\n%s", console_id);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                else
                {
                    DrawDialogOKTimer("idps file was not found in config folder.", 3000.0f);
                }

                r = load_spoofed_psid();
                if(r == SUCCESS)
                {
                    sprintf(temp_buffer, "PSID has been set to:\n\n%s", psid);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                else if(r  == 1)
                {
                    sprintf(temp_buffer, "PSID is already set:\n\n%s", psid);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                }
                else
                {
                    DrawDialogOKTimer("psid file was not found in config folder.", 3000.0f);
                }

                break;

            case 3: //restore default console id
                sprintf(temp_buffer, "%s/config/idps", self_path);
                unlink_secure(temp_buffer);

                sprintf(temp_buffer, "%s/config/psid", self_path);
                unlink_secure(temp_buffer);

                if( strcmp( console_id, default_console_id ) == 0 )
                {
                    sprintf(temp_buffer, "Console ID is already the default:\n\n%s", console_id);
                    DrawDialogOKTimer(temp_buffer, 3000.0f);
                    break; //default idps is already set
                }

                strcpy( console_id, default_console_id );
                set_console_id_lv2();

                sprintf(temp_buffer, "The default console id has been restored.\n\nDo you want to save the default console id?\n\nConsole ID: %s", console_id);
                if(DrawDialogYesNoDefaultYes(temp_buffer) == 1)
                {
                    save_spoofed_console_id();
                }
                break;

            default:
               break;
        }
    }

    if(new_pad & (BUTTON_TRIANGLE | BUTTON_CIRCLE))
    {
        menu_screen = 0; return;
    }
    else if(new_pad & BUTTON_UP)
    {
        frame_count = 32;
        ROT_DEC(select_option, 0, 3);
    }
    else if(new_pad & BUTTON_DOWN)
    {
        frame_count = 32;
        ROT_INC(select_option, 3, 0);
    }

}
//------------------------------------
