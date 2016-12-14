#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <cstdlib>
#include "iostream"
#include "string"
#include "bloom.h"
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <evhttp.h>
#include <queue>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/bufferevent.h>
//#include <libltdl/lt_system.h>
#include "thread_pool.h"
int pagenumber=0;
int thcount=0;
#define THREAD_MAX_NUM 1
char new_url1[16]="/news.sohu.com/";
FILE *f;
using namespace std;


pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
queue<string>url_queue;
char filename[50];

threadpool threadpool_spider;

BLOOM* bloom_url;

struct fuck {
    char url[300];
};//给线程传递参数

void href_finder(char *source_page, char *parent_url)
{
    //int coutpage=0;
    char *pointer, *space;
    pointer = (char*)malloc(1040*sizeof(char));
    space = pointer;
    strcpy(pointer, source_page);
    char *url_start, *url_end;
    char url[300];
    //char new_url[300];
    int a=0;

    while((url_start = strstr(pointer, "href=\""))!=NULL)
    {
        url_start += 6;
        if(url_start=='\0'){
            break;
        }


        url_end = strstr(url_start, "\"");
        if (url_end!=NULL) {
            char *new_url = (char *) calloc(300, sizeof(char));

            //strcpy(new_url, parent_url);
            if(url_end-url_start>=300){
                //printf("%s dayu300",)
                break;
            }

            strncpy(url, url_start, url_end-url_start);
            url[url_end-url_start] = '\0';
            pointer = url_end + 2;

            if(strstr(url,"#")|| strstr(url,"?")|| strstr(url,"http")|| strstr(url," ")||(url_end-url_start==0)||strstr(url,"java")||strstr(url,".com"))
            {
                //free(url);
                continue;
            }
            else if(url[0]=='/')
            {
                strcat(new_url, new_url1);
                strcat(new_url, url+1);
                //pthread_mutex_lock(&queue_lock);
                a=bloom_check(bloom_url,url,strlen(url));
             //   pthread_mutex_unlock(&queue_lock);
                if (a==1){

                    free(new_url);
                    continue;
                }
            }
            else if(url[0]!='/')
            {
                strcat(new_url, new_url1);
                strcat(new_url, url);

                //pthread_mutex_lock(&queue_lock);
                a=bloom_check(bloom_url,url,strlen(url));
               // pthread_mutex_unlock(&queue_lock);
                if (a==1){
                    free(new_url);
                    continue;
                }
            }
         //   printf("%s %d\n",parent_url,0);
          //  printf("%s %d\n",url,1);
        //    printf("%s %d\n",new_url,2);
            pthread_mutex_lock(&queue_lock);
            bloom_add(bloom_url,url,strlen(url));
            string string_url(new_url);
            url_queue.push(string_url);
            pthread_mutex_unlock(&queue_lock);
            free(new_url);

        } else {

            break;
        }

    }

    free(space);
}

