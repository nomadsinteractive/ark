/*
POSIX getopt for Windows

AT&T Public License

Code given out at the 1985 UNIFORUM conference in Dallas.
*/

#ifndef _GETOPT_H_
#define _GETOPT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int opterr;
extern int optind;
extern int optopt;
extern const char* optarg;
extern int getopt(int argc, const char** argv, const char* opts);

#ifdef __cplusplus
}
#endif

#endif  /* _GETOPT_H_ */

