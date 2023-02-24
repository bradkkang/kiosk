/* --------------------- usrinc/hmsapi.h ---------------------- */
/*                                                              */
/*              Copyright (c) 2000 - 2007 Tmax Soft Co., Ltd    */
/*                   All Rights Reserved                        */
/*                                                              */
/* ------------------------------------------------------------ */

#ifndef _TMAX_HMSAPI_H
#define _TMAX_HMSAPI_H


#include <usrinc/fbuf.h>



/* HMS Message Types */
#define HMS_CHAR      FB_CHAR
#define HMS_SHORT     FB_SHORT
#define HMS_INT       FB_INT
#define HMS_LONG      FB_LONG
#define HMS_FLOAT     FB_FLOAT
#define HMS_DOUBLE    FB_DOUBLE
#define HMS_STRING    FB_STRING
#define HMS_CARRAY    FB_CARRAY


/* Session Options */
#define HMS_AUTO_ACK               0
#define HMS_DUPS_OK_ACK            1
#define HMS_CLIENT_ACK             2
#define HMS_NORMAL_SESSION         0
#define HMS_TRANSACTED_SESSION     1
#define HMS_XA_SESSION             2
#define HMS_ASYNC_SESSION          3

/* Destination Types */
#define HMS_QUEUE                  0
#define HMS_TOPIC                  1
#define HMS_DURABLE                2

/* Delivery Modes */
#define HMS_DLV_NON_PERSISTENT     0
#define HMS_DLV_PERSISTENT         1

/* Acknowledge Options  (flags used in function hms_ack) */
#define HMS_ACK_ONE                0x01
#define HMS_ACK_UPTHROUGH          0x02

/* QueueBrowser Options (flags used in function hms_create_browser) */
#define HMS_QB_FIRST               0x01

/* Property */
#define HMSDestination             "$Destination"
#define HMSDeliveryMode            "$DeliveryMode"
#define HMSMessageID               "$MessageID"
#define HMSTimeStamp               "$TimeStamp"
#define HMSCorrelationID           "$CorrelationID"
#define HMSReplyTo                 "$ReplyTo"
#define HMSRedelivered             "$Redelivered"
#define HMSType                    "$Type"
#define HMSExpiration              "$Expiration"
#define HMSPriority                "$Priority"
#define HMSStartSec                "@HMSStartSec@"
#define HMSStartUsec               "@HMSStartUsec@"
#define HMSCallSvcname1            "@HMSCallSvcNM1@"
#define HMSCallSvcname2            "@HMSCallSvcNM2@"
#define HMSCallSvcname3            "@HMSCallSvcNM3@"
#define HMSCallSvcname4            "@HMSCallSvcNM4@"
#define HMSCallSvcname5            "@HMSCallSvcNM5@"
#define HMSCallSvcname6            "@HMSCallSvcNM6@"
#define HMSCallSvcname7            "@HMSCallSvcNM7@"
#define HMSCallSvcname8            "@HMSCallSvcNM8@"
#define HMSCallSvcname9            "@HMSCallSvcNM9@"
#define HMSCallSvcname10           "@HMSCallSvcNM10@"
#define HMSCallSvcname11           "@HMSCallSvcNM11@"
#define HMSCallSvcname12           "@HMSCallSvcNM12@"
#define HMSCallSvcname13           "@HMSCallSvcNM13@"
#define HMSCallSvcname14           "@HMSCallSvcNM14@"
#define HMSCallSvcname15           "@HMSCallSvcNM15@"
#define HMSCallSvcname16           "@HMSCallSvcNM16@"
#define HMSTmsName                 "@HMSTmsName@"

typedef FBUF hms_msg_t;            /*   HMS Message   */
typedef void* HMS_SHND;            /* Session  Handle */
typedef void* HMS_PHND;            /* Producer Handle */
typedef void* HMS_CHND;            /* Consumer Handle */
typedef void* HMS_BHND;            /* Browser  Handle */

