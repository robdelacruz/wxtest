#include "App.h"
#include "Frame.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit() {
    MyFrame *frame;

    frame = new MyFrame(wxT("MyFrame"));
    frame->Show(true);

    return true;
}

#if 0
static int readArgs(sqlite3 **retdb, str_t *retdbfile) {
    int z;
    sqlite3 *db = NULL;
    char *dbfile = NULL;
    str_t *err = str_new(0);

    int argc = wxGetApp().argc;
    char **argv = wxGetApp().argv;

    if (argc < 2) {
        printf("Usage:\n%s expfile\n", argv[0]);
        return 1;
    }

    for (int i=1; i < argc; i++) {
        char *s = argv[i];

        // prog -i dbfile
        if (strcmp(s, "-i") == 0) {
            if (i == argc-1) {
                printf("Usage:\n%s -i <expense file>\n", argv[0]);
                goto exit_error;
            }
            dbfile = argv[i+1];
            z = create_expense_file(dbfile, &db, err);
            if (z != 0)
                fprintf(stderr, "Error creating '%s': %s\n", dbfile, err->s);
            goto exit_ok;
        }

        // prog dbfile
        dbfile = s;
        break;
    }

    if (!file_exists(dbfile)) {
        z = create_expense_file(dbfile, &db, err);
        if (z != 0) {
            fprintf(stderr, "Can't create '%s': %s\n", dbfile, err->s);
            goto exit_error;
        }
        printf("Created expense file '%s'\n", dbfile);
    } else {
        z = open_expense_file(dbfile, &db, err);
        if (z != 0) {
            fprintf(stderr, "Can't open '%s': %s\n", dbfile, err->s);
            goto exit_error;
        }
    }

exit_ok:
    str_free(err);
    *retdb = db;
    str_assign(retdbfile, dbfile);
    return 0;

exit_error:
    str_free(err);
    *retdb = NULL;
    str_assign(retdbfile, (char*)"");
    return 1;
}
#endif

