#ifndef _MQ_MESSAGE_H_
#define _MQ_MESSAGE_H_

#include <unistd.h>

typedef struct {
    unsigned int msg_type;
    unsigned int param1;
    unsigned int param2;
    void *param3;
} mq_msg_t;

#endif /* _MQ_MESSAGE_H_ */
