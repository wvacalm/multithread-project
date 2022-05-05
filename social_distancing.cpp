// wvacalm
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include <unistd.h>
using namespace std;

int vaccinated_kids_count = 0;
sem_t mutex;
sem_t play_mutex;

struct thread_data
{
  int thread_id;
  string status;
};

void Play(int tid, string st)
{
  printf("%s thread %d is playing! \n", st.c_str(), tid);
  sleep(1);
  printf("%s thread %d is done! \n", st.c_str(), tid);
}

void *thread_call(void *threadarg)
{

  struct thread_data *my_data;
  my_data = (struct thread_data *)threadarg;
  if (my_data->status == "V")
  {

    do
    {
      sem_wait(&mutex);
      printf("%s Thread %d acquired mutex for incrementing count. \n", my_data->status.c_str(), my_data->thread_id);
      vaccinated_kids_count++;
      if (vaccinated_kids_count == 1)
      {
        printf("%s Thread %d trying to acquire play_mutex. \n", my_data->status.c_str(), my_data->thread_id);
        sem_wait(&play_mutex);
      }
      printf("%s Thread %d about to release mutex after incrementing. \n", my_data->status.c_str(), my_data->thread_id);
      sem_post(&mutex);
      Play(my_data->thread_id, my_data->status);
      sem_wait(&mutex);
      printf("%s Thread %d acquired mutex for decrementing count. \n", my_data->status.c_str(), my_data->thread_id);
      vaccinated_kids_count--;
      if (vaccinated_kids_count == 0)
      {
        printf("%s Thread %d about to release play_mutex. \n", my_data->status.c_str(), my_data->thread_id);
        sem_post(&play_mutex);
      }
      printf("%s Thread %d about to release mutex after decrementing. \n", my_data->status.c_str(), my_data->thread_id);
      sem_post(&mutex);

      sleep(1);
    } while (true);
  }
  else if (my_data->status == "NV")
  {
    do
    {
      printf("%s Thread %d trying to acquire play_mutex. \n", my_data->status.c_str(), my_data->thread_id);
      sem_wait(&play_mutex);
      Play(my_data->thread_id, my_data->status);
      sem_post(&play_mutex);

      sleep(1);
    } while (true);
  }
  pthread_exit(NULL);
  return 0;
}

int main(int argc, char **argv)
{

  if (argc < 3 || (isdigit(argv[2][0]) == 0) || (isdigit(argv[1][0]) == 0))
  {
    printf("Incorrect input! \n Program requires two integers, separated by space. \n EXAMPLE: ./social_distancing 3 6 \n");
    return 0;
  }

  sem_init(&mutex, 0, 1);
  sem_init(&play_mutex, 0, 1);

  int v_num = atoi(argv[1]);
  int nv_num = atoi(argv[2]);
  int total_threads = v_num + nv_num;
  pthread_t threads[total_threads];
  struct thread_data td[total_threads];
  int s;
  int i;

  // create thread
  for (i = 0; i < total_threads; i++)
  {
    td[i].thread_id = i;
    string status;
    if (v_num > 0)
    {
      status = "V";
      v_num--;
    }
    else if (nv_num > 0)
    {
      status = "NV";
      nv_num--;
    }
    td[i].status = status;
    s = pthread_create(&threads[i], NULL, thread_call, (void *)&td[i]);
    sleep(1);
    if (s)
    {
      printf("err: %d", s);
      exit(-1);
    }
  }

  for (i = 1; i < total_threads; i++)
  {
    pthread_detach(threads[i]);
    pthread_join(threads[i - 1], NULL);
  }
  return 0;
}

