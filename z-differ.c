/*
This utility creates a diff -- or patch -- between two z-code files.
*/

#define VERSION "1.0"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Compiler check that unsigned long ints are at least 4 bytes.

#if (ULONG_MAX < 4294967295)
#error "Code requires that unsigned long ints be at least 32 bits."
#endif

typedef struct {
    unsigned int zcode_version;
    unsigned int release;
    unsigned char version[6];
    unsigned long int filesize;
} FILE_DATA;

char game[33];
char *source_name, *target_name, *output_name;
FILE *source_file, *target_file, *output_file;
FILE_DATA source_data, target_data;

void open_files(void);
void verify_files(void);
void close_files(void);
void extract_game_name(int argv, char *argc[]);
void get_version_details(void);
void retrieve_details(FILE_DATA *data, FILE *f, char *fileDesc, char *name);
void create_patch(void);
void print_version(FILE_DATA *d);
void print_hex_digit(int d);

void open_files() {
    if ((target_file = fopen(target_name, "rb")) == NULL) {
        fprintf(stderr, "Unable to open target file '%s'\n", target_name);
        exit(1);
    }

    if ((source_file = fopen(source_name, "rb")) == NULL) {
        fclose(target_file);
        fprintf(stderr, "Unable to open source file '%s'\n", source_name);
        exit(1);
    }

    if ((output_file = fopen(output_name, "wb")) == NULL) {
        fclose(target_file);
        fclose(source_file);
        fprintf(stderr, "Unable to output patch file '%s'\n", output_name);
        exit(1);
    }
}

void close_files() {
    fclose(target_file);
    fclose(source_file);
    fclose(output_file);
}

void verify_files() {
    int c1, c2;

    // Get the first byte from the files.

    c1 = fgetc(target_file);
    c2 = fgetc(source_file);

    // If either c1 or c2 is not in the range 1 to 8, then the file is not
    // a valid Infocom z-code file.

    if ((c1 < 1) || (c1 > 8)) {
        fprintf(stderr, "Target file '%s' is not a valid z-code file.\n", target_name);
    }

    if ((c2 < 1) || (c2 > 8)) {
        fprintf(stderr, "Source file '%s' is not a valid z-code file.\n", source_name);
    }

    if ((c1 < 1) || (c1 > 8) || (c2 < 1) || (c2 > 8)) {
        close_files();
        exit(1);
    }
}

void extract_game_name(int argc, char *argv[]) {
    int counter;

    // First, make sure the name holder is cleared.

    for (counter = 0; counter < 34; counter++) {
        game[counter] = '\0';
    }

    // Set the parameter index for the first word of game name.

    counter = 4;

    // Extract words from the command line.

    while (counter < argc) {
        // The first check is for whether the first word begins with a quote
        // character ("). If so, it's ignored when copying.
        if ((counter == 4) && argv[counter][0] == '"') {
            if (strlen(argv[4]) > 32) {
                return;
            }
            strcpy(game, &argv[4][1]);
        } else {
            if (strlen(game) + 1 + strlen(argv[counter]) > 32) {
                return;
            }

            if (counter > 4) {
                strcat(game, " ");
            }

            strcat(game, argv[counter]);
        }

        ++counter;
    }

    if ((strlen(game) > 0) && (game[strlen(game) - 1] == '"')) {
        game[strlen(game) - 1] = '\0';
    }
}

void retrieve_details(FILE_DATA *data, FILE *f, char *fileDesc, char *fname) {
    unsigned int c, ctr;

    // Get the zcode version number
    fseek(f, 0, SEEK_SET);
    data->zcode_version = fgetc(f);

    // Get release number
    fseek(f, 2, SEEK_SET);

    // Get in HI byte of release and move into release value.
    c = fgetc(f);
    data->release = c*256;

    // Get in LO byte of release and move into release value.
    c = fgetc(f);
    data->release += c;

    // Get version
    fseek(f, 0x12, SEEK_SET);
    for (ctr = 0; ctr < 6; ++ctr) {
        data->version[ctr] = (char) fgetc(f);
    }

    // Get the filesize.
    fseek(f, 0x1A, SEEK_SET);

    // Get HI byte of compressed filesize and store in filesize.
    c = fgetc(f);
    data->filesize = ((unsigned int) c) << 8;

    // Get LO byte of compressed filesize and store in filesize.
    c = fgetc(f);
    data->filesize += ((unsigned int) c);

    // Apply filesize multiplier.
    switch(data->zcode_version) {
        case 1: case 2: case 3:
            data->filesize *= 2; break;
        case 4: case 5:
            data->filesize *= 4; break;
        case 6: case 7: case 8:
            data->filesize *= 8; break;
    }

    // Check if the filesize is zero.
    if (data->filesize == 0) {
        printf("Warning: %s '%s' filesize not specified, using entire file.\n", fileDesc, fname);
        printf("Note that if there's padding on the file, the patch file will be inaccurate.\n\n");

        // Work out the physical file size.
        fseek(f, 0, SEEK_END);
        data->filesize = ftell(f);
    }

    // Reset back to the start of the gamefile.
    fseek(f, 0, SEEK_SET);
}

