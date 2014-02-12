#!/usr/bin/python
# -*- coding: utf-8 -*-

# =============================================================================
# 2013+ Copyright (c) Kirill Smorodinnikov <shaitkir@gmail.com>
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# =============================================================================

"""
    Example script for showing how to iterate specified nodes or all nodes in specified groups
"""

import sys
from elliptics_recovery.etime import Time

sys.path.insert(0, "bindings/python/")
import elliptics

MODE_GROUP = "groups"
MODE_NODES = "nodes"
ALLOWED_MODES = (MODE_GROUP, MODE_NODES)


class Ctx(object):
    pass


def transf(str_id):
    val = int(str_id, 16)
    id = []
    while val > 0:
        id = [val % 256] + id
        val /= 256
    if len(id) < 64:
        id = [0] * (64 - len(id)) + id
    return id[:64]


def iterate_node(ctx, node):
    eid = ctx.session.routes.get_address_eid(node)
    print eid

    iflags = elliptics.iterator_flags.key_range
    if ctx.time_begin or ctx.time_end:
        iflags |= elliptics.iterator_flags.ts_range
    if ctx.data:
        iflags |= elliptics.iterator_flags.data

    if not ctx.time_begin:
        ctx.time_begin = elliptics.Time(0, 0)
    if not ctx.time_end:
        ctx.time_end = elliptics.Time(2**64-1, 2**64-1)

    print ctx.session.groups, eid, ctx.ranges, iflags, ctx.time_begin, ctx.time_end

    ranges = ctx.ranges

    for r in ranges:
        print repr(r.key_begin), repr(r.key_end)

    print bin(iflags)

    ctx.session.groups = [ctx.session.routes.get_address_group_id(node)]

    iterator = ctx.session.start_iterator(eid,
                                          ranges,
                                          elliptics.iterator_types.network,
                                          iflags,
                                          ctx.time_begin,
                                          ctx.time_end)

    for r in ranges:
        print repr(r.key_begin), repr(r.key_end)

    for result in iterator:
        if result.status != 0:
            raise AssertionError("Wrong status: {0}".format(result.status))

        print ("node: {0}, key: {1}, flags: {2}, ts: {3}/{4}, data: {5}"
               .format(node,
                       result.response.key,
                       result.response.user_flags,
                       result.response.timestamp.tsec,
                       result.response.timestamp.tnsec,
                       result.response_data))


def iterate_groups(ctx):
    routes = ctx.session.routes
    for g in routes.groups():
        group_routes = elliptics.RouteList(routes.filter_by_group_id(g))
        group_addresses = group_routes.addresses()
        for a in group_addresses:
            iterate_node(ctx, a)


