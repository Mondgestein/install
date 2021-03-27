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
        fprintf(fp, "DEVICE=%sSETVER.SYS\n", sysroot);
        fprintf(fp, "DOS=HIGH\n");
        fprintf(fp, "COUNTRY=%s,437,%sCOUNTRY.SYS\n", country, sysroot);
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

    strcat(source, "\\*.*");

    if (spawnl(P_WAIT, "FORMAT.COM", "FORMAT.COM", dstdrv, "/S", NULL) == -1)
    {
        printf("\n\n\nCannot execute FORMAT.COM\n");
        return 0;
    }

    if (spawnl(P_WAIT, "XCOPY.EXE", "XCOPY.EXE", source, argv[1], NULL) == -1)
    {
        printf("\n\n\nCannot execute XCOPY.EXE\n");
        return 0;
    }

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

