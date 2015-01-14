#ifndef _PAM_H_
#define _PAM_H_

#include <stdbool.h>

bool login(const char *username, const char *password, pid_t *child_pid);
bool logout(void);

#endif /* _PAM_H_ */