void get_version_details() {
    retrieve_details(&target_data, target_file, "Target File", target_name);
    retrieve_details(&source_data, source_file, "Source File", source_name);
}

void print_hex_digit(int d) {
    char digits[16] = "0123456789ABCDEF";
    printf("%c", digits[d]);
}

void print_version(FILE_DATA *d) {
    // Prints out the version given a data structure.
    // Format: release/version [vzcode-version]
    int ctr;
    int printable;

    printf("%d/", d->release);

    printable = 1;

    // Check to make sure all 6 version digits are printable. If not, print
    // all six in hexadecimal format.

    for (ctr = 0; (ctr < 6) && (printable); ctr++) {
        if ((printable) && ((d->version[ctr] < ' ') || (d->version[ctr] > '\127'))) {
            printable = 0;
        }
    }

    // Print digits depending on resulting printable flag.

    if (printable) {
        for (ctr = 0; ctr < 6; ctr++) {
            printf("%c", d->version[ctr]);
        }
    } else {
        printf("$");
        for (ctr = 0; ctr < 6; ctr++) {
            print_hex_digit(((unsigned char) d->version[ctr]) / 16);
            print_hex_digit(((unsigned char) d->version[ctr]) % 16);
        }
    }

    printf(" [v%d]", d->zcode_version);
}

void create_patch() {
    int ctr;
    unsigned long int target_file_pos, source_file_pos;
    unsigned int c1, c2;

    printf("Diff'ing game \"%s\" ", game);
    print_version(&target_data);
    printf("\nusing ");
    print_version(&source_data);
    printf(" as source.\n\n");

    // Write header details out.

    // Patch file identifier.
    fputs("PFG", output_file);

    // Gamefile name.
    for (ctr = 0; ctr < 32; ctr++) {
        fputc(game[ctr], output_file);
    }

    // Write input file version.
    fputc(target_data.zcode_version, output_file);
    fputc(target_data.release / 256, output_file);
    fputc(target_data.release % 256, output_file);

    for (ctr = 0; ctr < 6; ctr++) {
        fputc(target_data.version[ctr], output_file);
    }

    // Write encryption file version.
    fputc(source_data.zcode_version, output_file);
    fputc(source_data.release / 256, output_file);
    fputc(source_data.release % 256, output_file);

    for (ctr = 0; ctr < 6; ctr++) {
        fputc(source_data.version[ctr], output_file);
    }

    // Main loop for patching the file.
    target_file_pos = 0;
    source_file_pos = 0;

    for (;;) {
        // Read a byte in from both the target file and source file.
        if ((c1 = fgetc(target_file)) == (unsigned int) -1) {
            // The targetfile EOF passed. This should not happen even if a
            // filesize is not embedded in the gamefile, since the
            // initialization code simply takes the physical filesize anyway.
            fprintf(stderr, "EOF passed in Target File '%s'. Likely due to incorrrectly calculated filesize.\n\n", target_name);
            return;
        }

        if ((c2 = fgetc(source_file)) == (unsigned int) -1) {
            // Source file EOF passed. Again, this should never happen.
            fprintf(stderr, "EOF passed in Source File '%s'. Likely due to incorrrectly calculated filesize.\n\n", source_name);

            // Should this be here?
            //return;
        }

        // Patch the byte via XOR and write it out to the output file.
        fputc((char) (c1 ^ c2), output_file);
        ++target_file_pos;
        ++source_file_pos;

        // If reached the end of input gamefile's specified filesize, then
        // then finish the patching.
        if (target_file_pos >= target_data.filesize) {
            return;
        }

        // If the encryption file position moves past that specified in the
        // header, reset back to the file start. This ensures that files
        // with padding bytes can still be used for the actual patching
        // process.
        if (source_file_pos >= source_data.filesize) {
            source_file_pos = 0;
            fseek(source_file, 0, SEEK_SET);
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Z-DIFFER version " VERSION "\n\n");

    if (argc < 4) {
        printf("Produces a diff (patch file) between two Infocom game files.\n\n");
        printf("z-differ source_file.dat target_file.dat output_file.pat [name]\n\n");
        printf("  source_file.dat   Source file to use (from this)\n");
        printf("  target_file.dat   Target file to use (to this)\n");
        printf("  output_file.pat   Output name for resulting patch file\n");
        printf("  name              An optional name for the file\n\n");
        printf("The target_file.dat can later be regenerated with the command:\n\n");
        printf("z-patcher output_file.pat source_file.dat target_file.dat\n");
        exit(1);
    }

    source_name = argv[1];
    target_name = argv[2];
    output_name = argv[3];

    open_files();

    verify_files();

    extract_game_name(argc, argv);

    get_version_details();

    create_patch();

    close_files();

    printf("Patch produced successfully.\n");

    return 0;
}
