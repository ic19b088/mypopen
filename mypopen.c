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

#include "mypopen.h"

/**
 * @param pid Globale Variable für die Prozess-id
 * @param *file Globaler File-Pointer
 */
static pid_t pid = -1;
static FILE *file = NULL;

/**
 * @param pipe_read kennzeichnet das Leseende
 */
#define pipe_read 0

/**
 * @param pipe_write kennzeichnet das Schreibende
 */
#define pipe_write 1

/**
 * @brief Setzt static variablen zurück. Marko wurde Programmiererfreundlich in eine do {} while Schleife gekapselt.
 */
#define reset() do { \
pid = -1;\
file = NULL;\
} while (0)

FILE *mypopen(const char *command, const char *type) {

/**
 * @param fd
 * @param open_end
 * @param pipe_end
 * @param sys_io
 * @param mode
 */
    int fd[2];
    int open_end;
    int pipe_end;
    int sys_io;
    char *mode;

/**
 * @brief Überprüft ob mypopen bzw. mypclose bereits ausgeführt wurde.
 */
    if (pid >= 0) {
        errno = EAGAIN;
        return NULL;
    }
/**
 * @brief überprüft ob mehr als 1 Buchstabe eingegeben wurde.
 */
    if (type[1] != '\0') {
        errno = EINVAL;
        return NULL;
    }
/**
 * @brief Hier wird die Benutzereingabe überprüft und je nach Wert die entsprechenden Werte gesetzt.
 */
    if (type[0] == 'r') {
        pipe_end = pipe_read;
        mode = "r";
        open_end = pipe_write;
        sys_io = STDOUT_FILENO;
    } else if (type[0] == 'w') {
        pipe_end = pipe_write;
        mode = "w";
        open_end = pipe_read;
        sys_io = STDIN_FILENO;
    } else {
        errno = EINVAL;
        return NULL;
    }

/**
 * @brief Überprüfung ob Einrichten einer Pipe fehlgeschlagen ist.
 */
    if (pipe(fd) == -1) {
        return NULL;
    }
/**
 * @brief Wenn die pid -1 ist konnte kein Kindprozess erzeugt werden. --> Alles zu und raus.
 * @brief Wenn die pid 0 ist wird das Eltern-Ende geschlossen, der Filedeskriptor kopiert und das Prozessimage überschrieben.
 * @brief Wenn die pid die Prozess-Id des Kind-Prozesses ist, wird das Kind-Ende geschlossen.
 */
    pid = fork();
    if (pid == -1) {
        close(fd[pipe_write]);
        close(fd[pipe_read]);
        return NULL;
    } else if (pid == 0) {
        close(fd[pipe_end]);
        if (fd[open_end] != sys_io) {
            if (dup2(fd[open_end], sys_io) == -1) {
                close(fd[open_end]);
                _Exit(EXIT_FAILURE);
            }
            close(fd[open_end]);
        }
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        _Exit(EXIT_FAILURE);
    } else {
        close(fd[open_end]);
        if ((file = fdopen(fd[pipe_end], mode)) == NULL) {
            pid = -1;
            (void) close(fd[pipe_end]);
            return NULL;
        }
    }

    return file;
}

/* ### FB:Zu my pclose- nichts hinzuzufügen, super gemacht! :)*/
int mypclose(FILE *stream) {
    //überprüfen ob mypopen schon aufgerufen wurde
    if (pid < 0) {
        errno = ECHILD;
        return -1;
    }

    //überprüfen ob der richtige file-pointer übergeben wurde
    if (file != stream) {
        errno = EINVAL;
        return -1;
    }

    // stream schließen
    if (fclose(stream) == EOF) {
        // zurücksetzen da fclose() nicht nochmal aufgerufen werden darf
        reset();
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
            reset();
            return -1;
        }
    }

    // globals zurücksetzen
    reset();

    // exit-status überprüfen
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else {
        errno = ECHILD;
        return -1;
    }
}