def parse_args():
    from optparse import OptionParser
    ctx = Ctx()

    parser = OptionParser()
    parser.usage = "%prog type [options]"
    parser.description = __doc__
    parser.add_option("-g", "--groups", action="store", dest="groups", default=None,
                      help="Comma separated list of groups [default: all]")
    parser.add_option("-l", "--log", dest="log", default='/dev/stderr', metavar="FILE",
                      help="Output log messages from library to file [default: %default]")
    parser.add_option("-L", "--log-level", action="store", dest="log_level", default="1",
                      help="Elliptics client verbosity [default: %default]")
    parser.add_option("-r", "--remote", action="append", dest="remote",
                      help="Elliptics node address [default: %default]")
    parser.add_option("-d", "--data", action="store_true", dest="data", default=False,
                      help="Requests object's data with other info [default: %default]")
    parser.add_option("-k", "--key-begin", action="store", dest="key_begin", default="0",
                      help="Begin key of range for iterating")
    parser.add_option("-K", "--key-end", action="store", dest="key_end", default="-1",
                      help="End key of range for iterating")
    parser.add_option("-t", "--time-begin", action="store", dest="time_begin", default=None,
                      help="Begin timestamp of time range for iterating")
    parser.add_option("-T", "--time-end", action="store", dest="time_end", default=None,
                      help="End timestamp of time range for iterating")

    (options, args) = parser.parse_args()

    if len(args) > 1:
        raise ValueError("Too many arguments passed: {0}, expected: 1"
                         .format(len(args)))
    elif len(args) == 0:
        raise ValueError("Please specify one of following modes: {0}"
                         .format(ALLOWED_MODES))

    if args[0].lower() not in ALLOWED_MODES:
        raise ValueError("Unknown mode: '{0}', allowed: {1}"
                         .format(args[0], ALLOWED_MODES))
    ctx.iterate_mode = args[0].lower()

    try:
        if options.groups:
            ctx.groups = map(int, options.groups.split(','))
        else:
            ctx.groups = []
    except Exception as e:
        raise ValueError("Can't parse grouplist: '{0}': {1}".format(
            options.groups, repr(e)))
    print("Using group list: {0}".format(ctx.groups))

    try:
        ctx.log_file = options.log
        ctx.log_level = int(options.log_level)
    except Exception as e:
        raise ValueError("Can't parse log_level: '{0}': {1}"
                         .format(options.log_level, repr(e)))
    print("Using elliptics client log level: {0}".format(ctx.log_level))

    if not options.remote:
        raise ValueError("Please specify at least one remote address (-r option)")
    try:
        ctx.remotes = []
        for r in options.remote:
            ctx.remotes.append(elliptics.Address.from_host_port_family(r))
            print("Using remote host:port:family: {0}".format(ctx.remotes[-1]))
    except Exception as e:
        raise ValueError("Can't parse host:port:family: '{0}': {1}"
                         .format(options.remote, repr(e)))

    try:
        if options.time_begin:
            ctx.time_begin = Time.from_epoch(options.time_begin)
        else:
            ctx.time_begin = None
    except Exception:
        raise ValueError("Can't parse timestamp: '{0}': {1}"
                         .format(options.timestamp, repr(e)))
    print("Using time_begin: {0}".format(ctx.time_begin))

    try:
        if options.time_end:
            ctx.time_end = Time.from_epoch(options.time_end)
        else:
            ctx.time_end = None
    except Exception:
        raise ValueError("Can't parse timestamp: '{0}': {1}"
                         .format(options.timestamp, repr(e)))
    print("Using time_end: {0}".format(ctx.time_end))

    ctx.data = options.data

    key_range = elliptics.IteratorRange()

    try:
        if options.key_begin == '-1':
            key_range.key_begin = elliptics.Id([255] * 64, 0)
        elif options.key_begin:
            key_range.key_begin = elliptics.Id(transf(options.key_begin), 0)
        else:
            key_range.key_begin = elliptics.Id([0] * 64, 0)
    except Exception:
        raise ValueError("Can't parse key_begin: '{0}': {1}"
                         .format(options.key_begin, repr(e)))

    try:
        if options.key_end == '-1':
            key_range.key_end = elliptics.Id([255] * 64, 0)
        elif options.key_end:
            key_range.key_end = elliptics.Id(transf(options.key_end), 0)
        else:
            key_range.key_end = elliptics.Id([255] * 64, 0)
    except Exception:
        raise ValueError("Can't parse key_end: '{0}': {1}"
                         .format(options.key_end, repr(e)))

    ctx.ranges = [key_range]

    return ctx

if __name__ == '__main__':
    ctx = parse_args()

    ctx.elog = elliptics.Logger(ctx.log_file, ctx.log_level)
    ctx.node = elliptics.Node(ctx.elog)
    for r in ctx.remotes:
        try:
            ctx.node.add_remote(r)
        except Exception as e:
            print("Couldn't connect to remote: {0} got: {1}"
                  .format(r, e))
    ctx.session = elliptics.Session(ctx.node)
    if ctx.groups:
        ctx.session.groups = ctx.groups
    else:
        ctx.session.groups = ctx.session.routes.groups()
    print ctx.session.get_routes()

    if ctx.iterate_mode == MODE_NODES:
        for r in ctx.remotes:
            iterate_node(ctx, r)
    elif ctx.iterate_mode == MODE_GROUP:
        iterate_groups(ctx)
    else:
        raise RuntimeError("Unknown iteration mode '{0}' "
                           .format(ctx.iterate_mode))

    exit(0)
