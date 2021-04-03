/*
   Simple DOS 3.3 SELECT Clone

   Copyright (c) 2021 Ricardo Hanke
   Released under the terms of the GNU General Public License.
   See the file 'COPYING' in the main directory for details.
*/

#include <stdio.h>
#include <process.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <fcntl.h>
#include <direct.h>

static char copy_buffer[4096];



unsigned CopyFile(const char *src_file, const char *dst_file)
{
    unsigned bytes_read;
    unsigned bytes_written;
    unsigned result;
    int src_handle;
    int dst_handle;

    if ((result = _dos_open(src_file, O_RDONLY, &src_handle)) == 0)
    {
        if ((result = _dos_creat(dst_file, _A_NORMAL, &dst_handle)) == 0)
        {
            while ((result = _dos_read(src_handle, &copy_buffer, sizeof(copy_buffer), &bytes_read)) == 0 && bytes_read > 0)
            {
                if ((result = _dos_write(dst_handle, copy_buffer, bytes_read, &bytes_written)) != 0)
                {
                    break;
                }
            }

            _dos_close(dst_handle);
        }

        _dos_close(src_handle);
    }

    return result;
}



int wait_for_keystroke(void)
{
    int key;

    if ((key = _getch()) == 0)
    {
        key = _getch();
    }

    return key;
}



int prompt_to_continue(void)
{
    int key;
    
    printf("\n");
    printf("SELECT is used to install DOS the first\n");
    printf("time. SELECT erases everything on the\n");
    printf("specified target and then installs DOS.\n");
    printf("Do you want to continue (Y/N) ");

    fflush(stdout);

    while (key = wait_for_keystroke())
    {
        printf("%c\b", key);
        fflush(stdout);
        
        switch (key)
        {
            case 'n':
            case 'N':
            {
                return 0;
            }

            case 'y':
            case 'Y':
            {
                return 1;
            }

            default:
            {
                break;
            }
        }
    }

    return 0;
}



int create_config_file(const char *filename, const char *country, const char *sysroot)
{
    FILE *fp = fopen(filename, "w");

    if (fp != NULL)
    {
        fprintf(fp, "DEVICE=%s\\SETVER.SYS\n", sysroot);
        fprintf(fp, "DOS=HIGH\n");
        fprintf(fp, "COUNTRY=%s,437,%s\\COUNTRY.SYS\n", country, sysroot);
        fprintf(fp, "FILES=30\n");
        fclose(fp);
    }
    else
    {
        return -1;
    }
    
    return 0;
}



int create_autoexec_file(const char *filename, const char *keycode, const char *sysroot)
{
    FILE *fp = fopen(filename, "w");

    if (fp != NULL)
    {
        fprintf(fp, "@ECHO OFF\n");
        fprintf(fp, "PROMPT $p$g\n");
        fprintf(fp, "PATH %s\n", sysroot);
        fprintf(fp, "SET TEMP=%s\n", sysroot);
        fclose(fp);
    }
    else
    {
        return -1;
    }
    
    return 0;
}



int main(int argc, char *argv[])
{
    char source[_MAX_PATH];
    char dstdrv[_MAX_PATH];
    char buffer[_MAX_PATH];
    char src_buffer[_MAX_PATH];
    char dst_buffer[_MAX_PATH];
    struct find_t finfo;
    unsigned result;


    if (prompt_to_continue() == 0)
    {
        printf("\n\n");
        return 0;
    }

    if (argc != 4)
    {
        printf("\n\n\nIncorrect number of parameters\n");
        return 0;
    }

    _splitpath(argv[0], source, NULL, NULL, NULL);
    _splitpath(argv[1], dstdrv, NULL, NULL, NULL);

    if (spawnl(P_WAIT, "FORMAT.COM", "FORMAT.COM", dstdrv, "/S", NULL) == -1)
    {
        printf("\n\n\nCannot execute FORMAT.COM\n");
        return 0;
    }

    if (mkdir(argv[1]) != 0)
    {
        printf("\n\n\nCannot create target directory\n");
        return 0;
    }

    strcpy(src_buffer, source);
    strcat(src_buffer, "\\*.*");

    printf("\n\n");

    for (result = _dos_findfirst(src_buffer, _A_NORMAL & _A_RDONLY & _A_ARCH, &finfo); result == 0; result = _dos_findnext(&finfo))
    {
        strcpy(src_buffer, source);
        strcat(src_buffer,"\\");
        strcat(src_buffer, finfo.name);

        strcpy(dst_buffer, argv[1]);
        strcat(dst_buffer, "\\");
        strcat(dst_buffer, finfo.name);

        printf("%s\n", finfo.name);

        if (CopyFile(src_buffer, dst_buffer) != 0)
        {
            printf("\n\n\nCannot copy %s\n", finfo.name);
            return 0;
        }
    }

    _dos_findclose(&finfo);

    strcpy(buffer, dstdrv);
    strcat(buffer, "\\CONFIG.SYS");
    
    if (create_config_file(buffer, argv[2], argv[1]) == -1)
    {
        printf("\n\n\nCannot create CONFIG.SYS\n");
        return 0;
    }

    strcpy(buffer, dstdrv);
    strcat(buffer, "\\AUTOEXEC.BAT");

    if (create_autoexec_file(buffer, argv[3], argv[1]) == -1)
    {
        printf("\n\n\nCannot create AUTOEXEC.BAT\n");
        return 0;
    }

    return 0;
}

