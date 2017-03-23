/**
 * @file
 * @defgroup altcp Application layered TCP
 * @ingroup callbackstyle_api
 * Application layered TCP connection API (to be used from TCPIP thread)\n
 * This interface mimics the tcp callback API to the application while preventing
 * direct linking (much like virtual functions).
 * This way, an application can make use of other application layer protocols
 * on top of TCP without knowing the details (e.g. TLS, proxy connection).
 *
 * This file contains the common functions for altcp to work.
 */

/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmidt <goldsimon@gmx.de>
 *
 */

#include "lwip/opt.h"

#if LWIP_ALTCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/altcp.h"
#include "lwip/priv/altcp_priv.h"
#include "lwip/tcp.h"
#include "lwip/mem.h"

#include <string.h>

extern const struct altcp_functions altcp_tcp_functions;

struct altcp_pcb *
altcp_alloc(void)
{
  /* FIXME: pool alloc */
  struct altcp_pcb *ret = (struct altcp_pcb *)mem_malloc(sizeof(struct altcp_pcb));
  if (ret != NULL) {
    memset(ret, 0, sizeof(struct altcp_pcb));
  }
  return ret;
}

void
altcp_free(struct altcp_pcb *conn)
{
  /* FIXME: pool alloc */
  if (conn) {
    mem_free(conn);
  }
}

/**
 * @ingroup altcp
 * @see tcp_arg()
 */
void
altcp_arg(struct altcp_pcb *conn, void *arg)
{
  if (conn) {
    conn->arg = arg;
  }
}

/**
 * @ingroup altcp
 * @see tcp_accept()
 */
void
altcp_accept(struct altcp_pcb *conn, altcp_accept_fn accept)
{
  if (conn != NULL) {
    conn->accept = accept;
  }
}

/**
 * @ingroup altcp
 * @see tcp_recv()
 */
void
altcp_recv(struct altcp_pcb *conn, altcp_recv_fn recv)
{
  if (conn) {
    conn->recv = recv;
  }
}

/**
 * @ingroup altcp
 * @see tcp_sent()
 */
void
altcp_sent(struct altcp_pcb *conn, altcp_sent_fn sent)
{
  if (conn) {
    conn->sent = sent;
  }
}

/**
 * @ingroup altcp
 * @see tcp_poll()
 */
void
altcp_poll(struct altcp_pcb *conn, altcp_poll_fn poll, u8_t interval)
{
  if (conn) {
    conn->poll = poll;
    conn->pollinterval = interval;
    if (conn->fns && conn->fns->set_poll) {
      conn->fns->set_poll(conn, interval);
    }
  }
}

/**
 * @ingroup altcp
 * @see tcp_err()
 */
void
altcp_err(struct altcp_pcb *conn, altcp_err_fn err)
{
  if (conn) {
    conn->err = err;
  }
}

/* Generic functions calling the "virtual" ones */

/**
 * @ingroup altcp
 * @see tcp_recved()
 */
void
altcp_recved(struct altcp_pcb *conn, u16_t len)
{
  if (conn && conn->fns && conn->fns->recved) {
    conn->fns->recved(conn, len);
  }
}

/**
 * @ingroup altcp
 * @see tcp_bind()
 */
err_t
altcp_bind(struct altcp_pcb *conn, const ip_addr_t *ipaddr, u16_t port)
{
  if (conn && conn->fns && conn->fns->bind) {
    return conn->fns->bind(conn, ipaddr, port);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_connect()
 */
err_t
altcp_connect(struct altcp_pcb *conn, const ip_addr_t *ipaddr, u16_t port, altcp_connected_fn connected)
{
  if (conn && conn->fns && conn->fns->connect) {
    return conn->fns->connect(conn, ipaddr, port, connected);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_listen_with_backlog_and_err()
 */
struct altcp_pcb *
altcp_listen_with_backlog_and_err(struct altcp_pcb *conn, u8_t backlog, err_t *err)
{
  if (conn && conn->fns && conn->fns->listen) {
    return conn->fns->listen(conn, backlog, err);
  }
  return NULL;
}

/**
 * @ingroup altcp
 * @see tcp_abort()
 */
void
altcp_abort(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->abort) {
    conn->fns->abort(conn);
  }
}

/**
 * @ingroup altcp
 * @see tcp_close()
 */
err_t
altcp_close(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->close) {
    return conn->fns->close(conn);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_shutdown()
 */
err_t
altcp_shutdown(struct altcp_pcb *conn, int shut_rx, int shut_tx)
{
  if (conn && conn->fns && conn->fns->shutdown) {
    return conn->fns->shutdown(conn, shut_rx, shut_tx);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_write()
 */
err_t
altcp_write(struct altcp_pcb *conn, const void *dataptr, u16_t len, u8_t apiflags)
{
  if (conn && conn->fns && conn->fns->write) {
    return conn->fns->write(conn, dataptr, len, apiflags);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_output()
 */
err_t
altcp_output(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->output) {
    return conn->fns->output(conn);
  }
  return ERR_VAL;
}

/**
 * @ingroup altcp
 * @see tcp_mss()
 */
u16_t
altcp_mss(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->mss) {
    return conn->fns->mss(conn);
  }
  return 0;
}

/**
 * @ingroup altcp
 * @see tcp_sndbuf()
 */
u16_t
altcp_sndbuf(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->sndbuf) {
    return conn->fns->sndbuf(conn);
  }
  return 0;
}

/**
 * @ingroup altcp
 * @see tcp_sndqueuelen()
 */
u16_t
altcp_sndqueuelen(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->sndqueuelen) {
    return conn->fns->sndqueuelen(conn);
  }
  return 0;
}

/**
 * @ingroup altcp
 * @see tcp_setprio()
 */
void
altcp_setprio(struct altcp_pcb *conn, u8_t prio)
{
  if (conn && conn->fns && conn->fns->setprio) {
    conn->fns->setprio(conn, prio);
  }
}

err_t
altcp_get_tcp_addrinfo(struct altcp_pcb *conn, int local, ip_addr_t *addr, u16_t *port)
{
  if (conn && conn->fns && conn->fns->addrinfo) {
    return conn->fns->addrinfo(conn, local, addr, port);
  }
  return ERR_VAL;
}

#ifdef LWIP_DEBUG
enum tcp_state
altcp_dbg_get_tcp_state(struct altcp_pcb *conn)
{
  if (conn && conn->fns && conn->fns->dbg_get_tcp_state) {
    return conn->fns->dbg_get_tcp_state(conn);
  }
  return CLOSED;
}
#endif

#endif /* LWIP_ALTCP */