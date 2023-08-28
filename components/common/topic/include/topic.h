#ifndef __TOPIC_H_
#define __TOPIC_H_

#include <stdio.h>  
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>  
#include <sys/stat.h>  



//#define UTOPIC_USING_MUTEX

#ifdef __cplusplus
extern "C" {
#endif

#define TOPIC_ID(_name)       &__topic_##_name

#define TOPIC_DECLARE(_name) extern topic_node __topic_##_name

#define TOPIC_DEFINE(_name, _struct)     \
        topic_node __topic_##_name = {   \
        #_name,                          \
        sizeof(_struct),                 \
        0,                               \
        0,                               \
        NULL,                            \
        NULL,                            \
        NULL,                            \
    };

typedef struct topic_subscriber
{
    void *update;
    char subscriber[20];
    struct topic_subscriber *next;
} topic_subscriber;

typedef int topic_advert_t;

typedef struct topic_node
{
    const char *name;
    uint32_t size;
    uint32_t updated_num;
    uint32_t subscriber_num;
    void *topic_data;
    pthread_mutex_t *node_lock;
    topic_subscriber *subscriber_list;
} topic_node;

int topic_init(void);

int topic_advertise(topic_node *node);

int topic_unadvertise(topic_node *node);

int topic_subscribe(topic_node *node, uint64_t *event, char *self_name);

int topic_subscribe_auto(topic_node *node,uint64_t *event, char *name, uint8_t time_out);

int topic_unsubscribe_auto(topic_node *node, uint64_t *event,char *name, uint32_t time_out);

int topic_unsubscribe(topic_node *node, uint64_t *event, char *self_name);

int topic_publish(topic_node *node,void *structure);

int topic_check(uint64_t *update,int32_t timeout);

int topic_copy(topic_node *node,void *buffer);


#ifdef __cplusplus
}
#endif


#endif

