// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

#define VERSION "1.0.3"

// Abstractions for portability
#ifdef __GNUC__
#include <unistd.h>
#define TRUNCATE                  ftruncate
#define ISATTY                    isatty
#define FILENO                    fileno
#define SSCANF                    sscanf
#define FREAD(buf, buf_size, elem_size, elem_cnt, file)                        \
    fread(buf, elem_size, elem_cnt, file)
#define FOPEN(file, name, access)                                              \
    do {                                                                       \
        file = fopen(name, access);                                            \
    } while (0)

#else
#include <io.h>
#define TRUNCATE                  _chsize
#define ISATTY                    _isatty
#define FILENO                    _fileno
#define SSCANF                    sscanf_s
#define FREAD                     fread_s
#define FOPEN(file, name, access) fopen_s(&file, name, access)
#endif

#ifdef _WIN32
#include <fcntl.h>
#define SETMODE(file, mode)       _setmode(file, mode)
#else
#define SETMODE(file, mode)
#endif

#define XSTR(s)                   STR(s)
#define STR(x)                    #x
#define NUM_ELEMS(x)              (sizeof(x) / sizeof(x[0]))

#define STDIN_FILENAME            "[STDIN]"
#define ERR_STR                   "ERROR: "
#define WRN_STR                   "WARNING: "

#define SHOW_PAD_OUTPUT           0

typedef enum error_code {
    ERROR_NONE,
    ERROR_ARG_MISSING,
    ERROR_ARG_UNKOWN,
    ERROR_ARG_ORDER,
    ERROR_ARG_SINGLE_INSTANCE,
    ERROR_ARG_VALUE_MISSING,
    ERROR_ARG_VALUE_PARSING_FAILURE,
    ERROR_ARG_VALUE_OUT_OF_RANGE,
    ERROR_FILE_SYSTEM,
    ERROR_OUT_OF_RESOURCES,
    ERROR_OVERLAPPING_INPUT_DATA
} error_code_t;

typedef enum log_level {
    LOG_INF = 0x1,
    LOG_WRN = 0x2,
    LOG_ERR = 0x4
} log_level_t;

typedef struct file_entry {
    char *filename;
    long size;
    uint32_t offset;
} file_entry_t;

/*
 * The available command line argument flags/options.
 */
static const char *help_arg[] = { "-h", "--help" };
static const char *version_arg[] = { "--version" };
static const char *verbose_arg[] = { "-v", "--verbose" };
static const char *input_file_arg[] = { "-i", "--in-file" };
static const char *output_file_arg[] = { "-o", "--out-file" };
static const char *seek_arg[] = { "-s", "--seek" };
static const char *block_size_arg[] = { "-b", "--block-size" };
static const char *fill_byte_arg[] = { "-f", "--fill-byte" };
static const char *truncate_arg[] = { "-t", "--truncate" };

/*
 * Variables set by command line arguments.
 */
static log_level_t log_level = LOG_WRN;
static bool show_help = false;
static bool show_version = false;
static char *output_filename = NULL;
static uint32_t stdin_seek_blocks = 0;
static uint32_t block_size = 512;
static uint8_t fill_value = 0xFF;
static file_entry_t *input_files = NULL;
static bool truncate_out_file = false;

static int input_file_count = 0;

