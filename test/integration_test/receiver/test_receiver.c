/*
 * =====================================================================================
 *
 *       Filename:  test_session.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  02/09/2015 05:57:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <jnxc_headers/jnxsocket.h>
#include <jnxc_headers/jnxlog.h>
#include <jnxc_headers/jnx_tcp_socket.h>
#include "session_service.h"
#include "port_control.h"
#include <whisper_protocol_headers/wpmux.h>
#include <whisper_protocol_headers/wpprotocol.h>
#include "discovery.h"

static char *baddr = NULL;
static char *interface = NULL;

void mux_callback_hook(Wpmessage *message,void *args) {
  printf("Received mux_callback_hook\n");
}

static wp_mux *mux;

int linking_test_procedure(session *s, linked_session_type session_type,
    void *optargs) {
  JNXLOG(NULL, "Linking now the receiver session..");
  /*
     jnx_char *default_secure_comms = "6666";
     auth_comms_service *ac = auth_comms_create();
     ac->listener = jnx_socket_tcp_listener_create("9991",AF_INET,15);
     auth_comms_initiator_start(ac,ds,os,default_secure_comms);
     */
  return 0;
}

int unlinking_test_procedure(session *s, linked_session_type session_type,
    void *optargs) {

  //  auth_comms_stop(ac,s);
  return 0;
}

int app_accept_reject(discovery_service *ds, jnx_guid *initiator_guild,

    jnx_guid *session_guid) {
  return 0;
}

void test_receiver() {
  JNXLOG(NULL, "test_linking");
  session_service *service = session_service_create(linking_test_procedure,
      unlinking_test_procedure);

  //Lets generate the guid of some remote session
  jnx_guid h;
  jnx_guid_create(&h);

  peerstore *store = peerstore_init(local_peer_for_user("receiver_bob", 10,interface), 0);

  get_broadcast_ip(&baddr,interface);

  discovery_service *ds = discovery_service_create(1234, AF_INET, baddr, store);

  discovery_service_start(ds, BROADCAST_UPDATE_STRATEGY);



start:
  while (1) {

    jnx_list *olist = NULL;


    wpprotocol_mux_tick(mux);
    Wpmessage *omessage;
    if(wpprotocol_mux_pop(mux,omessage) == E_WMS_OKAY) {
      if(omessage) {
        JNXLOG(LDEBUG,"Received incoming message via the mux") ;


        switch(omessage->action->action) {
          case SELECTED_ACTION__CREATE_SESSION:
            JNXLOG(LDEBUG,"SELECTED_ACTION__CREATE_SESSION");

            //lets respond for the test_receiver
            jnx_char *data = malloc(strlen("Hello"));
            bzero(data,6);
            memcpy(data,"Hello",6);

            Wpmessage *message;
            wp_generation_state w = wpprotocol_generate_message(&message,omessage->sender,omessage->recipient,
                data,6,SELECTED_ACTION__RESPONDING_CREATED_SESSION);

            wpprotocol_mux_push(mux,message);

            break;

          case SELECTED_ACTION__RESPONDING_CREATED_SESSION:
            JNXLOG(LDEBUG,"SELECTED_ACTION__RESPONDING_CREATED_SESSION");
            break;

          case SELECTED_ACTION__SHARING_SESSION_KEY:
            JNXLOG(LDEBUG,"SELECTED_ACTION__SHARING_SESSION_KEY");
            break;
          case SELECTED_ACTION__COMPLETED_SESSION:
            JNXLOG(LDEBUG,"SELECTED_ACTION__COMPLETED_SESSION");
            break;
        }  

      }
    }

    if (session_service_fetch_all_sessions(service,
          &olist) != SESSION_STATE_NOT_FOUND) {

      printf("-----------------------------------------------\n");
      session *s = jnx_list_remove_front(&olist);

      while (!session_is_active(s)){
        sleep(1);
      }

      jnx_char *buffy = NULL;

      int size = 0;
      while (size <= 0) {
        size = session_message_read(s, &buffy);
      }
      if(size) {
        printf("size -> %d, buffy -> %s\n", size, buffy);

        JNXCHECK(session_is_active(s) == 1);

        session_service_unlink_sessions(service,E_AM_RECEIVER,
            ds,&(*s).session_guid);

        JNXCHECK(session_is_active(s) == 0);

        session_service_destroy_session(service,&(*s).session_guid);

        jnx_list_destroy(&olist);

        goto start;
        break;
      }    
    }
    if(olist)
      jnx_list_destroy(&olist);
  }

}

int main(int argc, char **argv) {
  mux = wpprotocol_mux_create("8080",AF_INET,mux_callback_hook,NULL);
  if (argc > 1) {
    interface = argv[1];
    printf("using interface %s", interface);
  }
  test_receiver();
  wpprotocol_mux_destroy(&mux);
  return 0;
}