void http_request_done(struct evhttp_request *req, void *arg){
    char buf[1024];
    int s;
    //int countpage=0;
   // char *html_content = (char *)calloc(sizeof(char) , 1024199 );
    char current_url[300] = {'\0'};
    if(req==NULL||req->response_code!=200){
        event_base_loopbreak((struct event_base *)arg);
        return;
    }
    if(evhttp_request_get_uri(req)){
        pthread_mutex_lock(&write_lock);
        f = fopen(filename, "a");
        fprintf(f, "%d %s %d\n",pagenumber++, evhttp_request_get_uri(req),(int)(req->body_size));
        fclose(f);
        pthread_mutex_unlock(&write_lock);
    }
    strcpy(current_url, evhttp_request_get_uri(req));

    while((s = evbuffer_remove(req->input_buffer, &buf, sizeof(buf) - 1)))
    {
        if(s==-1)
            break;
        buf[s] = '\0';
       // printf("%d\n",(int)sizeof(buf));
        if(!strstr(evhttp_request_get_uri(req), ".gif")&&!strstr(evhttp_request_get_uri(req),".js")&&!strstr(evhttp_request_get_uri(req), ".swf")&&!strstr(evhttp_request_get_uri(req),".css"))//||!strstr(evhttp_request_get_uri(req), ".js")||!strstr(evhttp_request_get_uri(req), ".css")||!strstr(evhttp_request_get_uri(req), ".htm")
        {
            //strcat(html_content, buf);
            href_finder(buf, current_url);
        }

    }
   /* if(!strstr(evhttp_request_get_uri(req), "html")&&!strstr(evhttp_request_get_uri(req), ".htm")&&!strstr(evhttp_request_get_uri(req), ".gif")&&!strstr(evhttp_request_get_uri(req),".js")&&!strstr(evhttp_request_get_uri(req), ".swf")&&!strstr(evhttp_request_get_uri(req),".css"))//||!strstr(evhttp_request_get_uri(req), ".js")||!strstr(evhttp_request_get_uri(req), ".css")||!strstr(evhttp_request_get_uri(req), ".htm")
    {
        href_finder(html_content, current_url);
        free(html_content);
        html_content = NULL;
    }*/
    event_base_loopbreak((struct event_base *)arg);
}

void hello_world_thread(void *arg)
{


    struct fuck *new_arg = (struct fuck*)arg;
   // printf("%s hello_world_thread\n",new_arg->url);

    struct event_base *base;
    struct evhttp_connection *conn;
    struct evhttp_request *req;
    base = event_base_new();
    if(!base)
        printf("New Event Base Fail!!!!!!\n");
    conn = evhttp_connection_base_new(base, NULL, "10.108.84.118", 80);
    if(!conn)
        printf("New Conn Fail!!!!!!\n");
    req = evhttp_request_new(http_request_done, base);
    if(!req)
        printf("New Req Fail!!!!!!\n");
    evhttp_add_header(req->output_headers, "Host", "localhost");
    char *req_url = (char *)calloc(256, sizeof(char));
    strcpy(req_url, new_arg->url);
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, req_url);
    evhttp_connection_set_timeout(req->evcon, 600);

    event_base_dispatch(base);

    free(req_url);
    evhttp_connection_free(conn);
    event_base_free(base);

    free(new_arg);
    pthread_detach(pthread_self());
    thcount=0;

}

int main(int argc, char * argv[]){

   /* if(argc!=3){
        printf("USAGE:crawel address outputfile\n");
        return 0;
    }*/
    strcpy(filename, "new_output.txt");
  //  int count = 0;
    //url_queue.push(strchr(argv[1], '4')+5);
    url_queue.push("/news.sohu.com/");
  //  pthread_t threads[THREAD_MAX_NUM];
    char *s = (char *) calloc(256, sizeof(char));

    if(!(bloom_url=bloom_create(40000000, 10)))
    {
        printf("Failure to create bloom filter.\n");
        return EXIT_FAILURE;
    }else{
        printf("Success to create bloom filter.\n");
    }

    threadpool_spider = create_threadpool(THREAD_MAX_NUM);
    bloom_add(bloom_url,"/news.sohu.com/",15);
    while (1)
    {
        if (!url_queue.empty())
        {
            //count = 0;
           // printf("ff\n");
            pthread_mutex_lock(&queue_lock);
            strcpy(s, url_queue.front().c_str());
            url_queue.pop();
           // printf("gg\n");
            pthread_mutex_unlock(&queue_lock);
            //thcount=0;
            struct fuck *new_fuck = (struct fuck*)malloc(sizeof(struct fuck));
            strcpy(new_fuck->url, s);
            dispatch(threadpool_spider, hello_world_thread, new_fuck);

        }
        else{
            sleep(1);
            thcount++;
            printf("%d\n",thcount);
            if(thcount == 30){
                break;
            }

        }


    }
    free(s);
    printf("INFO: All threads have been exit!!!\n");


    return 0;
}
