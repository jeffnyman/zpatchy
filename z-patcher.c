/*
This utility applies a patch file to a source z-code program file to produce
a target z-code program file.
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
} FILE_DATA;

unsigned long int source_file_size;

char game[33];
char *patch_name ,*source_name, *target_name;
FILE *patch_file, *source_file, *target_file;
FILE_DATA source_data, target_data;

void verify_patch_file(void);
void verify_source_file(void);
void verify_target_file(void);
void open_files(void);
void close_files(void);
void get_details(void);
void perform_patch_file(void);
void retrieve_details(FILE_DATA *data);
void print_hex_digit(FILE *stream, int d);
void print_version(FILE *s, FILE_DATA *d);

void close_files() {
    fclose(patch_file);

    if (target_file) {
        fclose(target_file);
    }

    if (source_file) {
        fclose(source_file);
    }
}

void verify_patch_file() {
    int c;
    c = fgetc(patch_file);

    if (c == 'P') {
        c = fgetc(patch_file);

        if (c == 'F') {
            c = fgetc(patch_file);

            if (c == 'G') {
                return;
            }
        }
    }

    close_files();
    fprintf(stderr, "'%s' is not a valid Infocom patch file.\n\n", patch_name);
    exit(1);
}

void verify_source_file() {
    int c;
    c = fgetc(source_file);

    if ((c < 1) || (c > 8)) {
        fprintf(stderr, "Source file '%s' is not a valid gamefile.\n\n", source_name);
        close_files();
        exit(1);
    }

    fseek(source_file, 0, SEEK_SET);
}

void verify_target_file() {
    unsigned int c, ctr;

    fseek(source_file, 0, SEEK_SET);
    c = fgetc(source_file);

    if (c != source_data.zcode_version) goto Error;

    fseek(source_file, 2, SEEK_SET);
    c = fgetc(source_file);

    if (c != (source_data.release / 256)) goto Error;

    c = fgetc(source_file);

    if (c != (source_data.release % 256)) goto Error;

    fseek(source_file, 0x12, SEEK_SET);

    for (ctr = 0; ctr < 6; ctr++) {
        c = fgetc(source_file);
        if (c != source_data.version[ctr]) goto Error;
    }

    fseek(source_file, 0x1A, SEEK_SET);
    source_file_size = 256 * ((unsigned int) fgetc(source_file));
    source_file_size += (unsigned int) fgetc(source_file);

    switch (source_data.zcode_version) {
        case 1: case 2: case 3:
            source_file_size *= 2;
            break;
        case 4: case 5:
            source_file_size *= 4;
            break;
        case 6: case 7: case 8:
            source_file_size *= 8;
            break;
    }

    if (source_file_size == 0) {
        fprintf(stderr, "Warning: Source file '%s' does not have an embedded file size, ", source_name);
        fprintf(stderr, "so the entire file\n");
        fprintf(stderr, "Will be used. You should make sure the gamefile does not have ");
        fprintf(stderr, "any padding, or an\n");
        fprintf(stderr, "inaccurate output file may be generated.\n\n");

        fseek(source_file, 0, SEEK_END);
        source_file_size = ftell(source_file);
    }

    fseek(source_file, 0, SEEK_SET);
    return;

Error:
    close_files();
    fprintf(stderr, "Target file '%s' does not match the specification in the header of\n", target_name);
    fprintf(stderr, "patch file '%s'. Make sure source file '%s'\n", patch_name, source_name);
    fprintf(stderr, "is actually version ");
    print_version(stderr, &source_data);
    fprintf(stderr, " of \"%s\".\n", game);
    exit(1);
}

void retrieve_details(FILE_DATA *data) {
    int c, ctr;

    data->zcode_version = fgetc(patch_file);

    // Get in HI byte of release and move into release number.
    c = fgetc(patch_file);
    data->release = c*256;

    // Get in LO byte of release and add into release value.
    c = fgetc(patch_file);
    data->release += c;

    for (ctr=0; ctr<6; ++ctr) {
        data->version[ctr] = (char) fgetc(patch_file);
    }
}

void get_details() {
    int ctr;

    for (ctr = 0; ctr < 32; ctr++) {
        game[ctr] = (char) fgetc(patch_file);
    }

    retrieve_details(&target_data);
    retrieve_details(&source_data);
}

void print_hex_digit(FILE *stream, int d) {
    // Prints out the version given a data structure.
    // Format: release/version [vzcode-version]
    char digits[16] = "0123456789ABCDEF";
    fprintf(stream, "%c", digits[d]);
}

void print_version(FILE *stream, FILE_DATA *d) {
    int ctr;
    int printable;

    fprintf(stream, "%d/", d->release);
    printable = 1;

    // Check to make sure all 6 version digits are printable. If not, print
    // all six in hexadecimal format.

    for (ctr = 0; (ctr < 6) && (printable); ctr++) {
        if ((printable) && ((d->version[ctr] < ' ') || (d->version[ctr] > '\127'))) {
            printable = 0;
        }
    }

    if (printable) {
        for (ctr = 0; ctr < 6; ctr++) {
            fprintf(stream, "%c", d->version[ctr]);
        }
    } else {
        fprintf(stream, "$");
        for (ctr = 0; ctr < 6; ctr++) {
            print_hex_digit(stream, ((unsigned char) d->version[ctr]) / 16);
            print_hex_digit(stream, ((unsigned char) d->version[ctr]) % 16);
        }
    }

    fprintf(stream, " [v%d]", d->zcode_version);
}

void open_files() {
    if ((patch_file = fopen(patch_name, "rb")) == NULL) {
        fprintf(stderr, "Could not open patch file '%s'.\n", patch_name);
        exit(1);
    }

    if ((source_file = fopen(source_name, "rb")) == NULL) {
        fclose(patch_file);
        fprintf(stderr, "Could not open source file '%s'.\n", source_name);
        exit(1);
    }

    if ((target_file = fopen(target_name, "wb")) == NULL) {
        fclose(patch_file);
        fclose(source_file);
        fprintf(stderr, "Could not open target file '%s'.\n", target_name);
        exit(1);
    }
}

void perform_patch_file() {
    unsigned long int filepos;
    unsigned int c1, c2;

    printf("Patching game \"%s\" ", game);
    print_version(stdout, &target_data);
    printf("\nusing ");
    print_version(stdout, &source_data);
    printf(" as source.\n\n");

    filepos = 0;

    for (;;) {
        if ((c1 = fgetc(patch_file)) == (unsigned int) -1) {
            return;
        }

        if ((c2 = fgetc(source_file)) == (unsigned int) -1) {
            filepos = 0;
            fseek(source_file, 0, SEEK_SET);
            c2 = fgetc(source_file);
        }

        fputc((char) (c1 ^ c2), target_file);
        ++filepos;

        if (filepos >= source_file_size) {
            filepos = 0;
            fseek(source_file, 0, SEEK_SET);
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Z-PATCHER version " VERSION "\n\n");

    if ((argc < 2) || ((argc == 2) &&
            ((strcmp(argv[1], "?") == 0) || (strcmp(argv[1], "/?") == 0)))) {
        printf("Applies a patch file produced to an Infocom z-code file.\n\n");
        printf("z_patcher patch_file.pat source_file.dat output_file.dat\n\n");
        printf("  patch_file.pat   Patch file to apply\n");
        printf("  source_file.dat   Existing z-code file to patch\n");
        printf("  target_file.dat  File name for patched z-code to produce\n\n");
        printf("You can also do the following:\n\n");
        printf("z_patcher patch_file.pat\n\n");
        printf("    Prints version details of the patch:\n");
        printf("    which version it requires and which version it produces.\n\n");
        exit(1);
    }

    // If a single file provided, it should be a patch file and so the details
    // of the file will be displayed.

    if (argc == 2) {
        patch_name = argv[1];

        if ((patch_file = fopen(patch_name, "rb")) == NULL) {
            fprintf(stderr, "Unable to open input patch file '%s'.\n", patch_name);
            exit(1);
        }

        source_file = NULL;
        target_file = NULL;

        verify_patch_file();
        get_details();

        printf("Game: %s\n", game);
        printf("Produces version: "); print_version(stdout, &target_data); printf("\n");
        printf("Requires version: "); print_version(stdout, &source_data); printf("\n\n");
        exit(0);
    }

    patch_name = argv[1];
    source_name = argv[2];
    target_name = argv[3];

    open_files();

    verify_patch_file();

    verify_source_file();

    get_details();

    verify_target_file();

    perform_patch_file();

    close_files();

    printf("Patch applied successfully.\n");

    return 0;
}
