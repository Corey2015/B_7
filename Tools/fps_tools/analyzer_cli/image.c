// NOTE: fwrite() in VC6 is very strange, so we use Windows way instead of the standard.
#if defined(__WINDOWS__)
    #include <windows.h>
#elif defined(__LINUX__)
    #include <stdio.h>
#endif

#include "common.h"


int
save_bmp(char    *path,
         uint8_t *img,
         size_t  size)
{
#if defined(__WINDOWS__)
	HANDLE fh;
	DWORD  written;
#elif defined(__LINUX__)
    FILE *fp = NULL;
#endif

    uint8_t file_header[] = {
        0x42, 0x4D,              // 'B' 'M'
        0x36, 0x04, 0x00, 0x00,  // File size in bytes
        0x00, 0x00,              // Reserved
        0x00, 0x00,              // Reserved
        0x36, 0x04, 0x00, 0x00,  // Image offset in bytes
    };

    uint8_t info_header[] = {
        0x28, 0x00, 0x00, 0x00,  // Info header size in bytes
        0x90, 0x00, 0x00, 0x00,  // Image width in pixels
        0x40, 0x00, 0x00, 0x00,  // Image height in pixels
        0x01, 0x00,              // Number of color planes
        0x08, 0x00,              // Bits per pixel
        0x00, 0x00, 0x00, 0x00,  // Image size in bytes
        0x00, 0x00, 0x00, 0x00,  // Compression
        0xE5, 0x4C, 0x00, 0x00,  // X resolution
        0xE5, 0x4C, 0x00, 0x00,  // Y resolution
        0x00, 0x01, 0x00, 0x00,  // Number of colors
        0x00, 0x00, 0x00, 0x00   // Important colors
    };

    uint8_t color_table[256 * 4];
    int     i;

    for (i = 0; i < 256; i++) {
        color_table[(i * 4) + 0] = i;
        color_table[(i * 4) + 1] = i;
        color_table[(i * 4) + 2] = i;
        color_table[(i * 4) + 3] = 0x00;
    }

#if defined(__WINDOWS__)
	fh = CreateFile((LPCTSTR) path, 
		            (GENERIC_READ | GENERIC_WRITE), 
		            (DWORD) 0, 
		            NULL, 
		            CREATE_ALWAYS, 
		            FILE_ATTRIBUTE_NORMAL, 
		            (HANDLE) NULL); 
	if (fh == INVALID_HANDLE_VALUE) {
		return -1;
	}

	WriteFile(fh, (LPVOID) file_header, sizeof(file_header), &written, NULL);
	WriteFile(fh, (LPVOID) info_header, sizeof(info_header), &written, NULL);
	WriteFile(fh, (LPVOID) color_table, sizeof(color_table), &written, NULL);
	WriteFile(fh, (LPVOID) img, size, &written, NULL);
	CloseHandle(fh);
	return 0;
#elif defined(__LINUX__)
    fp = fopen((char *) path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fwrite(file_header, 1, sizeof(file_header), fp);
    fwrite(info_header, 1, sizeof(info_header), fp);
    fwrite(color_table, 1, sizeof(color_table), fp);
    fwrite(img, 1, size, fp);
    fclose(fp);
    return 0;
#endif
}