static void print_help(char *arg0)
{
    printf("Usage:\n");
    printf("    %s [-h] [--version]\n\n", arg0);
    printf("    %s [-v] [-b <BSIZE>] [-t] -i <IN_FILE>:<N> [<IN_FILE>:<N> ...] -o <OUT_FILE>\n\n",
           arg0);
    printf("    cat <IN_FILE> | %s [-v] [-s <N>] [-b <BSIZE>] [-t] [-i <IN_FILE>:<N> ...] -o <OUT_FILE>\n\n",
           arg0);
    printf("Data partition image creation tool.\n\n");
    printf("A utility for copying binary data from one or more files into another.\n\n");
    printf("Options:\n");
    printf("    -h, --help                  This help menu.\n");
    printf("        --version               Print the version of this tool.\n");
    printf("    -v, --verbose               Print verbose output.\n");
    printf("    -t, --truncate              Specifies whether to truncate the output file.\n"
           "                                When set, the resulting filesize will depend\n"
           "                                on the last written offset in the file. If the\n"
           "                                output file contains data that extends past this\n"
           "                                offset, it will be lost/truncated.\n");
    printf("    -b, --block-size <BSIZE>    The max number of bytes to read/write at a time.\n"
           "                                This argument, if specified, must appear before\n"
           "                                the --in-file option. Default = 512.\n");
    printf("    -f, --fill-byte <FBYTE>     The 8-bit fill value when seeking to an offset\n"
           "                                past the current output filesize.\n"
           "                                Default = 0xFF.\n");
    printf("    -s, --seek <N>              Only applicable when using stdin as an input\n"
           "                                source. N is the offset in blocks, of size\n"
           "                                BSIZE, to start writing stdin to OUT_FILE.\n"
           "                                Default = 0.\n");
    printf("    -i, --in-file <IN_FILE>:<N> The file(s) to read data from and copy to the\n"
           "                                specified location in the OUT_FILE. IN_FILE is\n"
           "                                the filename. N is the offset in blocks, of size\n"
           "                                BSIZE, to start writing to in OUT_FILE. Multiple\n"
           "                                input files are to be separated by a space.\n");
    printf("    -o, --out-file <OUT_FILE>   The file to write the output to. If the file\n"
           "                                already exists, the state of the file will be\n"
           "                                used; regions where the input data is being\n"
           "                                written to will be lost/overwritten.\n");
}

static void write_log(log_level_t level, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (level >= log_level) {
        switch (level) {
        case LOG_WRN:
            printf(WRN_STR);
            break;
        case LOG_ERR:
            printf(ERR_STR);
            break;
        default:
            break;
        }
        vprintf(format, args);
    }

    va_end(args);
}

static error_code_t write_arg_error(error_code_t error_code, ...)
{
    va_list args;
    va_start(args, error_code);

    switch (error_code) {
    case ERROR_ARG_VALUE_PARSING_FAILURE:
        printf(ERR_STR "%s : ", va_arg(args, char *));
        printf("Argument value (%s) could not be parsed.\n",
               va_arg(args, char *));
        break;
    case ERROR_ARG_VALUE_OUT_OF_RANGE:
        printf(ERR_STR "%s : ", va_arg(args, char *));
        printf("Argument value (%s) is out-of-range.\n",
               va_arg(args, char *));
        break;
    case ERROR_ARG_VALUE_MISSING:
        printf(ERR_STR "%s : Argument value missing.\n",
               va_arg(args, char *));
        break;
    case ERROR_ARG_ORDER:
        printf(ERR_STR "%s : ", va_arg(args, char *));
        printf("Argument must be specified prior to %s.\n",
               va_arg(args, char *));
        break;
    case ERROR_ARG_UNKOWN:
        printf(ERR_STR "%s : Unkown argument.\n",
               va_arg(args, char *));
        break;
    case ERROR_ARG_SINGLE_INSTANCE:
        printf(ERR_STR "%s : Argument must only be specified once.\n",
               va_arg(args, char *));
        break;
    default:
        printf(ERR_STR "Application encountered an error (%d).\n",
               error_code);
        break;
    }

    va_end(args);

    return error_code;
}

static bool is_matching_arg(char *arg, const char *arg_options[],
                            int num_options)
{
    for (int i = 0; i < num_options; i++) {
        if (0 == strcmp(arg, arg_options[i]))
            return true;
    }

    return false;
}

static error_code_t next_arg_value(int argc, char *argv[], int *argi)
{
    if ((++(*argi) >= argc) || argv[*argi][0] == '-') {
        write_log(LOG_ERR, "Missing argument value (%s).\n", argv[*argi - 1]);
        return ERROR_ARG_VALUE_MISSING;
    }

    return ERROR_NONE;
}

