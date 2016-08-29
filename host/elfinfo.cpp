#include <cstdlib>
#include <gelf.h>
#include <err.h>
#include <sysexits.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <getopt.h>
#include <inttypes.h>
#include <elf.h>
#include <ctype.h>

using std::vector;

enum class SectionType {
    TEXT, DATA
};

const char *to_str(const SectionType &type) {
    switch (type) {
        case SectionType::TEXT:
            return "text";
        case SectionType::DATA:
            return "data";
    }
}

struct Section {
    SectionType type;
    const Elf64_Addr start;
    const Elf64_Addr end;
    const Elf64_Xword size;
    Elf64_Addr used;

    Section(SectionType type, Elf64_Addr start, Elf64_Xword size) : type(type), start(start), size(size),
                                                                    end(start + size), used(0) {
    }

    bool contains(Elf64_Addr address, Elf64_Xword size) const {
        return contains(address) && contains(address + size);
    }

    bool contains(Elf64_Addr address) const {
        return start <= address && address < end;
    }
};

vector<Section> sections;

char *filename = NULL;

char *program;

__attribute__((noreturn))
void usage(const char *reason = NULL) {
    if (reason != NULL) {
        fprintf(stderr, "%s\n", reason);
    }
    fprintf(stderr, "usage: %s -f file [-t start:size] [-d start:size]\n", program);
    fprintf(stderr, "  -t/-d/-b: add text/data section\n");
    fprintf(stderr, "At least one section has to be specified\n");
    exit(EX_USAGE);
}

void parse_start_size(char *input, Elf64_Addr &start, Elf64_Xword &size) {
    char *str_size = strchr(input, ':');

    if (str_size == NULL) {
        usage("bad section specification, missing ':'");
    }

    *str_size = '\0';
    str_size++;

    if (sscanf(input, "%" SCNi64, &start) != 1) {
        usage("bad section specification, could not parse start number");
    }

    size_t str_size_len = strlen(str_size);

    if (str_size_len < 1) {
        usage("bad section specification");
    }

    char suffix = str_size[str_size_len - 1];
    int modifier;

    if (!isdigit(suffix)) {
        switch (suffix) {
            case 'k':
            case 'K':
                modifier = 1024;
                break;
            case 'm':
            case 'M':
                modifier = 1024 * 1024;
                break;
            default:
                usage("bad size modifier, only 'k' and 'M' are allowed");
        }
    } else {
        modifier = 1;
    }

    if (sscanf(str_size, "%" SCNi64, &size) != 1) {
        usage("bad section specification, could not parse size number");
    }
    size = size * modifier;
}

bool debug = false;

void parse_args(int argc, char **argv) {
    int c;

    while ((c = getopt(argc, argv, "Df:t:d:")) != -1) {
        switch (c) {
            case 'D':
                debug = true;
                break;
            case 't':
            case 'd': {
                Elf64_Addr start;
                Elf64_Xword size;
                parse_start_size(optarg, start, size);
                SectionType type = c == 't' ? SectionType::TEXT : SectionType::DATA;
                sections.push_back(Section(type, start, size));
                break;
            }
            case 'f':
                filename = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    errx(EX_USAGE, "Option -%c requires an argument.\n", optopt);
                else
                    errx(EX_USAGE, "Unknown option `-%c'.\n", optopt);
            default:
                abort();
        }
    }

    if (filename == NULL || sections.empty()) {
        usage();
    }
}

void to_iso(Elf64_Addr i, char *buf) {
    const char *suffix;
    if (i > 1024 * 1024) {
        i /= 1024 * 1024;
        suffix = "M";
    } else if (i > 1024) {
        i /= 1024;
        suffix = "k";
    } else {
        suffix = "";
    }
    sprintf(buf, "%" PRIu64 "%s", i, suffix);
}

int main(int argc, char **argv) {
    program = argv[0];
    parse_args(argc, argv);

    if (elf_version(EV_CURRENT) == EV_NONE)
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));

    int fd;
    if ((fd = open(filename, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", argv[1]);

    Elf *e;
    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    if (elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "%s is not an ELF object.", argv[1]);

    size_t shstrndx;
    if (elf_getshdrstrndx(e, &shstrndx) != 0)
        errx(EX_SOFTWARE, "elf_getshdrstrndx() failed: %s.", elf_errmsg(-1));

    size_t program_header_count;
    if (elf_getphdrnum(e, &program_header_count) != 0)
        errx(EX_DATAERR, "elf_getphdrnum() failed: %s.", elf_errmsg(-1));

    size_t text_size = 0, data_size = 0, bss_size = 0;
    for (int i = 0; i < program_header_count; i++) {
        GElf_Phdr phdr;

        if (gelf_getphdr(e, i, &phdr) != &phdr)
            errx(EX_SOFTWARE, "getphdr() failed: %s.", elf_errmsg(-1));

        if (phdr.p_type == PT_LOAD) {
            SectionType expectedType;
            size_t *size;

            if (phdr.p_flags == (PF_X | PF_R | PF_W)) {
                if (debug) {
                    printf("Adding PH #%d as text\n", i);
                }

                expectedType = SectionType::TEXT;
                size = &text_size;
            } else if (phdr.p_flags == (PF_R | PF_W)) {
                expectedType = SectionType::DATA;
                if (phdr.p_filesz > 0) {
                    if (debug) {
                        printf("Adding PH #%d as data\n", i);
                    }
                    size = &data_size;
                }
                else {
                    if (debug) {
                        printf("Adding PH #%d as bss\n", i);
                    }
                    size = &bss_size;
                }
            } else {
                errx(EX_DATAERR, "Unknown flag combination: 0x%02x", phdr.p_flags);
            }

            auto s = std::find_if(sections.begin(), sections.end(), [&](Section &section) {
                return section.type == expectedType && section.contains(phdr.p_vaddr, phdr.p_memsz);
            });

            if (s == sections.end()) {
                fprintf(stderr,
                        "Could not find a section for elf header #%d of type %s, at address %" PRIx64 " with size %" PRId64 "\n",
                        i, to_str(expectedType), phdr.p_vaddr, phdr.p_memsz);
            }
            else {
                (*s).used += phdr.p_memsz;

                *size += phdr.p_memsz;
            }
        } else {
            // ignored
        };
    }

    printf("Size by sections\n");
    printf("Type    Start      End  Size   Used\n");
    std::for_each(sections.begin(), sections.end(), [&](Section &s) {
        char size[100];
        to_iso(s.size, size);
        int used_pct = (int) (double(s.used) / double(s.size) * 100.0);
        printf("%4s %08" PRIx64 " %08" PRIx64 " %5s %6" PRId64 " %3d%%\n", to_str(s.type), s.start, s.end, size, s.used,
               used_pct);
    });

    printf("\n");
    printf("Size by type\n");
    printf("text=%zu, data=%zu, bss=%zu\n", text_size, data_size, bss_size);
    return EXIT_SUCCESS;
}
