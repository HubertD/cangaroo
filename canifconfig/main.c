#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/netlink.h>
#include <netlink/version.h>
#include <netlink/route/link.h>
#include <netlink/route/link/can.h>

struct opts {

    char    *ifname;
    bool     do_bring_up;
    bool     do_bring_down;

    bool     do_set_bitrate;
    unsigned bitrate;

    bool     do_set_sample_point;
    unsigned sample_point;

    bool     do_set_fd_enable;
    bool     fd_enable;

    bool     do_set_fd_bitrate;
    unsigned fd_bitrate;

    bool     do_set_fd_sample_point;
    unsigned fd_sample_point;

    bool     do_set_restart_ms;
    unsigned restart_ms;

    bool     do_set_mode_listen_only;
    bool     mode_listen_only;

    bool     do_set_mode_oneshot;
    bool     mode_oneshot;

    bool     do_set_mode_triple_sampling;
    bool     mode_triple_sampling;

};

struct nldata {
    struct nl_sock *sock;
    struct nl_cache *cache;
    struct rtnl_link *link;
};


void print_usage(char *program_name)
{
    fprintf(
        stderr,
        "Usage: %s -i <socketcan interface>\n"
        "  -d          bring interface down before configuration\n"
        "  -u          bring interface up after configuration\n"
        "  -b <n>      set bitrate to n bits per second\n"
        "  -p <n>      set sample point to n/1000, e.g. 875 for 87.5%%\n"
        "  -f <on|off> enable/disable CanFD\n"
        "  -B <n>      set fd bitrate to n bits per second\n"
        "  -P <n>      set fd sample point to n/1000, e.g. 875 for 87.5%%\n"
        "  -r <n>      set auto restart time to n milliseconds, 0 for disable\n"
        "  -l <on|off> enable/disable listen-only mode\n"
        "  -o <on|off> enable/disable one-shot mode\n"
        "  -3 <on|off> enable/disable triple-sampling mode\n"
        "\n",
         program_name
    );
}

bool atobool(char *name, char *arg) {
    if (strcmp(arg, "on")==0) {
        return true;
    } else if (strcmp(arg, "off")==0) {
        return false;
    } else {
        fprintf(stderr, "Error: Argument of %s must be \"on\" or \"off\", not \"%s\"\n", name, arg);
        exit(EXIT_FAILURE);
    }
}

