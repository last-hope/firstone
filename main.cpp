#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <cstdlib>
#include "iostream"
#include "string"
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <evhttp.h>
#include <queue>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/bufferevent.h>

#define THREAD_MAX_NUM 50
FILE *f;
using namespace std;

pthread_mutex_t thread_num_lock = PTHREAD_MUTEX_INITIALIZER;
int thread_num = 0;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
queue<string>url_queue;
int thread_stat[512];

struct fuck {
    int thread_num;
    char url[300];
};//给线程传递参数

void href_finder(char *source_page, char *parent_url)
{
    char *pointer, *space;
    pointer = (char*)malloc(1040*sizeof(char));
    space = pointer;
    strcpy(pointer, source_page);
    char *url_start, *url_end;
    char url[300];
    while(url_start = strstr(pointer, "href=\""))
    {
        url_start += 6;
        url_end = strstr(url_start, "\"");
        if (url_end) {
            char *new_url = (char *) calloc(300, sizeof(char));
            strcpy(new_url, parent_url);
            strncpy(url, url_start, url_end-url_start);
           // if(url[0]=='?'||url[0]=='/'){
          //      free(url);
           //     goto while;
          //  }
            url[url_end-url_start] = '\0';
            pointer = url_end + 2;
            //puts(new_url);
            //puts(url);
            strcat(new_url, url);
            if(url[0] == '/' || url[0] == '?'|| url[0] == '0'|| url[0] == '2'|| url[0] == '1'|| url[0] == '3'|| url[0] == '4'|| url[0] == '5'|| url[0] == '6'|| url[0] == '7')
            {
                //continue;
            } else
            {

                pthread_mutex_lock(&queue_lock);
                string string_url(new_url);
                url_queue.push(string_url);
                pthread_mutex_unlock(&queue_lock);
                free(new_url);
            }

           // fprintf(stdout, "%s\n", new_url);
//            fclose(f);
        } else {
            free(space);
            return;
        }
    }
    free(space);
}

void http_request_done(struct evhttp_request *req, void *arg){
    char buf[1024];
    int s;

    char current_url[300] = {'\0'};
    f = fopen("//home/wxl/3000warriors/URL.txt", "a");
    if(evhttp_request_get_uri(req)){
        fprintf(f, "%s\n", evhttp_request_get_uri(req));
        //printf("%s\n", evhttp_request_get_uri(req));
    }
    fclose(f);
    strcpy(current_url, evhttp_request_get_uri(req));

    while(s = evbuffer_remove(req->input_buffer, &buf, sizeof(buf) - 1))
    {
        buf[s] = '\0';
        //puts(buf);
        if(strstr(evhttp_request_get_uri(req), "html")||strstr(evhttp_request_get_uri(req), "00"))
        {

        } else
        {
            printf("%s\n", evhttp_request_get_uri(req));
            href_finder(buf, current_url);
        }

    }

    // terminate event_base_dispatch()
    event_base_loopbreak((struct event_base *)arg);
}

void *hello_world_thread(void *arg)
{
    struct fuck *new_arg = (struct fuck*)arg;
    struct event_base *base;
    struct evhttp_connection *conn;
    struct evhttp_request *req;

    base = event_base_new();
    conn = evhttp_connection_base_new(base, NULL, "10.108.87.34", 80);
    req = evhttp_request_new(http_request_done, base);

    evhttp_add_header(req->output_headers, "Host", "localhost");
    //evhttp_add_header(req->output_headers, "Connection", "close");

    char *req_url = (char *)calloc(200, sizeof(char));
    strcpy(req_url, new_arg->url);
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, req_url);
    evhttp_connection_set_timeout(req->evcon, 600);
    event_base_dispatch(base);
//    event_base_free(base);
    pthread_mutex_lock(&thread_num_lock);
    thread_stat[new_arg->thread_num]=0;
    pthread_mutex_unlock(&thread_num_lock);
    free(req_url);
    event_base_free(base);
    return NULL;

}//

int main()
{
//    pthread_t threads[THREAD_MAX_NUM];
    int rc;
    int i;
    url_queue.push("/sohu/");
    pthread_t threads[512];
//    url_queue.push("/sohu/00/");
//    url_queue.push("/sohu/00/00/");
//    url_queue.push("/sohu/02/");

    while (1)
    {
        for (i = 0; i < 512; i++) {
            if (thread_stat[i] == 0) {
                if (!url_queue.empty()) {
                    char *s = (char *) calloc(128, sizeof(char));
                    strcpy(s, url_queue.front().c_str());
                    url_queue.pop();
                    pthread_mutex_lock(&thread_num_lock);
                    thread_stat[i]=1;
                    pthread_mutex_unlock(&thread_num_lock);
                    struct fuck new_fuck;
                    new_fuck.thread_num = i;
                    strcpy(new_fuck.url, s);
                    rc = pthread_create(&threads[i], NULL, hello_world_thread, &new_fuck);
                    if (rc)
                    {
                        printf("ERROR: pthread_create failed with %d\n", rc);
                        return -1;
                    }
                    pthread_join(threads[i], NULL);
                }
            }
        }

    }

//    for(i=0;i<THREAD_MAX_NUM;i++)
//    {
//        char *s = (char *) calloc(128, sizeof(char));
//        strcpy(s, url_queue.front().c_str());
//        url_queue.pop();
//        rc = pthread_create(&threads[i], NULL, hello_world_thread, s);
//        if (rc)
//        {
//            printf("ERROR: pthread_create failed with %d\n", rc);
//            return -1;
//        }
//        pthread_join(threads[i], NULL);
//    }

    printf("INFO: All threads have been exit!!!\n");
    while (!url_queue.empty()) {
        pthread_mutex_lock(&queue_lock);
        cout << url_queue.front() << endl;
        url_queue.pop();
        pthread_mutex_unlock(&queue_lock);
    }

    return 0;
}
