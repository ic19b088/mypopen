/**
 * @file mypopen.c
 *
 * Dies ist eine Kopie der Library-Funktionen popen und pclose.
 *
 * @author Neumayer Johann	    <ic19b088@technikum-wien.at>
 * @author Rankovics Aleksandar    <ic19b023@technikum-wien.at>
 *
 * @date 2020/05/10
 *
 * @version 1.0
 *
 */

// -------------------------------------------------------------- includes --
#include "mypopen.h"

// -------------------------------------------------------------- defines --

//Leseende
#define R_END 0

//Schreibende
#define W_END 1

//Setzt globale Variablen zurück
#define RESET_GLOBALS() do { \
pid = -1;\
fs = NULL;\
} while (0)

// -------------------------------------------------------------- globals --

/**
 * \brief mypopen() speichert über diese Variable die Process-ID des Kind-Prozesses
 *        für die spätere verarbeitung durch mypclose().
 *        Wird im Fehlerfall zurückgesetzt.
 *        Default: -1 weil 0 auf manchen Systemen eine gültige pid wäre
 */
static pid_t pid = -1;

/**
 * \brief mypopen() speichert mit dieser Variable den File-Pointer
 *        für die spätere Verarbeitung durch mypclose().
 *        Wird im Fehlerfall zurückgesetzt.
 */
static FILE *fs = NULL;

// ------------------------------------------------------------ functions --

FILE *mypopen(const char *command, const char *type) {
    int fd[2];
    /* ### FB: Beschreibung, was die Variablen ausdrücken fehlt oder aussagekräftigere Namen verwenden */
    int c_pipe_fd, p_pipe_fd, c_std_fd;
    char *p_open_mode;

    // überprüfen ob mypopen (ohne mypclose) bereits ausgeführt wurde
    if (pid >= 0) {
        errno = EAGAIN;
        return NULL;
    }

    // Überprüfung ob mehr als 'r' oder 'w' eingegeben wurde
    if (type[1] != '\0') {
        errno = EINVAL;
        return NULL;
    }

    /* ### FB: Übersichtliches Switch, vielleicht im Kommentar genauer beschreiben, was genau hier gemacht wird*/
    // Lese- oder Schreibe-Einstellungen wählen
/*
 * ### FB_TMG: Gute Idee, um nicht sinnlos Code zu duplizieren
 */
    switch (type[0]) {
        case 'r':
            p_pipe_fd = R_END;
            p_open_mode = "r";
            c_pipe_fd = W_END;
            c_std_fd = STDOUT_FILENO;
            break;
        case 'w':
            p_pipe_fd = W_END;
            p_open_mode = "w";
            c_pipe_fd = R_END;
            c_std_fd = STDIN_FILENO;
            break;
        default:
            errno = EINVAL;
            return NULL;
    }
    /* ### FB: Kommentar?*/
    if (pipe(fd) == -1) {
        return NULL;
    }

    switch (pid = fork()) {
        case -1: // ERROR -> konnte kein Kind erzeugt werden
            /* ### FB: Erklärung warum Typecast für folgende (void)'s */
            (void)close(fd[W_END]);
            (void)close(fd[R_END]);
/*
 * ### FB_TMG: Hier sollten Sie nicht explizit errno setzen (das macht
 * ja fork schon) [-1]
 */
            errno = EAGAIN;
            return NULL;

        case 0:                              // Kind-Prozess Routine
            (void)close(fd[p_pipe_fd]);      // eltern-ende schließen
            /* ### FB: Check ob STD-Input nicht schon Kindprozess ist - super*/
/*
 * ### FB_TMG: Check ob stdin/stdout nicht eh schon die Pipe ist
 */
            if (fd[c_pipe_fd] != c_std_fd) { // kein dup/close wenn schon gesetzt
                if (dup2(fd[c_pipe_fd], c_std_fd) == -1) {
                    (void)close(fd[c_pipe_fd]);
/*
 * ### FB_TMG: Sehr schön - Die Verwendung von _exit() verhindert, daß
 * atexit() Handlers aufgerufen werden
 * Noch besser wäre allerdings _Exit(), weil das ISO-C und somit portabel ist
 */
                    _exit(EXIT_FAILURE);
                }
                (void)close(fd[c_pipe_fd]);
            }
            (void)execl("/bin/sh", "sh", "-c", command, (char *)NULL);
            /* ### FB: gut, dass _exit() verwendet wurde */
/*
 * ### FB_TMG: Sehr schön - Die Verwendung von _exit() verhindert, daß
 * atexit() Handlers aufgerufen werden
 * Noch besser wäre allerdings _Exit(), weil das ISO-C und somit portabel ist
 */
            _exit(EXIT_FAILURE); // nur erreich wenn execl fehlschlägt

        default:
            // Eltern-Prozess Routine
            (void)close(fd[c_pipe_fd]); // kind-ende schließen
            if ((fs = fdopen(fd[p_pipe_fd], p_open_mode)) == NULL) {
                pid = -1;
                (void)close(fd[p_pipe_fd]);
                return NULL;
            }
            break;
    }
    return fs;
}
/* ### FB:Zu my pclose- nichts hinzuzufügen, super gemacht! :)*/
int mypclose(FILE *stream) {
    //überprüfen ob mypopen schon aufgerufen wurde
    if (pid < 0) {
        errno = ECHILD;
        return -1;
    }

    //überprüfen ob der richtige file-pointer übergeben wurde
    if (fs != stream) {
        errno = EINVAL;
        return -1;
    }

    // stream schließen
    if (fclose(stream) == EOF) {
        // zurücksetzen da fclose() nicht nochmal aufgerufen werden darf
        RESET_GLOBALS();
        return -1;
    }

    // auf kindprozess warten
    int status;
    pid_t wpid;
    while ((wpid = waitpid(pid, &status, 0)) != pid) {
        if (wpid == -1) {
            if (errno == EINTR) // only interrupted, wait again
                continue;

            errno = ECHILD;
            RESET_GLOBALS();
            return -1;
        }
    }

    // globals zurücksetzen
    RESET_GLOBALS();

    // exit-status überprüfen
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else {
        errno = ECHILD;
        return -1;
    }
}