static int num_arg_values(int argc, char *argv[], int argi)
{
    int num_values = 0;

    while (++argi < argc) {
        if (argv[argi][0] == '-')
            break;

        num_values++;
    }

    return num_values;
}

static bool parse_number(char *str, long *number)
{
    // Try parsing as hex value with "0x" specifier; otherwise parse as decimal.
    if (SSCANF(str, "0x%lX", number) != 1)
        if (SSCANF(str, "%ld", number) != 1)
            return false;

    return true;
}

static void swap_entry(file_entry_t *a, file_entry_t *b)
{
    file_entry_t tmp;
    tmp.filename = a->filename;
    tmp.offset = a->offset;

    a->filename = b->filename;
    a->offset = b->offset;

    b->filename = tmp.filename;
    b->offset = tmp.offset;
}

static void sort_offset_ascending(file_entry_t *entries, size_t count)
{
    size_t min_index;

    for (size_t i = 0; i < count; i++) {
        min_index = i;

        for (size_t j = i + 1; j < count; j++)
            if (entries[j].offset < entries[min_index].offset)
                min_index = j;

        if (i != min_index)
            swap_entry(&entries[i], &entries[min_index]);
    }
}

static void update_file_entry_size(FILE *file, file_entry_t *entry)
{
    long offset = ftell(file);
    fseek(file, 0L, SEEK_END);
    entry->size = ftell(file);

    /* On Windows stdin is seekable, a final CRLF is implied and needs to be
     * deducted from the reported filesize. On platforms where stdin is not
     * seekable, the reported size will be -1. */
#ifdef _WIN32
    const int num_implied_ctrl_chars = 2;
    if ((entry->size >= num_implied_ctrl_chars) && (file == stdin))
        entry->size -= num_implied_ctrl_chars;
#endif

    fseek(file, offset, SEEK_SET);
}

static error_code_t fill_data(FILE *file, size_t num_bytes)
{
    if (num_bytes == 0)
        return ERROR_NONE;

    char *fill_data_ptr;
    size_t alloc_size = num_bytes;

    if (alloc_size > block_size)
        alloc_size = block_size;

    fill_data_ptr = (fill_value == 0) ?
        calloc(1, alloc_size) :
        malloc(alloc_size);

    if (fill_data_ptr == NULL)
        return ERROR_OUT_OF_RESOURCES;

    if (fill_value != 0)
        memset(fill_data_ptr, fill_value, alloc_size);

    while (num_bytes) {
        size_t bytes_to_write = num_bytes;
        if (bytes_to_write > alloc_size)
            bytes_to_write = alloc_size;

        num_bytes -= fwrite(fill_data_ptr, 1, bytes_to_write, file);
    }

    free(fill_data_ptr);
    return ERROR_NONE;
}

static error_code_t copy_data(FILE *in_file, FILE *out_file, long in_filesize)
{
    size_t total_bytes_copied = 0;
    size_t alloc_size = block_size;
    char *copy_data_ptr = malloc(alloc_size);

    if (copy_data_ptr == NULL)
        return ERROR_OUT_OF_RESOURCES;

    while (1) {
        size_t elem_size = 1;
        size_t bytes_copied = 0;
        size_t bytes_to_write = 0;
        size_t bytes_read = FREAD(copy_data_ptr, alloc_size, elem_size,
                                  alloc_size, in_file);

        if (bytes_read == 0)
            break;

#ifdef _WIN32
        /* Windows stdin has an implied CR+LF at the end of the stream
         * which needs to be omitted for this application. */
        if ((in_file == stdin) &&
            ((long)(total_bytes_copied + bytes_read) > in_filesize)) {
            bytes_read -= ((total_bytes_copied + bytes_read) - in_filesize);
            if (bytes_read == 0)
                continue;
        }
#else
        (void) in_filesize; // Suppress unused parameter warning.
#endif

        bytes_to_write = bytes_read;

        while (bytes_copied < bytes_read) {
            size_t bytes_written = fwrite(&copy_data_ptr[bytes_copied],
                                          elem_size, bytes_to_write, out_file);
            bytes_copied += bytes_written;
            bytes_to_write -= bytes_written;
        }

        total_bytes_copied += bytes_copied;
    }

    free(copy_data_ptr);
    return ERROR_NONE;
}

