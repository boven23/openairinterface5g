/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "openair2/RRC/NR/rrc_fuzz_inject.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/ran_context.h"
#include "common/utils/ds/seq_arr.h"
#include "common/utils/LOG/log.h"
#include "common/utils/utils.h"
#include "f1ap_messages_types.h"
#include "intertask_interface.h"
#include "openair2/F1AP/f1ap_ids.h"
#include "openair2/RRC/NR/nr_rrc_defs.h"
#include "openair2/RRC/NR/rrc_gNB_UE_context.h"

#define RRC_FUZZ_MSG_HEARTBEAT 0xff
#define RRC_FUZZ_MSG_INJECT_UL_DCCH 0x01
#define RRC_FUZZ_MSG_QUERY_UE_LIST 0x02
#define RRC_FUZZ_MSG_QUERY_UE_STATE 0x03
#define RRC_FUZZ_MSG_CASE_MARKER 0x10
#define RRC_FUZZ_MAX_PAYLOAD_LEN 4096
#define RRC_FUZZ_MAX_MARKER_LEN 256
#define RRC_FUZZ_MAX_REPLY_LEN 8192

typedef struct rrc_fuzz_header_s {
  uint8_t msg_type;
  uint8_t srb_id;
  uint16_t reserved;
  uint32_t cu_ue_id;
  uint32_t rnti;
  uint32_t payload_len;
} __attribute__((packed)) rrc_fuzz_header_t;

typedef struct rrc_fuzz_thread_args_s {
  int port;
} rrc_fuzz_thread_args_t;

static pthread_t rrc_fuzz_thread;
static bool rrc_fuzz_started;

static bool recv_exact(int fd, void *buf, size_t len)
{
  uint8_t *pos = buf;
  while (len > 0) {
    ssize_t n = recv(fd, pos, len, 0);
    if (n == 0)
      return false;
    if (n < 0) {
      if (errno == EINTR)
        continue;
      return false;
    }
    pos += n;
    len -= n;
  }
  return true;
}

static void send_reply(int fd, const char *reply)
{
  send(fd, reply, strlen(reply), MSG_NOSIGNAL);
}

static void append_reply(char **pos, size_t *remaining, const char *fmt, ...)
{
  if (*remaining == 0)
    return;

  va_list ap;
  va_start(ap, fmt);
  int written = vsnprintf(*pos, *remaining, fmt, ap);
  va_end(ap);

  if (written < 0)
    return;

  if ((size_t)written >= *remaining) {
    *pos += *remaining - 1;
    *remaining = 1;
    return;
  }

  *pos += written;
  *remaining -= written;
}

static bool is_marker_char(uint8_t c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'
         || c == ':';
}

static void sanitize_marker_payload(const uint8_t *payload, uint32_t payload_len, char *out, size_t out_len)
{
  DevAssert(out_len > 0);

  const size_t copy_len = payload_len < out_len - 1 ? payload_len : out_len - 1;
  for (size_t i = 0; i < copy_len; i++)
    out[i] = is_marker_char(payload[i]) ? (char)payload[i] : '_';
  out[copy_len] = '\0';
}

static rrc_gNB_ue_context_t *find_target_ue(uint32_t cu_ue_id, uint32_t rnti)
{
  gNB_RRC_INST *rrc = RC.nrrrc[0];
  if (rrc == NULL)
    return NULL;

  if (cu_ue_id != 0)
    return rrc_gNB_get_ue_context(rrc, cu_ue_id);

  if (rnti != 0)
    return rrc_gNB_get_ue_context_by_rnti_any_du(rrc, (rnti_t)rnti);

  return NULL;
}

static void append_ue_state(char **pos, size_t *remaining, const rrc_gNB_ue_context_t *ue_context)
{
  const gNB_RRC_UE_t *UE = &ue_context->ue_context;
  const bool f1_data_exists = cu_exists_f1_ue_data(UE->rrc_ue_id);
  const f1_ue_data_t ue_data = f1_data_exists ? cu_get_f1_ue_data(UE->rrc_ue_id) : (f1_ue_data_t){0};

  append_reply(pos,
               remaining,
               " ue={cu_ue_id=%u,rnti=0x%04x,du_ue_id=%u,du_assoc_id=%d,srb1=%u,srb2=%u,security=%u,"
               "f1_context=%u,f1_data=%u,drb_count=%zu,pdu_session_count=%zu,ongoing_reconfiguration=%u}",
               UE->rrc_ue_id,
               UE->rnti,
               ue_data.secondary_ue,
               ue_data.du_assoc_id,
               UE->Srb[SRB1].Active ? 1 : 0,
               UE->Srb[SRB2].Active ? 1 : 0,
               UE->as_security_active ? 1 : 0,
               UE->f1_ue_context_active ? 1 : 0,
               f1_data_exists ? 1 : 0,
               seq_arr_size(&UE->drbs),
               seq_arr_size(&UE->pduSessions),
               UE->ongoing_reconfiguration ? 1 : 0);
}

