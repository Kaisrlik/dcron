
/*
 * SUBS.C
 *
 * Copyright 1994 Matthew Dillon (dillon@apollo.backplane.com)
 * May be distributed under the GNU General Public License
 */

#include "defs.h"

Prototype void logn(int level, const char *ctl, ...);
Prototype void log9(const char *ctl, ...);
Prototype void log_err(const char *ctl, ...);
Prototype void fdprintf(int fd, const char *ctl, ...);
Prototype int  ChangeUser(const char *user, short dochdir);
Prototype void vlog(int level, int MLOG_LEVEL, const char *ctl, va_list va);
Prototype char *xx_strdup(const char *);
Prototype void startlogger(void);
Prototype void initsignals(void);

void 
log9(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vlog(9, LOG_WARNING, ctl, va);
    va_end(va);
}

void 
logn(int level, const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vlog(level, LOG_NOTICE, ctl, va);
    va_end(va);
}

void 
log_err(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vlog(20, LOG_ERR, ctl, va);
    va_end(va);
}

void 
fdprintf(int fd, const char *ctl, ...)
{
    va_list va;
    char buf[2048];

    va_start(va, ctl);
    vsnprintf(buf, sizeof(buf), ctl, va);
    write(fd, buf, strlen(buf));
    va_end(va);
}

  void
vlog(int level, int MLOG_LEVEL, const char *ctl, va_list va)
  {
    char buf[2048];
    int  logfd;
  
      if (level >= LogLevel) {
	vsnprintf(buf,sizeof(buf), ctl, va);
	if (DebugOpt) fprintf(stderr,"%s",buf);
	else
	    if (LoggerOpt == 0) syslog(MLOG_LEVEL, "%s", buf);
	    else {
		if ((logfd = open(LogFile,O_WRONLY|O_CREAT|O_APPEND,0600)) >= 0){
		    write(logfd, buf, strlen(buf));
		    close(logfd);
		} else
		    fprintf(stderr,"Can't open log file. Err: %s",strerror(errno));
	    }
      }
  }
  
  int
  ChangeUser(const char *user, short dochdir)
{
    struct passwd *pas;

    /*
     * Obtain password entry and change privilages
     */

    if ((pas = getpwnam(user)) == 0) {
        logn(9, "failed to get uid for %s", user);
        return(-1);
    }
    setenv("USER", pas->pw_name, 1);
    setenv("HOME", pas->pw_dir, 1);
    setenv("SHELL", "/bin/sh", 1);

    /*
     * Change running state to the user in question
     */

    if (initgroups(user, pas->pw_gid) < 0) {
	logn(9, "initgroups failed: %s %s", user, strerror(errno));
	return(-1);
    }
    if (setregid(pas->pw_gid, pas->pw_gid) < 0) {
	logn(9, "setregid failed: %s %d", user, pas->pw_gid);
	return(-1);
    }
    if (setreuid(pas->pw_uid, pas->pw_uid) < 0) {
	logn(9, "setreuid failed: %s %d", user, pas->pw_uid);
	return(-1);
    }
    if (dochdir) {
	if (chdir(pas->pw_dir) < 0) {
	    logn(8, "chdir failed: %s %s", user, pas->pw_dir);
	    if (chdir(TMPDIR) < 0) {
		logn(9, "chdir failed: %s %s", user, pas->pw_dir);
		logn(9, "chdir failed: %s " TMPDIR, user);
		return(-1);
	    }
	}
    }
    return(pas->pw_uid);
}

#if 0

char *
xx_strdup(const char *str)
{
    char *ptr = malloc(strlen(str) + 1);

    if (ptr)
        strcpy(ptr, str);
    return(ptr);
}

#endif

void
startlogger (void) {
int logfd;

    if (LoggerOpt == 0)
	openlog("crond",LOG_CONS|LOG_PID,LOG_CRON);
    else {
	if ((logfd = open(LogFile,O_WRONLY|O_CREAT|O_APPEND,0600)) >= 0)
	    close(logfd);
	else
	    printf("Failed to open logfile '%s' reason: %s",LogFile,strerror(errno));
    }
}

void
initsignals (void) {
    signal(SIGHUP,SIG_IGN);
}
