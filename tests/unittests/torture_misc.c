#include <sys/types.h>
#include <pwd.h>

#define LIBSSH_STATIC
#include <libssh/priv.h>

#include "torture.h"
#include "misc.c"
#define DIR "/usr/local/bin/truc/much/.."

ssh_session session;

static void setup(void) {
    session = ssh_new();
}

static void teardown(void) {
    ssh_free(session);
}

START_TEST (torture_get_user_home_dir)
{
    struct passwd *pwd;
    char *user;

    pwd = getpwuid(getuid());

    user = ssh_get_user_home_dir();
    ck_assert_str_eq(user, pwd->pw_dir);

    SAFE_FREE(user);
}
END_TEST

START_TEST (torture_basename)
{
    char *path;
    path=ssh_basename(DIR "/test");
    ck_assert(path != NULL);
    ck_assert_str_eq(path, "test");
    SAFE_FREE(path);
    path=ssh_basename(DIR "/test/");
    ck_assert(path != NULL);
    ck_assert_str_eq(path, "test");
    SAFE_FREE(path);
}
END_TEST

START_TEST (torture_dirname)
{
    char *path;
    path=ssh_dirname(DIR "/test");
    ck_assert(path != NULL);
    ck_assert_str_eq(path, DIR );
    SAFE_FREE(path);
    path=ssh_dirname(DIR "/test/");
    ck_assert(path != NULL);
    ck_assert_str_eq(path, DIR);
    SAFE_FREE(path);
}
END_TEST

START_TEST (torture_ntohll)
{
    uint32_t sample = 1;
    unsigned char *ptr=(unsigned char *) &sample;
    uint64_t value = 0x0123456789abcdef;
    uint64_t check;
    if(ptr[0]==1){
      /* we're in little endian */
      check = 0xefcdab8967452301;
    } else {
      /* big endian */
      check = value;
    }
    value=ntohll(value);
    ck_assert(value == check);
}
END_TEST

START_TEST (torture_path_expand_tilde)
{
    char h[256];
    char *d;

    snprintf(h, 256 - 1, "%s/.ssh", getenv("HOME"));

    d = ssh_path_expand_tilde("~/.ssh");
    ck_assert_str_eq(d, h);
    free(d);

    d = ssh_path_expand_tilde("/guru/meditation");
    ck_assert_str_eq(d, "/guru/meditation");
    free(d);

    snprintf(h, 256 - 1, "~%s/.ssh", getenv("USER"));
    d = ssh_path_expand_tilde(h);

    snprintf(h, 256 - 1, "%s/.ssh", getenv("HOME"));
    ck_assert_str_eq(d, h);
    free(d);
}
END_TEST

START_TEST (torture_path_expand_escape)
{
    const char *s = "%d/%h/by/%r";
    char *e;

    session->sshdir = strdup("guru");
    session->host = strdup("meditation");
    session->username = strdup("root");

    e = ssh_path_expand_escape(session, s);
    ck_assert_str_eq(e, "guru/meditation/by/root");
    free(e);
}
END_TEST

START_TEST (torture_path_expand_known_hosts)
{
    char *tmp;

    session->sshdir = strdup("/home/guru/.ssh");

    tmp = ssh_path_expand_escape(session, "%d/known_hosts");
    ck_assert_str_eq(tmp, "/home/guru/.ssh/known_hosts");
    free(tmp);
}
END_TEST

Suite *torture_make_suite(void) {
  Suite *s = suite_create("libssh_misc");

  torture_create_case(s, "torture_get_user_home_dir", torture_get_user_home_dir);
  torture_create_case(s, "torture_basename", torture_basename);
  torture_create_case(s, "torture_dirname", torture_dirname);
  torture_create_case(s, "torture_ntohll", torture_ntohll);
  torture_create_case(s, "torture_path_expand_tilde", torture_path_expand_tilde);
  torture_create_case_fixture(s, "torture_path_expand_escape",
          torture_path_expand_escape, setup, teardown);
  torture_create_case_fixture(s, "torture_path_expand_known_hosts",
          torture_path_expand_known_hosts, setup, teardown);

  return s;
}