static error_code_t process_args(int argc, char *argv[])
{
    bool in_file_piped = false;
    bool in_file_present = false;
    bool out_file_present = false;

    in_file_piped = !ISATTY(FILENO(stdin));

    for (int i = 1; i < argc; i++) {
        long tmp;

        if (is_matching_arg(argv[i], help_arg, NUM_ELEMS(help_arg))) {
            show_help = true;
            return ERROR_NONE;
        } else if (is_matching_arg(argv[i], version_arg,
                                   NUM_ELEMS(version_arg))) {
            show_version = true;
            return ERROR_NONE;
        } else if (is_matching_arg(argv[i], verbose_arg,
                                   NUM_ELEMS(verbose_arg))) {
            log_level = LOG_INF;
        } else if (is_matching_arg(argv[i], truncate_arg,
                                   NUM_ELEMS(truncate_arg))) {
            truncate_out_file = true;
        } else if (is_matching_arg(argv[i], input_file_arg,
                                   NUM_ELEMS(input_file_arg))) {
            /* Check that this argument was not previously provided. The user
             * should be informed that only one instance of '-i' is supported.
             * This is particularly important due to the malloc. */
            if (in_file_present) {
                return write_arg_error(ERROR_ARG_SINGLE_INSTANCE, argv[i]);
            }

            input_file_count = num_arg_values(argc, argv, i);
            int num_entries_allocated = input_file_count;

            if (in_file_piped)
                num_entries_allocated++;

            if (num_entries_allocated == 0) {
                return write_arg_error(ERROR_ARG_VALUE_MISSING, argv[i]);
            }

            input_files = malloc(num_entries_allocated * sizeof(file_entry_t));
            if (input_files == NULL)
                return ERROR_OUT_OF_RESOURCES;

            /* Parse each argument in the form of "<FILENAME>:<N>" and
             * update the file entry table accordingly. */
            for (int j = 0; j < input_file_count; j++) {
                const char delim = ':';
                next_arg_value(argc, argv, &i);

                /* Handle tokenization in reverse. This avoids complication with
                 * fullpath handling on Windows with drive letters. */
                char *token = strrchr(argv[i], delim);
                long blocks;
                bool valid_value = false;

                if (token != NULL) {
                    *token = '\0';
                    token++;
                    input_files[j].filename = argv[i];
                    valid_value = parse_number(token, &blocks);
                }

                if (!valid_value) {
                    return write_arg_error(ERROR_ARG_VALUE_PARSING_FAILURE,
                                           input_file_arg[0], argv[i]);
                }

                input_files[j].offset = blocks * block_size;
                input_files[j].size = 0; // Size is determined during copy process.
            }

            in_file_present = true;
        } else if (is_matching_arg(argv[i], output_file_arg,
                                   NUM_ELEMS(output_file_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            output_filename = argv[i];
            out_file_present = true;
        } else if (is_matching_arg(argv[i], seek_arg, NUM_ELEMS(seek_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE) {
                return ERROR_ARG_VALUE_MISSING;
            } else if (!parse_number(argv[i], &tmp)) {
                return write_arg_error(ERROR_ARG_VALUE_PARSING_FAILURE,
                                       seek_arg[0], argv[i]);
            }

            if ((tmp < 0) || (tmp > UINT32_MAX)) {
                return write_arg_error(ERROR_ARG_VALUE_OUT_OF_RANGE,
                                       block_size_arg[0], argv[i]);
            }

             stdin_seek_blocks = (uint32_t)tmp;
        } else if (is_matching_arg(argv[i], block_size_arg,
                                   NUM_ELEMS(block_size_arg))) {
            if (in_file_present) {
                return write_arg_error(ERROR_ARG_ORDER,
                                       block_size_arg[0], input_file_arg[0]);
            }

            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            if (!parse_number(argv[i], &tmp)) {
                return write_arg_error(ERROR_ARG_VALUE_PARSING_FAILURE,
                                       block_size_arg[0], argv[i]);
            }

            if ((tmp <= 0) || (tmp > UINT32_MAX)) {
                return write_arg_error(ERROR_ARG_VALUE_OUT_OF_RANGE,
                                       block_size_arg[0], argv[i]);
            }

            block_size = (uint32_t)tmp;
        } else if (is_matching_arg(argv[i], fill_byte_arg,
                                   NUM_ELEMS(fill_byte_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            if (!parse_number(argv[i], &tmp)) {
                return write_arg_error(ERROR_ARG_VALUE_PARSING_FAILURE,
                                       fill_byte_arg[0], argv[i]);
            } else if ((tmp <= 0) || (tmp > UCHAR_MAX)) {
                return write_arg_error(ERROR_ARG_VALUE_OUT_OF_RANGE,
                                       fill_byte_arg[0], argv[i]);
            }

            fill_value = (uint8_t)tmp;
        } else {
            return write_arg_error(ERROR_ARG_UNKOWN, argv[i]);
        }
    }

    if (in_file_piped) {
        /* If "--in-file" was specified, space should have been
         * pre-allocated for "in_file_piped". Simply update the
         * last "input_files" entry. */
        if (input_files == NULL)
            input_files = malloc(sizeof(file_entry_t));

        if (input_files == NULL)
            return ERROR_OUT_OF_RESOURCES;

        input_files[input_file_count].filename = STDIN_FILENAME;
        input_files[input_file_count].offset = stdin_seek_blocks * block_size;
        input_files[input_file_count].size = 0; // Size is determined during copy process.
        input_file_count++;
    }

    if ((in_file_piped || in_file_present) && out_file_present) {
        return ERROR_NONE;
    } else {
        if (!out_file_present) {
            write_log(LOG_ERR, "Missing required %s argument.\n", output_file_arg[0]);
        }

        if (!(in_file_piped || in_file_present)) {
            write_log(LOG_ERR, "Missing required input from either %s or stdin.\n", input_file_arg[0]);
        }

        return ERROR_ARG_MISSING;
    }
}

static int fpeek(FILE *in_file)
{
    int c = fgetc(in_file);
    ungetc(c, in_file);

    return c;
}

int main(int argc, char *argv[])
{
    int exit_code = process_args(argc, argv);
    FILE *in_file = NULL;
    FILE *out_file = NULL;

    if (show_help) {
        print_help(argv[0]);
    } else if (exit_code) {
        printf("Specify --help for usage.\n");
    } else if (show_version) {
        printf("version %s\n", VERSION);
    }

    if (show_version || show_help || exit_code) {
        if (input_files)
            free(input_files);

        return exit_code;
    }

    FOPEN(out_file, output_filename, "rb+");

    /* If the file is null, then it is assumed to not exist.
     * Open again, using wb+ which will create the file if
     * it does not exit. */
    if (out_file == NULL)
        FOPEN(out_file, output_filename, "wb+");

    if (out_file == NULL) {
        write_log(LOG_ERR, "Failed to open file (%s).\n", output_filename);

        if (in_file != NULL)
            fclose(in_file);

        return ERROR_FILE_SYSTEM;
    }

    fseek(out_file, 0L, SEEK_END);

    /* Note: If larger file support is needed, the cast below should be
     * removed, and other application logic updated accordingly. */
    uint32_t out_filesize = (uint32_t)ftell(out_file);
    uint32_t out_filesize_old = out_filesize;

    /* To simplify the file navigation logic for each copy operation, the
     * input files are sorted based on their respective offset within the
     * output file. */
    sort_offset_ascending(input_files, input_file_count);

    write_log(LOG_INF, "Offset\t\tSize\t\tName\n");
    write_log(LOG_INF, "----------------------------------------\n");

    uint32_t last_write_offset = 0;

    for (int file_index = 0; file_index < input_file_count; file_index++) {
        bool is_stdin = strcmp(input_files[file_index].filename, STDIN_FILENAME) == 0;

        if (is_stdin) {
            in_file = stdin;

            /* On platforms such as Windows, stdin is a character device by
             * default. To avoid conversion of bytes equal to newline control
             * characters, change the mode to binary mode. */
            SETMODE(FILENO(stdin), _O_BINARY);
        } else {
            FOPEN(in_file, input_files[file_index].filename, "rb");
        }

        if (in_file == NULL) {
            write_log(LOG_ERR, "Failed to open file (%s).\n",
                      input_files[file_index].filename);
            exit_code = ERROR_FILE_SYSTEM;
            break;
        }

        update_file_entry_size(in_file, &input_files[file_index]);

        if (input_files[file_index].offset <= out_filesize) {
            if ((file_index == 0) ||
                (input_files[file_index].offset >= last_write_offset)) {
                fseek(out_file, input_files[file_index].offset, SEEK_SET);
            } else if ((input_files[file_index].size == 0) ||
                       ((input_files[file_index].size < 0) && (fpeek(in_file) == EOF))) {
                // Input is empty. Skip.
                fclose(in_file);
                continue;
            } else {
                exit_code = ERROR_OVERLAPPING_INPUT_DATA;
            }
        } else {
            size_t pad_size = input_files[file_index].offset - out_filesize;
            exit_code = fill_data(out_file, pad_size);
#if SHOW_PAD_OUTPUT
            write_log(LOG_INF, "0x%08X\t%d\t\t[PADDING]\n",
                      out_filesize,
                      pad_size);
#endif
        }

        /* A filesize < 0 indicates the source is not seekable and thus the
         * size cannot be pre-determined. Only skip files that report a
         * filesize of 0. */
        if ((input_files[file_index].size != 0) && (exit_code == ERROR_NONE))
            exit_code = copy_data(in_file, out_file, input_files[file_index].size);

        fclose(in_file);

        last_write_offset = ftell(out_file);
        fseek(out_file, 0L, SEEK_END);
        out_filesize = ftell(out_file);

        long written_size = (exit_code == ERROR_NONE) ?
                last_write_offset - input_files[file_index].offset :
                -1L;

        write_log(LOG_INF, "0x%08X\t%ld\t\t%s\n",
                  input_files[file_index].offset,
                  written_size,
                  input_files[file_index].filename);

        if (exit_code == ERROR_OUT_OF_RESOURCES) {
            write_log(LOG_ERR, "Failed to allocate memory.\n");
            break;
        } else if (exit_code == ERROR_OVERLAPPING_INPUT_DATA) {
            write_log(LOG_ERR, "Overlapping input data detected.\n");
            break;
        }

        if ((input_files[file_index].size >= 0) &&
            (written_size != input_files[file_index].size)) {
            write_log(LOG_WRN, "Computed input size (%ld) does not match written length (%ld).\n",
                      input_files[file_index].size,
                      written_size);
        }
    }

    if (truncate_out_file && (exit_code == ERROR_NONE) &&
        (TRUNCATE(FILENO(out_file), last_write_offset) != 0)) {
        write_log(LOG_ERR, "Failed to truncate file.\n");
        exit_code = ERROR_FILE_SYSTEM;
    }

    if (exit_code == ERROR_NONE) {
        fseek(out_file, 0L, SEEK_END);
        out_filesize = ftell(out_file);

        write_log(LOG_INF, "\nFilesize (old): %d Bytes\n", out_filesize_old);
        write_log(LOG_INF, "Filesize (new): %d Bytes\n", out_filesize);
    }

    fclose(out_file);

    if (input_files)
        free(input_files);

    return exit_code;
}