int parse_opts(int argc, char *argv[], struct opts *opts)
{
    int opt;
    memset(opts, 0, sizeof(*opts));

    while ((opt = getopt(argc, argv, "i:udb:p:f:B:P:r:l:o:3:h")) != -1) {

        switch (opt) {

        case 'i':
            opts->ifname = optarg;
            break;

        case 'u':
            opts->do_bring_up = true;
            break;

        case 'd':
            opts->do_bring_down = true;
            break;

        case 'b':
            opts->do_set_bitrate = true;
            opts->bitrate = atoi(optarg);
            break;

        case 'p':
            opts->do_set_sample_point = true;
            opts->sample_point = atoi(optarg);
            break;

        case 'f':
            opts->do_set_fd_enable = true;
            opts->fd_enable = atobool("-f", optarg);
            break;

        case 'B':
            opts->do_set_fd_bitrate = true;
            opts->fd_bitrate = atoi(optarg);
            break;

        case 'P':
            opts->do_set_fd_sample_point = true;
            opts->fd_sample_point = atoi(optarg);
            break;

        case 'r':
            opts->do_set_restart_ms = true;
            opts->restart_ms = atoi(optarg);
            break;

        case 'l':
            opts->do_set_mode_listen_only = true;
            opts->mode_listen_only = atobool("-l", optarg);
            break;

        case 'o':
            opts->do_set_mode_oneshot = true;
            opts->mode_oneshot = atobool("-o", optarg);
            break;

        case '3':
            opts->do_set_mode_triple_sampling = true;
            opts->mode_triple_sampling = atobool("-3", optarg);
            break;

        case 'h':
        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (opts->ifname == 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}


int netlink_connect(struct nldata *nldata, char *ifname)
{

    memset(nldata, 0, sizeof(*nldata));

    nldata->sock = nl_socket_alloc();
    if ( nldata->sock == NULL ) {
        perror("cannot allocate netlink socket");
        return EXIT_FAILURE;
    }

    if ( nl_connect(nldata->sock, NETLINK_ROUTE) != 0 ) {
        perror("cannot connect to kernel netlink interface");
    } else {

        if ( rtnl_link_alloc_cache(nldata->sock, AF_UNSPEC, &nldata->cache) != 0 ) {
            perror("cannot allocate link cache");
        } else {

            nldata->link = rtnl_link_get_by_name(nldata->cache, ifname);
            if (nldata->link == 0) {
                fprintf(stderr, "cannot find interface %s\n", ifname);
            } else {
                if (rtnl_link_is_can(nldata->link)) {

                    return EXIT_SUCCESS;

                } else {
                    fprintf(stderr, "not a can interface: %s\n", ifname);
                }

                rtnl_link_put(nldata->link);
                nldata->link = NULL;

            }

            nl_cache_free(nldata->cache);
            nldata->cache = NULL;

        }
        nl_close(nldata->sock);
    }

    nl_socket_free(nldata->sock);
    nldata->sock = NULL;
    return EXIT_FAILURE;
}

int netlink_close(struct nldata *nldata)
{
    if (nldata->link) {
        rtnl_link_put(nldata->link);
        nldata->link = NULL;
    }

    if (nldata->cache) {
        nl_cache_free(nldata->cache);
        nldata->cache = NULL;
    }

    if (nldata->sock) {
        nl_close(nldata->sock);
        nl_socket_free(nldata->sock);
        nldata->sock = NULL;
    }

    return EXIT_SUCCESS;
}


bool netlink_new_can_request(struct rtnl_link **req)
{
    if (*req != NULL) {
        rtnl_link_put(*req);
    }

    *req = rtnl_link_alloc();
    if (*req != NULL) {
        if (rtnl_link_set_type(*req, "can") != 0) {
            rtnl_link_put(*req);
            *req = NULL;
        }
    }

    if (*req == NULL) {
        fprintf(stderr, "cannot create netlink request object\n");
    }

    return *req != NULL;
}


int main(int argc, char *argv[])
{

    struct opts opts;
    struct nldata nldata;
    struct rtnl_link *req = NULL;
    int retval = EXIT_FAILURE;

    if (parse_opts(argc, argv, &opts) != 0) {
        exit(EXIT_FAILURE);
    }

    if (netlink_connect(&nldata, opts.ifname) != 0) {
        exit(EXIT_FAILURE);
    }

    do {

        if (opts.do_bring_down) {

            if (!netlink_new_can_request(&req)) {
                break;
            }

            rtnl_link_unset_flags(req, IFF_UP);
            if (rtnl_link_change(nldata.sock, nldata.link, req, 0) != 0) {
                perror("cannot bring interface down");
                break;
            }

        }

        if (!netlink_new_can_request(&req)) {
            exit(EXIT_FAILURE);
        }

        if (opts.do_set_bitrate) {
            if (rtnl_link_can_set_bitrate(req, opts.bitrate) != 0) {
                fprintf(stderr, "cannot set bitrate to %d\n", opts.bitrate);
                break;
            }
        }

        if (opts.do_set_sample_point) {
            if (rtnl_link_can_set_sample_point(req, opts.sample_point) != 0) {
                fprintf(stderr, "cannot set sample point to %.3f\n", opts.sample_point/1000.0);
                break;
            }
        }

        if (opts.do_set_fd_enable || opts.do_set_fd_bitrate || opts.do_set_fd_sample_point) {
            fprintf(stderr, "warning: canfd functions not implemented yet.\n");
        }

        if (opts.do_set_restart_ms) {
            if (rtnl_link_can_set_restart_ms(req, opts.restart_ms) != 0) {
                fprintf(stderr, "cannot set restart_ms to %d\n", opts.restart_ms);
            }
        }

        if (opts.do_set_mode_listen_only) {
            if (opts.mode_listen_only) {
                printf("setting listen-only flag to true\n");
                rtnl_link_can_set_ctrlmode(nldata.link, CAN_CTRLMODE_LISTENONLY);
            } else {
                printf("setting listen-only flag to false\n");
                rtnl_link_can_unset_ctrlmode(nldata.link, CAN_CTRLMODE_LISTENONLY);
            }
        }

        if (opts.do_set_mode_triple_sampling) {
            if (opts.mode_triple_sampling) {
                rtnl_link_can_set_ctrlmode(req, CAN_CTRLMODE_3_SAMPLES);
            } else {
                rtnl_link_can_unset_ctrlmode(req, CAN_CTRLMODE_3_SAMPLES);
            }
        }

        if (opts.do_set_mode_oneshot) {
            if (opts.mode_oneshot) {
                rtnl_link_can_set_ctrlmode(req, CAN_CTRLMODE_ONE_SHOT);
            } else {
                rtnl_link_can_unset_ctrlmode(req, CAN_CTRLMODE_ONE_SHOT);
            }
        }

        if (rtnl_link_change(nldata.sock, nldata.link, req, 0) != 0) {
            perror("cannot set link configuration");
            break;
        }

        if (opts.do_bring_up) {

            if (!netlink_new_can_request(&req)) {
                break;
            }

            rtnl_link_set_flags(req, IFF_UP);
            if (rtnl_link_change(nldata.sock, nldata.link, req, 0) != 0) {
                perror("cannot bring interface up");
                break;
            }

        }

        retval = EXIT_SUCCESS;

    } while (false);


    if (req != NULL) {
        rtnl_link_put(req);
    }

    netlink_close(&nldata);
    exit(retval);

}