static void send_ue_list(int client_fd)
{
  char reply[RRC_FUZZ_MAX_REPLY_LEN];
  char *pos = reply;
  size_t remaining = sizeof(reply);

  gNB_RRC_INST *rrc = RC.nrrrc[0];
  if (rrc == NULL) {
    send_reply(client_fd, "ERR rrc instance unavailable\n");
    return;
  }

  size_t count = 0;
  rrc_gNB_ue_context_t *ue_context = NULL;
  RB_FOREACH(ue_context, rrc_nr_ue_tree_s, &rrc->rrc_ue_head)
    count++;

  append_reply(&pos, &remaining, "OK ue_list count=%zu", count);
  RB_FOREACH(ue_context, rrc_nr_ue_tree_s, &rrc->rrc_ue_head)
    append_ue_state(&pos, &remaining, ue_context);
  append_reply(&pos, &remaining, "\n");

  send_reply(client_fd, reply);
  LOG_I(NR_RRC, "rrc_fuzz_injector: returned UE list count=%zu\n", count);
}

static void send_ue_state(int client_fd, uint32_t cu_ue_id, uint32_t rnti)
{
  rrc_gNB_ue_context_t *ue_context = find_target_ue(cu_ue_id, rnti);
  if (ue_context == NULL) {
    send_reply(client_fd, "ERR no UE context\n");
    return;
  }

  char reply[RRC_FUZZ_MAX_REPLY_LEN];
  char *pos = reply;
  size_t remaining = sizeof(reply);

  append_reply(&pos, &remaining, "OK ue_state");
  append_ue_state(&pos, &remaining, ue_context);
  append_reply(&pos, &remaining, "\n");

  send_reply(client_fd, reply);
  LOG_I(NR_RRC,
        "rrc_fuzz_injector: returned UE state cu_ue_id=%u rnti=%04x\n",
        ue_context->ue_context.rrc_ue_id,
        ue_context->ue_context.rnti);
}

static void handle_case_marker(int client_fd, uint8_t srb_id, uint32_t cu_ue_id, uint32_t rnti, const uint8_t *payload, uint32_t payload_len)
{
  char marker[RRC_FUZZ_MAX_MARKER_LEN + 1];
  sanitize_marker_payload(payload, payload_len, marker, sizeof(marker));

  LOG_I(NR_RRC,
        "rrc_fuzz_injector: CASE_MARKER marker=%s cu_ue_id=%u rnti=%04x srb=%u len=%u\n",
        marker,
        cu_ue_id,
        rnti,
        srb_id,
        payload_len);
  send_reply(client_fd, "OK marker\n");
}

static bool queue_ul_dcch(uint8_t srb_id, uint32_t cu_ue_id, uint32_t rnti, const uint8_t *payload, uint32_t payload_len)
{
  rrc_gNB_ue_context_t *ue_context = find_target_ue(cu_ue_id, rnti);
  if (ue_context == NULL) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: UE context not found cu_ue_id=%u rnti=%04x\n", cu_ue_id, rnti);
    return false;
  }

  const uint32_t resolved_cu_ue_id = ue_context->ue_context.rrc_ue_id;
  f1_ue_data_t ue_data = cu_get_f1_ue_data(resolved_cu_ue_id);

  MessageDef *msg = itti_alloc_new_message(TASK_UNKNOWN, 0, F1AP_UL_RRC_MESSAGE);
  if (msg == NULL) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: itti_alloc_new_message failed\n");
    return false;
  }

  uint8_t *rrc_container = malloc_or_fail(payload_len);
  memcpy(rrc_container, payload, payload_len);

  f1ap_ul_rrc_message_t *ul_rrc = &F1AP_UL_RRC_MESSAGE(msg);
  ul_rrc->gNB_CU_ue_id = resolved_cu_ue_id;
  ul_rrc->gNB_DU_ue_id = ue_data.secondary_ue;
  ul_rrc->srb_id = srb_id;
  ul_rrc->rrc_container = rrc_container;
  ul_rrc->rrc_container_length = payload_len;
  msg->ittiMsgHeader.originInstance = ue_data.du_assoc_id;

  LOG_I(NR_RRC,
        "rrc_fuzz_injector: received INJECT_UL_DCCH cu_ue_id=%u du_ue_id=%u rnti=%04x srb=%u len=%u\n",
        resolved_cu_ue_id,
        ue_data.secondary_ue,
        ue_context->ue_context.rnti,
        srb_id,
        payload_len);

  int ret = itti_send_msg_to_task(TASK_RRC_GNB, 0, msg);
  if (ret < 0) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: itti_send_msg_to_task failed ret=%d\n", ret);
    free(rrc_container);
    itti_free(TASK_UNKNOWN, msg);
    return false;
  }

  LOG_I(NR_RRC, "rrc_fuzz_injector: queued ITTI F1AP_UL_RRC_MESSAGE\n");
  return true;
}

