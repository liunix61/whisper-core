/*********************************************************************************
*     File Name           :     /home/jonesax/Work/whisper-core/src/session/session.c
*     Created By          :     jonesax
*     Creation Date       :     [2016-06-19 17:03]
*     Last Modified       :     [2016-06-20 10:57]
*     Description         :      
**********************************************************************************/

#include "session.h"

session *session_create() {
  session *s = malloc(sizeof(session));
  s->connection_request_list = jnx_list_create();
  jnx_guid_create(&(*s).id); 
  return s;
}
void session_add_connection(session *s, connection_request *c) {
  jnx_list_add(s->connection_request_list,c);
}
void session_destroy(session **s) {

  jnx_list_destroy(&(*s)->connection_request_list);
  free(*s);
  *s = NULL;
}
