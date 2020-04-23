/**
 * @file mypopen.h
 * Betriebssysteme mypopen
 * Beispiel 2
 *
 * Dies ist eine Libary, welche die Funktionen mypopen() und mypclose() enthält
 * Diese sind den libc Aufrufen popen() und pclose() nachempfunden.
 *
 * @author Baliko Markus	<ic15b001@technikum-wien.at>
 * @author Haubner Alexander    <ic15b033@technikum-wien.at>
 * @author Riedmann Michael     <ic15b054@technikum-wien.at>
 *
 * @date 2016/04/20
 *
 * @version 1.0
 *
 */

#ifndef MYPOPEN_H
#define MYPOPEN_H

#include <stdio.h>
/*
 * ### FB_TMG: Das einzige include file, das sie hier brauchen ist
 * stdio.h (für das FILE typedef)
 */
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>

/**
 * Die Funktionsweise ist der libc Funktion "popen" nachempfunden.
 * Es wird ein Kommando als char-array übernommen. Dieses wird in einem
 * Kindprozess über eine Shell ("sh -c") ausgeführt. Dabei wird eine Pipe
 * vom Eltern- zum Kindprozess aufgabaut und je nach Modus (type Argument)
 * das Schreibe- oder Leseende zurückgegeben.
 *
 * @param command  Gibt Kommando an, welches in einem Kind-Prozess ausgeführt werden soll (z.B "ls -al")
 * @param type     "r" oder "w", für lese- oder schreibe-modus der pipe
 *
 * @return NULL bei Error, sonst der File-Pointer des gewählten Pipe-Endpunkts
 */
FILE *mypopen(const char *command, const char *type);

/**
 * Die Funktionsweise ist der libc Funktion "pclose" nachempfunden.
 * Es wird ein, mittels mypopen() erstellter File-Pointer übernommen und
 * ähnlich wie fclose() abgebaut. Zusätzlich wird noch auf den Abschluss des
 * Kindprozesses gewartet und der Return-Code zurückgegeben.
 *
 * @param stream Mit mypopen geöffneter Filestream
 *
 * @return Im Fehlerfall wird -1 zurückgegeben und errno gesetzt,
 *         sonst wird der exit-code es Kind-Prozesses zurückgegeben.
 */
int mypclose(FILE *stream);

#endif /* MYPOPEN_H */