static void handle_client(int client_fd)
{
  while (true) {
    rrc_fuzz_header_t header;
    if (!recv_exact(client_fd, &header, sizeof(header)))
      return;

    const uint8_t msg_type = header.msg_type;
    const uint8_t srb_id = header.srb_id;
    const uint32_t cu_ue_id = ntohl(header.cu_ue_id);
    const uint32_t rnti = ntohl(header.rnti);
    const uint32_t payload_len = ntohl(header.payload_len);

    if (msg_type == RRC_FUZZ_MSG_HEARTBEAT) {
      if (payload_len != 0) {
        send_reply(client_fd, "ERR heartbeat payload must be empty\n");
        return;
      }
      send_reply(client_fd, "OK heartbeat\n");
      continue;
    }

    if (msg_type == RRC_FUZZ_MSG_QUERY_UE_LIST) {
      if (payload_len != 0) {
        send_reply(client_fd, "ERR query_ue_list payload must be empty\n");
        return;
      }
      send_ue_list(client_fd);
      continue;
    }

    if (msg_type == RRC_FUZZ_MSG_QUERY_UE_STATE) {
      if (payload_len != 0) {
        send_reply(client_fd, "ERR query_ue_state payload must be empty\n");
        return;
      }
      if (cu_ue_id == 0 && rnti == 0) {
        send_reply(client_fd, "ERR query_ue_state requires cu_ue_id or rnti\n");
        return;
      }
      send_ue_state(client_fd, cu_ue_id, rnti);
      continue;
    }

    if (msg_type == RRC_FUZZ_MSG_CASE_MARKER) {
      if (payload_len == 0 || payload_len > RRC_FUZZ_MAX_MARKER_LEN) {
        send_reply(client_fd, "ERR invalid marker payload_len\n");
        return;
      }

      uint8_t payload[RRC_FUZZ_MAX_MARKER_LEN];
      if (!recv_exact(client_fd, payload, payload_len))
        return;

      handle_case_marker(client_fd, srb_id, cu_ue_id, rnti, payload, payload_len);
      continue;
    }

    if (msg_type != RRC_FUZZ_MSG_INJECT_UL_DCCH) {
      send_reply(client_fd, "ERR unknown msg_type\n");
      return;
    }

    if (srb_id < 1 || srb_id > 2) {
      send_reply(client_fd, "ERR invalid srb_id\n");
      return;
    }

    if (payload_len == 0 || payload_len > RRC_FUZZ_MAX_PAYLOAD_LEN) {
      send_reply(client_fd, "ERR invalid payload_len\n");
      return;
    }

    uint8_t *payload = malloc(payload_len);
    if (payload == NULL) {
      send_reply(client_fd, "ERR out of memory\n");
      return;
    }

    if (!recv_exact(client_fd, payload, payload_len)) {
      free(payload);
      return;
    }

    bool queued = queue_ul_dcch(srb_id, cu_ue_id, rnti, payload, payload_len);
    free(payload);
    send_reply(client_fd, queued ? "OK queued\n" : "ERR queue failed\n");
  }
}

static void *rrc_fuzz_thread_main(void *arg)
{
  rrc_fuzz_thread_args_t *args = arg;
  const int port = args->port;
  free(args);

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: socket failed: %s\n", strerror(errno));
    return NULL;
  }

  int one = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(port);

  if (bind(server_fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: bind 127.0.0.1:%d failed: %s\n", port, strerror(errno));
    close(server_fd);
    return NULL;
  }

  if (listen(server_fd, 4) < 0) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: listen failed: %s\n", strerror(errno));
    close(server_fd);
    return NULL;
  }

  LOG_I(NR_RRC, "rrc_fuzz_injector: listening on 127.0.0.1:%d\n", port);

  while (true) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
      if (errno == EINTR)
        continue;
      LOG_E(NR_RRC, "rrc_fuzz_injector: accept failed: %s\n", strerror(errno));
      continue;
    }
    handle_client(client_fd);
    close(client_fd);
  }

  return NULL;
}

int rrc_fuzz_injector_start(int port)
{
  if (port <= 0 || port > 65535) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: invalid port %d\n", port);
    return -1;
  }

  if (rrc_fuzz_started)
    return 0;

  rrc_fuzz_thread_args_t *args = malloc_or_fail(sizeof(*args));
  args->port = port;

  int ret = pthread_create(&rrc_fuzz_thread, NULL, rrc_fuzz_thread_main, args);
  if (ret != 0) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: pthread_create failed: %s\n", strerror(ret));
    free(args);
    return -1;
  }

  pthread_detach(rrc_fuzz_thread);
  rrc_fuzz_started = true;
  return 0;
}

void rrc_fuzz_injector_start_from_env(void)
{
  const char *port_env = getenv("OAI_RRC_FUZZ_INJECTOR_PORT");
  if (port_env == NULL || port_env[0] == '\0')
    return;

  char *end = NULL;
  long port = strtol(port_env, &end, 10);
  if (*end != '\0' || port <= 0 || port > 65535) {
    LOG_E(NR_RRC, "rrc_fuzz_injector: invalid OAI_RRC_FUZZ_INJECTOR_PORT=%s\n", port_env);
    return;
  }

  rrc_fuzz_injector_start((int)port);
}