typedef void __EXPORT Abendfunc(HMS_SHND *sess);

/* Message API */
hms_msg_t *hms_alloc(HMS_SHND * sess, long size);
void hms_free(hms_msg_t * msg);
int hms_get_property(hms_msg_t * msg, char *name, int *type, char *value, long *len);
int hms_set_property(hms_msg_t * msg, char *name, int type, char *value, long len);
int hms_get_body(hms_msg_t * msg, char *data, long *len);
int hms_set_body(hms_msg_t * msg, char *data, long len);


/* Session API */
HMS_SHND *hms_create_session(char *hms, int transacted, int ackmode, int flags);
HMS_SHND *hms_create_xa_session(char *hms, int flags);
HMS_SHND *hms_create_async_session(char *hms, Abendfunc *func, int flags);
int hms_close_session(HMS_SHND * sess, int flags);
int hms_commit(HMS_SHND * sess, int flags);
int hms_rollback(HMS_SHND * sess, int flags);
int hms_recover(HMS_SHND * sess, int flags);


/* Producer API */
HMS_PHND *hms_create_producer(HMS_SHND * sess, char *des, int destype, char *name, int flags);
#define hms_create_sender(sess, des, name, flags) \
    hms_create_producer(sess, des, HMS_QUEUE, name, flags)
#define hms_create_publisher(sess, des, name, flags) \
    hms_create_producer(sess, des, HMS_TOPIC, name, flags)
int hms_close_producer(HMS_PHND * prod, int flags);
#define hms_close_sender(prod, flags) \
    hms_close_producer(prod, flags)
#define hms_close_publisher(prod, flags) \
    hms_close_producer(prod, flags)
int hms_sendex(HMS_PHND * prod, hms_msg_t * msg, int dlvmode, int priority, int ttl, int flags);
#define hms_send(prod, msg, flags) \
    hms_sendex(prod, msg, HMS_DLV_NON_PERSISTENT, 0, 0, flags)


/* Consumer API */
HMS_CHND *hms_create_consumer(HMS_SHND * sess, char *des, int destype,
                              char *name, char *msgselector, char *svcname, int flags);
#define hms_create_receiver(sess, des, name, msgselector, svcname, flags) \
    hms_create_consumer(sess, des, HMS_QUEUE, name, msgselector, svcname, flags)
#define hms_create_subscriber(sess, des, name, msgselector, svcname, flags) \
    hms_create_consumer(sess, des, HMS_TOPIC, name, msgselector, svcname, flags)
#define hms_create_durable_subscriber(sess, des, name, msgselector, svcname, flags) \
    hms_create_consumer(sess, des, HMS_DURABLE, name, msgselector, svcname, flags)
int hms_close_consumer(HMS_CHND * cons, int flags);
#define hms_close_receiver(cons, flags) \
    hms_close_consumer(cons, flags)
#define hms_close_subscriber(cons, flags) \
    hms_close_consumer(cons, flags)
#define hms_close_durable_subscriber(cons, flags) \
    hms_close_consumer(cons, flags)
int hms_unsubscribe_durable_subscriber(HMS_SHND * sess, char *des, char *name, int flags);

int hms_recvex(HMS_CHND * cons, hms_msg_t **msg, long timeout, int flags);
#define hms_recv(cons, msg, flags) \
    hms_recvex(cons, msg, 0, flags)
int hms_ack(hms_msg_t *msg, int flags);


/* QueueBrowser API */
HMS_BHND *hms_create_browser(HMS_SHND * sess, char *queue, char *msgselector, int flags);
int hms_close_browser(HMS_BHND * browser, int flags);
int hms_browser_nextmsg(HMS_BHND * browser, hms_msg_t **msg, int flags);
int hms_browser_get_msgselector(HMS_BHND * browser, char *buf, long *len, int flags);
int hms_browser_get_queue(HMS_BHND * browser, char *buf, long *len, int flags);
#endif
