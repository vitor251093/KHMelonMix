#include "glib.h"

#include "strlcompat.h"
#include "../src/slirp.h"
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>

GRand *g_rand_new() {
    GRand *rand = (GRand *) malloc(sizeof(GRand));
    *rand = 32148920;
    return rand;
}

// TODO: Implement the logging functions

void g_rand_free(GRand *rand) {
    free((void *) rand);
}

uint32_t g_rand_int_range(GRand *rand, uint32_t begin, uint32_t end) {
    uint32_t x = (uint32_t) *rand;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *rand = x;
    return ((x - begin) % end) + begin;
}

int g_strv_length(char **str_array) {
    int result = 0;
    while (str_array[result])
        result++;
    return result;
}

void g_error(const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    char text[1024] = "[libslirp] ";
    strlcat(text, msg, sizeof(text));
    //retro_vlog(RETRO_LOG_ERROR, text, va);
    va_end(va);
}

void g_critical(const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    char text[1024] = "[libslirp CRITICAL] ";
    strlcat(text, msg, sizeof(text));
    //retro_vlog(RETRO_LOG_ERROR, text, va);
    va_end(va);
}

void g_warning(const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    char text[1024] = "[libslirp] ";
    strlcat(text, msg, sizeof(text));
    //retro_vlog(RETRO_LOG_WARN, text, va);
    va_end(va);
}

void g_debug(const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    char text[1024] = "[libslirp] ";
    strlcat(text, msg, sizeof(text));
    //retro_vlog(RETRO_LOG_DEBUG, text, va);
    va_end(va);
}

void slirp_insque(void *a, void *b)
{
    struct slirp_quehead *element = (struct slirp_quehead *)a;
    struct slirp_quehead *head = (struct slirp_quehead *)b;
    element->qh_link = head->qh_link;
    head->qh_link = (struct slirp_quehead *)element;
    element->qh_rlink = (struct slirp_quehead *)head;
    ((struct slirp_quehead *)(element->qh_link))->qh_rlink =
            (struct slirp_quehead *)element;
}

void slirp_remque(void *a)
{
    struct slirp_quehead *element = (struct slirp_quehead *)a;
    ((struct slirp_quehead *)(element->qh_link))->qh_rlink = element->qh_rlink;
    ((struct slirp_quehead *)(element->qh_rlink))->qh_link = element->qh_link;
    element->qh_rlink = NULL;
}

/**
 * g_strstr_len:
 * @haystack: a nul-terminated string
 * @haystack_len: the maximum length of @haystack in bytes. A length of -1
 *     can be used to mean "search the entire string", like `strstr()`.
 * @needle: the string to search for
 *
 * Searches the string @haystack for the first occurrence
 * of the string @needle, limiting the length of the search
 * to @haystack_len or a nul terminator byte (whichever is reached first).
 *
 * Returns: a pointer to the found occurrence, or
 *    %NULL if not found.
 */
gchar *
g_strstr_len (const gchar *haystack,
              gssize       haystack_len,
              const gchar *needle)
{
    g_return_val_if_fail (haystack != NULL, NULL);
    g_return_val_if_fail (needle != NULL, NULL);

    if (haystack_len < 0)
        return strstr (haystack, needle);
    else
    {
        const gchar *p = haystack;
        gsize needle_len = strlen (needle);
        gsize haystack_len_unsigned = haystack_len;
        const gchar *end;
        gsize i;

        if (needle_len == 0)
            return (gchar *)haystack;

        if (haystack_len_unsigned < needle_len)
            return NULL;

        end = haystack + haystack_len - needle_len;

        while (p <= end && *p)
        {
            for (i = 0; i < needle_len; i++)
                if (p[i] != needle[i])
                    goto next;

            return (gchar *)p;

            next:
            p++;
        }

        return NULL;
    }
}

struct gfwd_list *add_guestfwd(struct gfwd_list **ex_ptr, SlirpWriteCb write_cb,
                               void *opaque, struct in_addr addr, int port)
{
    g_critical("add_guestfwd unexpectedly required in stub\n");
    return NULL;
}

struct gfwd_list *add_exec(struct gfwd_list **ex_ptr, const char *cmdline,
                           struct in_addr addr, int port)
{
    g_critical("add_exec unexpectedly required in stub\n");
    return NULL;
}

int remove_guestfwd(struct gfwd_list **ex_ptr, struct in_addr addr, int port)
{
    g_critical("remove_guestfwd unexpectedly required in stub\n");
    return 0;
}

struct gfwd_list *add_unix(struct gfwd_list **ex_ptr, const char *unixsock,
                           struct in_addr addr, int port)
{
    g_critical("add_unix unexpectedly required in stub\n");
    return NULL;
}

int slirp_bind_outbound(struct socket *so, unsigned short af)
{
    int ret = 0;
    struct sockaddr *addr = NULL;
    int addr_size = 0;

    if (af == AF_INET && so->slirp->outbound_addr != NULL) {
        addr = (struct sockaddr *)so->slirp->outbound_addr;
        addr_size = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6 && so->slirp->outbound_addr6 != NULL) {
        addr = (struct sockaddr *)so->slirp->outbound_addr6;
        addr_size = sizeof(struct sockaddr_in6);
    }

    if (addr != NULL) {
        ret = bind(so->s, addr, addr_size);
    }
    return ret;
}

int open_unix(struct socket *so, const char *unixpath)
{
#ifdef G_OS_UNIX
    struct sockaddr_un sa;
    int s;

    DEBUG_CALL("open_unix");
    DEBUG_ARG("so = %p", so);
    DEBUG_ARG("unixpath = %s", unixpath);

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    if (g_strlcpy(sa.sun_path, unixpath, sizeof(sa.sun_path)) >= sizeof(sa.sun_path)) {
        g_critical("Bad unix path: %s", unixpath);
        return 0;
    }

    s = slirp_socket(PF_UNIX, SOCK_STREAM, 0);
    if (s < 0) {
        g_critical("open_unix(): %s", strerror(errno));
        return 0;
    }

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        g_critical("open_unix(): %s", strerror(errno));
        closesocket(s);
        return 0;
    }

    so->s = s;
    slirp_set_nonblock(so->s);
    so->slirp->cb->register_poll_fd(so->s, so->slirp->opaque);

    return 1;
#else
    g_assert_not_reached();
#endif
}

int fork_exec(struct socket *so, const char *ex)
{
    g_critical("fork_exec unexpectedly required in stub\n");
    return 0;
}

gint
g_ascii_strcasecmp (const gchar *s1,
                    const gchar *s2)
{
    gint c1, c2;

    g_return_val_if_fail (s1 != NULL, 0);
    g_return_val_if_fail (s2 != NULL, 0);

    while (*s1 && *s2)
    {
        c1 = (gint)(guchar) tolower(*s1);
        c2 = (gint)(guchar) tolower(*s2);
        if (c1 != c2)
            return (c1 - c2);
        s1++; s2++;
    }

    return (((gint)(guchar) *s1) - ((gint)(guchar) *s2));
}


gboolean g_str_has_prefix(const gchar* str,const gchar* prefix)
{
    return strncmp(str, prefix, strlen(prefix));
}