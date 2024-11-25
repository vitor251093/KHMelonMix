project(libslirp)

add_library(libslirp
        src/arp_table.c
        src/bootp.c
        src/cksum.c
        src/dhcpv6.c
        src/dnssearch.c
        src/if.c
        src/ip_icmp.c
        src/ip_input.c
        src/ip_output.c
        src/ip6_icmp.c
        src/ip6_input.c
        src/ip6_output.c
        src/mbuf.c
        src/ndp_table.c
        src/ncsi.c
        src/sbuf.c
        src/slirp.c
        src/socket.c
        src/tcp_input.c
        src/tcp_output.c
        src/tcp_subr.c
        src/tcp_timer.c
        src/tftp.c
        src/udp.c
        src/udp6.c
        src/util.c
)

target_compile_definitions(libslirp PRIVATE GLIB_SIZEOF_VOID_P=8)
target_compile_definitions(libslirp PRIVATE UNIX)

set(glib ./glib-stub)
add_subdirectory(${glib} ./glib-stub)
target_include_directories(libslirp PRIVATE "src" PUBLIC "src")

target_link_libraries(libslirp glib)