///@file mypopen.h

#ifndef MYPOPEN_H
#define MYPOPEN_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>

/**
 * @brief Erzeugt einen Kindprozess, Dupliziert Filediskreptoren und Überladet Kindprozess
 * @brief Zusätzlich wird eine Pipe vom Elternprozess zum Kindprozess aufgebaut und die nicht benötigten
 * @brief Enden geschlossen
 * @param command Befehl mit dem der Kindprozess überschrieben werden soll
 * @param type "r"ead / "w"rite (Lese oder Schreibzugriff)
 * @return Im Fehlerfall NULL, ansonsten ein File-Pointer
 */
FILE *mypopen(const char *command, const char *type);

/**
 * @brief Die Funktion mypclose bekommt einen Filepointer übergeben und Schließt den offenen Filestream.
 * @brief Anschließend wartet die Funktion auf die Terminierung des Kindprozesses.
 * @param stream Filestream *
 * @return -1 im Fehlerfall, ansonsten der Exit-Code des Kindprozesses.
 */
int mypclose(FILE *stream);

#endif /* MYPOPEN_H */