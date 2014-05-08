#include <security/pam_appl.h>
#include <security/pam_misc.h>

#define SERVICE_NAME "display_manager"

#define err(name)                                   \
    do {                                            \
        fprintf(stderr, "%s: %s\n", name,           \
                pam_strerror(pam_handle, result));  \
        return;                                     \
    } while (1);                                    \

static int conv(int num_msg, const struct pam_message **msg,
                struct pam_response **resp, void *appdata_ptr);

static pam_handle_t *pam_handle;

void login(const char *username, const char *password) {
    const char *data[2] = {username, password};
    struct pam_conv pam_conv = {
        conv, data
    };
    int result = pam_start(SERVICE_NAME, NULL, &pam_conv, &pam_handle);
    if (result != PAM_SUCCESS) {
        err("pam_start");
    }

    result = pam_set_item(pam_handle, PAM_USER, username);
    if (result != PAM_SUCCESS) {
        err("pam_set_item");
    }

    result = pam_authenticate(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        err("pam_authenticate");
    }
}

static int conv(int num_msg, const struct pam_message **msg,
                 struct pam_response **resp, void *appdata_ptr) {
    int i;

    *resp = calloc(num_msg, sizeof(struct pam_response));
    if (*resp == NULL) {
        return PAM_BUF_ERR;
    }

    int result = PAM_SUCCESS;
    for (i = 0; i < num_msg; i++) {
        char *username, *password;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            username = ((char **) appdata_ptr)[0];
            (*resp)[i].resp = strdup(username);
            break;
        case PAM_PROMPT_ECHO_OFF:
            password = ((char **) appdata_ptr)[1];
            (*resp)[i].resp = strdup(password);
            break;
        case PAM_ERROR_MSG:
            g_printerr("%s\n", msg[i]->msg);
            result = PAM_CONV_ERR;
            break;
        case PAM_TEXT_INFO:
            g_print("%s\n", msg[i]->msg);
            break;
        }
        if (result != PAM_SUCCESS) {
            break;
        }
    }

    if (result != PAM_SUCCESS) {
        free(*resp);
        *resp = 0;
    }

    return result;
}
