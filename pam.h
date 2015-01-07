#ifndef _PAM_H_
#define _PAM_H_

void login(const char *username, const char *password, pid_t *child_pid);
void logout(void);

#endif /* _PAM_H_ */
